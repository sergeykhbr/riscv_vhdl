/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      GPIO functional model.
 */

#ifndef __DEBUGGER_SOCSIM_PLUGIN_GPIO_H__
#define __DEBUGGER_SOCSIM_PLUGIN_GPIO_H__

#include "iclass.h"
#include "iservice.h"
#include "coreservices/imemop.h"

namespace debugger {

class GPIO : public IService, 
             public IMemoryOperation {
public:
    GPIO(const char *name);
    ~GPIO();

    /** IService interface */
    virtual void postinitService();

    /** IMemoryOperation */
    virtual void transaction(Axi4TransactionType *payload);
    
    virtual uint64_t getBaseAddress() {
        return baseAddress_.to_uint64();
    }
    virtual uint64_t getLength() {
        return length_.to_uint64();
    }

private:
    AttributeType baseAddress_;
    AttributeType length_;
    AttributeType dip_;

    struct gpio_map {
        volatile uint32_t led;
        volatile uint32_t dip;
        volatile uint32_t reg2;
        volatile uint32_t reg3;
        volatile uint32_t led_period;
        volatile uint32_t reg5;
        volatile uint32_t reg6;
    } regs_;
};

DECLARE_CLASS(GPIO)

}  // namespace debugger

#endif  // __DEBUGGER_SOCSIM_PLUGIN_GPIO_H__
