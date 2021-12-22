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

#include "api_core.h"
#include "prci.h"

namespace debugger {

PRCI::PRCI(const char *name) : RegMemBankGeneric(name),
    hfxosccfg(static_cast<IService *>(this), "hfxosccfg", 0x00),
    core_pllcfg(static_cast<IService *>(this), "core_pllcfg", 0x04),
    core_plloutdiv(static_cast<IService *>(this), "core_plloutdiv", 0x08),
    ddr_pllcfg(static_cast<IService *>(this), "ddr_pllcfg", 0x0C),
    ddr_plloutdiv(static_cast<IService *>(this), "ddr_plloutdiv", 0x10),
    gemgxl_pllcfg(static_cast<IService *>(this), "gemgxl_pllcfg", 0x1C),
    gemgxl_plloutdiv(static_cast<IService *>(this), "gemgxl_plloutdiv", 0x20),
    core_clk_sel_reg(static_cast<IService *>(this), "core_clk_sel_reg", 0x24),
    devices_reset_n(static_cast<IService *>(this), "devices_reset_n", 0x28),
    clk_mux_status(static_cast<IService *>(this), "clk_mux_status", 0x2C),
    dvfs_core_pllcfg(static_cast<IService *>(this), "dvfs_core_pllcfg", 0x38),
    dvfs_core_plloutdiv(static_cast<IService *>(this), "dvfs_core_plloutdiv", 0x3C),
    corepllsel(static_cast<IService *>(this), "corepllsel", 0x40),
    hfpclk_pllcfg(static_cast<IService *>(this), "hfpclk_pllcfg", 0x50),
    hfpclk_plloutdiv(static_cast<IService *>(this), "hfpclk_plloutdiv", 0x54),
    hfpclkpllsel(static_cast<IService *>(this), "hfpclkpllsel", 0x58),
    hfpclk_div_reg(static_cast<IService *>(this), "hfpclk_div_reg", 0x5C),
    prci_plls(static_cast<IService *>(this), "prci_plls", 0xe0) {
}

void PRCI::postinitService() {
    RegMemBankGeneric::postinitService();
}

}  // namespace debugger

