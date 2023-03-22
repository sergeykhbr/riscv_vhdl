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
 */

#ifndef __DEBUGGER_CPU_ARM7_FUNCTIONAL_H__
#define __DEBUGGER_CPU_ARM7_FUNCTIONAL_H__

#include "arm-isa.h"
#include "instructions.h"
#include "generic/cpu_generic.h"
#include "coreservices/icpuarm.h"
#include "cmds/cmd_br_arm7.h"
#include "cmds/cmd_reg_arm7.h"
#include "cmds/cmd_regs_arm7.h"

namespace debugger {

class CpuCortex_Functional : public CpuGeneric,
                             public ICpuArm {
 public:
     explicit CpuCortex_Functional(const char *name);
     virtual ~CpuCortex_Functional();

    /** IService interface */
    virtual void postinitService();
    virtual void predeleteService();

    /** IHap */
    virtual void hapTriggered(EHapType type, uint64_t param,
                              const char *descr);

    /** IResetListener interface */
    virtual void reset(IFace *isource);

    /** ICpuGeneric interface */
    virtual void raiseSignal(int idx);
    virtual void lowerSignal(int idx);
    virtual void raiseSoftwareIrq();
    virtual uint64_t getIrqAddress(int idx) { return 0; }

    /** ICpuArm */
    virtual void setInstrMode(EArmInstructionModes mode) {
        const uint32_t MODE[ArmInstrModes_Total] = {0u, 1u};
        p_psr_->u.T = MODE[mode];
    }
    virtual EArmInstructionModes getInstrMode() {
        const EArmInstructionModes MODE[2] = {ARM_mode, THUMB_mode};
        return MODE[p_psr_->u.T];
    }
    virtual uint32_t getZ() { return p_psr_->u.Z; }
    virtual void setZ(uint32_t z) { p_psr_->u.Z = z; }
    virtual uint32_t getC() { return p_psr_->u.C; }
    virtual void setC(uint32_t c) { p_psr_->u.C = c; }
    virtual uint32_t getN() { return p_psr_->u.N; }
    virtual void setN(uint32_t n) { p_psr_->u.N = n; }
    virtual uint32_t getV() { return p_psr_->u.V; }
    virtual void setV(uint32_t v) { p_psr_->u.V = v; }
    virtual uint32_t getA() { return p_psr_->u.A; }
    virtual void setA(uint32_t v) { p_psr_->u.A = v; }
    virtual uint32_t getI() { return p_psr_->u.I; }
    virtual void setI(uint32_t v) { p_psr_->u.I = v; }
    virtual uint32_t getF() { return p_psr_->u.F; }
    virtual void setF(uint32_t v) { p_psr_->u.F = v; }

    // DZ = CP15[19] Divide-by-zero (generate fault exception)
    virtual uint32_t getDZ() { return 0; }

    virtual void StartITBlock(uint32_t firstcond, uint32_t mask);
    virtual bool InITBlock() { return ITBlockMask_ != 0; }
    virtual bool LastInITBlock() { return (ITBlockMask_ & 0xF) == 0x8; }
    virtual uint32_t ITBlockCondition() { return ITBlockCondition_; }

    virtual void enterException(int idx);
    virtual void exitException(uint32_t exc_return);

 protected:
    /** CpuGeneric common methods */
    virtual uint64_t getResetAddress();
    virtual EEndianessType endianess() { return LittleEndian; }
    virtual GenericInstruction *decodeInstruction(Reg64Type *cache);
    virtual void generateIllegalOpcode();
    virtual void handleException(int e) {}
    virtual void handleInterrupts() {}
    virtual void trackContextEnd() override;
    virtual void traceOutput() override;
    
    void addArm7tmdiIsa();
    void addThumb2Isa();
    unsigned addSupportedInstruction(ArmInstruction *instr);
    uint32_t hash32(uint32_t val) { return (val >> 24) & 0xf; }

 private:
    AttributeType defaultMode_;
    AttributeType vendorID_;
    AttributeType vectorTable_;

    static const int INSTR_HASH_TABLE_SIZE = 1 << 4;
    AttributeType listInstr_[INSTR_HASH_TABLE_SIZE];
    GenericInstruction *isaTableArmV7_[ARMV7_Total];

    ProgramStatusRegsiterType *p_psr_;

    char errmsg_[256];

    //CmdBrArm *pcmd_br_;
    //CmdRegArm *pcmd_reg_;
    //CmdRegsArm *pcmd_regs_;

    // CPSR contains fields IT[7:0]
    //     IT[7:5] = cond_base, when IT Block enabled, 4'b0000 otherwise
    //     IT[4:0] = ITBlockMask_
    bool ITBlockEnabled;
    uint32_t ITBlockCondition_ : 4;
    uint32_t ITBlockBaseCond_ : 4;
    uint32_t ITBlockMask_ : 5;
};

DECLARE_CLASS(CpuCortex_Functional)

}  // namespace debugger

#endif  // __DEBUGGER_CPU_RISCV_FUNCTIONAL_H__
