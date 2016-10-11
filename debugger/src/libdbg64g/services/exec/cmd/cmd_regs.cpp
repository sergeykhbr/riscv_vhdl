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
        "Return:\n"
        "    String if no names specified, list of int64_t otherwise.\n"
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

void CmdRegs::exec(AttributeType *args, AttributeType *res) {
    if (!isValid(args)) {
        generateError(res, "Wrong argument list");
        return;
    }

    Reg64Type u;
    if (args->size() != 1) {
        res->make_list(args->size() - 1);
        for (unsigned i = 1; i < args->size(); i++) {
            const char *name = (*args)[i].to_string();
            tap_->read(info_->reg2addr(name), 8, u.buf);
            (*res)[i - 1].make_uint64(u.val);
        }
        return;
    } 
    
    AttributeType soclst, regdict(Attr_Dict);
    info_->getRegsList(&soclst);
    for (unsigned i = 0; i < soclst.size(); i++) {
        const char *name = soclst[i].to_string();
        tap_->read(info_->reg2addr(name), 8, u.buf);
        regdict[name].make_uint64(u.val);
    }
    convert_to_str(&regdict, res);
}

void CmdRegs::convert_to_str(AttributeType *lst, AttributeType *res) {
    char tstr[1024];
    int tcnt = 0;

    tcnt += RISCV_sprintf(&tstr[tcnt], sizeof(tstr) - tcnt, 
            "\nra: %016" RV_PRI64 "x    \n", (*lst)["ra"].to_uint64());

    tcnt += RISCV_sprintf(&tstr[tcnt], sizeof(tstr) - tcnt,
    "                        s0:  %016" RV_PRI64 "x   a0:  %016" RV_PRI64 "x\n",
    (*lst)["s0"].to_uint64(), (*lst)["a0"].to_uint64());

    tcnt += RISCV_sprintf(&tstr[tcnt], sizeof(tstr) - tcnt,
    "sp: %016" RV_PRI64 "x    s1:  %016" RV_PRI64 "x   a1:  %016" RV_PRI64 "x\n"
    ,(*lst)["sp"].to_uint64(), (*lst)["s1"].to_uint64(), (*lst)["a1"].to_uint64());

    tcnt += RISCV_sprintf(&tstr[tcnt], sizeof(tstr) - tcnt,
    "gp: %016" RV_PRI64 "x    s2:  %016" RV_PRI64 "x   a2:  %016" RV_PRI64 "x\n"
    ,(*lst)["gp"].to_uint64(), (*lst)["s2"].to_uint64(), (*lst)["a2"].to_uint64());

    tcnt += RISCV_sprintf(&tstr[tcnt], sizeof(tstr) - tcnt,
    "tp: %016" RV_PRI64 "x    s3:  %016" RV_PRI64 "x   a3:  %016" RV_PRI64 "x\n"
    ,(*lst)["tp"].to_uint64(), (*lst)["s3"].to_uint64(), (*lst)["a3"].to_uint64());

    tcnt += RISCV_sprintf(&tstr[tcnt], sizeof(tstr) - tcnt,
    "                        s4:  %016" RV_PRI64 "x   a4:  %016" RV_PRI64 "x\n"
    ,(*lst)["s4"].to_uint64(), (*lst)["a4"].to_uint64());

    tcnt += RISCV_sprintf(&tstr[tcnt], sizeof(tstr) - tcnt,
    "t0: %016" RV_PRI64 "x    s5:  %016" RV_PRI64 "x   a5:  %016" RV_PRI64 "x\n"
    ,(*lst)["t0"].to_uint64(), (*lst)["s5"].to_uint64(), (*lst)["a5"].to_uint64());

    tcnt += RISCV_sprintf(&tstr[tcnt], sizeof(tstr) - tcnt,
    "t1: %016" RV_PRI64 "x    s6:  %016" RV_PRI64 "x   a6:  %016" RV_PRI64 "x\n"
    ,(*lst)["t1"].to_uint64(), (*lst)["s6"].to_uint64(), (*lst)["a6"].to_uint64());

    tcnt += RISCV_sprintf(&tstr[tcnt], sizeof(tstr) - tcnt,
    "t2: %016" RV_PRI64 "x    s7:  %016" RV_PRI64 "x   a7:  %016" RV_PRI64 "x\n"
    ,(*lst)["t2"].to_uint64(), (*lst)["s7"].to_uint64(), (*lst)["a7"].to_uint64());

    tcnt += RISCV_sprintf(&tstr[tcnt], sizeof(tstr) - tcnt,
    "t3: %016" RV_PRI64 "x    s8:  %016" RV_PRI64 "x\n"
    ,(*lst)["t3"].to_uint64(), (*lst)["s8"].to_uint64());
    
    tcnt += RISCV_sprintf(&tstr[tcnt], sizeof(tstr) - tcnt,
    "t4: %016" RV_PRI64 "x    s9:  %016" RV_PRI64 "x\n"
    ,(*lst)["t4"].to_uint64(), (*lst)["s9"].to_uint64());

    tcnt += RISCV_sprintf(&tstr[tcnt], sizeof(tstr) - tcnt,
    "t5: %016" RV_PRI64 "x    s10: %016" RV_PRI64 "x   pc:  %016" RV_PRI64 "x\n"
    ,(*lst)["t5"].to_uint64(), (*lst)["s10"].to_uint64(), (*lst)["pc"].to_uint64());

    tcnt += RISCV_sprintf(&tstr[tcnt], sizeof(tstr) - tcnt,
    "t6: %016" RV_PRI64 "x    s11: %016" RV_PRI64 "x   npc: %016" RV_PRI64 "x\n"
    ,(*lst)["t6"].to_uint64(), (*lst)["s11"].to_uint64(), (*lst)["npc"].to_uint64());

    res->make_string(tstr);
}

}  // namespace debugger
