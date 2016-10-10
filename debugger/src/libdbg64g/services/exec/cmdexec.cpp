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
#include "cmd/cmd_read.h"
#include "cmd/cmd_write.h"
#include "cmd/cmd_run.h"
#include "cmd/cmd_halt.h"
#include "cmd/cmd_csr.h"
#include "cmd/cmd_exit.h"
#include "cmd/cmd_memdump.h"

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

    cmds_["csr"].make_iface(new CmdCsr(itap_, info_));
    cmds_["exit"].make_iface(new CmdExit(itap_, info_));
    cmds_["halt"].make_iface(new CmdHalt(itap_, info_));
    cmds_["isrunning"].make_iface(new CmdIsRunning(itap_, info_));
    cmds_["loadelf"].make_iface(new CmdLoadElf(itap_, info_));
    cmds_["log"].make_iface(new CmdLog(itap_, info_));
    cmds_["memdump"].make_iface(new CmdMemDump(itap_, info_));
    cmds_["read"].make_iface(new CmdRead(itap_, info_));
    cmds_["run"].make_iface(new CmdRun(itap_, info_));
    cmds_["reg"].make_iface(new CmdReg(itap_, info_));
    cmds_["regs"].make_iface(new CmdRegs(itap_, info_));
    cmds_["write"].make_iface(new CmdWrite(itap_, info_));
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
        if (e->format(&listArgs, res, &u) == CMD_IS_OUTPUT) {
            outf(u.to_string());
        }
        if (listArgs[0u].is_equal("log")) {
            //@todo log enabling
        }
        break;

    }
    if (!cmdFound) {
        outf("Use 'help' to print list of the supported commands\n");
    }

#if 0
    
    if (strcmp(listArgs[0u].to_string(), "write") == 0) {
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
