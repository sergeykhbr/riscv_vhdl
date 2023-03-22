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

#pragma once

#include "iclass.h"
#include "iservice.h"
#include "ihap.h"
#include "async_tqueue.h"
#include "coreservices/ithread.h"
#include "coreservices/icpuriscv.h"
#include "coreservices/imemop.h"
#include "coreservices/iclock.h"
#include "coreservices/icmdexec.h"
#include "coreservices/iirq.h"
#include "rtl_wrapper.h"
#include "tap_bitbang.h"
#include "bus_slv.h"
#include "ambalib/types_amba.h"
#include "ambalib/axi2apb.h"
#include "riverlib/workgroup.h"
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
    virtual void initService(const AttributeType *args) override;
    virtual void postinitService() override;
    virtual void predeleteService() override ;

    /** IClock */
    virtual uint64_t getStepCounter() {
        return wrapper_->getClockCounter();
    }


    virtual void registerStepCallback(IClockListener *cb, uint64_t t) {
        wrapper_->registerStepCallback(cb, t);
    }

    bool moveStepCallback(IClockListener *cb, uint64_t t) {
        registerStepCallback(cb, t);
        return false;
    }

    virtual double getFreqHz() {
        return static_cast<double>(freqHz_.to_uint64());
    }

    /** IHap */
    virtual void hapTriggered(EHapType type, uint64_t param,
                              const char *descr);

    virtual void stop();

 protected:
    /** IThread interface */
    virtual void busyLoop();

 private:
    void createSystemC();
    void deleteSystemC();

 private:
    AttributeType hartid_;
    AttributeType asyncReset_;
    AttributeType cpuNum_;
    AttributeType l2CacheEnable_;
    AttributeType clint_;
    AttributeType plic_;
    AttributeType bus_;
    AttributeType cmdexec_;
    AttributeType dmibar_;
    AttributeType freqHz_;
    AttributeType InVcdFile_;
    AttributeType OutVcdFile_;
    event_def config_done_;

    IIrqController *iirqloc_;
    IIrqController *iirqext_;
    ICmdExecutor *icmdexec_;
    IMemoryOperation *ibus_;

    sc_signal<bool> w_clk;
    sc_signal<bool> w_sys_nrst;
    sc_signal<bool> w_dmi_nrst;
    sc_signal<sc_uint<64>> wb_mtimer;

    // AXI4 input structure:
    sc_signal<dev_config_type> xcfg;
    // Interrupt lines:
    sc_signal<sc_uint<CFG_CPU_MAX>> wb_msip;
    sc_signal<sc_uint<CFG_CPU_MAX>> wb_mtip;
    sc_signal<sc_uint<CFG_CPU_MAX>> wb_meip;
    sc_signal<sc_uint<CFG_CPU_MAX>> wb_seip;
    // Debug interface
    sc_signal<bool> w_ndmreset;
    sc_signal<bool> w_trst;
    sc_signal<bool> w_tck;
    sc_signal<bool> w_tms;
    sc_signal<bool> w_tdi;
    sc_signal<bool> w_tdo;

    sc_signal<mapinfo_type> wb_dmi_mapinfo;
    sc_signal<dev_config_type> wb_dmi_cfg;
    sc_signal<apb_in_type> wb_dmi_apbi;
    sc_signal<apb_out_type> wb_dmi_apbo;
    
    sc_signal<axi4_l2_in_type> l2i;
    sc_signal<axi4_l2_out_type> l2o;
    sc_signal<axi4_master_in_type> msti;
    sc_signal<axi4_master_out_type> msto;
    sc_signal<axi4_master_in_type> acpi;
    sc_signal<axi4_master_out_type> acpo;
    sc_signal<axi4_slave_in_type> xslvi;
    sc_signal<axi4_slave_out_type> xslvo;
    sc_signal<apb_in_type> apbi;
    sc_signal<apb_out_type> apbo;

    sc_trace_file *i_vcd_;      // stimulus pattern
    sc_trace_file *o_vcd_;      // reference pattern for comparision
    RtlWrapper *wrapper_;
    TapBitBang *tapbb_;
    BusSlave *dmislv_;
    Workgroup *group0_;
};

DECLARE_CLASS(CpuRiscV_RTL)

}  // namespace debugger

