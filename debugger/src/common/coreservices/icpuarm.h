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
#pragma once

#include <iface.h>
#include <inttypes.h>
#include <arm-isa.h>

namespace debugger {

static const char *const IFACE_CPU_ARM = "ICpuArm";

class ICpuArm : public IFace {
 public:
    ICpuArm() : IFace(IFACE_CPU_ARM) {}

    virtual void setInstrMode(EArmInstructionModes mode) = 0;
    virtual EArmInstructionModes getInstrMode() = 0;

    /** Zero flag */
    virtual uint32_t getZ() = 0;
    virtual void setZ(uint32_t z) = 0;

    /** Unsigned higer or same (carry flag) */
    virtual uint32_t getC() = 0;
    virtual void setC(uint32_t c) = 0;

    /** Negative flag */
    virtual uint32_t getN() = 0;
    virtual void setN(uint32_t n) = 0;

    /** Overflow flag */
    virtual uint32_t getV() = 0;
    virtual void setV(uint32_t v) = 0;

    /** Impersize Data abort */
    virtual uint32_t getA() = 0;
    virtual void setA(uint32_t v) = 0;

    /** IRQ interrupt */
    virtual uint32_t getI() = 0;
    virtual void setI(uint32_t v) = 0;

    /** FIQ interrupt */
    virtual uint32_t getF() = 0;
    virtual void setF(uint32_t v) = 0;

    // DZ = CP15[19] Divide-by-zero (generate fault exception)
    virtual uint32_t getDZ() { return 0; }

    /** This function returns TRUE if execution is currently in an IT block 
        and FALSE otherwise. IT allows one of four following Thumb instructions
        (the IT block) to be conditional */
    virtual void StartITBlock(uint32_t firstcond, uint32_t mask) = 0;
    virtual bool InITBlock() = 0;
    virtual bool LastInITBlock() = 0;

    virtual void enterException(int idx) = 0;
    virtual void exitException(uint32_t npc) = 0;
};

}  // namespace debugger


