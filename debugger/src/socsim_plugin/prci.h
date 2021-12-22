/**
 *  Copyright 2021 Sergey Khabarov, sergeykhbr@gmail.com
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

#include <iclass.h>
#include <iservice.h>
#include "generic/mapreg.h"
#include "generic/rmembank_gen1.h"

namespace debugger {

class PRCI : public RegMemBankGeneric {
 public:
    explicit PRCI(const char *name);

    /** IService interface */
    virtual void postinitService() override;

 private:

    MappedReg32Type hfxosccfg;            // [0x00] Crystal Oscillator Configuraiton and Status
    MappedReg32Type core_pllcfg;          // [0x04] PLL Configuraiton and Status
    MappedReg32Type core_plloutdiv;       // [0x08] PLL Final Divide Configuration
    MappedReg32Type ddr_pllcfg;           // [0x0C] PLL config and status
    MappedReg32Type ddr_plloutdiv;        // [0x10] PLL Final Divide configuration
    MappedReg32Type gemgxl_pllcfg;        // [0x1C] PLL config and status
    MappedReg32Type gemgxl_plloutdiv;     // [0x20] PLL Final Divide configuration
    MappedReg32Type core_clk_sel_reg;     // [0x24] Select core clock source. 0=coreclkpll, 1=ext hfclk
    MappedReg32Type devices_reset_n;      // [0x28] Software controlled resets (active low)
    MappedReg32Type clk_mux_status;       // [0x2C] Current selection of each clock mux
    MappedReg32Type dvfs_core_pllcfg;     // [0x38] PLL Config and status
    MappedReg32Type dvfs_core_plloutdiv;  // [0x3C] PLL Final Divide Configuration
    MappedReg32Type corepllsel;           // [0x40] Select which PLL output to use for core clock. 0=corepll, 1=dvfscorepll
    MappedReg32Type hfpclk_pllcfg;        // [0x50] PLL Config and status
    MappedReg32Type hfpclk_plloutdiv;     // [0x54] PLL Final Divide Configuration
    MappedReg32Type hfpclkpllsel;         // [0x58] Select src for Periphery Clock (pclk).0=hfpclkpll, 1=ext hfclk
    MappedReg32Type hfpclk_div_reg;       // [0x5C] HFPCLK PLL divider value
    MappedReg32Type prci_plls;            // [0xe0] Indicated presence of each PLL
};

DECLARE_CLASS(PRCI)

}  // namespace debugger
