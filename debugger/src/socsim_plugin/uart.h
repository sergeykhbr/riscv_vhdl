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
#include "coreservices/iserial.h"
#include "coreservices/iwire.h"
#include "coreservices/irawlistener.h"
#include <string>

namespace debugger {

class UART : public IService, 
             public IMemoryOperation,
             public ISerial {
public:
    UART(const char *name);
    ~UART();

    /** IService interface */
    virtual void postinitService();

    /** IMemoryOperation */
    virtual ETransStatus b_transport(Axi4TransactionType *trans);

    /** ISerial */
    virtual int writeData(const char *buf, int sz);
    virtual void registerRawListener(IFace *listener);
    virtual void unregisterRawListener(IFace *listener);

private:
    AttributeType irqctrl_;
    AttributeType listeners_;  // non-registering attribute
    IWire *iwire_;

    std::string input_;
    static const int RX_FIFO_SIZE = 16;
    char rxfifo_[RX_FIFO_SIZE];
    char *p_rx_wr_;
    char *p_rx_rd_;
    int rx_total_;
    mutex_def mutexListeners_;

    struct uart_map {
        volatile uint32_t status;
        volatile uint32_t scaler;
        uint32_t rsrv[2];
        volatile uint32_t data;
    } regs_;
};

DECLARE_CLASS(UART)

}  // namespace debugger

#endif  // __DEBUGGER_SOCSIM_PLUGIN_UART_H__
