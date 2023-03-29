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

#include <api_types.h>
#include <iface.h>
#include <attribute.h>
#include "iservice.h"
#include "coreservices/ijtag.h"
#include "coreservices/imemop.h"

namespace debugger {

static const char *IFACE_COMMAND = "ICommand";

static const int CMD_VALID      = 0;
static const int CMD_WRONG_ARGS = 1;
static const int CMD_INVALID    = 2;

const uint32_t REG_ADDR_ERROR = 0xFFFFFFFFul;

class ICommand : public IFace {
 public:
    ICommand(IService *parent, const char *name)
        : IFace(IFACE_COMMAND),
        cmdParent_(parent) {
        cmdName_.make_string(name);
    }

    virtual const char *cmdName() { return cmdName_.to_string(); }
    virtual const char *briefDescr() { return briefDescr_.to_string(); }
    virtual const char *detailedDescr() { return detailedDescr_.to_string(); }
    virtual int isValid(AttributeType *args) = 0;
    virtual void exec(AttributeType *args, AttributeType *res) = 0;

    virtual void generateError(AttributeType *res, const char *descr) {
        res->make_list(3);
        (*res)[0u].make_string("ERROR");
        (*res)[1].make_string(cmdName_.to_string());
        (*res)[2].make_string(descr);
    }

 protected:
    AttributeType cmdName_;
    AttributeType briefDescr_;
    AttributeType detailedDescr_;
    IService *cmdParent_;
};


class ICommandRiscv : public ICommand {
 public:
    ICommandRiscv(IService *parent, const char *name, IJtag *ijtag)
        : ICommand(parent, name), ijtag_(ijtag) {}

 protected:
    virtual uint32_t regsize(const char *name) {
        return IJtag::CMD_AAxSIZE_64BITS;
    }

    virtual uint32_t reg2addr(const char *name) {
        static const ECpuRegMapping RISCV_REG_MAP[] = {
            {"dcsr",    8, 0x7b0},
            {"dpc",     8, 0x7b1},
            {"pc",      8, 0x7b1}, // the same as CSR_dpc,
            {"cycle",   8, 0xc00},
            {"time",    8, 0xc01},
            {"insret",  8, 0xc02},
            {"r0",    8, 0x1000},
            {"ra",    8, 0x1001},
            {"sp",    8, 0x1002},
            {"gp",    8, 0x1003},
            {"tp",    8, 0x1004},
            {"t0",    8, 0x1005},
            {"t1",    8, 0x1006},
            {"t2",    8, 0x1007},
            {"s0",    8, 0x1008},
            {"s1",    8, 0x1009},
            {"a0",    8, 0x100A},
            {"a1",    8, 0x100B},
            {"a2",    8, 0x100C},
            {"a3",    8, 0x100D},
            {"a4",    8, 0x100E},
            {"a5",    8, 0x100F},
            {"a6",    8, 0x1010},
            {"a7",    8, 0x1011},
            {"s2",    8, 0x1012},
            {"s3",    8, 0x1013},
            {"s4",    8, 0x1014},
            {"s5",    8, 0x1015},
            {"s6",    8, 0x1016},
            {"s7",    8, 0x1017},
            {"s8",    8, 0x1018},
            {"s9",    8, 0x1019},
            {"s10",   8, 0x101A},
            {"s11",   8, 0x101B},
            {"t3",    8, 0x101C},
            {"t4",    8, 0x101D},
            {"t5",    8, 0x101E},
            {"t6",    8, 0x101F},
            {"stktr_cnt",   8, 0xC040},     // Non-standart extension. Stack trace counter
            {"stktr_buf",   8, 0xC080},     // Non-standart extension. Stack trace buffer
            {"",            0, 0}
        };
        const ECpuRegMapping *preg = &RISCV_REG_MAP[0];

        while (preg->name[0]) {
            if (strcmp(name, preg->name) == 0) {
                return preg->offset;
            }
            preg++;
        }
        return REG_ADDR_ERROR;
    }

    virtual uint32_t clear_cmderr() {
        IJtag::dmi_abstractcs_type abstractcs;
        abstractcs.u32 = 0;
        abstractcs.bits.cmderr = 1;     // W1C bit to clear sticky cmderr
        return write_dmi(IJtag::DMI_ABSTRACTCS, abstractcs.u32);
    }

    virtual uint32_t bytesz2cmdsz(size_t sz) {
        if (sz >= 8) {
            return 3;
        } else if (sz >= 4) {
            return 2;
        } else if (sz >= 2) {
            return 1;
        }
        return 0;
    }

