/**
 * @file
 * @copyright  Copyright 2017 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Fast Search Engnine (FSE) black-box model.
 */

#ifndef __DEBUGGER_SOCSIM_PLUGIN_FSEV2_H__
#define __DEBUGGER_SOCSIM_PLUGIN_FSEV2_H__

#include "iclass.h"
#include "iservice.h"
#include "coreservices/imemop.h"

namespace debugger {

class FseV2 : public IService, 
              public IMemoryOperation {
public:
    FseV2(const char *name);

    /** IMemoryOperation */
    virtual ETransStatus b_transport(Axi4TransactionType *trans);

private:
    static const int FSE2_CHAN_MAX = 32;

    struct fsev2_chan_fields {
        volatile uint32_t common;//prn, acc_ms, carr_steps, coh_ena
        volatile int32_t carr_nco_f0;
        volatile int32_t carr_nco_dlt;
        volatile int32_t carr_nco_letter;
        volatile uint32_t max;
        volatile uint32_t ind;
        volatile uint32_t noise;
        volatile int32_t dopler;
    };

    struct fsev2_map {
       fsev2_chan_fields chan[FSE2_CHAN_MAX];

       volatile uint32_t hw_id; // msec ram capacity and hw_id
       volatile uint32_t control;
       volatile uint32_t ms_marker;
       volatile uint32_t carr_nco_th;
       volatile uint32_t code_nco_th;
       volatile int32_t carr_nco_if;
       volatile uint32_t code_nco;
       uint32_t reserved;
    } regs_;
};

DECLARE_CLASS(FseV2)

}  // namespace debugger

#endif  // __DEBUGGER_SOCSIM_PLUGIN_FSEV2_H__
