/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Source code interface: disassembler.
 */

#ifndef __DEBUGGER_PLUGIN_ISRCCODE_H__
#define __DEBUGGER_PLUGIN_ISRCCODE_H__

#include "iface.h"
#include <inttypes.h>
#include "attribute.h"

namespace debugger {

static const char *const IFACE_SOURCE_CODE = "ISourceCode";

class ISourceCode : public IFace {
public:
    ISourceCode() : IFace(IFACE_SOURCE_CODE) {}

    /** Disasm input data buffer.
     *
     * @return disassembled instruction length
     */
    virtual int disasm(uint64_t pc,
                       AttributeType *data,
                       int offset,
                       AttributeType *mnemonic,
                       AttributeType *comment) =0;
};

}  // namespace debugger

#endif  // __DEBUGGER_PLUGIN_ISRCCODE_H__
