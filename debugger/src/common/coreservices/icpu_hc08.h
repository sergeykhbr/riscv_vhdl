/**
 * @file
 * @copyright  Copyright 2017 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      CPU HC08 specific interface.
 */

#ifndef __DEBUGGER_COMMON_CORESERVICES_ICPU_HC08_H__
#define __DEBUGGER_COMMON_CORESERVICES_ICPU_HC08_H__

#include <inttypes.h>
#include <iface.h>

namespace debugger {

static const char *const IFACE_CPU_HC08 = "ICpuHC08";

enum ERegNames {
    Reg_A,          // 0
    Reg_HX,         // 1
    Reg_SP,         // 2
    Reg_CCR,        // 3
    Reg_PPAGE,      // 4
    Reg_ClkHz,      // 5
    Reg_rsrv6,      // 6
    Reg_rsrv7,
    Reg_rsrv8,
    Reg_rsrv9,
    Reg_rsrv10,
    Reg_rsrv11,
    Reg_rsrv12,
    Reg_rsrv13,
    Reg_rsrv14,
    Reg_rsrv15,
    Reg_rsrv16,
    Reg_rsrv17,
    Reg_rsrv18,
    Reg_rsrv19,
    Reg_rsrv20,
    Reg_rsrv21,
    Reg_rsrv22,
    Reg_rsrv23,
    Reg_rsrv24,
    Reg_rsrv25,
    Reg_rsrv26,
    Reg_rsrv27,
    Reg_rsrv28,
    Reg_rsrv29,
    Reg_rsrv30,
    Reg_rsrv31,
    Reg_Total
};

/** Signal types */
enum EResetType {
    RESET_Unused0,
    RESET_LVI,      // Low-voltage inhibit Reset Bit
    RESET_Unused2,
    RESET_ILAD,     // Illegal Address Reset Bit
    RESET_ILOP,     // Illegal Opcode Reset Bit
    RESET_COP,      // Compute Operating Properly Reset Bit
    RESET_PIN,      // External Reset Bit (nRST)
    RESET_PON,      // POWER-on Reset Bit
};

class ICpuHC08 : public IFace {
 public:
    ICpuHC08() : IFace(IFACE_CPU_HC08) {}

    /** Fast access to memory mapped registers */
    virtual Reg64Type *getpRegs() = 0;

    /** External IRQ line status (need for BIH, BIL instructions) */
    virtual bool getIRQ() = 0;

    /** Update COP watchdog settings */
    virtual void updateCOP() = 0;

    /** Reset sequence has ben writen */
    virtual void resetCOP() = 0;
    virtual void vectorUpdated() = 0;
};

}  // namespace debugger

#endif  // __DEBUGGER_COMMON_CORESERVICES_ICPU_HC08_H__
