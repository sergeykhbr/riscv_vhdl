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
    reqReadBr_.make_string("br");
    reqReadNpc_.make_string("reg npc");
    dsu_sw_br_ = ~0;
    dsu_hw_br_ = ~0;
}

EBreakHandler::~EBreakHandler() {
    igui_->removeFromQueue(static_cast<IGuiCmdHandler *>(this));
}

void EBreakHandler::skip() {
    igui_->registerCommand(static_cast<IGuiCmdHandler *>(this),
                            reqReadBr_.to_string(), &brList_, true);
    igui_->registerCommand(static_cast<IGuiCmdHandler *>(this),
                            reqReadNpc_.to_string(), &respReadNpc_, true);
}

void EBreakHandler::handleResponse(const char *cmd) {
    if (strcmp(cmd, "br") == 0) {
        return;
    }
    uint64_t br_addr = respReadNpc_.to_uint64();
    uint32_t br_instr = 0;
    uint64_t br_flags;
    for (unsigned i = 0; i < brList_.size(); i++) {
        const AttributeType &br = brList_[i];
        if (br_addr == br[BrkList_address].to_uint64()) {
            br_instr = br[BrkList_instr].to_int();
            br_flags = br[BrkList_flags].to_uint64();
            break;
        }
    }
    if (br_instr == 0) {
        return;
    }
    if (br_flags & BreakFlag_HW) {
        RISCV_sprintf(reqWriteMem_, sizeof(reqWriteMem_),
                "write 0x%08" RV_PRI64 "x 8 0x%" RV_PRI64 "x",
                dsu_hw_br_, br_addr);
    } else {
        RISCV_sprintf(reqWriteMem_, sizeof(reqWriteMem_),
                "write 0x%08" RV_PRI64 "x 16 [0x%" RV_PRI64 "x,0x%x]",
                dsu_sw_br_, br_addr, br_instr);
    }
    igui_->registerCommand(NULL, reqWriteMem_, &respmemWrite_, true);
}

}  // namespace debugger
