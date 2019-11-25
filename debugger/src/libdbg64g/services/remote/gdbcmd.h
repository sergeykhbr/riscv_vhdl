/*
 *  Copyright 2019 Andrey Sudnik
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

#ifndef __DEBUGGER_SERVICES_REMOTE_GDBCMD_H__
#define __DEBUGGER_SERVICES_REMOTE_GDBCMD_H__

#include "tcpcmd_gen.h"

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


class GdbCommands : public TcpCommandsGen {
 public:
    explicit GdbCommands(IService *parent);

 protected:
    virtual int processCommand(const char *cmdbuf, int bufsz);
    virtual bool isStartMarker(char s) {
        if (s == '+' || s == '-') {
            handleHandshake(s);
        }
        return s == '$';
    }
    virtual bool isEndMarker(const char *s, int sz) {
        return s[sz - 3] == '#';
    }

 private:
    void handleHandshake(char s);
    void handlePacket(char *data);
    uint8_t checksum(const char *data, const int sz);
    void sendPacket(const char *data);

    // RSP packet handlers
    void handleStopReasonQuery();
    void handleContinue();
    void handleDetach();
    void handleGetRegisters();
    void handleSetRegisters();
    void handleSetThread();
    void handleKill();
    void handleGetMemory();
    void handleWriteMemoryHex();
    void handleReadRegister();
    void handleWriteRegister();
    void handleQuery();
    void handleGeneralSet();
    void handleStep();
    void handleThreadAlive();
    void handleVCommand();
    void handleWriteMemory();
    void handleBreakpoint();

    void appendRegValue(char *s, uint32_t value);

 private:
    //RspPacket previous_packet;
    //bool is_ack_mode;
    //bool last_success_;
    char packet_data_[1 << 16];
    enum EState {
        State_AckMode,
        State_WaitAckToSwitch,
        State_NoAckMode
    } estate_;
};

}
#endif  // __DEBUGGER_SERVICES_REMOTE_GDBCMD_H__
