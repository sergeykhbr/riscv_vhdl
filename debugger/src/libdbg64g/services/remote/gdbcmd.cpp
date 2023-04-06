/*
 * Copyright 2023 Sergey Khabarov, sergeykhbr@gmail.com
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

#include "gdbcmd.h"
#include <string>

namespace debugger {
/*
RspPacket::RspPacket(const char* const c_str) : RspPacket() {
    append('$');
    append(c_str);
    append('#');
    RISCV_sprintf(&data[size], sizeof(data) - size, "%02x",
                checksum(c_str, strlen(c_str)));
    size += 2; // for checksum
    data[size] = '\0';
}

void RspPacket::removeCRC(){
    data[size - 3] = '\0';
    size -= 3;
}

void RspPacket::append(const char* c_str){
    size_t length = strlen(c_str);
    memcpy(&data[size], c_str, length);
    size += length;
}

uint8_t RspPacket::checksum(const char *data, const size_t sz) {
    uint8_t sum = 0;
    for (size_t i = 0; i < sz; i++) {
        sum += static_cast<unsigned char>(data[i]);
    }
    return sum;
}
*/

TcpClientGdb::TcpClientGdb(const char *name)
    : TcpClient(0, name) {
    msgcnt_ = 0;
    enableAckMode_ = false;
}

bool TcpClientGdb::isStartMarker(char s) {
    return (s == '+' || s == '-' || s == '$');
}

bool TcpClientGdb::isEndMarker(const char *s, int sz) {
    return (s[0] == '+' || s[0] == '-' || (sz >= 3 && s[sz - 3] == '#'));
}

int TcpClientGdb::processRxBuffer(const char *ibuf, int ilen) {
    for (int i = 0; i < ilen; i++) {
        if (msgcnt_ == 0 && isStartMarker(ibuf[i])) {
            msg_[msgcnt_++] = ibuf[i];
        } else if (msgcnt_ >= sizeof(msg_)) {
            RISCV_error("msg buffer overflow %d", msgcnt_);
            return -1;
        } else {
            msg_[msgcnt_++] = ibuf[i];
        }
        msg_[msgcnt_] = '\0';

        if (msgcnt_ && isEndMarker(ibuf, i)) {
            if (checkPayload(msg_, msgcnt_) != 0) {
                msg_[msgcnt_ - 3] = '\0';
                handlePacket(&msg_[1]);
            }
            msgcnt_ = 0;
        }
    }
    return 0;
}

int TcpClientGdb::checkPayload(const char *cmdbuf, int bufsz) {
    if (cmdbuf[0] == '-') {
        enableAckMode_ = true;
        return 0;
    }
    if (cmdbuf[0] == '+') {
        enableAckMode_ = false;
        return 0;
    }

    uint8_t crc8 = checksum(&cmdbuf[1], bufsz - 4);
    uint8_t expected_crc8 = static_cast<uint8_t>(atoi(&cmdbuf[bufsz - 2]));

    if (crc8 != expected_crc8) {
        RISCV_info("wrong checksum: %02x != %02x", crc8, expected_crc8);
        return 0;
    }

    return bufsz - 4;;
}

void TcpClientGdb::handlePacket(const char *data) {
    switch (data[0]) {
    case '!':
        sendPacket("OK");
        break;
    case '?':   // Stop reason query.
        handleStopReasonQuery();
        break;
    case 'c' :  // Continue (at addr)
    case 'C' :  // Continue with signal.
        handleContinue();
        break;
    case 'D' :  // Detach.
        handleDetach();
        break;
    case 'g' :  // Read general registers.
        handleGetRegisters();
        break;
    case 'G' :  // Write general registers.
        handleSetRegisters();
        break;
    case 'H' :  // Set thread for subsequent operations.
        handleSetThread();
        break;
    case 'k' :  // Kill.
        handleKill();
        break;
    case 'm' :  // Read memory.
        handleGetMemory(data);
        break;
    case 'M' :  // Write memory (hex).
        handleWriteMemoryHex();
        break;
    case 'p' :  // Read register.
        handleReadRegister(data);
        break;
    case 'P' :  // Write register.
        handleWriteRegister(data);
        break;
    case 'q' :  // General query.
        handleQuery(data);
        break;
    case 'Q' :  // General set.
        handleGeneralSet(data);
        break;
    case 's' :  // Single step.
    case 'S' :  // Step with signal.
        handleStep();
        break;
    case 'T' :  // Thread liveness query.
        handleThreadAlive();
        break;
    case 'v' :  // v command.
        handleVCommand(data);
        break;
    case 'X' :  // Write memory (binary).
        handleWriteMemory(data);
        break;
    case 'z' :  // Remove breakpoint/watchpoint.
    case 'Z' :  // Insert breakpoint/watchpoint.
        handleBreakpoint(data);
        break;
    case '#':   // end of message
        return;
    default:
        RISCV_info("Unknown RSP packet: %s", data);
    }
}

