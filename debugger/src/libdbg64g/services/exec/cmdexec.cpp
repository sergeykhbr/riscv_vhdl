/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Command Executor implementation.
 */

#include <string.h>
#include "cmdexec.h"
#include "coreservices/icommand.h"
#include "cmd/cmd_regs.h"
#include "cmd/cmd_reg.h"
#include "cmd/cmd_loadelf.h"
#include "cmd/cmd_log.h"
#include "cmd/cmd_isrunning.h"

namespace debugger {

/** Class registration in the Core */
REGISTER_CLASS(CmdExecutor)

CmdExecutor::CmdExecutor(const char *name) 
    : IService(name) {
    registerInterface(static_cast<ICmdExecutor *>(this));
    registerAttribute("Tap", &tap_);
    registerAttribute("SocInfo", &socInfo_);

    //console_.make_list(0);
    tap_.make_string("");
    socInfo_.make_string("");
    listeners_.make_list(0);

    RISCV_mutex_init(&mutexExec_);

    tmpbuf_ = new uint8_t[tmpbuf_size_ = 4096];
    outbuf_ = new char[outbuf_size_ = 4096];
}

CmdExecutor::~CmdExecutor() {
    RISCV_mutex_destroy(&mutexExec_);
    delete [] tmpbuf_;
    delete [] outbuf_;
    for (unsigned i = 0; i < cmds_.size(); i++) {
        delete cmds_.dict_value(i)->to_iface();
    }
}

void CmdExecutor::postinitService() {
    itap_ = static_cast<ITap *>
            (RISCV_get_service_iface(tap_.to_string(), IFACE_TAP));
    info_ = static_cast<ISocInfo *>
            (RISCV_get_service_iface(socInfo_.to_string(), 
                                    IFACE_SOC_INFO));

    cmds_.make_dict();

    ICommand *t1 = new CmdRegs(itap_, info_);
    cmds_["regs"].make_iface(t1);
    cmds_["reg"].make_iface(new CmdReg(itap_, info_));
    cmds_["loadelf"].make_iface(new CmdLoadElf(itap_, info_));
    cmds_["log"].make_iface(new CmdLog(itap_, info_));
    cmds_["isrunning"].make_iface(new CmdIsRunning(itap_, info_));
}

bool CmdExecutor::exec(const char *line, AttributeType *res, bool silent) {
    RISCV_mutex_lock(&mutexExec_);
    res->make_nil();

    AttributeType cmd;
    if (line[0] == '[' || line[0] == '}') {
        cmd.from_config(line);
    } else {
        cmd.make_string(line);
    }

    if (cmd.is_string()) {
        /** Process simple string command */
        processSimple(&cmd, res);
    } else {
        /** Process scripted in JSON format command */
    }

    RISCV_mutex_unlock(&mutexExec_);

    /** Do not output any information into console in silent mode: */
    if (silent) {
        return false;
    }
    IRawListener *iraw;
    for (unsigned i = 0; i < listeners_.size(); i++) {
        iraw = static_cast<IRawListener *>(listeners_[i].to_iface());
        iraw->updateData(outbuf_, outbuf_cnt_);
    }
    return (outbuf_cnt_ != 0);
    
}

void CmdExecutor::registerRawListener(IFace *iface) {
    AttributeType t1(iface);
    listeners_.add_to_list(&t1);
}

void CmdExecutor::processSimple(AttributeType *cmd, AttributeType *res) {
    outbuf_[outbuf_cnt_ = 0] = '\0';
    if (cmd->size() == 0) {
        return;
    }

    AttributeType listArgs(Attr_List);
    splitLine(const_cast<char *>(cmd->to_string()), &listArgs);
    if (!listArgs[0u].is_string()) {
        outf("Wrong command format\n");
        return;
    }

    if (listArgs[0u].is_equal("help")) {
        if (listArgs.size() == 1) {
            outf("** List of supported commands: **\n");
            for (unsigned i = 0; i < cmds_.size(); i++) {
                ICommand *cmd = static_cast<ICommand *>(cmds_[i].to_iface());
                outf("%13s   - %s\n", cmd->cmdName(), cmd->briefDescr());
            }
            outf("\n");
        } else {
            // @todo specific command
        }
        return;
        /*
        outf("      memdump   - Dump memory to file\n");
        outf("      csr       - Access to CSR registers\n");
        outf("      write     - Write memory\n");
        outf("      read      - Read memory\n");
        outf("** List of supported commands by simulator only: **\n");
        outf("      stop/break/s    - Stop simulation\n");
        outf("      run/go/c  - Run simulation for a specify "
                               "number of steps\n");
        outf("      regs      - List of registers values\n");
        outf("      br        - Breakpoint operation\n");
        outf("\n");*/
    }

    AttributeType u;
    bool cmdFound = false;
    for (unsigned i = 0; i < cmds_.size(); i++) {
        ICommand *e = 
            static_cast<ICommand *>(cmds_.dict_value(i)->to_iface());
        if (e->isValid(&listArgs) == CMD_INVALID) {
            continue;
        }

        cmdFound = true;
        if (e->exec(&listArgs, res) == CMD_FAILED) {
            outf(e->detailedDescr());
            break;
        } 
        /** Command was successful: */
        if (e->format(res, &u) == CMD_IS_OUTPUT) {
            outf(u.to_string());
        }
        if (listArgs[0u].is_equal("log")) {
            //@todo log enabling
        } else if (listArgs[0u].is_equal("exit")) {
            RISCV_break_simulation();
        }
        break;

    }
    if (!cmdFound) {
        outf("Use 'help' to print list of the supported commands\n");
    }

#if 0
    
    if (strcmp(listArgs[0u].to_string(), "memdump") == 0) {
        if (listArgs.size() == 4 || listArgs.size() == 5) {
            memDump(&listArgs);
        } else {
            outf("Description:\n");
            outf("    Dump memory to file (default in Binary format).\n");
            outf("Usage:\n");
            outf("    memdump <addr> <bytes> [filepath] [bin|hex]\n");
            outf("Example:\n");
            outf("    memdump 0x0 8192 dump.bin\n");
            outf("    memdump 0x40000000 524288 dump.hex hex\n");
            outf("    memdump 0x10000000 128 \"c:/My Documents/dump.bin\"\n");
        }
    } else if (listArgs[0u].is_equal("regs") || listArgs[0u].is_equal("reg")) {
        AttributeType u;
        ICommand *e = static_cast<ICommand *>(cmds_["regs"].to_iface());
        if (!e->exec(&listArgs, res)) {
            outf(e->detailedDescr());
        } else if (e->format(res, &u)) {
            outf(u.to_string());
        }
    } else if (strcmp(listArgs[0u].to_string(), "csr") == 0) {
        if (listArgs.size() == 2) {
            readCSR(&listArgs);
        } else if (listArgs.size() == 3) {
            writeCSR(&listArgs);
        } else {
            outf("Description:\n");
            outf("    Access to CSRs registers of the CPU.\n");
            outf("Usage:\n");
            outf("    READ:  csr <addr|name>\n");
            outf("    WRITE: csr <addr|name> <value>\n");
            outf("Example:\n");
            outf("    csr MCPUID\n");
            outf("    csr 0x762 1\n");
        }
    } else if (strcmp(listArgs[0u].to_string(), "write") == 0) {
        if (listArgs.size() == 4) {
            writeMem(&listArgs);
        } else {
            outf("Description:\n");
            outf("    Write memory.\n");
            outf("Usage:\n");
            outf("    write <addr> <bytes> [value]\n");
            outf("Example:\n");
            outf("    write 0xfffff004 4 0x20160323\n");
            outf("    write 0x10040000 16 "
                        "[0xaabbccdd00112233, 0xaabbccdd00112233]\n");
        }
    } else if (strcmp(listArgs[0u].to_string(), "read") == 0) {
        if (listArgs.size() == 2) {
            AttributeType item(Attr_Integer, 4);
            listArgs.add_to_list(&item);
            readMem(&listArgs);
        } else if (listArgs.size() == 3) {
            readMem(&listArgs);
        } else {
            outf("Description:\n");
            outf("    32-bits aligned memory reading. "
                        "Default bytes = 4 bytes.\n");
            outf("Usage:\n");
            outf("    read <addr> <bytes>\n");
            outf("Example:\n");
            outf("    read 0xfffff004 16\n");
            outf("    read 0xfffff004\n");
        }
    } else if (strcmp(listArgs[0u].to_string(), "exit") == 0) {
        RISCV_break_simulation();
    } else if (strcmp(listArgs[0u].to_string(), "halt") == 0
            || strcmp(listArgs[0u].to_string(), "stop") == 0
            || strcmp(listArgs[0u].to_string(), "s") == 0) {
        if (listArgs.size() == 1) {
            halt(&listArgs);
        } else {
            outf("Description:\n");
            outf("    Stop simulation.\n");
            outf("Example:\n");
            outf("    halt\n");
            outf("    stop\n");
            outf("    s\n");
        }
    } else if (strcmp(listArgs[0u].to_string(), "run") == 0
            || strcmp(listArgs[0u].to_string(), "go") == 0
            || strcmp(listArgs[0u].to_string(), "c") == 0) {
        if (listArgs.size() == 1 
            || (listArgs.size() == 2 && listArgs[1].is_integer())) {
            run(&listArgs);
        } else {
            outf("Description:\n");
            outf("    Run simulation for a specified number of steps.\n");
            outf("Usage:\n");
            outf("    run <N steps>\n");
            outf("Example:\n");
            outf("    run\n");
            outf("    go 1000\n");
            outf("    c 1\n");
        }
    } else if (strcmp(listArgs[0u].to_string(), "br") == 0) {
        if (listArgs.size() == 3 && listArgs[1].is_string()) {
            br(&listArgs);
        } else {
            outf("Description:\n");
            outf("    Add or remove memory breakpoint.\n");
            outf("Usage:\n");
            outf("    br add <addr>\n");
            outf("    br rm <addr>\n");
            outf("Example:\n");
            outf("    br add 0x10000000\n");
            outf("    br rm 0x10000000\n");
        }
    } else {
        outf("Use 'help' to print list of the supported commands\n");
    }
#endif
}

void CmdExecutor::splitLine(char *str, AttributeType *listArgs) {
    char *end = str;
    bool last = false;
    bool inside_string = false;
    char string_marker;
    while (*str) {
        if (*end == '\0') {
            last = true;
        } else if (*end == ' ' && !inside_string) {
            *end = '\0';
            while (*(end + 1) == ' ') {
                *(++end) = '\0';
            }
        } else  if (!inside_string && (*end == '"' || *end == '\'')) {
            inside_string = true;
            string_marker = *end;
        } else if (inside_string && string_marker == *end) {
            inside_string = false;
        }

        if (*end == '\0') {
            AttributeType item;
            if ((str[0] >= '0' && str[0] <= '9')
             || (str[0] == '[') || (str[0] == '"') || (str[0] == '\'')) {
                item.from_config(str);
            } else {
                item.make_string(str);
            }
            listArgs->add_to_list(&item);
            if (!last) {
                splitLine(end + 1, listArgs);
            }
            break;
        }
        ++end;
    }
}

void CmdExecutor::readCSR(AttributeType *listArgs) {
    uint64_t csr = 0;
    uint64_t addr = info_->csr2addr((*listArgs)[1].to_string());

    if (addr == static_cast<uint64_t>(-1)) {
        outf("Unknown CSR '%s'\n", (*listArgs)[1].to_string());
        return;
    }

    itap_->read(addr, 8, reinterpret_cast<uint8_t *>(&csr));
    outf("CSR[%03x] => %016" RV_PRI64 "x\n", 
        static_cast<uint32_t>((addr >> 4) & 0xfff), csr);
   
    if (!(*listArgs)[1].is_string()) {
        return;
    }
    if (strcmp((*listArgs)[1].to_string(), "MCPUID") == 0) {
        static const char *MCPUID_BASE[4] = {
            "RV32I", "RV32E", "RV64I", "RV128I"
        };
        outf("    Base: %s", MCPUID_BASE[(csr >> 62) & 0x3]);
        // total 26 extensions
        char extenstion[2] = {0};
        for (int i = 0; i < 26; i++) {
            if (csr & (1LL << i)) {
                extenstion[0] = 'A' + i;
                if (extenstion[0] == 'I') {
                    continue;
                }
                outf("%s", extenstion);
            }
        }
        outf("\n");
    } else if (strcmp((*listArgs)[1].to_string(), "MTIME") == 0) {
        outf("    Clock: %" RV_PRI64 "d; ms: %.1f\n", csr, 
                            static_cast<double>(csr)/60000.0);
    }
}

void CmdExecutor::writeCSR(AttributeType *listArgs) {
    uint64_t addr;
    uint64_t csr;
    addr = info_->csr2addr((*listArgs)[1].to_string());
    if (addr == static_cast<uint64_t>(-1)) {
        outf("Unknown CSR '%s'\n", (*listArgs)[1].to_string());
        return;
    }
        
    csr = (*listArgs)[2].to_uint64();
    itap_->write(addr, 8, reinterpret_cast<uint8_t *>(&csr));
}

void CmdExecutor::readMem(AttributeType *listArgs) {
    uint64_t addr_start, addr_end, inv_i;
    uint64_t addr = (*listArgs)[1].to_uint64();
    int bytes = static_cast<int>((*listArgs)[2].to_uint64());
    if (bytes > tmpbuf_size_) {
        delete [] tmpbuf_;
        tmpbuf_size_ = bytes;
        tmpbuf_ = new uint8_t[tmpbuf_size_ + 1];
    }
    addr_start = addr & ~0xFll;
    addr_end = (addr + bytes + 15 )& ~0xFll;

    itap_->read(addr, bytes, tmpbuf_);
    for (uint64_t i = addr_start; i < addr_end; i++) {
        if ((i & 0xf) == 0) {
            outf("[%016" RV_PRI64 "x]: ", i);
        }
        inv_i = (i & ~0xFll) | (0xfll - (i & 0xfll));
        if ((addr <= inv_i) && (inv_i < (addr + bytes))) {
            outf(" %02x", tmpbuf_[inv_i - addr]);
        } else {
            outf(" ..");
        }
        if ((i & 0xf) == 0xf) {
            outf("\n");
        }
    }
}

void CmdExecutor::writeMem(AttributeType *listArgs) {
    uint64_t addr = (*listArgs)[1].to_uint64();
    uint64_t val = (*listArgs)[3].to_uint64();
    int bytes = static_cast<int>((*listArgs)[2].to_uint64());

    if ((*listArgs)[3].is_integer()) {
        reinterpret_cast<uint64_t *>(tmpbuf_)[0] = val;
    } else if ((*listArgs)[3].is_list()) {
        for (unsigned i = 0; i < (*listArgs)[3].size(); i++) {
            val = (*listArgs)[3][i].to_uint64();
            reinterpret_cast<uint64_t *>(tmpbuf_)[i] = val;
        }
    } else {
        outf("Wrong write format\n");
        return;
    }
    itap_->write(addr, bytes, tmpbuf_);
}

void CmdExecutor::memDump(AttributeType *listArgs) {
    const char *filename = (*listArgs)[3].to_string();
    FILE *fd = fopen(filename, "w");
    if (fd == NULL) {
        outf("Can't open '%s' file", filename);
        return;
    }
    uint64_t addr = (*listArgs)[1].to_uint64();
    int len = static_cast<int>((*listArgs)[2].to_uint64());
    uint8_t *dumpbuf = tmpbuf_;
    if (len > tmpbuf_size_) {
        dumpbuf = new uint8_t[len];
    }

    itap_->read(addr, len, dumpbuf);

    if (listArgs->size() == 5
        && strcmp((*listArgs)[4].to_string(), "hex") == 0) {
        char t1[256];
        int t1_cnt = 0;
        for (int i = 0; i < len; i++) {
            t1_cnt += sprintf(&t1[t1_cnt], "%02x", 
                dumpbuf[(i & ~0xf) | (0xf - (i & 0xf))]);
            if ((i & 0xf) == 0xf) {
                t1_cnt += sprintf(&t1[t1_cnt], "\n");
                fwrite(t1, t1_cnt, 1, fd);
                t1_cnt = 0;
            }
        }
        if (len & 0xf) {
            fwrite(t1, t1_cnt, 1, fd);
        }
    } else {
        fwrite(dumpbuf, len, 1, fd);
    }
    fclose(fd);
    if (dumpbuf != tmpbuf_) {
        delete [] dumpbuf;
    }
}

void CmdExecutor::halt(AttributeType *listArgs) {
    uint8_t val[sizeof(uint64_t)];
    *(reinterpret_cast<uint64_t *>(val)) = info_->valueHalt();
    itap_->write(info_->addressRunControl(), 8, val);
}

void CmdExecutor::run(AttributeType *listArgs) {
    uint8_t val[sizeof(uint64_t)];
    *(reinterpret_cast<uint64_t *>(val)) = info_->valueRun();
    if (listArgs->size() == 1) {
        itap_->write(info_->addressRunControl(), 8, val);
    } else if (listArgs->size() == 2) {
        *(reinterpret_cast<uint64_t *>(val)) = (*listArgs)[1].to_uint64();
        itap_->write(info_->addressStepCounter(), 8, val);

        *(reinterpret_cast<uint64_t *>(val)) = info_->valueRunStepping();
        itap_->write(info_->addressRunControl(), 8, val);
    }
}

void CmdExecutor::regs(AttributeType *listArgs, AttributeType *res) {
    AttributeType reglst;
    if (listArgs->size() != 1) {
        return;
    }
    uint64_t val;
    info_->getRegsList(&reglst);
    res->make_list(reglst.size());
    for (unsigned i = 0; i < reglst.size(); i++) {
        itap_->read(info_->reg2addr(reglst[i].to_string()), 
                                    8, reinterpret_cast<uint8_t *>(&val));
        (*res)[i].make_uint64(val);
    }

    itap_->read(info_->reg2addr("ra"), 8, reinterpret_cast<uint8_t *>(&val));
    outf("ra: %016" RV_PRI64 "x    \n", val);

    outf("                        ", NULL);
    itap_->read(info_->reg2addr("s0"), 8, reinterpret_cast<uint8_t *>(&val));
    outf("s0:  %016" RV_PRI64 "x   ", val);
    itap_->read(info_->reg2addr("a0"), 8, reinterpret_cast<uint8_t *>(&val));
    outf("a0:  %016" RV_PRI64 "x   \n", val);

    itap_->read(info_->reg2addr("sp"), 8, reinterpret_cast<uint8_t *>(&val));
    outf("sp: %016" RV_PRI64 "x    ", val);
    itap_->read(info_->reg2addr("s1"), 8, reinterpret_cast<uint8_t *>(&val));
    outf("s1:  %016" RV_PRI64 "x   ", val);
    itap_->read(info_->reg2addr("a1"), 8, reinterpret_cast<uint8_t *>(&val));
    outf("a1:  %016" RV_PRI64 "x   \n", val);

    itap_->read(info_->reg2addr("gp"), 8, reinterpret_cast<uint8_t *>(&val));
    outf("gp: %016" RV_PRI64 "x    ", val);
    itap_->read(info_->reg2addr("s2"), 8, reinterpret_cast<uint8_t *>(&val));
    outf("s2:  %016" RV_PRI64 "x   ", val);
    itap_->read(info_->reg2addr("a2"), 8, reinterpret_cast<uint8_t *>(&val));
    outf("a2:  %016" RV_PRI64 "x   \n", val);

    itap_->read(info_->reg2addr("tp"), 8, reinterpret_cast<uint8_t *>(&val));
    outf("tp: %016" RV_PRI64 "x    ", val);
    itap_->read(info_->reg2addr("s3"), 8, reinterpret_cast<uint8_t *>(&val));
    outf("s3:  %016" RV_PRI64 "x   ", val);
    itap_->read(info_->reg2addr("a3"), 8, reinterpret_cast<uint8_t *>(&val));
    outf("a3:  %016" RV_PRI64 "x   \n", val);

    outf("                        ", NULL);
    itap_->read(info_->reg2addr("s4"), 8, reinterpret_cast<uint8_t *>(&val));
    outf("s4:  %016" RV_PRI64 "x   ", val);
    itap_->read(info_->reg2addr("a4"), 8, reinterpret_cast<uint8_t *>(&val));
    outf("a4:  %016" RV_PRI64 "x   \n", val);

    itap_->read(info_->reg2addr("t0"), 8, reinterpret_cast<uint8_t *>(&val));
    outf("t0: %016" RV_PRI64 "x    ", val);
    itap_->read(info_->reg2addr("s5"), 8, reinterpret_cast<uint8_t *>(&val));
    outf("s5:  %016" RV_PRI64 "x   ", val);
    itap_->read(info_->reg2addr("a5"), 8, reinterpret_cast<uint8_t *>(&val));
    outf("a5:  %016" RV_PRI64 "x   \n", val);

    itap_->read(info_->reg2addr("t1"), 8, reinterpret_cast<uint8_t *>(&val));
    outf("t1: %016" RV_PRI64 "x    ", val);
    itap_->read(info_->reg2addr("s6"), 8, reinterpret_cast<uint8_t *>(&val));
    outf("s6:  %016" RV_PRI64 "x   ", val);
    itap_->read(info_->reg2addr("a6"), 8, reinterpret_cast<uint8_t *>(&val));
    outf("a6:  %016" RV_PRI64 "x   \n", val);

    itap_->read(info_->reg2addr("t2"), 8, reinterpret_cast<uint8_t *>(&val));
    outf("t2: %016" RV_PRI64 "x    ", val);
    itap_->read(info_->reg2addr("s7"), 8, reinterpret_cast<uint8_t *>(&val));
    outf("s7:  %016" RV_PRI64 "x   ", val);
    itap_->read(info_->reg2addr("a7"), 8, reinterpret_cast<uint8_t *>(&val));
    outf("a7:  %016" RV_PRI64 "x   \n", val);
    
    itap_->read(info_->reg2addr("t3"), 8, reinterpret_cast<uint8_t *>(&val));
    outf("t3: %016" RV_PRI64 "x    ", val);
    itap_->read(info_->reg2addr("s8"), 8, reinterpret_cast<uint8_t *>(&val));
    outf("s8:  %016" RV_PRI64 "x   \n", val);

    itap_->read(info_->reg2addr("t4"), 8, reinterpret_cast<uint8_t *>(&val));
    outf("t4: %016" RV_PRI64 "x    ", val);
    itap_->read(info_->reg2addr("s9"), 8, reinterpret_cast<uint8_t *>(&val));
    outf("s9:  %016" RV_PRI64 "x   \n", val);

    itap_->read(info_->reg2addr("t5"), 8, reinterpret_cast<uint8_t *>(&val));
    outf("t5: %016" RV_PRI64 "x    ", val);
    itap_->read(info_->reg2addr("s10"), 8, reinterpret_cast<uint8_t *>(&val));
    outf("s10: %016" RV_PRI64 "x   ", val);
    itap_->read(info_->reg2addr("pc"), 8, reinterpret_cast<uint8_t *>(&val));
    outf("pc:  %016" RV_PRI64 "x  \n", val);

    itap_->read(info_->reg2addr("t6"), 8, reinterpret_cast<uint8_t *>(&val));
    outf("t6: %016" RV_PRI64 "x    ", val);
    itap_->read(info_->reg2addr("s11"), 8, reinterpret_cast<uint8_t *>(&val));
    outf("s11: %016" RV_PRI64 "x   ", val);
    itap_->read(info_->reg2addr("npc"), 8, reinterpret_cast<uint8_t *>(&val));
    outf("npc: %016" RV_PRI64 "x   \n", val);
}

void CmdExecutor::br(AttributeType *listArgs) {
    uint8_t value[sizeof(uint64_t)];
    *(reinterpret_cast<uint64_t *>(value)) = (*listArgs)[2].to_uint64();
    if (strcmp((*listArgs)[1].to_string(), "add") == 0) {
        itap_->write(info_->addressBreakCreate(), 8, value);
    } else if (strcmp((*listArgs)[1].to_string(), "rm") == 0) {
        itap_->write(info_->addressBreakRemove(), 8, value);
    }
}


int CmdExecutor::outf(const char *fmt, ...) {
    if (outbuf_cnt_ > (outbuf_size_ - 128)) {
        char *t = new char [2*outbuf_size_];
        memcpy(t, outbuf_, outbuf_size_);
        delete [] outbuf_;
        outbuf_size_ *= 2;
        outbuf_ = t;
    }
    va_list arg;
    va_start(arg, fmt);
    outbuf_cnt_ += vsprintf(&outbuf_[outbuf_cnt_], fmt, arg);
    va_end(arg);
    return outbuf_cnt_;
}

}  // namespace debugger
