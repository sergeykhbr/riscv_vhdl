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
#include "pnp.h"

#define VENDOR_GNSSSENSOR        0x00F1

#define GNSSSENSOR_EMPTY         0x5577     /// Dummy device
#define GNSSSENSOR_BOOTROM       0x0071     /// Boot ROM Device ID
#define GNSSSENSOR_FWIMAGE       0x0072     /// FW ROM image Device ID
#define GNSSSENSOR_SRAM          0x0073     /// Internal SRAM block Device ID
#define GNSSSENSOR_PNP           0x0074     /// Configuration Registers Module Device ID provided by gnsslib
#define GNSSSENSOR_SPI_FLASH     0x0075     /// SD-card controller Device ID provided by gnsslib
#define GNSSSENSOR_GPIO          0x0076     /// General purpose IOs Device ID provided by gnsslib
#define GNSSSENSOR_RF_CONTROL    0x0077     /// RF front-end controller Device ID provided by gnsslib
#define GNSSSENSOR_ENGINE        0x0078     /// GNSS Engine Device ID provided by gnsslib
#define GNSSSENSOR_ENGINE_STUB   0x0068     /// GNSS Engine stub
#define GNSSSENSOR_FSE_V2        0x0079     /// Fast Search Engines Device ID provided by gnsslib
#define GNSSSENSOR_UART          0x007a     /// rs-232 UART Device ID
#define GNSSSENSOR_ACCELEROMETER 0x007b     /// Accelerometer Device ID provided by gnsslib
#define GNSSSENSOR_GYROSCOPE     0x007c     /// Gyroscope Device ID provided by gnsslib
#define GNSSSENSOR_IRQCTRL       0x007d     /// Interrupt controller
#define GNSSSENSOR_ETHMAC        0x007f
#define GNSSSENSOR_DSU           0x0080
#define GNSSSENSOR_GPTIMERS      0x0081


#define TECH_INFERRED       0
#define TECH_VIRTEX6        36
#define TECH_KINTEX7        49

#define ADDR_NASTI_SLAVE_FWIMAGE    0x00100000
#define ADDR_NASTI_SLAVE_SRAM       0x10000000
#define ADDR_NASTI_SLAVE_GPIO       0x80000000
#define ADDR_NASTI_SLAVE_UART1      0x80001000
#define ADDR_NASTI_SLAVE_IRQCTRL    0x80002000
#define ADDR_NASTI_SLAVE_GNSSENGINE 0x80003000
#define ADDR_NASTI_SLAVE_RFCTRL     0x80004000
#define ADDR_NASTI_SLAVE_GPTIMERS   0x80005000
#define ADDR_NASTI_SLAVE_FSEGPS     0x8000a000
#define ADDR_NASTI_SLAVE_ETHMAC     0x80040000
#define ADDR_NASTI_SLAVE_PNP        0xfffff000


PNP::PNP() {
        map_.hwid = 0x20151217LL;
        map_.rsrv1 =  0;
        map_.idt = 0;

        int idx = 0;
        map_.slaves[idx].xmask = 0xffffe000;
        map_.slaves[idx].xaddr = 0;
        map_.slaves[idx].did = GNSSSENSOR_BOOTROM;
        map_.slaves[idx].vid = VENDOR_GNSSSENSOR;
        map_.slaves[idx].size = PNP_CONFIG_DEFAULT_BYTES;

        idx++;
        map_.slaves[idx].xmask = 0xfffc0000;
        map_.slaves[idx].xaddr = 0x00100000;
        map_.slaves[idx].did = GNSSSENSOR_FWIMAGE;
        map_.slaves[idx].vid = VENDOR_GNSSSENSOR;
        map_.slaves[idx].size = PNP_CONFIG_DEFAULT_BYTES;

        idx++;
        map_.slaves[idx].xmask = 0xfff80000;
        map_.slaves[idx].xaddr = 0x10000000;
        map_.slaves[idx].did = GNSSSENSOR_SRAM;
        map_.slaves[idx].vid = VENDOR_GNSSSENSOR;
        map_.slaves[idx].size = PNP_CONFIG_DEFAULT_BYTES;

        idx++;
        map_.slaves[idx].xmask = 0xfffff000;
        map_.slaves[idx].xaddr = ADDR_NASTI_SLAVE_GPIO;
        map_.slaves[idx].did = GNSSSENSOR_GPIO;
        map_.slaves[idx].vid = VENDOR_GNSSSENSOR;
        map_.slaves[idx].size = PNP_CONFIG_DEFAULT_BYTES;

        idx++;
        map_.slaves[idx].xmask = 0xfffff000;
        map_.slaves[idx].xaddr = ADDR_NASTI_SLAVE_UART1;
        map_.slaves[idx].did = GNSSSENSOR_UART;
        map_.slaves[idx].vid = VENDOR_GNSSSENSOR;
        map_.slaves[idx].size = PNP_CONFIG_DEFAULT_BYTES;

        idx++;
        map_.slaves[idx].xmask = 0xfffff000;
        map_.slaves[idx].xaddr = ADDR_NASTI_SLAVE_IRQCTRL;
        map_.slaves[idx].did = GNSSSENSOR_IRQCTRL;
        map_.slaves[idx].vid = VENDOR_GNSSSENSOR;
        map_.slaves[idx].size = PNP_CONFIG_DEFAULT_BYTES;

        idx++;
        map_.slaves[idx].xmask = 0xfffff000;
        map_.slaves[idx].xaddr = ADDR_NASTI_SLAVE_GNSSENGINE;
        map_.slaves[idx].did = GNSSSENSOR_ENGINE_STUB;
        map_.slaves[idx].vid = VENDOR_GNSSSENSOR;
        map_.slaves[idx].size = PNP_CONFIG_DEFAULT_BYTES;

        idx++;
        map_.slaves[idx].xmask = 0xfffff000;
        map_.slaves[idx].xaddr = ADDR_NASTI_SLAVE_GPTIMERS;
        map_.slaves[idx].did = GNSSSENSOR_GPTIMERS;
        map_.slaves[idx].vid = VENDOR_GNSSSENSOR;
        map_.slaves[idx].size = PNP_CONFIG_DEFAULT_BYTES;

        idx++;
        map_.slaves[idx].xmask = 0xfffff000;
        map_.slaves[idx].xaddr = ADDR_NASTI_SLAVE_RFCTRL;
        map_.slaves[idx].did = GNSSSENSOR_RF_CONTROL;
        map_.slaves[idx].vid = VENDOR_GNSSSENSOR;
        map_.slaves[idx].size = PNP_CONFIG_DEFAULT_BYTES;

#ifdef ENABLE_FSE
        idx++;
        map_.slaves[idx].xmask = 0xfffff000;
        map_.slaves[idx].xaddr = ADDR_NASTI_SLAVE_FSEGPS;
        map_.slaves[idx].did = GNSSSENSOR_FSE_V2;
        map_.slaves[idx].vid = VENDOR_GNSSSENSOR;
        map_.slaves[idx].size = PNP_CONFIG_DEFAULT_BYTES;
#endif

#ifdef ENABLE_MAC
        idx++;
        map_.slaves[idx].xmask = 0xfffc0000;
        map_.slaves[idx].xaddr = 0x80040000;
        map_.slaves[idx].did = GNSSSENSOR_ETHMAC;
        map_.slaves[idx].vid = VENDOR_GNSSSENSOR;
        map_.slaves[idx].size = PNP_CONFIG_DEFAULT_BYTES;
#endif

        map_.tech =  ((idx + 1) << 8) | (TECH_VIRTEX6);
}