void TcpClientGdb::handleQuery(const char *data) {
    if (strcmp("qAttached", data) == 0) {
        /* Always attaching to an existing process, let thread id to be 1 */
        sendPacket("1");
    } else if (strcmp("qC", data) == 0) {
        /* Return the current thread ID.
         * Reply QCpid - Where pid is a HEX encoded 16 bit process id.
         * Set thread id 1, because we don't have other threads. */
        sendPacket("QC1");
    } else if (strncmp("qCRC", data, strlen("qCRC")) == 0) {
        /* Return CRC of memory area.
         * Return Error 01 */
        sendPacket("E01");
    } else if (strcmp("qfThreadInfo", data) == 0) {
        /* Obtain a list of active thread ids from the target (OS)
         * this query works iteratively:
         * it may require more than one query/reply sequence to obtain the
         * entire list of threads.
         * The first query of the sequence will be the qfThreadInfo query;
         * subsequent queries in the sequence will be the qsThreadInfo query
         * GDB will respond to each reply with a request for more thread ids
         * using the qsThreadInfo query
         *
         * reply m<id>          a single thread id
         * reply m<id>,<id>,... a comma-separated list of thread ids
         * reply l 	            denotes end of list.
         */
        sendPacket("m1");
    } else if (strcmp("qsThreadInfo", data) == 0) {
        /* Return info about more active threads.
         * We have no more, so return the end of list marker, 'l' */
        sendPacket("l");
    } else if (strncmp("qGetTLSAddr:", 
                       data, strlen("qGetTLSAddr:")) == 0) {
        /* We don't support this feature */
        sendPacket("");
    } else if (strncmp("qL", data, strlen("qL")) == 0) {
        /* Deprecated and replaced by 'qfThreadInfo' */
        sendPacket("qM001");
    } else if (strcmp("qOffsets", data) == 0) {
        /* Report any relocation */
        sendPacket("Text=0;Data=0;Bss=0");
    } else if (strncmp("qP", data, strlen("qP")) == 0) {
        /* Deprecated and replaced by 'qThreadExtraInfo' */
        sendPacket("");
    } else if (strncmp("qRcmd,", data, strlen("qRcmd,")) == 0) {
        /* This is used to interface to commands to do "stuff" */
        sendPacket("");
    } else if (strncmp("qSupported", 
                        data, strlen("qSupported")) == 0) {
        /* Report a list of the features we support.
         * 1000h == 4096
         * 500h  == 1280 */
        sendPacket("PacketSize=500;QStartNoAckMode+;vContSupported+");
        //QNonStop+
    } else if (strncmp("qSymbol:", data, strlen("qSymbol:")) == 0) {
        /* Offer to look up symbols. Ignore for now */
        sendPacket("OK");
    } else if (strncmp("qThreadExtraInfo,",
                       data, strlen("qThreadExtraInfo,")) == 0) {
        /* Report that we are runnable */
        char tstr[128] = "\0";
        RISCV_sprintf (tstr, sizeof(tstr), "%02x%02x%02x%02x%02x%02x%02x%02x",
             'R', 'u', 'n', 'n', 'a', 'b', 'l', 'e');
        sendPacket(tstr);
    } else if (strncmp("qTStatus", data, strlen("qTStatus")) == 0) {
        /* Don't support tracing, return empty packet. */
        sendPacket("");
    } else if (strncmp("qXfer:", data, strlen("qXfer:")) == 0) {
        /* No 'qXfer' requests supported, return empty packet. */
        sendPacket("");
    } else {
        RISCV_error("Unrecognized RSP query: %s \n", data);
        sendPacket("");
    }
}

