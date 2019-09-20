/*
 *  Copyright 2019 Sergey Khabarov, sergeykhbr@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include "utils.h"
#include "server.h"
#include "client.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

DpiServer::DpiServer(const AttributeType& config) : IThread("Server", config) {
    hsock_ = 0;
    listClient_.make_list(0);
    request_.make_list(Req_ListSize);
    request_[Req_SourceName].make_string("DpiServer");
    LIB_event_create(&event_cmd_, "DpiServerEventCmd");
}

DpiServer::~DpiServer() {
    LIB_event_close(&event_cmd_);
}

void DpiServer::busyLoop() {
    socket_def client_sock;
    int err;
    fd_set readSet;
    timeval timeout;
    timeout.tv_sec = 2;
    timeout.tv_usec = 0;

    if (createServerSocket()) {
        return;
    }

    if (listen(hsock_, 1) < 0) {
        LIB_printf("DpiServer: %s\n", "listen() failed");
        return;
    }

    /** By default socket was created with Blocking mode */
    setBlockingMode(config_["BlockingMode"].to_bool());

    while (isEnabled()) {
        /** linux modifies flags and time after each call of select() */
        FD_ZERO(&readSet);
        FD_SET(hsock_, &readSet);
        timeout.tv_sec = 2;
        timeout.tv_usec = 0;
        err = select(hsock_ + 1, &readSet, NULL, NULL, &timeout);
        if (err < 0) {
            LIB_printf("DpiServer: %s\n", " accept() failed");
            loopEnable_.state = false;
            break;
        }
        if (err == 0) {
            // Timeout
            message_hartbeat();
            continue;
        }

        client_sock = accept(hsock_, 0, 0);

        config_["ClientConfig"]["Index"].make_uint64(listClient_.size());
        IThread *pclient = new DpiClient(client_sock, config_["ClientConfig"]);
        AttributeType t1(pclient);
        listClient_.add_to_list(&t1);
        pclient->run();
    }
    closeServerSocket();
}

int DpiServer::createServerSocket() {
    char hostName[256];
    if (gethostname(hostName, sizeof(hostName)) < 0) {
        return -1;
    }

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    /**
     * Check availability of IPv4 address assigned via attribute 'hostIP'.
     * If it woudn't be found use the last avaialble IP address.
     */
    int retval;
    struct addrinfo* result = NULL;
    struct addrinfo* ptr = NULL;
    retval = getaddrinfo(hostName, "0", &hints, &result);
    if (retval != 0) {
        return -1;
    }

    AttributeType &hostIP = config_["HostIP"];
    AttributeType &hostPort = config_["HostPort"];
    if (hostIP.size() == 0 || hostIP.is_equal("127.0.0.1")) {
        memset(&sockaddr_ipv4_, 0, sizeof(struct sockaddr_in));
        sockaddr_ipv4_.sin_family = AF_INET;
        sockaddr_ipv4_.sin_addr.s_addr = inet_addr("127.0.0.1");
    } else {
        for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
            // Find only IPV4 address, ignore others.
            if (ptr->ai_family != AF_INET) {
                continue;
            }
            sockaddr_ipv4_ = *((struct sockaddr_in*)ptr->ai_addr);

            if (hostIP.is_equal(inet_ntoa(sockaddr_ipv4_.sin_addr))) {
                break;
            }
        }
    }
    sockaddr_ipv4_.sin_port = htons(static_cast<uint16_t>(hostPort.to_int()));
    LIB_printf("Selected Host IPv4 %s:%d\n",
        inet_ntoa(sockaddr_ipv4_.sin_addr),
        hostPort.to_uint32());

    hsock_ = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (hsock_ < 0) {
        LIB_printf("Error: %s\n", "socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)");
        return -1;
    }

    int res = bind(hsock_,
        reinterpret_cast<struct sockaddr*>(&sockaddr_ipv4_),
        sizeof(sockaddr_ipv4_));
    if (res != 0) {
        LIB_printf("Error: bind(hsock_, \"%s\", ...)\n", hostIP.to_string());
        return -1;
    }

    addr_size_t addr_sz = sizeof(sockaddr_ipv4_);
    res = getsockname(hsock_,
        reinterpret_cast<struct sockaddr*>(&sockaddr_ipv4_),
        &addr_sz);

    LIB_printf("IPv4 address %s:%d . . . opened\n",
        inet_ntoa(sockaddr_ipv4_.sin_addr),
        ntohs(sockaddr_ipv4_.sin_port));

    return 0;
}


bool DpiServer::setBlockingMode(bool mode) {
    int ret;
#if defined(_WIN32) || defined(__CYGWIN__)
    // 0 = disable non-blocking mode
    // 1 = enable non-blocking mode
    u_long arg = mode ? 0 : 1;
    ret = ioctlsocket(hsock_, FIONBIO, &arg);
    if (ret == SOCKET_ERROR) {
        LIB_printf("Set non-blocking socket failed\n", 0);
    }
#else
    int flags = fcntl(hsock_, F_GETFL, 0);
    if (flags < 0) {
        LIB_printf("error: %s\n", "fcntl() failed");
        return false;
    }
    flags = mode ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
    ret = fcntl(hsock_, F_SETFL, flags);
#endif
    if (ret == 0) {
        // success
        config_["BlockingMode"].make_boolean(mode);
        return true;
    } else {
        LIB_printf("error: fcntl() failed to set mode %d", mode);
    }
    return false;
}

void DpiServer::closeServerSocket() {
    if (hsock_ < 0) {
        return;
    }

#if defined(_WIN32) || defined(__CYGWIN__)
    closesocket(hsock_);
#else
    shutdown(hsock_, SHUT_RDWR);
    close(hsock_);
#endif
    hsock_ = -1;
}

void DpiServer::setResponse(const AttributeType &resp) {
    AttributeType t1 = resp;
    t1.to_config();
    LIB_printf("DpiServer: %s\n", t1.to_string());
    LIB_event_set(&event_cmd_);
}

void DpiServer::message_hartbeat() {
    LIB_event_clear(&event_cmd_);
    request_[Req_CmdType].make_string("HartBeat");
    request_[Req_Data].make_uint64(listClient_.size());
    dpi_put_fifo_request(static_cast<ICommand *>(this));
    int err = LIB_event_wait_ms(&event_cmd_, 10000);
    if (err) {
        LIB_printf("DpiServer: %s\n", "DPI didn't respond");
    }
}

void DpiServer::message_client_connected() {
    LIB_event_clear(&event_cmd_);
    request_[Req_CmdType].make_string("ClientAdd");
    request_[Req_Data].make_int64(listClient_.size());
    dpi_put_fifo_request(static_cast<ICommand *>(this));
    int err = LIB_event_wait_ms(&event_cmd_, 10000);
    if (err) {
        LIB_printf("DpiServer: %s\n", "DPI didn't respond");
    }
}
