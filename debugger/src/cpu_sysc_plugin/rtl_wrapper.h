/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      SystemC CPU wrapper. To interact with the SoC simulator. */

#ifndef __DEBUGGER_RTL_WRAPPER_H__
#define __DEBUGGER_RTL_WRAPPER_H__

#include "async_tqueue.h"
#include "coreservices/ibus.h"
#include "coreservices/icpuriscv.h"
#include "coreservices/iclklistener.h"
#include "riverlib/river_cfg.h"
#include <systemc.h>

namespace debugger {

class RtlWrapper : public sc_module,
                   public ICpuRiscV,
                   public INbResponse {
public:
    sc_clock o_clk;
    sc_out<bool> o_nrst;
    // Timer:
    sc_in<sc_uint<RISCV_ARCH>> i_time;
    // Memory interface:
    sc_out<bool> o_req_mem_ready;
    sc_in<bool> i_req_mem_valid;
    sc_in<bool> i_req_mem_write;
    sc_in<sc_uint<BUS_ADDR_WIDTH>> i_req_mem_addr;
    sc_in<sc_uint<BUS_DATA_BYTES>> i_req_mem_strob;
    sc_in<sc_uint<BUS_DATA_WIDTH>> i_req_mem_data;
    sc_out<bool> o_resp_mem_data_valid;
    sc_out<sc_uint<BUS_DATA_WIDTH>> o_resp_mem_data;
    /** Interrupt line from external interrupts controller. */
    sc_out<bool> o_interrupt;
    // Debug interface
    sc_out<bool> i_dsu_valid;                          // Debug access from DSU command is valid
    sc_out<bool> i_dsu_write;                          // Write Debug value
    sc_out<sc_uint<16>> i_dsu_addr;                    // Debug register address
    sc_out<sc_uint<RISCV_ARCH>> i_dsu_wdata;           // Write value
    sc_in<sc_uint<RISCV_ARCH>> o_dsu_rdata;            // Read value


    struct RegistersType {
        sc_signal<sc_uint<BUS_DATA_WIDTH>> resp_mem_data;
        sc_signal<bool> resp_mem_data_valid;
        sc_signal<sc_uint<3>> wait_state_cnt;
        sc_signal<sc_bv<5>> nrst;
        sc_signal<bool> interrupt;
        sc_signal<bool> dbg_access;
    } r, v;
    bool w_nrst;
    bool w_interrupt;

    void clk_gen();
    void comb();
    void registers();
    void clk_negedge_proc();

    SC_HAS_PROCESS(RtlWrapper);

    RtlWrapper(sc_module_name name);
    virtual ~RtlWrapper();

public:
    void setBus(IBus *v) { ibus_ = v; }

    /** Default time resolution 1 picosecond. */
    void setClockHz(double hz);
   
    /** ICpuRiscV interface */
    virtual void registerStepCallback(IClockListener *cb, uint64_t t);
    virtual void raiseSignal(int idx);
    virtual void lowerSignal(int idx);
    virtual bool isHalt();
    virtual void halt() {}
    virtual void go() {}
    virtual void step(uint64_t cnt) {}
    virtual uint64_t getReg(uint64_t idx) { return 0; }
    virtual void setReg(uint64_t idx, uint64_t val) {}
    virtual uint64_t getCsr(uint64_t idx) { return 0; }
    virtual void setCsr(uint64_t idx, uint64_t val) {}
    virtual uint64_t getPC() { return 0; }
    virtual void setPC(uint64_t val) {}
    virtual uint64_t getNPC() { return 0; }
    virtual void setNPC(uint64_t val) {}
    virtual void addBreakpoint(uint64_t addr) {}
    virtual void removeBreakpoint(uint64_t addr) {}
    virtual void hitBreakpoint(uint64_t addr) {}

    /** INbResponse */
    virtual void nb_response(Axi4TransactionType *trans) {}

private:
    uint64_t mask2offset(uint8_t mask);
    uint32_t mask2size(uint8_t mask);       // nask with removed offset

private:
    IBus *ibus_;
    int clockCycles_;   // default in [ps]
    AsyncTQueueType step_queue_;
    uint64_t step_cnt_z;

    struct DebugPortType {
        bool valid;
        bool write;
        uint64_t addr;
        uint64_t wdata;
    } dbg_port_;
};

}  // namespace debugger

#endif  // __DEBUGGER_RTL_WRAPPER_H__