void TcpClientGdb::handleStopReasonQuery() {
    sendPacket("S05");
}

void TcpClientGdb::handleContinue() {
    AttributeType res;
    if (!ijtag_) {
        sendPacket("E01");
        return;
    }
    iexec_->exec("run", &res, false);
    sendPacket("S05");
}

void TcpClientGdb::handleDetach() {
    /* Reply "OK" to GDB and close socket. */
    sendPacket("OK");
}

void TcpClientGdb::appendRegValue(char *s, uint32_t value) {
    char byte_hex[3];

    RISCV_sprintf(byte_hex, 3, "%02X", value & 0xFF);
    strncat(s, byte_hex, 2);
    RISCV_sprintf(byte_hex, 3, "%02X", (value >> 8) & 0xFF);
    strncat(s, byte_hex, 2);
    RISCV_sprintf(byte_hex, 3, "%02X", (value >> 16) & 0xFF);
    strncat(s, byte_hex, 2);
    RISCV_sprintf(byte_hex, 3, "%02X", (value >> 24) & 0xFF);
    strncat(s, byte_hex, 2);
}

void TcpClientGdb::handleGetRegisters() {
    char resp[512] = "\0";
    AttributeType res;

    if (iexec_) {
        iexec_->exec("reg", &res, false);
    }
    appendRegValue(resp, res["r0"].to_uint32());
    appendRegValue(resp, res["r1"].to_uint32());
    appendRegValue(resp, res["r2"].to_uint32());
    appendRegValue(resp, res["r3"].to_uint32());
    appendRegValue(resp, res["r4"].to_uint32());
    appendRegValue(resp, res["r5"].to_uint32());
    appendRegValue(resp, res["r6"].to_uint32());
    appendRegValue(resp, res["r7"].to_uint32());
    appendRegValue(resp, res["r8"].to_uint32());
    appendRegValue(resp, res["r9"].to_uint32());
    appendRegValue(resp, res["r10"].to_uint32());
    appendRegValue(resp, res["r11"].to_uint32());
    appendRegValue(resp, res["fp"].to_uint32());
    appendRegValue(resp, res["sp"].to_uint32());
    appendRegValue(resp, res["lr"].to_uint32());
    appendRegValue(resp, res["pc"].to_uint32());
    strncat(resp, "xxxxxxxxxxxxxxxxxxxxxxxxx", 96 / 8 * 2);  // f0
    strncat(resp, "xxxxxxxxxxxxxxxxxxxxxxxxx", 96 / 8 * 2);
    strncat(resp, "xxxxxxxxxxxxxxxxxxxxxxxxx", 96 / 8 * 2);
    strncat(resp, "xxxxxxxxxxxxxxxxxxxxxxxxx", 96 / 8 * 2);
    strncat(resp, "xxxxxxxxxxxxxxxxxxxxxxxxx", 96 / 8 * 2);
    strncat(resp, "xxxxxxxxxxxxxxxxxxxxxxxxx", 96 / 8 * 2);
    strncat(resp, "xxxxxxxxxxxxxxxxxxxxxxxxx", 96 / 8 * 2);
    strncat(resp, "xxxxxxxxxxxxxxxxxxxxxxxxx", 96 / 8 * 2);  // f7
    strncat(resp, "xxxxxxxxxxxxxxxxxxxxxxxxx", 32 / 8 * 2);  // fps
    appendRegValue(resp, res["cpsr"].to_uint32());
    sendPacket(resp);
}

void TcpClientGdb::handleSetRegisters() {
    sendPacket("");
}

