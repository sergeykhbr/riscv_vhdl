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
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

DpiServer::DpiServer(const AttributeType& config) : IFace(DPI_SERVER_IFACE) {
    config_ = config;
    LIB_event_create(&loopEnable_, DPI_SERVER_IFACE);
    threadInit_.Handle = 0;
}

void DpiServer::runThread(void* arg) {
    reinterpret_cast<DpiServer*>(arg)->busyLoop();
#if defined(_WIN32) || defined(__CYGWIN__)
    _endthreadex(0);
#else
    pthread_exit(0);
#endif
}

bool DpiServer::run() {
    threadInit_.func = reinterpret_cast<lib_thread_func>(runThread);
    threadInit_.args = this;
    LIB_thread_create(&threadInit_);

    if (threadInit_.Handle) {
        LIB_event_set(&loopEnable_);
    }
    return loopEnable_.state;
}

void DpiServer::stop() {
    LIB_event_clear(&loopEnable_);
    if (threadInit_.Handle) {
        LIB_thread_join(threadInit_.Handle, 50000);
    }
    threadInit_.Handle = 0;
}

void DpiServer::busyLoop() {
    if (createServerSocket()) {
        return;
    }

    if (listen(hsock_, 1) < 0) {
        LIB_printf("error: %s", "listen() failed\n");
        return;
    }

    /** By default socket was created with Blocking mode */
    if (!config_["BlockingMode"].to_bool()) {
        setBlockingMode(false);
    }

    while (isEnabled()) {
        LIB_sleep_ms(1);
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
    LIB_printf("Selected Host IPv4 %s:%d",
        inet_ntoa(sockaddr_ipv4_.sin_addr),
        hostPort.to_uint32());

    hsock_ = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (hsock_ < 0) {
        LIB_printf("%s", "Error: socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)");
        return -1;
    }

    int res = bind(hsock_,
        reinterpret_cast<struct sockaddr*>(&sockaddr_ipv4_),
        sizeof(sockaddr_ipv4_));
    if (res != 0) {
        LIB_printf("Error: bind(hsock_, \"%s\", ...)", hostIP.to_string());
        return -1;
    }

    addr_size_t addr_sz = sizeof(sockaddr_ipv4_);
    res = getsockname(hsock_,
        reinterpret_cast<struct sockaddr*>(&sockaddr_ipv4_),
        &addr_sz);

    LIB_printf("IPv4 address %s:%d . . . opened",
        inet_ntoa(sockaddr_ipv4_.sin_addr),
        ntohs(sockaddr_ipv4_.sin_port));

    return 0;
}

void DpiServer::setRcvTimeout(socket_def skt, int timeout_ms) {
    if (!timeout_ms) {
        return;
    }
    struct timeval tv;
#if defined(_WIN32) || defined(__CYGWIN__)
    /** On windows timeout of the setsockopt() function is the DWORD
        * size variable in msec, so we use only the first field in timeval
        * struct and directly assgign argument.
        */
    tv.tv_sec = timeout_ms;
    tv.tv_usec = 0;
#else
    tv.tv_usec = (timeout_ms % 1000) * 1000;
    tv.tv_sec = timeout_ms / 1000;
#endif
    setsockopt(skt, SOL_SOCKET, SO_RCVTIMEO,
        reinterpret_cast<char*>(&tv), sizeof(struct timeval));
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
        return false;
    }
    flags = mode ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
    ret = fcntl(hsock_, F_SETFL, flags);
#endif
    if (ret == 0) {
        // success
        config_["BlockingMode"].make_boolean(mode);
        return true;
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


int DpiServer::getRequest(AttributeType &req) {
    LIB_sleep_ms(1);
    req.make_string("hartbeat");
    return 0;
}

void DpiServer::sendResponse(AttributeType &resp) {
    AttributeType t1 = resp;
    t1.to_config();
    LIB_printf("sv response: %s\n", t1.to_string());

    //if (data->txcnt > 1) {
    //    LIB_printf("No response from simulator. "
    //        "Increase simulation length reqcnt=%d\n", data->txcnt);
    //}
}
