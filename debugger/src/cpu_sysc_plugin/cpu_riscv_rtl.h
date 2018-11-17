/*
 *  Copyright 2018 Sergey Khabarov, sergeykhbr@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 * @details    Use the following targets attributes to generate trace files:
 *             GenerateRef - Generate memory and registers write accesses
 *                           trace files to compare them with functional model
 *             InVcdFile   - Stimulus VCD file
 *             OutVcdFile  - Reference VCD file with any number of signals
 *
 * @note       When GenerateRef is true Core uses step counter instead 
 *             of clock counter to generate callbacks.
 */

#ifndef __DEBUGGER_CPU_RISCV_RTL_H__
#define __DEBUGGER_CPU_RISCV_RTL_H__

#include "iclass.h"
#include "iservice.h"
#include "ihap.h"
#include "async_tqueue.h"
#include "coreservices/ithread.h"
#include "coreservices/icpuriscv.h"
#include "coreservices/imemop.h"
#include "coreservices/iclock.h"
#include "rtl_wrapper.h"
#include "riverlib/river_top.h"
#include <systemc.h>

namespace debugger {

class CpuRiscV_RTL : public IService, 
                 public IThread,
                 public IClock,
                 public IHap {
public:
    CpuRiscV_RTL(const char *name);
    virtual ~CpuRiscV_RTL();

    /** IService interface */
    virtual void postinitService();

    /** IClock */
    virtual uint64_t getStepCounter() {
        return wb_time.read() + 2;
    }

    virtual void registerStepCallback(IClockListener *cb, uint64_t t) {
        wrapper_->registerStepCallback(cb, t);
    }

    bool moveStepCallback(IClockListener *cb, uint64_t t) {
        //if (queue_.move(cb, t)) {
        //    return true;
        //}
        registerStepCallback(cb, t);
        return false;
    }


    virtual double getFreqHz() { return 1.0; }

    /** IHap */
    virtual void hapTriggered(IFace *isrc, EHapType type, const char *descr);

    virtual void stop();

protected:
    /** IThread interface */
    virtual void busyLoop();

private:
    void createSystemC();
    void deleteSystemC();

private:
    AttributeType bus_;
    AttributeType freqHz_;
    AttributeType InVcdFile_;
    AttributeType OutVcdFile_;
    AttributeType GenerateRef_;
    event_def config_done_;
    IMemoryOperation *ibus_;

    sc_signal<bool> w_clk;
    sc_signal<bool> w_nrst;
    // Timer:
    sc_signal<sc_uint<64>> wb_time;
    // Memory interface:
    sc_signal<bool> w_req_mem_ready;
    sc_signal<bool> w_req_mem_valid;
    sc_signal<bool> w_req_mem_write;
    sc_signal<sc_uint<BUS_ADDR_WIDTH>> wb_req_mem_addr;
    sc_signal<sc_uint<BUS_DATA_BYTES>> wb_req_mem_strob;
    sc_signal<sc_uint<BUS_DATA_WIDTH>> wb_req_mem_data;
    sc_signal<bool> w_resp_mem_data_valid;
    sc_signal<sc_uint<BUS_DATA_WIDTH>> wb_resp_mem_data;
    /** Interrupt line from external interrupts controller. */
    sc_signal<bool> w_interrupt;
    // Debug interface
    sc_signal<bool> w_dport_valid;
    sc_signal<bool> w_dport_write;
    sc_signal<sc_uint<2>> wb_dport_region;
    sc_signal<sc_uint<12>> wb_dport_addr;
    sc_signal<sc_uint<RISCV_ARCH>> wb_dport_wdata;
    sc_signal<bool> w_dport_ready;
    sc_signal<sc_uint<RISCV_ARCH>> wb_dport_rdata;


    sc_trace_file *i_vcd_;      // stimulus pattern
    sc_trace_file *o_vcd_;      // reference pattern for comparision
    RiverTop *top_;
    RtlWrapper *wrapper_;
};

DECLARE_CLASS(CpuRiscV_RTL)

}  // namespace debugger

#endif  // __DEBUGGER_CPU_RISCV_RTL_H__
