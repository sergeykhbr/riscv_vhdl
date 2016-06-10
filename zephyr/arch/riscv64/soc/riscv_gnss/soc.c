/*
 * Copyright (c) 2016, GNSS Sensor Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file
 * @brief System/hardware module for GNSS RISC-V family processor
 *
 * This module provides routines to initialize and support board-level hardware
 * for the GNSS RISC-V family processor.
 */

#include <nanokernel.h>
#include <device.h>
#include <init.h>
#include <soc.h>
#include <misc/printk.h>

extern IsrEntryType isr_table[CONFIG_NUM_IRQS];

const char *const VENDOR_NAME = "GNSS Sensor Ltd.";

const char *const EMPTY_ID_NAME = "Empty slot";

const char *const UNKOWN_ID_NAME = "Unknown";

static const char *const GNSS_SENSOR_DEVICE_NAMES[] = {
    "GNSS Engine stub",         // 0x68
    "Reserved",                 // 0x69
    "Reserved",                 // 0x6a
    "Reserved",                 // 0x6b
    "Reserved",                 // 0x6c
    "Reserved",                 // 0x6d
    "Reserved",                 // 0x6e
    "Reserved",                 // 0x6f
    "Reserved",                 // 0x70
    "Boot ROM",                 // 0x71
    "FW Image ROM",             // 0x72
    "Internal SRAM",            // 0x73
    "Plug'n'Play support",      // 0x74
    "SD Controller",            // 0x75
    "Generic GPIO",             // 0x76
    "RF front-end controller",  // 0x77
    "GNSS Engine",              // 0x78
    "GPS FSE",                  // 0x79
    "Generic UART",             // 0x7a
    "Accelerometer",            // 0x7b
    "Gyroscope",                // 0x7c
    "Interrupt Controller",     // 0x7d
    "Reserved",                 // 0x7e
    "Ethernet MAC",             // 0x7f
    "Debug Support Unit (DSU)", // 0x80
    "GP Timers"                 // 0x81
};

/**
 * @brief Get technology name
 */
const char *const get_tech_name(uint32_t tech)
{
    switch (tech) {
    case TECH_INFERRED: return "inferred";
    case TECH_VIRTEX6: return "Virtex6";
    case TECH_KINTEX7: return "Kintex7";
    default:;
    }
    return "unknown";
}

/**
 * @brief Get device Vendor name by its ID
 */
static const char *get_vendor_name(uint16_t vid)
{
    if (vid != VENDOR_GNSSSENSOR) {
        return UNKOWN_ID_NAME;
    }
    return VENDOR_NAME;
}

/**
 * @brief Get device Name by Vendor ID and Device ID
 */
static const char *get_device_name(uint16_t vid, uint16_t did)
{
    if (vid != VENDOR_GNSSSENSOR) {
        return UNKOWN_ID_NAME;
    }
    if (did == GNSSSENSOR_EMPTY) {
        return EMPTY_ID_NAME;
    }
    if (did < GNSSSENSOR_ENGINE_STUB || did > GNSSSENSOR_GPTIMERS) {
        return UNKOWN_ID_NAME;
    }
    return GNSS_SENSOR_DEVICE_NAMES[did - GNSSSENSOR_ENGINE_STUB];
}

/**
 * @brief Print Plug'n'Play information
 *
 * This function reads information from the PNP slave device that is mapped
 * into hardcoded address __PNP (0xFFFFF000).
 */
void soc_print_pnp()
{
    uint32_t slaves_total, tech, hwid;
    uint16_t vid, did;
    adr_type xaddr, xmask, xsize;

    tech = READ32(&__PNP->tech);
    slaves_total = (tech >> 8) & 0xff;
    printk("# RISC-V:  Rocket-Chip demonstration design\n");
    hwid = READ32(&__PNP->hwid);
    printk("# HW id:   0x%x\n", hwid);
    hwid = READ32(&__PNP->fwid);
    printk("# FW id:   0x%x\n", hwid);
    printk("# Target technology: %s\n", get_tech_name(tech & 0xFF));

    for (uint32_t i = 0; i < slaves_total; i++) {
        vid = READ16(&__PNP->slaves[i].vid);
        did = READ16(&__PNP->slaves[i].did);
        xaddr = READ32(&__PNP->slaves[i].xaddr);
        xmask = READ32(&__PNP->slaves[i].xmask);
        xmask ^= 0xFFFFFFFF;
        xsize = xmask + 1;

        printk("# AXI4: slv%d: %s    %s\n", i, 
            get_vendor_name(vid), get_device_name(vid, did));

        printk("#    %x...%x, size = ",
            (unsigned long)xaddr, (unsigned long)(xaddr + xmask));
        if (xsize < 1024) {
            printk("%d bytes\n", (int)xsize);
        } else if (xsize < 1024*1024) {
            printk("%d KB\n", (int)(xsize >> 10));
        } else {
            printk("%d MB\n", (int)(xsize >> 20));
        }
    }
}

/**
 *
 * @brief perform basic hardware initialization
 *
 * Hardware initialized:
 * - interrupt unit
 *
 * RETURNS: N/A
 */
static int riscv_gnss_soc_init(struct device *arg)
{
	ARG_UNUSED(arg);
    WRITE64(&__IRQCTRL->isr_table, (uint64_t)isr_table);
    WRITE32(&__IRQCTRL->irq_lock, 0);
	return 0;
}

/**
 * @brief Check hardware target configuration is it inferred or not.
 *
 * inferred hardware target is used for RTL simulation of the whole SOC design.
 */
uint32_t soc_is_rtl_simulation()
{
    uint32_t tech = READ32(&__PNP->tech);
    return (tech & 0xff) == TECH_INFERRED ? 1: 0;
}

SYS_INIT(riscv_gnss_soc_init, PRIMARY, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT);

