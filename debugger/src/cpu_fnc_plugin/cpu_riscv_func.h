/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      CPU functional simlator class definition.
 */

#ifndef __DEBUGGER_CPU_RISCV_FUNCTIONAL_H__
#define __DEBUGGER_CPU_RISCV_FUNCTIONAL_H__

#include <cpu_generic.h>
#include <riscv-isa.h>
#include "instructions.h"
#include "coreservices/icpuriscv.h"
#include "coreservices/isocinfo.h"

namespace debugger {

#ifdef ENABLE_OBOSOLETE

class CpuRiscV_Functional : public IService, 
                 public IThread,
                 public ICpuGeneric,
                 public ICpuRiscV,
                 public IClock,
                 public IHap {
 public:
    CpuRiscV_Functional(const char *name);
    virtual ~CpuRiscV_Functional();

    /** IService interface */
    virtual void postinitService();

    /** ICpuGeneric interface */
    virtual void raiseSignal(int idx);
    virtual void lowerSignal(int idx);
    virtual void nb_transport_debug_port(DebugPortTransactionType *trans,
                                         IDbgNbResponse *cb);

    /** ICpuRiscV interface */

    /** IClock */
    virtual uint64_t getStepCounter() { return cpu_context_.step_cnt; }
    virtual void registerStepCallback(IClockListener *cb, uint64_t t);
    virtual double getFreqHz() { return freqHz_.to_int(); }

    /** IHap */
    virtual void hapTriggered(IFace *isrc, EHapType type, const char *descr);

 protected:
    /** IThread interface */
    virtual void busyLoop();

private:
    bool isHalt() { return dbg_state_ == STATE_Halted; }
    void halt(const char *descr=NULL);
    void go();
    void step(uint64_t cnt);
    uint64_t getReg(uint64_t idx);
    void setReg(uint64_t idx, uint64_t val);
    uint64_t getPC();
    void setPC(uint64_t val);
    uint64_t getNPC();
    void setNPC(uint64_t val);
    void addBreakpoint(uint64_t addr);
    void removeBreakpoint(uint64_t addr);
    void hitBreakpoint(uint64_t addr);

    CpuContextType *getpContext() { return &cpu_context_; }
    uint32_t hash32(uint32_t val) { return (val >> 2) & 0x1f; }

    void updatePipeline();
    void updateState();
    void updateDebugPort();
    void updateQueue();

    bool isRunning();
    void reset();
    void handleTrap();
    void fetchInstruction();
    IInstruction *decodeInstruction(uint32_t *rpayload);
    void executeInstruction(IInstruction *instr, uint32_t *rpayload);
    void debugRegOutput(const char *marker, CpuContextType *pContext);

private:
    static const int INSTR_HASH_TABLE_SIZE = 1 << 5;

    AttributeType isEnable_;
    AttributeType bus_;
    AttributeType listExtISA_;
    AttributeType freqHz_;
    AttributeType generateRegTraceFile_;
    AttributeType generateMemTraceFile_;
    AttributeType resetVector_;
    event_def config_done_;

    ClockAsyncTQueueType queue_;
    uint64_t last_hit_breakpoint_;

    Axi4TransactionType trans_;
    uint32_t cacheline_[512/4];

    // Registers:
    AttributeType listInstr_[INSTR_HASH_TABLE_SIZE];
    CpuContextType cpu_context_;

    enum EDebugState {
        STATE_Halted,
        STATE_Normal,
        STATE_Stepping
    } dbg_state_;
    uint64_t dbg_step_cnt_;

    char tstr[1024];
    uint64_t iregs_prev[32]; // to detect changes
    struct DebugPortType {
        bool valid;
        DebugPortTransactionType *trans;
        IDbgNbResponse *cb;

        // local registers
        uint64_t stepping_mode_steps;
    } dport;
};

#endif

class CpuRiver_Functional : public CpuGeneric,
                            public ICpuRiscV {
 public:
    explicit CpuRiver_Functional(const char *name);
    virtual ~CpuRiver_Functional();

    /** IService interface */
    virtual void postinitService();

    /** IResetListener itnterface */
    virtual void reset(bool active);

    /** ICpuGeneric interface */
    virtual void raiseSignal(int idx);
    virtual void lowerSignal(int idx);

    // Common River methods shared with instructions:
    uint64_t *getpRegs() { return portRegs_.getpR64(); }
    uint64_t readCSR(int idx);
    void writeCSR(int idx, uint64_t val);

 protected:
    /** CpuGeneric common methods */
    virtual bool isBreakpoint() { return false; }
    virtual GenericInstruction *decodeInstruction(Reg64Type *cache);
    virtual void generateIllegalOpcode();
    virtual void handleTrap();

    void addIsaUserRV64I();
    void addIsaPrivilegedRV64I();
    void addIsaExtensionA();
    void addIsaExtensionF();
    void addIsaExtensionM();
    unsigned addSupportedInstruction(RiscvInstruction *instr);
    uint32_t hash32(uint32_t val) { return (val >> 2) & 0x1f; }

 private:
    AttributeType listExtISA_;
    AttributeType vendorID_;
    AttributeType vectorTable_;

    static const int INSTR_HASH_TABLE_SIZE = 1 << 5;
    AttributeType listInstr_[INSTR_HASH_TABLE_SIZE];

    GenericReg64Bank portCSR_;
    GenericReg64Bank portRegs_;

    DsuMapType::udbg_type::debug_region_type::breakpoint_control_reg br_ctrl;
    bool br_status_ena;     // show breakpoint bit in common status register
    bool br_inject_fetch;
    uint64_t br_address_fetch;
    uint32_t br_instr_fetch;
};

DECLARE_CLASS(CpuRiver_Functional)

}  // namespace debugger

#endif  // __DEBUGGER_CPU_RISCV_FUNCTIONAL_H__
