/**
 * @file
 * @copyright  Copyright 2017 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Hardreset module.
 */

#ifndef __SOCSIM_PLUGIN_HARDRESET_H__
#define __SOCSIM_PLUGIN_HARDRESET_H__

#include <iclass.h>
#include <iservice.h>
#include "coreservices/ireset.h"

namespace debugger {

class HardReset : public IService,
                  public IReset {
 public:
    HardReset(const char *name);

    /** IService interface */
    virtual void postinitService();

    /** IReset interface (generic) */
    virtual void powerOnPressed();
    virtual void powerOnReleased();

 private:
    AttributeType resetDevices_;
};

DECLARE_CLASS(HardReset)

}  // namespace debugger

#endif  // __SOCSIM_PLUGIN_HARDRESET_H__
