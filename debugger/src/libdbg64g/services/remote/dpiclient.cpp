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
#include <string.h>

namespace debugger {

enum EDpiResponseList {
    DpiResp_CmdType,
    DpiResp_Data,
    DpiResp_ListSize
};


DpiClient::DpiClient(const char *name) : IService(name) {
    registerInterface(static_cast<IThread *>(this));
    registerInterface(static_cast<IDpi *>(this));
    registerInterface(static_cast<ITap *>(this));
    registerAttribute("Enable", &isEnable_);
    registerAttribute("CmdExecutor", &cmdexec_);
    registerAttribute("Timeout", &timeout_);
    registerAttribute("HostIP", &hostIP_);
    registerAttribute("HostPort", &hostPort_);

    RISCV_event_create(&event_cmd_, name);
    RISCV_mutex_init(&mutex_tx_);

    char tstr[256];
    RISCV_sprintf(tstr, sizeof(tstr), "['%s','HartBeat']", name);
    reqHartBeat_.make_string(tstr);
    cmdcnt_ = 0;
    txcnt_ = 0;
    hsock_ = 0;
    hartbeatTime_ = 0;
    hartbeatClkcnt_ = 0;
}

DpiClient::~DpiClient() {
    RISCV_mutex_destroy(&mutex_tx_);
    RISCV_event_close(&event_cmd_);
}

void DpiClient::postinitService() {
    iexec_ = static_cast<ICmdExecutor *>(
            RISCV_get_service_iface(cmdexec_.to_string(), 
                                    IFACE_CMD_EXECUTOR));
    if (!iexec_) {
        RISCV_error("Can't get ICmdExecutor interface %s",
                    cmdexec_.to_string());
    }

    if (isEnable_.to_bool()) {
        if (!run()) {
            RISCV_error("Can't create thread.", NULL);
            return;
        }
    }
}

void DpiClient::predeleteService() {
}

void DpiClient::busyLoop() {
    int err;
    int rxbytes;
    connected_ = false;
    while (isEnabled()) {
        if (hsock_ == 0) {
            connected_ = false;
            if (createServerSocket() < 0) {
                RISCV_sleep_ms(1000);
            }
            continue;
        }

        if (!connected_) {
            RISCV_important("connecting to host %s:%d",
                hostIP_.to_string(), hostPort_.to_int());
            err = connect(hsock_,
                          (struct sockaddr *)&sockaddr_ipv4_,
                          sizeof(struct sockaddr));
            if (err < 0) {
#if defined(_WIN32) || defined(__CYGWIN__)
                RISCV_error("error connect(): %s", strerror(GetLastError()));
#else
                RISCV_error("error connect(): %s", strerror(errno));
#endif
                RISCV_sleep_ms(2000);
                continue;
            }
            connected_ = true;
            cmdcnt_ = 0;
            txcnt_ = 0;
        }

        rxbytes = recv(hsock_, rcvbuf, sizeof(rcvbuf), 0);

        if (rxbytes <= 0) {
            // Timeout:
            writeTx(reqHartBeat_.to_string(), reqHartBeat_.size() + 1);
            rxbytes = 0;
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
    if (strstr(cmdbuf_, "KeepAlive")) {
        /** Async. message from server thread. Do nothing */
        return;
    }
    if (strstr(cmdbuf_, "HartBeat")) {
        /** Sync. response on HartBeat request from client thread */
        respHartBeat_.from_config(cmdbuf_);
        if (!respHartBeat_.is_list() ||
            respHartBeat_.size() < DpiResp_ListSize) {
            RISCV_error("Wrong hartbeat response %s", cmdbuf_);
            return;
        }
        AttributeType &data = respHartBeat_[DpiResp_Data];
        hartbeatTime_ = data["tm"].to_float();
        hartbeatClkcnt_ = data["clkcnt"].to_uint64();
        return;
    }
    //RISCV_debug("i<=%s", cmdbuf_);
    syncResponse_.from_config(cmdbuf_);
    RISCV_event_set(&event_cmd_);
}

void DpiClient::processTx() {
    const char *ptx = txbuf_;

    RISCV_mutex_lock(&mutex_tx_);
    int total = static_cast<int>(txcnt_);
    int txbytes;
    while (total > 0) {
        txbytes = send(hsock_, ptx, total, 0);
        if (txbytes <= 0) {
            RISCV_error("processTx error: txcnt=%d", txbytes);
            closeServerSocket();
            total = 0;
            break;
        }
        total -= txbytes;
        ptx += txbytes;

    }
    txcnt_ -= txcnt_;
    RISCV_mutex_unlock(&mutex_tx_);
}

void DpiClient::writeTx(const char *buf, unsigned size) {
    RISCV_mutex_lock(&mutex_tx_);
    if ((txcnt_ + size) >= sizeof(txbuf_)) {
        RISCV_error("Tx overflow %d: %s", size, buf);
        size = sizeof(txbuf_) - txcnt_;
    }
    memcpy(&txbuf_[txcnt_], buf, size);
    txcnt_ += size;
    RISCV_mutex_unlock(&mutex_tx_);
}

bool DpiClient::syncRequest(const char *buf, unsigned size) {
    if (!connected_) {
        return false;
    }
    RISCV_event_clear(&event_cmd_);
    writeTx(buf, size);
    processTx();
    RISCV_event_wait(&event_cmd_);
    if (!syncResponse_.is_list() ||
        syncResponse_.size() < DpiResp_ListSize) {
        RISCV_error("%s", "Wrong response format");
        return false;
    }
    return true;
}

int DpiClient::createServerSocket() {
    struct timeval tv;
    int nodelay = 1;

    hsock_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (hsock_ < 0) {
        hsock_ = 0;
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
    setsockopt(hsock_, SOL_SOCKET, TCP_NODELAY,
                    reinterpret_cast<char *>(&nodelay), sizeof(nodelay));


    /* gethostbyname: get the server's DNS entry */
    struct hostent *server = gethostbyname(hostIP_.to_string());
    if (server == NULL) {
#if defined(_WIN32) || defined(__CYGWIN__)
        RISCV_error("error gethostbyname(): %s", strerror(GetLastError()));
#else
        RISCV_error("error gethostbyname(): %s", strerror(errno));
#endif
    }

    memset(&sockaddr_ipv4_, 0, sizeof(struct sockaddr_in));
    sockaddr_ipv4_.sin_family = AF_INET;
    inet_pton(AF_INET, hostIP_.to_string(), &sockaddr_ipv4_.sin_addr);
    sockaddr_ipv4_.sin_port = htons(static_cast<uint16_t>(hostPort_.to_int()));

    RISCV_info("IPv4 address %s:%d . . . created",
                inet_ntoa(sockaddr_ipv4_.sin_addr),
                ntohs(sockaddr_ipv4_.sin_port));
    return 0;
}

void DpiClient::closeServerSocket() {
    connected_ = false;
    if (hsock_ == 0) {
        return;
    }

#if defined(_WIN32) || defined(__CYGWIN__)
    closesocket(hsock_);
#else
    shutdown(hsock_, SHUT_RDWR);
    close(hsock_);
#endif
    hsock_ = 0;
}

void DpiClient::axi4_write(uint64_t addr, int bytes, uint64_t data) {
    char tstr[1024];
    AttributeType resp;
    int sz = RISCV_sprintf(tstr, sizeof(tstr),
        "["
           "'%s',"
           "'AXI4',"
           "{"
               "'we':1,"
               "'addr':0x%" RV_PRI64 "x,"
               "'bytes':%d,"
               "'wdata':[0x%" RV_PRI64 "x]"
            "}"
        "]",
        getObjName(), addr, bytes, data);
    syncRequest(tstr, sz + 1);
}

void DpiClient::axi4_read(uint64_t addr, int bytes, uint64_t *data) {
    char tstr[1024];
    int sz = RISCV_sprintf(tstr, sizeof(tstr),
        "["
           "'%s',"
           "'AXI4',"
           "{"
               "'we':0,"
               "'addr':0x%" RV_PRI64 "x,"
               "'bytes':%d"
           "}"
        "]",
        getObjName(), addr, bytes);

    if (!syncRequest(tstr, sz + 1)) {
        return;
    }
    AttributeType &d = syncResponse_[DpiResp_Data];
    AttributeType &rdata = d["rdata"];
    *data = rdata[0u].to_uint64();
}

void DpiClient::msgRead(uint64_t addr, int bytes) {
    tmpsz_ = RISCV_sprintf(tmpbuf_, sizeof(tmpbuf_),
        "["
           "'%s',"
           "'AXI4',"
           "{"
               "'we':0,"
               "'addr':0x%" RV_PRI64 "x,"
               "'bytes':%d"
           "}"
        "]",
        getObjName(), addr, bytes);
}

void DpiClient::msgWrite(uint64_t addr, int bytes, uint8_t *buf) {
    uint64_t off = addr & 0x7;
    Reg64Type t;
    if (off != 0 && (off + bytes) > 8) {
        RISCV_error("Wrong write request %016" RV_PRI64 "x", addr);
        return;
    }
    if (bytes < 8) {
        t.val = 0;
        memcpy(t.buf, buf, bytes);

        tmpsz_ = RISCV_sprintf(tmpbuf_, sizeof(tmpbuf_),
            "["
               "'%s',"
               "'AXI4',"
               "{"
                   "'we':1,"
                   "'addr':0x%" RV_PRI64 "x,"
                   "'bytes':%d,"
                   "'wdata':[0x%" RV_PRI64 "x]"
                "}"
            "]",
            getObjName(), addr, bytes, t.val);
    } else {
        tmpsz_ = RISCV_sprintf(tmpbuf_, sizeof(tmpbuf_),
            "["
               "'%s',"
               "'AXI4',"
               "{"
                   "'we':1,"
                   "'addr':0x%" RV_PRI64 "x,"
                   "'bytes':%d,"
                   "'wdata':[",
            getObjName(), addr, bytes);

        for (int i = 0; i < bytes/8; i++) {
            if (i != 0) {
                tmpbuf_[tmpsz_++] = ',';
            }
            memcpy(t.buf, &buf[8*i], 8);
            tmpsz_ += RISCV_sprintf(&tmpbuf_[tmpsz_], sizeof(tmpbuf_) - tmpsz_,
                    "0x%" RV_PRI64 "x", t.val);
        }

        tmpsz_ += RISCV_sprintf(&tmpbuf_[tmpsz_], sizeof(tmpbuf_) - tmpsz_,
            "%s",
                   "]"
                "}"
            "]");
    }
}

int DpiClient::read(uint64_t addr, int bytes, uint8_t *obuf) {
    uint8_t *pout = obuf;
    int bytes_total = bytes;
    Reg64Type t;

    // Read unaligned first qword
    if ((addr & 0x7) != 0 || bytes < 8) {
        int toffset;
        int tbytes;
        toffset = addr & 0x7;
        tbytes = 8 - toffset;
        if (tbytes > bytes_total) {
            tbytes = bytes_total;
        }
        msgRead(addr & ~0x7ull, 8);                 // 8-bytes alignment
        if (!syncRequest(tmpbuf_, tmpsz_ + 1)) {
            return bytes;
        }
        AttributeType &d = syncResponse_[DpiResp_Data];
        AttributeType &rdata = d["rdata"];
        t.val = rdata[0u].to_uint64();
        memcpy(pout, &t.buf[toffset], tbytes);

        addr += tbytes;
        pout += tbytes;
        bytes_total -= tbytes;
    }

    // Request aligned/burst transaction:
    int burstbytes = bytes_total & ~0x7;
    int bulksz;
    while (burstbytes) {
        bulksz = burstbytes;
        if (bulksz > BURST_LEN_MAX) {
            bulksz = BURST_LEN_MAX;
        }
        msgRead(addr, bulksz);                 // 8-bytes alignment
        if (!syncRequest(tmpbuf_, tmpsz_ + 1)) {
            return bytes;
        }
        AttributeType &d = syncResponse_[DpiResp_Data];
        AttributeType &rdata = d["rdata"];
        for (unsigned i = 0; i < rdata.size(); i++) {
            t.val = rdata[i].to_uint64();
            memcpy(pout, t.buf, 8);
            pout += 8;
        }

        addr += bulksz;
        bytes_total -= bulksz;
        burstbytes -= bulksz;
    }

    // Ending unaligned bytes
    if (bytes_total) {
        msgRead(addr, 8);                 // 8-bytes alignment
        if (!syncRequest(tmpbuf_, tmpsz_ + 1)) {
            return bytes;
        }
        AttributeType &d = syncResponse_[DpiResp_Data];
        AttributeType &rdata = d["rdata"];
        Reg64Type t;
        t.val = rdata[0u].to_uint64();
        memcpy(pout, t.buf, bytes_total);

        addr += bytes_total;
        pout += bytes_total;
        bytes_total = 0;
    }
    return bytes;
}

int DpiClient::write(uint64_t addr, int bytes, uint8_t *ibuf) {
    uint8_t *pin = ibuf;
    int bytes_total = bytes;

    // Read unaligned first qword
    if ((addr & 0x7) != 0 || bytes < 8) {
        int toffset;
        int tbytes;
        toffset = addr & 0x7;
        tbytes = 8 - toffset;
        if (tbytes > bytes_total) {
            tbytes = bytes_total;
        }
        msgWrite(addr, tbytes, pin);
        syncRequest(tmpbuf_, tmpsz_ + 1);

        addr += tbytes;
        pin += tbytes;
        bytes_total -= tbytes;
    }

    // Request aligned/burst transaction:
    int burstbytes = bytes_total & ~0x7;
    int bulksz;
    while (burstbytes) {
        bulksz = burstbytes;
        if (bulksz > BURST_LEN_MAX) {
            bulksz = BURST_LEN_MAX;
        }
        msgWrite(addr, bulksz, pin);
        syncRequest(tmpbuf_, tmpsz_ + 1);

        pin += bulksz;
        addr += bulksz;
        bytes_total -= bulksz;
        burstbytes -= bulksz;
    }

    // Ending unaligned bytes
    if (bytes_total) {
        msgWrite(addr, bytes_total, pin);
        syncRequest(tmpbuf_, tmpsz_ + 1);
    }
    return bytes;
}

bool DpiClient::is_irq() {
    return false;
}

int DpiClient::get_irq() {
    return 0;
}

}  // namespace debugger