void TcpClientGdb::handleSetThread() {
    /* Set the thread number of subsequent operations. For now ignore
       silently and just reply "OK" */
    sendPacket("OK");
}

void TcpClientGdb::handleKill() {
    sendPacket("OK");
}

void TcpClientGdb::handleGetMemory(const char *data) {
    /* Handle a RSP read memory (symbolic) request
     * Syntax is: m<addr>,<length>:
     * The response is the bytes.
     * Lowest address first, encoded as pairs of hex digits.
     * The length given is the number of bytes to be read.
     */
    unsigned long address;
    int len;
    if (RISCV_sscanf(data, "m%lx,%x:", &address, &len) != 2) {
        RISCV_info("Failed to recognize RSP read memory command: %s",
                    data);
        sendPacket("E01");
        return;
    }

    AttributeType res;
    char tstr[256];
    RISCV_sprintf(tstr, sizeof(tstr), "read 0x%lx %d", address, len);

    if (iexec_) {
        iexec_->exec(tstr, &res, false);
    }

    uint8_t *bts = res.data();
    tstr[0] = 0;
    for (size_t offset = 0; offset < res.size(); offset++) {
        char byte_hex[3];
        RISCV_sprintf(byte_hex, sizeof(byte_hex), "%02x", bts[offset]);
        strncat(tstr, byte_hex, 2);
    }

    sendPacket(tstr);
}

void TcpClientGdb::handleWriteMemoryHex() {
    sendPacket("");
}

void TcpClientGdb::handleReadRegister(const char *data) {
    unsigned int regnum;

    if (RISCV_sscanf(data, "p%x", &regnum) != 1) {
        RISCV_info("Failed to recognize RSP read register "
                   "command: %s", data);
        sendPacket("E01");
        return;
    }

    AttributeType res;
    if (iexec_) {
        iexec_->exec("reg", &res, false);
    }

    char resp[256] = "\0";
    appendRegValue(resp, res.dict_value(regnum)->to_uint32());
    sendPacket(resp);
}

void TcpClientGdb::handleWriteRegister(const char *data) {
    unsigned       regnum;            /* Register index */
    char           byte0[3];  /* Data to set */
    char           byte1[3];
    char           byte2[3];
    char           byte3[3];

    if (RISCV_sscanf(data, "P%x=%2s%2s%2s%2s",
               &regnum, byte0, byte1, byte2, byte3) != 5) {
        RISCV_info("Failed to recognize RSP write register "
                   "command: %s", data);
        sendPacket("E01");
        return;
    }

    AttributeType res;
    char tstr[256];

    /** Hardcode for ARM only */
    char regname[10] = "\0";
    if (regnum < 12) {
        RISCV_sprintf(regname, sizeof(regname), "r%d", regnum);
    } else if (regnum == 12) {
        RISCV_sprintf(regname, sizeof(regname), "fp");
    } else if (regnum == 13) {
        RISCV_sprintf(regname, sizeof(regname), "sp");
    } else if (regnum == 14) {
        RISCV_sprintf(regname, sizeof(regname), "lr");
    } else if (regnum == 15) {
        RISCV_sprintf(regname, sizeof(regname), "pc");
    } else if (regnum == 26) {
        RISCV_sprintf(regname, sizeof(regname), "cpsr");
    } else {
        RISCV_info("Failed to recognize register: %d", regnum);
        sendPacket("E01");
        return;
    }

    RISCV_sprintf(tstr, sizeof(tstr), "reg %s 0x%s%s%s%s",
                  regname, byte3, byte2, byte1, byte0);
    RISCV_info("command: %s", tstr);

    if (iexec_) {
        iexec_->exec(tstr, &res, false);
    }
    sendPacket("OK");
}

void TcpClientGdb::handleGeneralSet(const char *data) {
    if (strncmp("QStartNoAckMode",
        data, strlen("QStartNoAckMode")) == 0) {
        //estate_ = State_WaitAckToSwitch;
    }
    sendPacket("OK");
}

void TcpClientGdb::handleStep() {
    sendPacket("");
}

void TcpClientGdb::handleThreadAlive() {
    sendPacket("OK");
}

