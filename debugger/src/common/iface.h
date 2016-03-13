/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Base interface declaration of the Core.
 */

#ifndef __DEBUGGER_IFACE_H__
#define __DEBUGGER_IFACE_H__

namespace debugger {

class IFace {
public:
    IFace(const char *name) : ifname_(name) {}
    virtual ~IFace() {}

    /** Get brief information. */
    virtual const char *getBrief() { return "Brief info not defined"; }

    /** Get detailed description. */
    virtual const char *getDetail() { return "Detail info not defined"; }

    /** Get interface name. */
    const char *getFaceName() { return ifname_; }

protected:
    const char *ifname_;
};

}  // namespace debugger

#endif  // __DEBUGGER_IFACE_H__
