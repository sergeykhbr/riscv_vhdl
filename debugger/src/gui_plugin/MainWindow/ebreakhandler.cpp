/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Breakpoint feedback controller.
 * @details    This class allows to read/write DSU control operations allowing
 *             to correctly continue execution on breakpoints.
 */

#include "ebreakhandler.h"
#include "coreservices/isrccode.h"

namespace debugger {

EBreakHandler::EBreakHandler(IGui *gui) {
    igui_ = gui;
    readBr_.make_string("br");
    readNpc_.make_string("reg npc");
    dsu_sw_br_ = ~0;
    dsu_hw_br_ = ~0;
}

EBreakHandler::~EBreakHandler() {
    igui_->removeFromQueue(static_cast<IGuiCmdHandler *>(this));
}

void EBreakHandler::skip() {
    igui_->registerCommand(static_cast<IGuiCmdHandler *>(this),
                            &readBr_, true);
    igui_->registerCommand(static_cast<IGuiCmdHandler *>(this),
                            &readNpc_, true);
}

void EBreakHandler::handleResponse(AttributeType *req,
                                   AttributeType *resp) {
    char tstr[128];
    AttributeType memWrite;
    if (req->is_equal("br")) {
        brList_ = *resp;
        return;
    }
    uint64_t br_addr = resp->to_uint64();
    uint32_t br_instr = 0;
    bool br_hw;
    for (unsigned i = 0; i < brList_.size(); i++) {
        const AttributeType &br = brList_[i];
        if (br_addr == br[BrkList_address].to_uint64()) {
            br_instr = br[BrkList_instr].to_int();
            br_hw = br[BrkList_hwflag].to_bool();
            break;
        }
    }
    if (br_instr == 0) {
        return;
    }
    if (br_hw) {
        RISCV_sprintf(tstr, sizeof(tstr),
                "write 0x%08" RV_PRI64 "x 8 0x%" RV_PRI64 "x",
                dsu_hw_br_, br_addr);
    } else {
        RISCV_sprintf(tstr, sizeof(tstr),
                "write 0x%08" RV_PRI64 "x 16 [0x%" RV_PRI64 "x,0x%x]",
                dsu_sw_br_, br_addr, br_instr);
    }
    memWrite.make_string(tstr);
    igui_->registerCommand(NULL, &memWrite, true);
}

}  // namespace debugger
