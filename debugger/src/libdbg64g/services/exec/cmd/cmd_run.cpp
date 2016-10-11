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

    DsuRunControlRegType runctrl;
    if (args->size() == 1) {
        runctrl.val = 0;
        tap_->write(info_->addressRunControl(), 8, runctrl.buf);
    } else if (args->size() == 2) {
        runctrl.val = (*args)[1].to_uint64();
        tap_->write(info_->addressStepCounter(), 8, runctrl.buf);

        runctrl.val = 0;
        runctrl.bits.stepping = 1;
        tap_->write(info_->addressRunControl(), 8, runctrl.buf);
    }
}

}  // namespace debugger
