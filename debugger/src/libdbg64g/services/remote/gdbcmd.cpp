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

#include "gdbcmd.h"

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

GdbCommands::GdbCommands(IService *parent) : TcpCommandsGen(parent) {
    estate_ = State_AckMode;
}

int GdbCommands::processCommand(const char *cmdbuf, int bufsz) {
    if (cmdbuf[0] != '$') {
        RISCV_info("wrong packet preambule: %02x", cmdbuf[0]);
        return bufsz;
    }

    if (bufsz < 3 || cmdbuf[bufsz - 3] != '#') {
        RISCV_info("wrong checksum format: sz=%d; %02x",
                    bufsz, cmdbuf[bufsz - 3]);
        return bufsz;
    }

    char comp_sum[3] = {0};
    RISCV_sprintf(comp_sum, 3, "%02x", checksum(&cmdbuf[1], bufsz-4));

    if (comp_sum[0] != cmdbuf[bufsz-2] || comp_sum[1] != cmdbuf[bufsz-1]) {
        RISCV_info("wrong checksum: %02x", comp_sum);
        return bufsz;
    }

    // Remove '$' start symbol and CRC at the end
    memcpy(&packet_data_, &cmdbuf[1], bufsz - 4);
    packet_data_[bufsz - 4] = '\0';

    handlePacket(packet_data_);
    return bufsz;
}

void GdbCommands::handleHandshake(char s) {
    switch (estate_) {
    case State_AckMode:
        break;
    case State_WaitAckToSwitch:
        if (s == '+') {
            estate_ = State_NoAckMode;
        }
        break;
    case State_NoAckMode:
        break;
    default:;
    }
}

void GdbCommands::handlePacket(char *data) {
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
        handleGetMemory();
        break;
    case 'M' :  // Write memory (hex).
        handleWriteMemoryHex();
        break;
    case 'p' :  // Read register.
        handleReadRegister();
        break;
    case 'P' :  // Write register.
        handleWriteRegister();
        break;
    case 'q' :  // General query.
        handleQuery();
        break;
    case 'Q' :  // General set.
        handleGeneralSet();
        break;
    case 's' :  // Single step.
    case 'S' :  // Step with signal.
        handleStep();
        break;
    case 'T' :  // Thread liveness query.
        handleThreadAlive();
        break;
    case 'v' :  // v command.
        handleVCommand();
        break;
    case 'X' :  // Write memory (binary).
        handleWriteMemory();
        break;
    case 'z' :  // Remove breakpoint/watchpoint.
    case 'Z' :  // Insert breakpoint/watchpoint.
        handleBreakpoint();
        break;
    default:
        RISCV_info("Unknown RSP packet: %s", data);
    }
}

void GdbCommands::handleQuery() {
    if (strcmp("qAttached", packet_data_) == 0) {
        /* Always attaching to an existing process, let thread id to be 1 */
        sendPacket("1");
    } else if (strcmp("qC", packet_data_) == 0) {
        /* Return the current thread ID.
         * Reply QCpid - Where pid is a HEX encoded 16 bit process id.
         * Set thread id 1, because we don't have other threads. */
        sendPacket("QC1");
    } else if (strncmp("qCRC", packet_data_, strlen("qCRC")) == 0) {
        /* Return CRC of memory area.
         * Return Error 01 */
        sendPacket("E01");
    } else if (strcmp("qfThreadInfo", packet_data_) == 0) {
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
    } else if (strcmp("qsThreadInfo", packet_data_) == 0) {
        /* Return info about more active threads.
         * We have no more, so return the end of list marker, 'l' */
        sendPacket("l");
    } else if (strncmp("qGetTLSAddr:", 
                       packet_data_, strlen("qGetTLSAddr:")) == 0) {
        /* We don't support this feature */
        sendPacket("");
    } else if (strncmp("qL", packet_data_, strlen("qL")) == 0) {
        /* Deprecated and replaced by 'qfThreadInfo' */
        sendPacket("qM001");
    } else if (strcmp("qOffsets", packet_data_) == 0) {
        /* Report any relocation */
        sendPacket("Text=0;Data=0;Bss=0");
    } else if (strncmp("qP", packet_data_, strlen("qP")) == 0) {
        /* Deprecated and replaced by 'qThreadExtraInfo' */
        sendPacket("");
    } else if (strncmp("qRcmd,", packet_data_, strlen("qRcmd,")) == 0) {
        /* This is used to interface to commands to do "stuff" */
        sendPacket("");
    } else if (strncmp("qSupported", 
                        packet_data_, strlen("qSupported")) == 0) {
        /* Report a list of the features we support.
         * 1000h == 4096
         * 500h  == 1280 */
        sendPacket("PacketSize=500;QStartNoAckMode+;vContSupported+");
        //QNonStop+
    } else if (strncmp("qSymbol:", packet_data_, strlen("qSymbol:")) == 0) {
        /* Offer to look up symbols. Ignore for now */
        sendPacket("OK");
    } else if (strncmp("qThreadExtraInfo,",
                       packet_data_, strlen("qThreadExtraInfo,")) == 0) {
        /* Report that we are runnable */
        char tstr[128] = "\0";
        RISCV_sprintf (tstr, sizeof(tstr), "%02x%02x%02x%02x%02x%02x%02x%02x",
             'R', 'u', 'n', 'n', 'a', 'b', 'l', 'e');
        sendPacket(tstr);
    } else if (strncmp("qTStatus", packet_data_, strlen("qTStatus")) == 0) {
        /* Don't support tracing, return empty packet. */
        sendPacket("");
    } else if (strncmp("qXfer:", packet_data_, strlen("qXfer:")) == 0) {
        /* No 'qXfer' requests supported, return empty packet. */
        sendPacket("");
    } else {
        RISCV_error("Unrecognized RSP query: %s \n", packet_data_);
        sendPacket("");
    }
}

