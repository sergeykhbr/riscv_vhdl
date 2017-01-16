/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Run simulation.
 */

#include "cmd_run.h"

namespace debugger {

CmdRun::CmdRun(ITap *tap, ISocInfo *info) 
    : ICommand ("run", tap, info) {

    briefDescr_.make_string("Run simulation for a specify number of steps\"");
    detailedDescr_.make_string(
        "Description:\n"
        "    Run simulation for a specified number of steps.\n"
        "Usage:\n"
        "    run <N steps>\n"
        "Example:\n"
        "    run\n"
        "    go 1000\n"
        "    c 1\n");
}


bool CmdRun::isValid(AttributeType *args) {
    AttributeType &name = (*args)[0u];
    if ((name.is_equal("run") || name.is_equal("c") || name.is_equal("go"))
     && (args->size() == 1 || args->size() == 2)) {
        return CMD_VALID;
    }
    return CMD_INVALID;
}

void CmdRun::exec(AttributeType *args, AttributeType *res) {
    res->make_nil();
    if (!isValid(args)) {
        generateError(res, "Wrong argument list");
        return;
    }
    DsuMapType *dsu = info_->getpDsu();
    Reg64Type runctrl;
    uint64_t addr_run_ctrl = reinterpret_cast<uint64_t>(&dsu->udbg.v.control);
    uint64_t addr_step_cnt = 
        reinterpret_cast<uint64_t>(&dsu->udbg.v.stepping_mode_steps);

    if (args->size() == 1) {
        runctrl.val = 0;
        tap_->write(addr_run_ctrl, 8, runctrl.buf);
    } else if (args->size() == 2) {
        runctrl.val = (*args)[1].to_uint64();
        tap_->write(addr_step_cnt, 8, runctrl.buf);

        DsuMapType::udbg_type::debug_region_type::control_reg ctrl;
        ctrl.val = 0;
        ctrl.bits.stepping = 1;
        runctrl.val = ctrl.val;
        tap_->write(addr_run_ctrl, 8, runctrl.buf);
    }
}

}  // namespace debugger
