/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Command Executor implementation.
 */

#include <string.h>
#include "cmdexec.h"
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
#include "cmd/cmd_br.h"

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

    RISCV_mutex_init(&mutexExec_);

    tmpbuf_ = new uint8_t[tmpbuf_size_ = 4096];
    outbuf_ = new char[outbuf_size_ = 4096];
}

CmdExecutor::~CmdExecutor() {
    RISCV_mutex_destroy(&mutexExec_);
    delete [] tmpbuf_;
    delete [] outbuf_;
    for (unsigned i = 0; i < cmds_.size(); i++) {
        delete cmds_[i].to_iface();
    }
}

void CmdExecutor::postinitService() {
    itap_ = static_cast<ITap *>
            (RISCV_get_service_iface(tap_.to_string(), IFACE_TAP));
    info_ = static_cast<ISocInfo *>
            (RISCV_get_service_iface(socInfo_.to_string(), 
                                    IFACE_SOC_INFO));

    cmds_.make_list(13);
    cmds_[0u].make_iface(new CmdBr(itap_, info_));
    cmds_[1].make_iface(new CmdCsr(itap_, info_));
    cmds_[2].make_iface(new CmdExit(itap_, info_));
    cmds_[3].make_iface(new CmdHalt(itap_, info_));
    cmds_[4].make_iface(new CmdIsRunning(itap_, info_));
    cmds_[5].make_iface(new CmdLoadElf(itap_, info_));
    cmds_[6].make_iface(new CmdLog(itap_, info_));
    cmds_[7].make_iface(new CmdMemDump(itap_, info_));
    cmds_[8].make_iface(new CmdRead(itap_, info_));
    cmds_[9].make_iface(new CmdRun(itap_, info_));
    cmds_[10].make_iface(new CmdReg(itap_, info_));
    cmds_[11].make_iface(new CmdRegs(itap_, info_));
    cmds_[12].make_iface(new CmdWrite(itap_, info_));
}

void CmdExecutor::exec(const char *line, AttributeType *res, bool silent) {
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
        return;
    }
    //RISCV_printf0("%s", outbuf_);
}

void CmdExecutor::processSimple(AttributeType *cmd, AttributeType *res) {
    outbuf_[outbuf_cnt_ = 0] = '\0';
    if (cmd->size() == 0) {
        return;
    }

    ICommand *icmd;
    AttributeType listArgs(Attr_List);
    splitLine(const_cast<char *>(cmd->to_string()), &listArgs);
    if (!listArgs[0u].is_string()) {
        RISCV_error("Wrong command format", NULL);
        return;
    }

    if (listArgs[0u].is_equal("help")) {
        if (listArgs.size() == 1) {
            RISCV_printf0("** List of supported commands: **", NULL);
            for (unsigned i = 0; i < cmds_.size(); i++) {
                icmd = static_cast<ICommand *>(cmds_[i].to_iface());
                RISCV_printf0("%13s   - %s",
                        icmd->cmdName(), icmd->briefDescr());
            }
        } else {
            const char *helpcmd = listArgs[1].to_string();
            icmd = getICommand(helpcmd);
            if (icmd) {
                RISCV_printf0("\n%s", icmd->detailedDescr());
            } else {
                RISCV_error("Command '%s' not found", helpcmd);
            }
        }
        return;
    }

    AttributeType u;
    icmd = getICommand(&listArgs);
    if (!icmd) {
        RISCV_error("Command '%s' not found. Use 'help' to list commands",
                    listArgs[0u].to_string());
        return;
    }
    icmd->exec(&listArgs, res);

    if (cmdIsError(res)) {
        RISCV_error("Command '%s' error: '%s'", 
            (*res)[1].to_string(), (*res)[2].to_string());
    }
}

bool CmdExecutor::cmdIsError(AttributeType *res) {
    if (!res->is_list() || res->size() != 3) {
        return false;
    }
    if (!(*res)[0u].is_string()) {
        return false;
    }
    return (*res)[0u].is_equal("ERROR");
}

ICommand *CmdExecutor::getICommand(AttributeType *args) {
    ICommand *ret = 0;
    for (unsigned i = 0; i < cmds_.size(); i++) {
        ret = static_cast<ICommand *>(cmds_[i].to_iface());
        if (ret && (ret->isValid(args) == CMD_VALID)) {
            return ret;
        }
    }
    return 0;
}

ICommand *CmdExecutor::getICommand(const char *name) {
    ICommand *ret = 0;
    for (unsigned i = 0; i < cmds_.size(); i++) {
        ret = static_cast<ICommand *>(cmds_[i].to_iface());
        if (ret && strcmp(ret->cmdName(), name) == 0) {
            return ret;
        }
    }
    return ret;
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