void GdbCommands::handleStopReasonQuery() {
    sendPacket("S05");
}

void GdbCommands::handleContinue() {
    AttributeType res;
    if (!iexec_) {
        sendPacket("E01");
        return;
    }
    RISCV_event_clear(&eventHalt_);
    iexec_->exec("run", &res, false);
    RISCV_event_wait(&eventHalt_);
    sendPacket("S05");
}

void GdbCommands::handleDetach() {
    /* Reply "OK" to GDB and close socket. */
    sendPacket("OK");
}

void GdbCommands::appendRegValue(char *s, uint32_t value) {
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

void GdbCommands::handleGetRegisters() {
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

void GdbCommands::handleSetRegisters() {
    sendPacket("");
}

void GdbCommands::handleSetThread() {
    /* Set the thread number of subsequent operations. For now ignore
       silently and just reply "OK" */
    sendPacket("OK");
}

void GdbCommands::handleKill() {
    sendPacket("OK");
}

void GdbCommands::handleGetMemory() {
    /* Handle a RSP read memory (symbolic) request
     * Syntax is: m<addr>,<length>:
     * The response is the bytes.
     * Lowest address first, encoded as pairs of hex digits.
     * The length given is the number of bytes to be read.
     */
    unsigned long address;
    int len;
    if (RISCV_sscanf(packet_data_, "m%lx,%x:", &address, &len) != 2) {
        RISCV_info("Failed to recognize RSP read memory command: %s",
                    packet_data_);
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

void GdbCommands::handleWriteMemoryHex() {
    sendPacket("");
}

void GdbCommands::handleReadRegister() {
    unsigned int regnum;

    if (RISCV_sscanf(packet_data_, "p%x", &regnum) != 1) {
        RISCV_info("Failed to recognize RSP read register "
                   "command: %s", packet_data_);
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

void GdbCommands::handleWriteRegister() {
    unsigned       regnum;            /* Register index */
    char           byte0[3];  /* Data to set */
    char           byte1[3];
    char           byte2[3];
    char           byte3[3];

    if (RISCV_sscanf(packet_data_, "P%x=%2s%2s%2s%2s",
               &regnum, byte0, byte1, byte2, byte3) != 5) {
        RISCV_info("Failed to recognize RSP write register "
                   "command: %s", packet_data_);
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

void GdbCommands::handleGeneralSet() {
    if (strncmp("QStartNoAckMode",
        packet_data_, strlen("QStartNoAckMode")) == 0) {
        estate_ = State_WaitAckToSwitch;
    }
    sendPacket("OK");
}

void GdbCommands::handleStep() {
    sendPacket("");
}

void GdbCommands::handleThreadAlive() {
    sendPacket("OK");
}

void GdbCommands::handleVCommand() {
    if (strcmp("vMustReplyEmpty", packet_data_) == 0) {
        sendPacket("");
    } else if (strncmp(packet_data_, "vCont", 5) == 0) {
        const char *packet_ptr = packet_data_;
        packet_ptr += 5;
        if (*packet_ptr == '?') {
            sendPacket("vCont;c;C");
            return;
        }
        if (*packet_ptr == ';') {
            packet_ptr++;
        }
        if (*packet_ptr == 'c') {
            RISCV_info("Continue packet: %s", packet_data_);
            handleContinue();
        }
        if (*packet_ptr == 's') {
            AttributeType res;
            RISCV_info("Step packet: %s", packet_data_);
            iexec_->exec("c 1", &res, false);
            sendPacket("S05");
        }
    }
}

void GdbCommands::handleWriteMemory() {
    size_t         len;             /* Number of bytes to write */
    unsigned long  address;         /* Where to write the memory */
    const char     *data_ptr;       /* Pointer to the binary data */

    if (RISCV_sscanf(packet_data_, "X%lx,%lx:", &address, &len) != 2) {
        RISCV_info("Failed to recognize RSP write memory %s",
                   packet_data_);
        sendPacket("E01");
        return;
    }

    data_ptr = static_cast<char*>(memchr(packet_data_, ':', DATA_MAX));
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

void GdbCommands::handleBreakpoint() {
    int type;
    uint64_t address;  /* Address specified */
    int len;
    char zZ;       /* 'Z' : add breakpoint, 'z' : remove breakopint. */

    if (RISCV_sscanf(packet_data_, "%c%1d,%lx,%1d",
                &zZ, &type, &address, &len) != 4) {
        RISCV_info("Failed to recognize RSP add breakpoint: %s", packet_data_);
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
            br_add(addr, &res);
        } else {
            br_rm(addr, &res);
        }
        sendPacket("OK");
    } else {
        RISCV_info("Failed to recognize RSP breakpoint type: %d", type);
        sendPacket("E01");
    }
}

void GdbCommands::sendPacket(const char *data) {
    int tsz = static_cast<int>(strlen(data));
    respcnt_ = 0;
    if (estate_ != State_NoAckMode) {
        respbuf_[respcnt_++] = '+';
    }
    respbuf_[respcnt_++] = '$';
    memcpy(&respbuf_[respcnt_], data, tsz);
    respcnt_ += tsz;

    // Add checksum
    respbuf_[respcnt_++] = '#';
    RISCV_sprintf(&respbuf_[respcnt_], resptotal_ - respcnt_, "%02x",
                checksum(data, tsz));
    respcnt_ += 2;
    respbuf_[respcnt_] = '\0';
}

uint8_t GdbCommands::checksum(const char *data, const int sz) {
    uint8_t sum = 0;
    for (int i = 0; i < sz; i++) {
        sum += static_cast<uint8_t>(data[i]);
    }
    return sum;
}

}  // namespace debugger
