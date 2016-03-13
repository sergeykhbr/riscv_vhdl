/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Demo plugin interface.
 */

#ifndef __DEBUGGER_SIMPLE_PLUGIN_H__
#define __DEBUGGER_SIMPLE_PLUGIN_H__

#include "iface.h"

namespace debugger {

static const char *const IFACE_SIMPLE_PLUGIN = "ITap";

static const char *const ISimplePlugin_brief = 
"Simple plugin interface example.";

static const char *const ISimplePlugin_detail = 
"This interface is used to interact with the plugin library.";

class ISimplePlugin : public IFace {
public:
    ISimplePlugin() : IFace(IFACE_SIMPLE_PLUGIN) {}

    virtual const char *getBrief() { return ISimplePlugin_brief; }

    virtual const char *getDetail() { return ISimplePlugin_detail; }

    virtual int exampleAction(int val) =0;
};

}  // namespace debugger

#endif  // __DEBUGGER_SIMPLE_PLUGIN_H__