    virtual uint32_t read_memory(uint64_t addr, size_t sz, uint8_t *obuf) {
        IJtag::dmi_command_type command;
        Reg64Type r64;
        uint32_t cmderr;
        uint32_t tsz;

        r64.val = addr;
        write_dmi(IJtag::DMI_ABSTRACT_DATA2, r64.buf32[0]);
        write_dmi(IJtag::DMI_ABSTRACT_DATA3, r64.buf32[1]);

        command.u32 = 0;
        command.memaccess.cmdtype = 2;      // abstract memory command
        command.memaccess.aamvirtual = 0;
        command.memaccess.aampostincrement = 1;
        command.memaccess.write = 0;
        do {
            command.memaccess.aamsize = bytesz2cmdsz(sz);      // 0=8B; 1=16B; 2=32B; 3=64B
            tsz = (1 << command.memaccess.aamsize);

            write_dmi(IJtag::DMI_COMMAND, command.u32);
            cmderr = wait_dmi();

            if (cmderr) {
                clear_cmderr();
                return cmderr;
            }

            r64.buf32[0] = read_dmi(IJtag::DMI_ABSTRACT_DATA0);
            if (tsz > 4) {
                r64.buf32[1] = read_dmi(IJtag::DMI_ABSTRACT_DATA1);
            }
            memcpy(obuf, r64.buf, tsz);
            sz -= tsz;
            obuf += tsz;
        } while (sz);
        return 0;
    }

    virtual uint32_t write_memory(uint64_t addr, size_t sz, uint8_t *ibuf) {
        IJtag::dmi_command_type command;
        Reg64Type r64;
        uint32_t cmderr;
        uint32_t tsz;

        r64.val = addr;
        write_dmi(IJtag::DMI_ABSTRACT_DATA2, r64.buf32[0]);
        write_dmi(IJtag::DMI_ABSTRACT_DATA3, r64.buf32[1]);

        command.u32 = 0;
        command.memaccess.cmdtype = 2;      // abstract memory command
        command.memaccess.aamvirtual = 0;
        command.memaccess.aampostincrement = 1;
        command.memaccess.write = 1;
        do {
            command.memaccess.aamsize = bytesz2cmdsz(sz);      // 0=8B; 1=16B; 2=32B; 3=64B
            tsz = (1 << command.memaccess.aamsize);

            memcpy(r64.buf, ibuf, tsz);
            write_dmi(IJtag::DMI_ABSTRACT_DATA0, r64.buf32[0]);
            if (tsz > 4) {
                write_dmi(IJtag::DMI_ABSTRACT_DATA1, r64.buf32[1]);
            }

            write_dmi(IJtag::DMI_COMMAND, command.u32);
            cmderr = wait_dmi();

            if (cmderr) {
                clear_cmderr();
                return cmderr;
            }

            sz -= tsz;
            ibuf += tsz;
        } while (sz);
        return 0;
    }

    virtual uint32_t read_dmi(uint32_t addr) {
        return ijtag_->scanDmi(addr, 0, IJtag::DmiOp_Read);
    }

    virtual uint32_t write_dmi(uint32_t addr, uint32_t data) {
        return ijtag_->scanDmi(addr, data, IJtag::DmiOp_Write);
    }

    virtual uint32_t wait_dmi() {
        IJtag::dmi_abstractcs_type abstractcs;
        do {
            abstractcs.u32 = read_dmi(IJtag::DMI_ABSTRACTCS);
        } while (abstractcs.bits.busy == 1);
        return abstractcs.bits.cmderr;
    }

    virtual uint32_t get_reg(uint32_t regaddr, uint32_t regsize, Reg64Type *res) {
        IJtag::dmi_command_type command;
        uint32_t cmderr;

        command.u32 = 0;
        command.regaccess.cmdtype = 0;
        command.regaccess.aarsize = regsize;
        command.regaccess.transfer = 1;
        command.regaccess.aarpostincrement = 1;
        command.regaccess.regno = regaddr;

        write_dmi(IJtag::DMI_COMMAND, command.u32);
        cmderr = wait_dmi();

        if (command.regaccess.aarpostincrement) {
            command.regaccess.regno++;
        }

        res->buf32[0] = read_dmi(IJtag::DMI_ABSTRACT_DATA0);
        res->buf32[1] = read_dmi(IJtag::DMI_ABSTRACT_DATA1);
        return cmderr;
    }

    virtual uint32_t get_reg(const char *regname, Reg64Type *res) {
        return get_reg(reg2addr(regname), regsize(regname), res);
    }

    virtual uint32_t set_reg(const char *regname, Reg64Type *val) {
        IJtag::dmi_command_type command;
        uint32_t cmderr;

        write_dmi(IJtag::DMI_ABSTRACT_DATA0, val->buf32[0]);
        write_dmi(IJtag::DMI_ABSTRACT_DATA1, val->buf32[1]);

        command.u32 = 0;
        command.regaccess.cmdtype = 0;
        command.regaccess.aarsize = regsize(regname);
        command.regaccess.write = 1;
        command.regaccess.transfer = 1;
        command.regaccess.aarpostincrement = 1;
        command.regaccess.regno = reg2addr(regname);

        write_dmi(IJtag::DMI_COMMAND, command.u32);
        cmderr = wait_dmi();

        if (command.regaccess.aarpostincrement) {
            command.regaccess.regno++;
        }
        return cmderr;
    }

 protected:
    IJtag *ijtag_;
};

}  // namespace debugger
