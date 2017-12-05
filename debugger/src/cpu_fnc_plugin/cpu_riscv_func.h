/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      CPU functional simlator class definition.
 */

#ifndef __DEBUGGER_CPU_RISCV_FUNCTIONAL_H__
#define __DEBUGGER_CPU_RISCV_FUNCTIONAL_H__

#include <riscv-isa.h>
#include "instructions.h"
#include "generic/cpu_generic.h"
#include "coreservices/icpuriscv.h"
#include "coreservices/isocinfo.h"

namespace debugger {

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
    virtual EEndianessType endianess() { return LittleEndian; }
    virtual GenericInstruction *decodeInstruction(Reg64Type *cache);
    virtual void generateIllegalOpcode();
    virtual void handleTrap();
    /** Tack Registers changes during execution */
    virtual void trackContextStart();
    /** // Stop tracking and write trace file */
    virtual void trackContextEnd();

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

    GenericReg64Bank portRegs_;
    GenericReg64Bank portSavedRegs_;
    GenericReg64Bank portCSR_;
};

DECLARE_CLASS(CpuRiver_Functional)

}  // namespace debugger

#endif  // __DEBUGGER_CPU_RISCV_FUNCTIONAL_H__
