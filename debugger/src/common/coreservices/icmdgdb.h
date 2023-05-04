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

namespace debugger {

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
        needAck_ = false;
        isAcked_ = false;
        needResp_ = false;
        isResp_ = false;
    }
    explicit GdbCommandGeneric(const char *data)
        : GdbCommandGeneric(data, strlen(data)) {}

    explicit GdbCommandGeneric() : GdbCommandGeneric("", 0) {}

    virtual const char *to_string() { return cmdstr_; }
    virtual int getStringSize() { return cmdsz_; }
    virtual bool isAcked() { return needAck_ == false || isAcked_; }
    virtual void setNeedAck() { needAck_ = true; }
    virtual void setAck() { isAcked_ = true; }
    virtual bool isResponded() { return needResp_ == false || isResp_; }
    virtual void handleResponse(const char *data, const int sz) { isResp_ = true; }

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
    bool needAck_;
    bool isAcked_;
    bool needResp_;
    bool isResp_;
};

class GdbCommandGenericRequest : public GdbCommandGeneric {
 public:
    GdbCommandGenericRequest(const char *data)
        : GdbCommandGeneric(data) {
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


static const char *IFACE_GDB_EXECUTOR = "IGdbExecutor";

class IGdbExecutor : public IFace {
 public:
    IGdbExecutor() : IFace(IFACE_GDB_EXECUTOR) {}

    virtual void exec(GdbCommandGeneric *cmd) = 0;

 protected:
};


}
