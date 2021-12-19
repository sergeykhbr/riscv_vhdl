/*
 *  Copyright 2019 Sergey Khabarov, sergeykhbr@gmail.com
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

#include <inttypes.h>
#include <string.h>
#include <axi_maps.h>
#include "fw_api.h"

#define WAS_GNSS_ISR_MARKER 0xcafe0001ull

void isr_gnss_ss(void) {
    pnp_map *pnp = (pnp_map *)ADDR_BUS0_XSLV_PNP;
    pnp->fwdbg1 = WAS_GNSS_ISR_MARKER;
}

void test_gnss_ss(uint64_t bar) {
    pnp_map *pnp = (pnp_map *)ADDR_BUS0_XSLV_PNP;
    rfctrl_map *rf = (rfctrl_map *)(bar + 0*0x1000);
    GnssEngine_map *gnss = (GnssEngine_map *)(bar + 1*0x1000);
    fsev2_map *fse = (fsev2_map *)(bar + 2*0x1000);

    register_ext_interrupt_handler(CFG_IRQ_GNSS_SS, isr_gnss_ss);
    fw_enable_plic_irq(CTX_CPU0_M_MODE, CFG_IRQ_GNSS_SS);

    // rf controller
    pnp->fwdbg1 = rf->subsystem_config;
    rf->conf1 = 0x34;

    // engine access
    pnp->fwdbg1 = 0;
    gnss->tmr.rw_MsLength = 2000;

    int t1 = 0x00000800;
    asm("csrs mie, %0" : :"r"(t1));  // enable external irq from PLIC

    while (pnp->fwdbg1 != WAS_GNSS_ISR_MARKER) {}

    fw_disable_plic_irq(CTX_CPU0_M_MODE, CFG_IRQ_GNSS_SS);
    asm("csrc mie, %0" : :"r"(t1));  // disable external irq from PLIC

    pnp->fwdbg1 = gnss->tmr.rw_tow;

    pnp->fwdbg1 = gnss->misc.GenericChanCfg;
    gnss->chn[0].u.w.CodeNcoTh = 1023*40000;

    // fse gps
    pnp->fwdbg1 = fse->hw_id;
    fse->chan[0].carr_nco_f0 = 555;
    fse->carr_nco_th = 0xFFEE;
}
