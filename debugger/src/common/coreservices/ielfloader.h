/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Interface of elf-file loader.
 */

#ifndef __DEBUGGER_ELFLOADER_H__
#define __DEBUGGER_ELFLOADER_H__

#include "iface.h"

namespace debugger {

static const char *const IFACE_ELFLOADER = "IElfLoader";

class IElfLoader : public IFace {
public:
    IElfLoader() : IFace(IFACE_ELFLOADER) {}

    virtual int loadFile(const char *filename) =0;

    /// @todo access methods to symbols
};

}  // namespace debugger

#endif  // __DEBUGGER_ELFLOADER_H__
