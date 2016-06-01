/*
 * Copyright (c) 2016 Intel Corporation.
 * Copyright (c) 2013-2015 Wind River Systems, Inc.
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

extern IsrEntryType isr_table[CONFIG_NUM_IRQS];

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
    WRITE32(&__UART1->scaler, 260);
	return 0;
}

uint32_t READ32(volatile uint32_t *addr) {
#ifdef _WIN32
    uint32_t ret;
    LIBH_read((uint64_t)((size_t)addr), (uint8_t *)&ret, 4);
    return ret;
#else
    return *addr;
#endif
}

uint64_t READ64(volatile uint64_t *addr) {
#ifdef _WIN32
    uint64_t ret;
    LIBH_read((uint64_t)((size_t)addr), (uint8_t *)&ret, 8);
    return ret;
#else
    return *addr;
#endif
}

void WRITE32(volatile uint32_t *addr, uint32_t val) {
#ifdef _WIN32
    LIBH_write((uint64_t)((size_t)addr), (uint8_t *)&val, 4);
#else
    *addr = val;
#endif
}

void WRITE64(volatile uint64_t *addr, uint64_t val) {
#ifdef _WIN32
    LIBH_write((uint64_t)((size_t)addr), (uint8_t *)&val, 8);
#else
    *addr = val;
#endif
}

SYS_INIT(riscv_gnss_soc_init, PRIMARY, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT);
