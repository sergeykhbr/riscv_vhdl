/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Breakpoint feedback controller.
 * @details    This class allows to read/write DSU control operations allowing
 *             to correctly continue execution on breakpoints.
 */

#pragma once

#include "api_core.h"   // MUST BE BEFORE QtWidgets.h or any other Qt header.
#include "igui.h"

namespace debugger {

class EBreakHandler : public IGuiCmdHandler {
public:
    EBreakHandler(IGui *gui);
    ~EBreakHandler();

    void setBrAddressFetch(uint64_t addr) { dsu_sw_br_ = addr; }
    void setHwRemoveBreakpoint(uint64_t addr) { dsu_hw_br_ = addr; }

    /** IGuiCmdHandler */
    virtual void handleResponse(AttributeType *req, AttributeType *resp);

    /** Write address and instruction into fetcher to skip EBREAK once */
    void skip();

private:
    AttributeType readBr_;
    AttributeType readNpc_;
    AttributeType brList_;
    IGui *igui_;
    uint64_t dsu_sw_br_;
    uint64_t dsu_hw_br_;
}; 

}  // namespace debugger