void TcpClientGdb::handleVCommand(const char *data) {
    if (strcmp("vMustReplyEmpty", data) == 0) {
        sendPacket("");
    } else if (strncmp(data, "vCont", 5) == 0) {
        const char *packet_ptr = data;
        packet_ptr += 5;
        if (*packet_ptr == '?') {
            sendPacket("vCont;c;C");
            return;
        }
        if (*packet_ptr == ';') {
            packet_ptr++;
        }
        if (*packet_ptr == 'c') {
            RISCV_info("Continue packet: %s", data);
            handleContinue();
        }
        if (*packet_ptr == 's') {
            AttributeType res;
            RISCV_info("Step packet: %s", data);
            iexec_->exec("c 1", &res, false);
            sendPacket("S05");
        }
    }
}

void TcpClientGdb::handleWriteMemory(const char *data) {
    size_t         len;             /* Number of bytes to write */
    unsigned long  address;         /* Where to write the memory */
    const char     *data_ptr;       /* Pointer to the binary data */

    if (RISCV_sscanf(data, "X%lx,%lx:", &address, &len) != 2) {
        RISCV_info("Failed to recognize RSP write memory %s",
                   data);
        sendPacket("E01");
        return;
    }

    data_ptr = static_cast<const char *>(memchr(data, ':', DATA_MAX));
    data_ptr++;
    char data_hex[2 * DATA_MAX + 1];
    data_hex[0] = '\0';

    for (size_t offset = 0; offset < len; offset++) {
        char byte_hex[3];
        RISCV_sprintf(byte_hex, sizeof(byte_hex), "%02x", data_ptr[offset]);
        strncat(data_hex, byte_hex, 2);
    }

    AttributeType res;
    // 32 should be enough for 'write 0x000000 9999 '
    char tstr[DATA_MAX * 2 + 32];
    RISCV_sprintf(tstr, sizeof(tstr), "write 0x%lx %lu 0x%s",
                  address, len, data_hex);

    if (iexec_) {
        iexec_->exec(tstr, &res, false);
    }

    sendPacket("OK");
}

void TcpClientGdb::handleBreakpoint(const char *data) {
    int type;
    uint64_t address;  /* Address specified */
    int len;
    char zZ;       /* 'Z' : add breakpoint, 'z' : remove breakopint. */

    if (RISCV_sscanf(data, "%c%1d,%lx,%1d",
                &zZ, &type, &address, &len) != 4) {
        RISCV_info("Failed to recognize RSP add breakpoint: %s", data);
        sendPacket("E01");
        return;
    }

    /* Sanity check that the length is 4 */
    if (len != 4) {
        RISCV_info("Warning: length is not 4, but %d", len);
        len = 4;
    }

    /* Sort out the type of breakpoint, currently only memory is supported */
    AttributeType addr, res;
    addr.make_uint64(address);
    if (type == 0) {
        /* Memory breakpoint */
        if (zZ == 'Z') {
#if 0
            br_add(addr, &res);
#endif
        } else {
#if 0
            br_rm(addr, &res);
#endif
        }
        sendPacket("OK");
    } else {
        RISCV_info("Failed to recognize RSP breakpoint type: %d", type);
        sendPacket("E01");
    }
}

void TcpClientGdb::sendPacket(const char *data) {
    int tsz = static_cast<int>(strlen(data));
    if (enableAckMode_) {
        writeTxBuffer("+", 1);
    }
    writeTxBuffer("$", 1);
    writeTxBuffer(data, tsz);

    // Add checksum
    writeTxBuffer("#", 1);
    uint8_t crc8 = checksum(data, tsz);
    char tstr[3];
    RISCV_sprintf(tstr, sizeof(tstr), "%02x", crc8);
    writeTxBuffer(tstr, 2);
}

uint8_t TcpClientGdb::checksum(const char *data, const int sz) {
    uint8_t sum = 0;
    for (int i = 0; i < sz; i++) {
        sum += static_cast<uint8_t>(data[i]);
    }
    return sum;
}

}  // namespace debugger
