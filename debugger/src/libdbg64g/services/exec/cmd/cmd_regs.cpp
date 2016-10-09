/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Read only registers values.
 */

#include "cmd_regs.h"

namespace debugger {

CmdRegs::CmdRegs(ITap *tap, ISocInfo *info) 
    : ICommand ("regs", tap, info) {

    briefDescr_.make_string("List of Core's registers values");
    detailedDescr_.make_string(
        "Description:\n"
        "    Print values of CPU's registers.\n"
        "Usage:\n"
        "    regs\n"
        "    regs name1 name2 ..\n"
        "Example:\n"
        "    regs\n"
        "    regs a0 s0 sp\n");
}

bool CmdRegs::isValid(AttributeType *args) {
    if ((*args)[0u].is_equal(cmdName_.to_string()) && args->size() >= 1) {
        return CMD_VALID;
    }
    return CMD_INVALID;
}

bool CmdRegs::exec(AttributeType *args, AttributeType *res) {
    res->make_dict();
    if (!isValid(args)) {
        return CMD_FAILED;
    }

    AttributeType soclst;
    if (args->size() != 1) {
        for (unsigned i = 1; i < args->size(); i++) {
            const char *name = (*args)[i].to_string();
            (*res)[name].make_uint64(0);
        }
    } else {
        info_->getRegsList(&soclst);
        for (unsigned i = 0; i < soclst.size(); i++) {
            const char *name = soclst[i][0u].to_string();
            (*res)[name].make_uint64(0);
        }
    }

    uint64_t val;
    for (unsigned i = 0; i < res->size(); i++) {
        tap_->read(info_->reg2addr(res->dict_key(i)->to_string()), 
                                    8, reinterpret_cast<uint8_t *>(&val));
        (*res)[i].make_uint64(val);
    }

    return CMD_SUCCESS;
}

bool CmdRegs::format(AttributeType *res, AttributeType *out) {
    if (!res->is_dict()) {
        return CMD_NO_OUTPUT;
    }
    char tstr[1024];
    int tcnt = 0;

    /** Full or not-full list of registers hould be printed: */
    if (res->size() != info_->getRegsTotal()) {
        for (unsigned i = 0; i < res->size(); i++) {
            RISCV_sprintf(tstr, sizeof(tstr), 
            "%s: %016" RV_PRI64 "x\n", res->dict_key(i)->to_string(),
                                       res->dict_value(i)->to_uint64());
        }
        out->make_string(tstr);
        return CMD_IS_OUTPUT;
    }

    tcnt += RISCV_sprintf(tstr, sizeof(tstr) - tcnt, 
            "ra: %016" RV_PRI64 "x    \n", (*res)["ra"].to_uint64());

    tcnt += RISCV_sprintf(tstr, sizeof(tstr) - tcnt,
    "                        s0:  %016" RV_PRI64 "x   a0:  %016" RV_PRI64 "x\n",
    (*res)["s0"].to_uint64(), (*res)["a0"].to_uint64());

    tcnt += RISCV_sprintf(tstr, sizeof(tstr) - tcnt,
    "sp: %016" RV_PRI64 "x    s1:  %016" RV_PRI64 "x   a1:  %016" RV_PRI64 "x\n"
    ,(*res)["sp"].to_uint64(), (*res)["s1"].to_uint64(), (*res)["a1"].to_uint64());

    tcnt += RISCV_sprintf(tstr, sizeof(tstr) - tcnt,
    "gp: %016" RV_PRI64 "x    s2:  %016" RV_PRI64 "x   a2:  %016" RV_PRI64 "x\n"
    ,(*res)["gp"].to_uint64(), (*res)["s2"].to_uint64(), (*res)["a2"].to_uint64());

    tcnt += RISCV_sprintf(tstr, sizeof(tstr) - tcnt,
    "tp: %016" RV_PRI64 "x    s3:  %016" RV_PRI64 "x   a3:  %016" RV_PRI64 "x\n"
    ,(*res)["tp"].to_uint64(), (*res)["s3"].to_uint64(), (*res)["a3"].to_uint64());

    tcnt += RISCV_sprintf(tstr, sizeof(tstr) - tcnt,
    "                        s4:  %016" RV_PRI64 "x   a4:  %016" RV_PRI64 "x\n"
    ,(*res)["s4"].to_uint64(), (*res)["a4"].to_uint64());

    tcnt += RISCV_sprintf(tstr, sizeof(tstr) - tcnt,
    "t0: %016" RV_PRI64 "x    s5:  %016" RV_PRI64 "x   a5:  %016" RV_PRI64 "x\n"
    ,(*res)["t0"].to_uint64(), (*res)["s5"].to_uint64(), (*res)["a5"].to_uint64());

    tcnt += RISCV_sprintf(tstr, sizeof(tstr) - tcnt,
    "t1: %016" RV_PRI64 "x    s6:  %016" RV_PRI64 "x   a6:  %016" RV_PRI64 "x\n"
    ,(*res)["t1"].to_uint64(), (*res)["s6"].to_uint64(), (*res)["a6"].to_uint64());

    tcnt += RISCV_sprintf(tstr, sizeof(tstr) - tcnt,
    "t2: %016" RV_PRI64 "x    s7:  %016" RV_PRI64 "x   a7:  %016" RV_PRI64 "x\n"
    ,(*res)["t2"].to_uint64(), (*res)["s7"].to_uint64(), (*res)["a7"].to_uint64());

    tcnt += RISCV_sprintf(tstr, sizeof(tstr) - tcnt,
    "t3: %016" RV_PRI64 "x    s8:  %016" RV_PRI64 "x\n"
    ,(*res)["t3"].to_uint64(), (*res)["s8"].to_uint64());
    
    tcnt += RISCV_sprintf(tstr, sizeof(tstr) - tcnt,
    "t4: %016" RV_PRI64 "x    s9:  %016" RV_PRI64 "x\n"
    ,(*res)["t4"].to_uint64(), (*res)["s9"].to_uint64());

    tcnt += RISCV_sprintf(tstr, sizeof(tstr) - tcnt,
    "t5: %016" RV_PRI64 "x    s10: %016" RV_PRI64 "x   pc:  %016" RV_PRI64 "x\n"
    ,(*res)["t5"].to_uint64(), (*res)["s10"].to_uint64(), (*res)["pc"].to_uint64());

    tcnt += RISCV_sprintf(tstr, sizeof(tstr) - tcnt,
    "t6: %016" RV_PRI64 "x    s11: %016" RV_PRI64 "x   npc: %016" RV_PRI64 "x\n"
    ,(*res)["t6"].to_uint64(), (*res)["s11"].to_uint64(), (*res)["npc"].to_uint64());

    out->make_string(tstr);
    return CMD_IS_OUTPUT;
}

}  // namespace debugger
