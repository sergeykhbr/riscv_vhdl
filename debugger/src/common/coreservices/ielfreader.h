/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Interface of elf-file reader.
 */

#ifndef __DEBUGGER_ELFREADER_H__
#define __DEBUGGER_ELFREADER_H__

#include <stdint.h>
#include "iface.h"
#include "attribute.h"

namespace debugger {

static const char *const IFACE_ELFREADER = "IElfReader";

enum ESymbolType {
    SYMBOL_TYPE_FILE     = 0x01,
    SYMBOL_TYPE_FUNCTION = 0x02,
    SYMBOL_TYPE_DATA     = 0x04
};

enum ESymbolInfoListItem {
    Symbol_Name,
    Symbol_Addr,
    Symbol_Size,
    Symbol_Type,
    Symbol_Total
};


class IElfReader : public IFace {
public:
    IElfReader() : IFace(IFACE_ELFREADER) {}

    virtual int readFile(const char *filename) =0;

    virtual unsigned loadableSectionTotal() =0;

    virtual const char *sectionName(unsigned idx) =0;

    virtual uint64_t sectionAddress(unsigned idx) =0;

    virtual uint64_t sectionSize(unsigned idx) =0;

    virtual uint8_t *sectionData(unsigned idx) =0;

    virtual void getSymbols(AttributeType *list) =0;

    virtual void addressToSymbol(uint64_t addr, AttributeType *name) =0;
};

}  // namespace debugger

#endif  // __DEBUGGER_ELFREADER_H__
