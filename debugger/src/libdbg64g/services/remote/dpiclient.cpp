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

#include "dpiclient.h"

namespace debugger {

DpiClient::DpiClient(const char *name) : IService(name) {
    registerInterface(static_cast<IThread *>(this));
    registerInterface(static_cast<IDpi *>(this));
    registerAttribute("Enable", &isEnable_);
    registerAttribute("Timeout", &timeout_);
    registerAttribute("HostIP", &hostIP_);
    registerAttribute("HostPort", &hostPort_);

    RISCV_event_create(&event_cmd_, name);
    RISCV_mutex_init(&mutex_tx_);

    char tstr[256];
    RISCV_sprintf(tstr, sizeof(tstr), "['%s','HartBeat']", name);
    hartBeat_.make_string(tstr);
    cmdcnt_ = 0;
    txcnt_ = 0;
}

DpiClient::~DpiClient() {
    RISCV_mutex_destroy(&mutex_tx_);
    RISCV_event_close(&event_cmd_);
}

void DpiClient::postinitService() {
    createServerSocket();

    if (isEnable_.to_bool()) {
        if (!run()) {
            RISCV_error("Can't create thread.", NULL);
            return;
        }
    }
}

void DpiClient::busyLoop() {
    int err;
    int rxbytes;
    AttributeType hartBeatResp;
    connected_ = false;
    while (isEnabled()) {
        if (!connected_) {
            RISCV_important("connecting to host %s:%d",
                hostIP_.to_string(), hostPort_.to_int());
            err = connect(hsock_,
                          (struct sockaddr *)&sockaddr_ipv4_,
                          sizeof(struct sockaddr));
            if (err < 0) {
                RISCV_error("connection failed", 0);
                RISCV_sleep_ms(2000);
                continue;
            }
            connected_ = true;
        }

        rxbytes = recv(hsock_, rcvbuf, sizeof(rcvbuf), 0);

        if (rxbytes == 0) {
            RISCV_error("Socket error: rxbytes=%d", rxbytes);
            connected_ = false;
            continue;
        } 
        if (rxbytes < 0) {
            // Timeout:
            writeTx(hartBeat_.to_string(), hartBeat_.size() + 1);
            continue;
        }

        for (int i = 0; i < rxbytes; i++) {
            cmdbuf_[cmdcnt_++] = rcvbuf[i];
            if (rcvbuf[i] != '\0') {
                continue;
            }
            processRx();
            cmdcnt_ = 0;
        }
        processTx();
    }
    closeServerSocket();
}

void DpiClient::processRx() {
    RISCV_debug("i<=%s", cmdbuf_);
    response_.from_config(cmdbuf_);
    if (!response_.is_list() || response_.size() < 2) {
        RISCV_error("%s", "Wrong response format");
        return;
    }
    
    if (!response_[1].is_equal("HartBeat")) {
        RISCV_event_set(&event_cmd_);
    }
    /*
    request_.from_config(cmdbuf_);
    if (!request_.is_list()) {
        RISCV_error("Wrong message: %s\n", cmdbuf_);
        sendBuffer(wrongFormat_.to_string(), wrongFormat_.size() + 1);
        return;
    }

    RISCV_event_clear(&event_cmd_);
    dpi_put_fifo_request(static_cast<ICommand *>(this));
    RISCV_event_wait(&event_cmd_);

    sendBuffer(response_.to_string(), response_.size() + 1);*/
}

void DpiClient::processTx() {
    const char *ptx = txbuf_;
    int total = static_cast<int>(txcnt_);
    int txbytes;
    while (total > 0) {
        txbytes = send(hsock_, ptx, total, 0);
        if (txbytes <= 0) {
            RISCV_error("processTx error: txcnt=%d", txbytes);
            connected_ = false;
            break;
        }
        total -= txbytes;
        ptx += txbytes;
    }
    txcnt_ = 0;
}

void DpiClient::writeTx(const char *buf, unsigned size) {
    RISCV_mutex_lock(&mutex_tx_);
    memcpy(&txbuf_[txcnt_], buf, size);
    txcnt_ += size;
    RISCV_mutex_unlock(&mutex_tx_);
}

void DpiClient::syncRequest(const char *buf, unsigned size,
                            AttributeType &resp) {
    RISCV_event_clear(&event_cmd_);
    writeTx(buf, size);
    RISCV_event_wait(&event_cmd_);
    resp.clone(&response_);
}

int DpiClient::createServerSocket() {
    struct timeval tv;
    addr_size_t addr_sz;

    memset(&sockaddr_ipv4_, 0, sizeof(struct sockaddr_in));
    sockaddr_ipv4_.sin_family = AF_INET;
    sockaddr_ipv4_.sin_addr.s_addr = inet_addr(hostIP_.to_string());
    sockaddr_ipv4_.sin_port = htons(static_cast<uint16_t>(hostPort_.to_int()));

    hsock_ = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (hsock_ < 0) {
        RISCV_error("%s", "Error: socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)");
        return -1;
    }

#if defined(_WIN32) || defined(__CYGWIN__)
    /** On windows timeout of the setsockopt() function is the DWORD
        * size variable in msec, so we use only the first field in timeval
        * struct and directly assgign argument.
        */
    tv.tv_sec = timeout_.to_int();
    tv.tv_usec = 0;
#else
    tv.tv_usec = (timeout_.to_int() % 1000) * 1000;
    tv.tv_sec = timeout_.to_int() / 1000;
#endif
    setsockopt(hsock_, SOL_SOCKET, SO_RCVTIMEO,
                    reinterpret_cast<char *>(&tv), sizeof(struct timeval));

    addr_sz = sizeof(sockaddr_ipv4_);
    getsockname(hsock_,
                reinterpret_cast<struct sockaddr *>(&sockaddr_ipv4_),
                &addr_sz);

    RISCV_info("IPv4 address %s:%d . . . created",
                inet_ntoa(sockaddr_ipv4_.sin_addr),
                ntohs(sockaddr_ipv4_.sin_port));

    return 0;
}

void DpiClient::closeServerSocket() {
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

void DpiClient::axi4_write(uint64_t addr, uint64_t data) {
    char tstr[1024];
    AttributeType resp;
    int sz = RISCV_sprintf(tstr, sizeof(tstr),
        "["
           "'%s',"
           "'AXI4',"
           "{"
               "'we':1,"
               "'addr':0x%" RV_PRI64 "x,"
               "'wdata':[0x%" RV_PRI64 "x]"
            "}"
        "]",
        addr, data);
    syncRequest(tstr, sz + 1, resp);
}

void DpiClient::axi4_read(uint64_t addr, uint64_t *data) {
    char tstr[1024];
    AttributeType resp;
    int sz = RISCV_sprintf(tstr, sizeof(tstr),
        "['%s','AXI4',{'we':0,'addr':0x%" RV_PRI64 "x}]", addr);
    syncRequest(tstr, sz + 1, resp);
}

bool DpiClient::is_irq() {
    return false;
}

int DpiClient::get_irq() {
    return 0;
}

}  // namespace debugger
