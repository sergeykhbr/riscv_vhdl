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

#include <types.h>
#include <utils.h>
#include "client.h"
#include "c_dpi.h"

DpiClient::DpiClient(socket_def skt) : IThread("Client") {
    LIB_event_create(&event_cmd_,"dpi_server_event");
    cmdcnt_ = 0;
    hsock_ = skt;
    keepAlive_.make_string("['DpiClient','KeepAlive']");
    wrongFormat_.make_string("['DpiClient','WrongFormat']");
}

DpiClient::~DpiClient() {
    LIB_event_close(&event_cmd_);
}

void DpiClient::postinit(const AttributeType &config) {
    config_.clone(&config);
}

void DpiClient::busyLoop() {
    int rxbytes;
    int sockerr;
    int reuse = 1;
    addr_size_t sockerr_len = sizeof(sockerr);

    struct timeval tv;
#if defined(_WIN32) || defined(__CYGWIN__)
    /** On windows timeout of the setsockopt() function is the DWORD
        * size variable in msec, so we use only the first field in timeval
        * struct and directly assgign argument.
        */
    tv.tv_sec = config_["Timeout"].to_int();
    tv.tv_usec = 0;
#else
    tv.tv_usec = (config_["Timeout"].to_int() % 1000) * 1000;
    tv.tv_sec = config_["Timeout"].to_int() / 1000;
#endif
    setsockopt(hsock_, SOL_SOCKET, SO_RCVTIMEO,
        reinterpret_cast<char*>(&tv), sizeof(struct timeval));

    if (setsockopt(hsock_, SOL_SOCKET, SO_REUSEADDR,
        reinterpret_cast<char*>(&reuse), sizeof(int)) < 0) {
        LIB_printf("DpiClien%d: setsockopt(SO_REUSEADDR) failed\n",
                    config_["Index"].to_int());
    }

    cmdcnt_ = 0;
    while (isEnabled()) {
        rxbytes = recv(hsock_, rcvbuf, sizeof(rcvbuf), 0);
        getsockopt(hsock_, SOL_SOCKET, SO_ERROR,
                    reinterpret_cast<char *>(&sockerr), &sockerr_len);

        if (rxbytes == 0) {
            LIB_printf("Socket error: rxbytes=%d, sockerr=%d",
                        rxbytes, sockerr);
            loopEnable_.state = false;
            break;
        }
        if (rxbytes < 0) {
            // Timeout:
            sendBuffer(keepAlive_.to_string(), keepAlive_.size() + 1);
        }

        for (int i = 0; i < rxbytes; i++) {
            cmdbuf_[cmdcnt_++] = rcvbuf[i];
            if (rcvbuf[i] != '\0') {
                continue;
            }
            processRequest();
            cmdcnt_ = 0;
        }
    }
    closeSocket();
}

void DpiClient::processRequest() {
    request_.from_config(cmdbuf_);
    if (!request_.is_list()) {
        LIB_printf("Wrong message: %s\n", cmdbuf_);
        sendBuffer(wrongFormat_.to_string(), wrongFormat_.size() + 1);
        return;
    }

    LIB_event_clear(&event_cmd_);
    dpi_put_fifo_request(static_cast<ICommand *>(this));
    LIB_event_wait(&event_cmd_);

    sendBuffer(response_.to_string(), response_.size() + 1);
}

void DpiClient::sendBuffer(const char *buf, unsigned size) {
    const char *ptx= buf;
    int total = static_cast<int>(size);
    int txbytes;
    while (total > 0) {
        txbytes = send(hsock_, ptx, total, 0);
        if (txbytes <= 0) {
            LIB_printf("KeepAlive error: txcnt=%d", txbytes);
            loopEnable_.state = false;
            break;
        }
        total -= txbytes;
        ptx += txbytes;
    }
}

void DpiClient::setResponse(const AttributeType &resp) {
    response_.attr_free();
    response_ = resp;
    response_.to_config();
    LIB_event_set(&event_cmd_);
}

void DpiClient::closeSocket() {
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
