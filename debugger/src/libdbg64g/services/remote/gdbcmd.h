/*
 *  Copyright 2023 Sergey Khabarov, sergeykhbr@gmail.com
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

#pragma once

#include "coreservices/ijtag.h"
#include "coreservices/icmdexec.h"
#include "generic/tcpclient.h"

namespace debugger {

static const int DATA_MAX = 4096;

/*struct RspPacket {
    RspPacket() : size(0), is_good(false) {}
    RspPacket(const char* const c_str);

    void removeCRC();
    void append(const char& c) { data[size++] = c; }
    void append(const char* c_str);

    uint8_t checksum(const char *data, const size_t sz);

 public:
    char    data[DATA_MAX];
    size_t  size;
    bool    is_good;
};*/

class OcdWrapperCommandGeneric {
 public:
    OcdWrapperCommandGeneric() {}

    virtual int processRxBuffer(const char *buf, int sz) { return sz; }
};

class GdbCommandGeneric : public OcdWrapperCommandGeneric {
 public:
    GdbCommandGeneric(const char *data, int datasz) : OcdWrapperCommandGeneric() {
        char tcrc[3];
        cmdsz_ = 0;
        cmdstr_[cmdsz_++] = '$';
        for (int i = 0; i < datasz; i++) {
            cmdstr_[cmdsz_++] = data[i];
        }
        cmdstr_[cmdsz_++] = '#';
        RISCV_sprintf(tcrc, sizeof(tcrc), "%02x",
                        crc8(&cmdstr_[1], cmdsz_ - 2));

        cmdstr_[cmdsz_++] = tcrc[0];
        cmdstr_[cmdsz_++] = tcrc[1];
        cmdstr_[cmdsz_] = '\0';
        request_ = false;
        needAck_ = false;
        isAcked_ = false;
    }
    explicit GdbCommandGeneric(const char *data)
        : GdbCommandGeneric(data, strlen(data)) {}

    explicit GdbCommandGeneric() : GdbCommandGeneric("", 0) {}

    virtual const char *to_string() { return cmdstr_; }
    virtual int getStringSize() { return cmdsz_; }
    virtual bool isAcked() { return needAck_ == false || isAcked_; }
    virtual void setNeedAck() { needAck_ = true; }
    virtual void setAck() { isAcked_ = true; }
    virtual bool isRequest() { return request_; }
    virtual void setRequest() { request_ = true; }
    virtual void clearRequest() { request_ = false; }
    virtual bool isEqual(const char *str) {
        const char *p = &cmdstr_[1];
        while (*str) {
            if (*str++ != *p++) {
                return false;
            }
        }
        return true;
    }
    virtual void handleResponse(GdbCommandGeneric *resp) {}

    uint8_t crc8(const char *data, const int sz) {
        uint8_t sum = 0;
        for (int i = 0; i < sz; i++) {
            sum += static_cast<uint8_t>(data[i]);
        }
        return sum;
    }

 protected:
    char cmdstr_[1<<12];
    int cmdsz_;
    bool request_;
    bool needAck_;
    bool isAcked_;
};

class GdbCommandGenericRequest : public GdbCommandGeneric {
 public:
    GdbCommandGenericRequest(const char *data)
        : GdbCommandGeneric(data) {
        request_ = true;
    }
};


class GdbCommand_QStartNoAckMode : public GdbCommandGenericRequest {
 public:
    GdbCommand_QStartNoAckMode() : GdbCommandGenericRequest("QStartNoAckMode") {
        setNeedAck();
    }
};

class GdbCommand_Continue : public GdbCommandGenericRequest {
 public:
    GdbCommand_Continue() : GdbCommandGenericRequest("c") {}
};


class GdbCommand_Step : public GdbCommandGenericRequest {
 public:
    GdbCommand_Step() : GdbCommandGenericRequest("s") {
    }
};

/** "Ctrl-C", on the other hand, is defined and implemented for all transport
 * mechanisms. It is represented by sending the single byte 0x03 without any
 of the usual packet overhead
*/
class GdbCommand_Halt : public GdbCommandGenericRequest {
 public:
    GdbCommand_Halt() : GdbCommandGenericRequest("") {
        cmdsz_ = 0;
        cmdstr_[cmdsz_++] = 0x03;
    }
};



class TcpClientGdb : public TcpClient {
 public:
    explicit TcpClientGdb(const char *name);

 protected:
    /** TcpClient */
    virtual int processRxBuffer(const char *ibuf, int ilen);

 private:
    virtual bool isStartMarker(char s);
    virtual bool isEndMarker(const char *s, int sz);
    virtual int checkPayload(const char *cmdbuf, int bufsz);
    virtual void handlePacket(const char *data);
    virtual uint8_t checksum(const char *data, const int sz);
    virtual void sendPacket(const char *data);

    // RSP packet handlers
    void handleStopReasonQuery();
    void handleContinue();
    void handleDetach();
    void handleGetRegisters();
    void handleSetRegisters();
    void handleSetThread();
    void handleKill();
    void handleGetMemory(const char *data);
    void handleWriteMemoryHex();
    void handleReadRegister(const char *data);
    void handleWriteRegister(const char *data);
    void handleQuery(const char *data);
    void handleGeneralSet(const char *data);
    void handleStep();
    void handleThreadAlive();
    void handleVCommand(const char *data);
    void handleWriteMemory(const char *data);
    void handleBreakpoint(const char *data);

    void appendRegValue(char *s, uint32_t value);

 private:
    //RspPacket previous_packet;
    //bool is_ack_mode;
    //bool last_success_;
    char msg_[1 << 16];
    int msgcnt_;
    bool enableAckMode_;

    IJtag *ijtag_;
    ICmdExecutor *iexec_;
};

DECLARE_CLASS(TcpClientGdb)

}
