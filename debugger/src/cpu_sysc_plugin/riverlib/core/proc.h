/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      CPU pipeline implementation.
 */

#ifndef __DEBUGGER_RIVERLIB_PROC_H__
#define __DEBUGGER_RIVERLIB_PROC_H__

#include <systemc.h>
#include "../river_cfg.h"
#include "fetch.h"
#include "decoder.h"
#include "execute.h"
#include "memaccess.h"
#include "execute.h"
#include "regibank.h"
#include "br_predic.h"

namespace debugger {

SC_MODULE(Processor) {
    sc_in<bool> i_clk;
    sc_in<bool> i_nrst;
    sc_in<bool> i_cache_hold;
    // Control path:
    sc_out<bool> o_req_ctrl_valid;
    sc_out<sc_uint<AXI_ADDR_WIDTH>> o_req_ctrl_addr;
    sc_in<bool> i_resp_ctrl_valid;
    sc_in<sc_uint<AXI_ADDR_WIDTH>> i_resp_ctrl_addr;
    sc_in<sc_uint<32>> i_resp_ctrl_data;
    // Data path:
    sc_out<bool> o_req_data_valid;
    sc_out<bool> o_req_data_write;
    sc_out<sc_uint<2>> o_req_data_size; // 0=1bytes; 1=2bytes; 2=4bytes; 3=8bytes
    sc_out<sc_uint<AXI_ADDR_WIDTH>> o_req_data_addr;
    sc_out<sc_uint<RISCV_ARCH>> o_req_data_data;
    sc_in<bool> i_resp_data_valid;
    sc_in<sc_uint<AXI_ADDR_WIDTH>> i_resp_data_addr;
    sc_in<sc_uint<RISCV_ARCH>> i_resp_data_data;


    void comb();
    void registers();

    SC_HAS_PROCESS(Processor);

    Processor(sc_module_name name_, sc_trace_file *vcd=0);
    virtual ~Processor();

private:
    struct FetchType {
        sc_signal<bool> valid;
        sc_signal<sc_uint<AXI_ADDR_WIDTH>> pc;
        sc_signal<sc_uint<32>> instr;
        sc_signal<bool> imem_req_valid;
        sc_signal<sc_uint<AXI_ADDR_WIDTH>> imem_req_addr;
        sc_signal<bool> predict_miss;
    };
    struct InstructionDecodeType {
        sc_signal<sc_uint<AXI_ADDR_WIDTH>> pc;
        sc_signal<sc_uint<32>> instr;
        sc_signal<bool> instr_valid;
        sc_signal<bool> sign_ext;
        sc_signal<sc_bv<ISA_Total>> isa_type;
        sc_signal<sc_bv<Instr_Total>> instr_vec;
        sc_signal<bool> user_level;
        sc_signal<bool> priv_level;
        sc_signal<bool> exception;
    };
    struct ExecuteType {
        sc_signal<bool> valid;
        sc_signal<sc_uint<32>> instr;
        sc_signal<sc_uint<AXI_ADDR_WIDTH>> pc;
        sc_signal<sc_uint<AXI_ADDR_WIDTH>> npc;

        sc_signal<sc_uint<5>> radr1;
        sc_signal<sc_uint<RISCV_ARCH>> rdata1;
        sc_signal<sc_uint<5>> radr2;
        sc_signal<sc_uint<RISCV_ARCH>> rdata2;
        sc_signal<sc_uint<5>> res_addr;
        sc_signal<sc_uint<RISCV_ARCH>> res_data;

        sc_signal<bool> memop_load;
        sc_signal<bool> memop_store;
        sc_signal<sc_uint<2>> memop_size;
        sc_signal<sc_uint<AXI_ADDR_WIDTH>> memop_addr;
        sc_signal<bool> hazard_hold;

    };
    struct MemoryType {
        sc_signal<bool> valid;
        sc_signal<sc_uint<32>> instr;
        sc_signal<sc_uint<AXI_ADDR_WIDTH>> pc;
    };
    struct WriteBackType {
        sc_signal<sc_uint<AXI_ADDR_WIDTH>> pc;
        sc_signal<bool> wena;
        sc_signal<sc_uint<5>> waddr;
        sc_signal<sc_uint<RISCV_ARCH>> wdata;
    };
    struct CsrType {
        sc_signal<sc_uint<12>> addr;
        sc_signal<bool> wena;
        sc_signal<sc_uint<RISCV_ARCH>> rdata;
        sc_signal<sc_uint<RISCV_ARCH>> wdata;

        sc_signal<bool> ie;                           // Interrupt enable bit
        sc_signal<sc_uint<AXI_ADDR_WIDTH>> idt;       // Interrupt descriptor table
        sc_signal<sc_uint<2>> mode;                   // Current processor mode
    };

    struct PipelineType {
        FetchType f;
        InstructionDecodeType d;
        ExecuteType e;
        MemoryType m;
        WriteBackType w;
    } w;
    CsrType csr;
    struct RegistersType {
        sc_signal<sc_uint<3>> dbgCnt;
        //sc_signal<sc_uint<AXI_ADDR_WIDTH>> predict_npc;
    } v, r;

    sc_signal<sc_uint<AXI_ADDR_WIDTH>> wb_npc_predict;
    sc_signal<sc_uint<RISCV_ARCH>> wb_ra;   // Return address
    sc_signal<bool> w_any_hold;


    InstrFetch *fetch0;
    InstrDecoder *dec0;
    InstrExecute *exec0;
    MemAccess *mem0;

    BranchPredictor *predic0;
    RegIntBank *iregs0;

};


}  // namespace debugger

#endif  // __DEBUGGER_RIVERLIB_PROC_H__
