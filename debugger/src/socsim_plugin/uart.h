/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      UART functional model.
 */

#ifndef __DEBUGGER_SOCSIM_PLUGIN_UART_H__
#define __DEBUGGER_SOCSIM_PLUGIN_UART_H__

#include "iclass.h"
#include "iservice.h"
#include "coreservices/imemop.h"
#include "coreservices/iconsole.h"
#include <string>

namespace debugger {

class UART : public IService, 
             public IMemoryOperation {
public:
    UART(const char *name);
    ~UART();

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
    AttributeType console_;
    IConsole *iconsole_;

    std::string input_;

    struct uart_map {
        volatile uint32_t data;
        volatile uint32_t status;
        volatile uint32_t scaler;
    } regs_;
};

DECLARE_CLASS(UART)

}  // namespace debugger

#endif  // __DEBUGGER_SOCSIM_PLUGIN_UART_H__
