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
#include "csr.h"
#include "br_predic.h"

#define GENERATE_DEBUG_FILE

#ifdef GENERATE_DEBUG_FILE
#include <fstream>
#endif


namespace debugger {

SC_MODULE(Processor) {
    sc_in<bool> i_clk;
    sc_in<bool> i_nrst;                                 // Reset. Active LOW
    sc_in<bool> i_cache_hold;                           // Cache is busy, hold the pipeline
    // Control path:
    sc_out<bool> o_req_ctrl_valid;                      // Request to ICache is valid
    sc_out<sc_uint<BUS_ADDR_WIDTH>> o_req_ctrl_addr;    // Requesting address to ICache
    sc_in<bool> i_resp_ctrl_valid;                      // ICache response is valid
    sc_in<sc_uint<BUS_ADDR_WIDTH>> i_resp_ctrl_addr;    // Response address must be equal to the latest request address
    sc_in<sc_uint<32>> i_resp_ctrl_data;                // Read value
    // Data path:
    sc_out<bool> o_req_data_valid;                      // Request to DCache is valid
    sc_out<bool> o_req_data_write;                      // Read/Write transaction
    sc_out<sc_uint<2>> o_req_data_size;                 // Size [Bytes]: 0=1B; 1=2B; 2=4B; 3=8B
    sc_out<sc_uint<BUS_ADDR_WIDTH>> o_req_data_addr;    // Requesting address to DCache
    sc_out<sc_uint<RISCV_ARCH>> o_req_data_data;        // Writing value
    sc_in<bool> i_resp_data_valid;                      // DCache response is valid
    sc_in<sc_uint<BUS_ADDR_WIDTH>> i_resp_data_addr;    // DCache response address must be equal to the latest request address
    sc_in<sc_uint<RISCV_ARCH>> i_resp_data_data;        // Read value
    // External interrupt pin
    sc_in<bool> i_ext_irq;                              // PLIC interrupt accordingly with spec
    sc_out<sc_uint<64>> o_step_cnt;                     // Number of valid executed instructions

    void comb();
    void registers();
#ifdef GENERATE_DEBUG_FILE
    void negedge_dbg_print();
#endif

    SC_HAS_PROCESS(Processor);

    Processor(sc_module_name name_, sc_trace_file *vcd=0);
    virtual ~Processor();

private:
    struct FetchType {
        sc_signal<bool> valid;
        sc_signal<sc_uint<BUS_ADDR_WIDTH>> pc;
        sc_signal<sc_uint<32>> instr;
        sc_signal<bool> imem_req_valid;
        sc_signal<sc_uint<BUS_ADDR_WIDTH>> imem_req_addr;
        sc_signal<bool> predict_miss;
    };

    struct InstructionDecodeType {
        sc_signal<sc_uint<BUS_ADDR_WIDTH>> pc;
        sc_signal<sc_uint<32>> instr;
        sc_signal<bool> instr_valid;
        sc_signal<bool> memop_store;
        sc_signal<bool> memop_load;
        sc_signal<bool> memop_sign_ext;
        sc_signal<sc_uint<2>> memop_size;
        sc_signal<bool> rv32;                       // 32-bits instruction
        sc_signal<bool> unsigned_op;                // Unsigned operands
        sc_signal<sc_bv<ISA_Total>> isa_type;
        sc_signal<sc_bv<Instr_Total>> instr_vec;
        sc_signal<bool> exception;
    };

    struct ExecuteType {
        sc_signal<bool> valid;
        sc_signal<sc_uint<32>> instr;
        sc_signal<sc_uint<BUS_ADDR_WIDTH>> pc;
        sc_signal<sc_uint<BUS_ADDR_WIDTH>> npc;

        sc_signal<sc_uint<5>> radr1;
        sc_signal<sc_uint<5>> radr2;
        sc_signal<sc_uint<5>> res_addr;
        sc_signal<sc_uint<RISCV_ARCH>> res_data;
        sc_signal<bool> trap_ena;                    // Trap pulse
        sc_signal<sc_uint<5>> trap_code;             // bit[4] : 1=interrupt; 0=exception; bits[3:0]=code
        sc_signal<sc_uint<BUS_ADDR_WIDTH>> trap_pc;  // trap on pc
        sc_signal<bool> xret;
        sc_signal<sc_uint<12>> csr_addr;
        sc_signal<bool> csr_wena;
        sc_signal<sc_uint<RISCV_ARCH>> csr_wdata;

        sc_signal<bool> memop_sign_ext;
        sc_signal<bool> memop_load;
        sc_signal<bool> memop_store;
        sc_signal<sc_uint<2>> memop_size;
        sc_signal<sc_uint<BUS_ADDR_WIDTH>> memop_addr;
        sc_signal<bool> pipeline_hold;           // Hold pipeline from Execution stage

    };

    struct MemoryType {
        sc_signal<bool> valid;
        sc_signal<sc_uint<32>> instr;
        sc_signal<sc_uint<BUS_ADDR_WIDTH>> pc;
        sc_signal<sc_uint<64>> step_cnt;
    };

    struct WriteBackType {
        sc_signal<sc_uint<BUS_ADDR_WIDTH>> pc;
        sc_signal<bool> wena;
        sc_signal<sc_uint<5>> waddr;
        sc_signal<sc_uint<RISCV_ARCH>> wdata;
    };

    struct IntRegsType {
        sc_signal<sc_uint<RISCV_ARCH>> rdata1;
        sc_signal<sc_uint<RISCV_ARCH>> rdata2;
        sc_signal<sc_uint<RISCV_ARCH>> ra;      // Return address
    } ireg;

    struct CsrType {
        sc_signal<sc_uint<RISCV_ARCH>> rdata;

        sc_signal<bool> ie;                     // Interrupt enable bit
        sc_signal<sc_uint<BUS_ADDR_WIDTH>> mtvec;// Interrupt descriptor table
        sc_signal<sc_uint<2>> mode;             // Current processor mode
    } csr;

    /** 5-stages CPU pipeline */
    struct PipelineType {
        FetchType f;                            // Fetch instruction stage
        InstructionDecodeType d;                // Decode instruction stage
        ExecuteType e;                          // Execute instruction
        MemoryType m;                           // Memory load/store
        WriteBackType w;                        // Write back registers value
    } w;

    struct RegistersType {
        sc_signal<sc_uint<64>> clk_cnt;         // Total number of clock since reset
    } v, r;

    sc_signal<sc_uint<BUS_ADDR_WIDTH>> wb_npc_predict;
    sc_signal<bool> w_any_hold;                 // Hold pipeline by any reason


    InstrFetch *fetch0;
    InstrDecoder *dec0;
    InstrExecute *exec0;
    MemAccess *mem0;

    BranchPredictor *predic0;
    RegIntBank *iregs0;
    CsrRegs *csr0;

#ifdef GENERATE_DEBUG_FILE
    char tstr[1024];
    ofstream *reg_dbg;
    ofstream *mem_dbg;
    bool mem_dbg_write_flag;
    uint64_t dbg_mem_value_mask;
    uint64_t dbg_mem_write_value;
#endif
};


}  // namespace debugger

#endif  // __DEBUGGER_RIVERLIB_PROC_H__
