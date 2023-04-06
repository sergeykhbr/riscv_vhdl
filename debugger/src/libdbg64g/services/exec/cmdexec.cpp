/*
 *  Copyright 2018 Sergey Khabarov, sergeykhbr@gmail.com
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

#include <string.h>
#include "cmdexec.h"
#include "cmd/cmd_halt.h"
#include "cmd/cmd_resume.h"
#include "cmd/cmd_isrunning.h"
#include "cmd/cmd_status.h"
#include "cmd/cmd_loadelf.h"
#include "cmd/cmd_loadh86.h"
#include "cmd/cmd_loadsrec.h"
#include "cmd/cmd_log.h"
#include "cmd/cmd_read.h"
#include "cmd/cmd_write.h"
#include "cmd/cmd_exit.h"
#include "cmd/cmd_memdump.h"
#include "cmd/cmd_cpi.h"
#include "cmd/cmd_reset.h"
#include "cmd/cmd_loadbin.h"
#include "cmd/cmd_elf2raw.h"
#include "cmd/cmd_cpucontext.h"
#include "cmd/cmd_reg.h"

namespace debugger {

CmdExecutor::CmdExecutor(const char *name) 
    : TcpClient(0, name) {
    registerInterface(static_cast<ICmdExecutor *>(this));
    registerAttribute("Bus", &bus_);
    registerAttribute("Jtag", &jtag_);

    cmds_.make_list(0);

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
    ibus_ = static_cast<IMemoryOperation *>
            (RISCV_get_service_iface(bus_.to_string(), IFACE_MEMORY_OPERATION));

    ijtag_ = static_cast<IJtag *>
            (RISCV_get_service_iface(jtag_.to_string(), IFACE_JTAG));

    // Core commands registration:
    ICommand *tcmd;
    registerCommand(new CmdHalt(this, ijtag_));
    registerCommand(new CmdResume(this, ijtag_));
    registerCommand(new CmdIsRunning(this, ijtag_));
    registerCommand(new CmdStatus(this, ijtag_));
    registerCommand(new CmdReg(this, ijtag_));
    registerCommand(new CmdCpi(this, ijtag_));
    registerCommand(new CmdCpuContext(this, ijtag_));
    registerCommand(new CmdElf2Raw(this));
    registerCommand(new CmdExit(this));
    registerCommand(new CmdLoadBin(this, ijtag_));
    registerCommand(new CmdLoadElf(this, ijtag_));
    registerCommand(new CmdLoadH86(this, ijtag_));
    registerCommand(new CmdLoadSrec(this, ijtag_));
    registerCommand(new CmdLog(this));
    registerCommand(new CmdMemDump(this, ijtag_));
    registerCommand(tcmd = new CmdRead(this, ijtag_));
    registerCommand(new CmdReset(this, ijtag_));
    registerCommand(tcmd = new CmdWrite(this, ijtag_));
}

void CmdExecutor::registerCommand(ICommand *icmd) {
    AttributeType t1(icmd);
    cmds_.add_to_list(&t1);
}

void CmdExecutor::unregisterCommand(ICommand *icmd) {
    for (unsigned i = 0; i < cmds_.size(); i++) {
        if (cmds_[i].to_iface() == icmd) {
            cmds_.remove_from_list(i);
            break;
        }
    }
}

void CmdExecutor::exec(const char *line, AttributeType *res, bool silent) {
    RISCV_mutex_lock(&mutexExec_);

    AttributeType cmd;
    if (line[0] == '[' || line[0] == '}') {
        cmd.from_config(line);
    } else {
        cmd.make_string(line);
    }

    AttributeType listArgs(Attr_List), *cmd_parsed;
    if (cmd.is_string()) {
        splitLine(const_cast<char *>(cmd.to_string()), &listArgs);
        cmd_parsed = &listArgs;
    } else {
        cmd_parsed = &cmd;
    }
    processSimple(cmd_parsed, res);

    RISCV_mutex_unlock(&mutexExec_);

    /** Do not output any information into console in silent mode: */
    if (silent) {
        return;
    }
    //RISCV_printf0("%s", outbuf_);
}

void CmdExecutor::commands(const char *substr, AttributeType *res) {
    if (!res->is_list()) {
        res->make_list(0);
    }
    AttributeType item;
    item.make_list(3);
    for (unsigned i = 0; i < cmds_.size(); i++) {
        ICommand *icmd = static_cast<ICommand *>(cmds_[i].to_iface());
        if (strstr(icmd->cmdName(), substr)) {
            item[0u].make_string(icmd->cmdName());
            item[1].make_string(icmd->briefDescr());
            item[2].make_string(icmd->detailedDescr());
            res->add_to_list(&item);
        }
    }
}

void CmdExecutor::processSimple(AttributeType *cmd, AttributeType *res) {
    outbuf_[outbuf_cnt_ = 0] = '\0';
    if (cmd->size() == 0) {
        return;
    }

    ICommand *icmd;
    if (!(*cmd)[0u].is_string()) {
        res->attr_free();
        res->make_nil();
        RISCV_error("Wrong command format", NULL);
        return;
    }

    if ((*cmd)[0u].is_equal("help")) {
        res->attr_free();
        res->make_nil();
        if (cmd->size() == 1) {
            RISCV_printf0("** List of supported commands: **", NULL);
            for (unsigned i = 0; i < cmds_.size(); i++) {
                icmd = static_cast<ICommand *>(cmds_[i].to_iface());
                RISCV_printf0("%13s   - %s",
                        icmd->cmdName(), icmd->briefDescr());
            }
        } else {
            AttributeType helparg;
            helparg.make_list(1);
            helparg[0u].make_string((*cmd)[1].to_string());
            getICommand(&helparg, &icmd);
            if (icmd) {
                RISCV_printf0("\n%s", icmd->detailedDescr());
            } else {
                RISCV_error("Command '%s' not found", helparg[0u].to_string());
            }
        }
        return;
    }

    int err = getICommand(cmd, &icmd);
    if (!icmd) {
        res->attr_free();
        res->make_nil();
        RISCV_error("Command '%s' not found. "
                    "Use 'help' to list commands", (*cmd)[0u].to_string());
        return;
    } else if (err == CMD_WRONG_ARGS) {
        res->attr_free();
        res->make_nil();
        RISCV_error("Command '%s' has been called with invalid arguments. "
            "Use 'help %s' to check the required arguments",
            (*cmd)[0u].to_string(), (*cmd)[0u].to_string());
        return;
    }
    icmd->exec(cmd, res);

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

int CmdExecutor::getICommand(AttributeType *args, ICommand **pcmd) {
    *pcmd = 0;
    int err = CMD_INVALID;
    ICommand *iitem;
    for (unsigned i = 0; i < cmds_.size(); i++) {
        iitem = static_cast<ICommand *>(cmds_[i].to_iface());
        if (!iitem) {
            continue;
        }
        err = iitem->isValid(args);
        if (err != CMD_INVALID) {
            *pcmd = iitem;
            return err;
        }
    }
    return CMD_INVALID;
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
             || ((str[0] == '-' || str[0] == '+') &&
                  str[1] >= '0' && str[1] <= '9')
             || (str[0] == '[') || (str[0] == '"') || (str[0] == '\'')
             || (str[0] == '{') || (str[0] == '(')) {
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
