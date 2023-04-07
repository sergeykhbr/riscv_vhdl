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

class GdbCommandGeneric {
 public:
    GdbCommandGeneric() {
        cmdstr_[0] = '\0';
        cmdsz_ = 0;
    }
    virtual const char *to_string() { return cmdstr_; }
    virtual int getStringSize() { return cmdsz_; }

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
};

class GdbCommand_Detach : public GdbCommandGeneric {
 public:
    GdbCommand_Detach() : GdbCommandGeneric() {
        int tsz = RISCV_sprintf(cmdstr_, sizeof(cmdstr_), "$%s#", "D");
        tsz += RISCV_sprintf(&cmdstr_[tsz], sizeof(cmdstr_) - tsz, "%02x",
                             crc8(&cmdstr_[1], tsz - 2));
        cmdsz_ = tsz;
    }
};

class GdbCommand_QStartNoAckMode : public GdbCommandGeneric {
 public:
    GdbCommand_QStartNoAckMode() : GdbCommandGeneric() {
        int tsz = RISCV_sprintf(cmdstr_, sizeof(cmdstr_), "$%s#", "QStartNoAckMode");
        tsz += RISCV_sprintf(&cmdstr_[tsz], sizeof(cmdstr_) - tsz, "%02x",
                             crc8(&cmdstr_[1], tsz - 2));
        cmdsz_ = tsz;
    }
};


class GdbCommand_vContRequest : public GdbCommandGeneric {
 public:
    GdbCommand_vContRequest() : GdbCommandGeneric() {
        int tsz = RISCV_sprintf(cmdstr_, sizeof(cmdstr_), "$%s?#", "vCont");
        tsz += RISCV_sprintf(&cmdstr_[tsz], sizeof(cmdstr_) - tsz, "%02x",
                             crc8(&cmdstr_[1], tsz - 2));
        cmdsz_ = tsz;
    }
};

class GdbCommand_vCtrlC : public GdbCommandGeneric {
 public:
    GdbCommand_vCtrlC() : GdbCommandGeneric() {
        int tsz = RISCV_sprintf(cmdstr_, sizeof(cmdstr_), "$%s#", "vCtrlC");
        tsz += RISCV_sprintf(&cmdstr_[tsz], sizeof(cmdstr_) - tsz, "%02x",
                             crc8(&cmdstr_[1], tsz - 2));
        cmdsz_ = tsz;
    }
};

class GdbCommand_Continue : public GdbCommandGeneric {
 public:
    GdbCommand_Continue() : GdbCommandGeneric() {
        int tsz = RISCV_sprintf(cmdstr_, sizeof(cmdstr_), "$%s#", "C");
        tsz += RISCV_sprintf(&cmdstr_[tsz], sizeof(cmdstr_) - tsz, "%02x",
                             crc8(&cmdstr_[1], tsz - 2));
        cmdsz_ = tsz;
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
