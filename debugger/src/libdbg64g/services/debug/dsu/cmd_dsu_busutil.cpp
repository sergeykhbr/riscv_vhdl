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
 *
 * @details    Read CPU dport registers: clock counter and per
 *             master counters with read/write transactions to compute
 *             utilization characteristic.
 */

#include "cmd_dsu_busutil.h"
#include <riscv-isa.h>

namespace debugger {

CmdDsuBusUtil::CmdDsuBusUtil(ITap *tap) : ICommand ("busutil", tap) {

    briefDescr_.make_string("Read per master bus utilization in percentage "
                            "of time");
    detailedDescr_.make_string(
        "Description:\n"
        "    Read and normalize per master bus utilization statistic\n"
        "    using information about total number of clocks and counters\n"
        "    of clocks spending on read/write transactions.\n"
        "Warning:\n"
        "    For functional simulation accumulated utilization may exceed\n"
        "    100.0 percentage of bus because all masters can request data\n"
        "    at the same step without arbiter implementation.\n"
        "Output format:\n"
        "    [[d,d]*]\n"
        "         d - Write access for master[0] in range 0 to 100.\n"
        "         d - Read access for master[0] in range 0 to 100.\n"
        "         * - For each master.\n"
        "Example:\n"
        "    busutil\n");

    clock_cnt_z_ = 0;
    memset(bus_util_z_, 0, sizeof(bus_util_z_));
    mst_total_ = 0;
}

int CmdDsuBusUtil::isValid(AttributeType *args) {
    if (!(*args)[0u].is_equal(cmdName_.to_string())) {
        return CMD_INVALID;
    }
    if (args->size() == 1) {
        return CMD_VALID;
    }
    return CMD_WRONG_ARGS;
}

void CmdDsuBusUtil::exec(AttributeType *args, AttributeType *res) {
    uint64_t addr;
    Reg64Type rd;
    res->make_nil();
    if (mst_total_ == 0) {
        addr = 0xFFFFF008;     // PNP module should be present in a system
        tap_->read(addr, 4, rd.buf);
        mst_total_ = rd.buf[2];     // Bits [23:16] mst_total
    }

    if (mst_total_ == 0 || mst_total_ >= 64) {
        generateError(res, "Can't detect masters total");
        return;
    }
    res->make_list(mst_total_);

    struct MasterStatType {
        Reg64Type w_cnt;
        Reg64Type r_cnt;
    } mst_stat;
    Reg64Type cnt_total;
    addr = DSUREGBASE(csr[CSR_time]);
    tap_->read(addr, 8, cnt_total.buf);
    double d_cnt_total = static_cast<double>(cnt_total.val - clock_cnt_z_);
    if (d_cnt_total == 0) {
        return;
    }

    addr = DSUREGBASE(ulocal.v.bus_util);
    for (unsigned i = 0; i < mst_total_; i++) {
        AttributeType &mst = (*res)[i];
        if (!mst.is_list() || mst.size() != 2) {
            mst.make_list(2);
        }
        tap_->read(addr, 16, mst_stat.w_cnt.buf);
        mst[0u].make_floating(100.0 *
            static_cast<double>(mst_stat.w_cnt.val - bus_util_z_[i].w_cnt)
            / d_cnt_total);
        mst[1].make_floating(100.0 *
            static_cast<double>(mst_stat.r_cnt.val - bus_util_z_[i].r_cnt)
            / d_cnt_total);

        bus_util_z_[i].w_cnt = mst_stat.w_cnt.val;
        bus_util_z_[i].r_cnt = mst_stat.r_cnt.val;
        addr += 16;
    }
    clock_cnt_z_ = cnt_total.val;
}

}  // namespace debugger
