/**
 * @file
 * @copyright  Copyright 2017 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Hardreset module.
 */

#include <api_core.h>
#include "hardreset.h"

namespace debugger {

HardReset::HardReset(const char *name)  : IService(name) {
    registerInterface(static_cast<IReset *>(this));
    registerAttribute("ResetDevices",
                      static_cast<IAttribute *>(&resetDevices_));
}

void HardReset::postinitService() {
    const char *devname, *portname;
    IFace *irst;
    AttributeType item;
    for (unsigned i = 0; i < resetDevices_.size(); i++) {
        AttributeType &dev = resetDevices_[i];
        if (dev.is_string()) {
            devname = dev.to_string();
            irst = RISCV_get_service_iface(devname, IFACE_RESET_LISTENER);
        } else if (dev.is_list() && dev.size() >= 2) {
            devname = dev[0u].to_string();
            portname = dev[1].to_string();
            irst = RISCV_get_service_port_iface(devname, portname,
                                                IFACE_RESET_LISTENER);
        }

        if (irst) {
            item.make_iface(irst);
            resetListeners_.add_to_list(&item);
        } else {
            RISCV_error("%s hasn't IResetListener itnerface", devname);
        }
    }
}

void HardReset::powerOnPressed() {
    reset(true);
}

void HardReset::powerOnReleased() {
    reset(false);
}

}  // namespace debugger

