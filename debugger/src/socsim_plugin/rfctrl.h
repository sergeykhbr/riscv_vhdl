/**
 * @file
 * @copyright  Copyright 2017 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      RF front-end black-box model.
 */

#ifndef __DEBUGGER_SOCSIM_PLUGIN_RFCTRL_H__
#define __DEBUGGER_SOCSIM_PLUGIN_RFCTRL_H__

#include "iclass.h"
#include "iservice.h"
#include "coreservices/imemop.h"

namespace debugger {

class RfController : public IService, 
                     public IMemoryOperation {
public:
    RfController(const char *name);
    ~RfController();

    /** IService interface */
    virtual void postinitService();

    /** IMemoryOperation */
    virtual ETransStatus b_transport(Axi4TransactionType *trans);
    
private:
    struct rfctrl_map {
        volatile uint32_t conf1;		// 0x00
        volatile uint32_t conf2;		// 0x04
        volatile uint32_t conf3;		// 0x08/
        volatile uint32_t pllconf;		// 0x0C/
        volatile uint32_t div;	        // 0x10
        volatile uint32_t fdiv;	        // 0x14
        volatile uint32_t strm;	        // 0x18
        volatile uint32_t clkdiv;		// 0x1C
        volatile uint32_t test1;		// 0x20
        volatile uint32_t test2;		// 0x24
        volatile uint32_t scale;		// 0x28
        volatile uint32_t run;		    // 0x2C
        volatile uint32_t reserved1[3];	// 0x30,0x34,0x38
        volatile uint32_t rw_ant_status;// 0x3C
    } regs_;
};

DECLARE_CLASS(RfController)

}  // namespace debugger

#endif  // __DEBUGGER_SOCSIM_PLUGIN_RFCTRL_H__
