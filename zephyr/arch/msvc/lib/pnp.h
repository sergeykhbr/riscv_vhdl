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
#pragma once

#include "imapdev.h"
#include <memory>

class PNP : public IMappedDevice {
public:
    PNP();

    virtual bool isAddrValid(uint64_t addr) {
        return (addr >= 0x0FFFFF000ull && addr < 0x100000000ull);
    }
    virtual void write(uint64_t addr, uint8_t *buf, int size) {
        uint64_t off = addr - 0xFFFFF000;
        memcpy(reinterpret_cast<char *>(&map_) + off, buf, size);
    }
    virtual void read(uint64_t addr, uint8_t *buf, int size) {
        uint64_t off = addr - 0xFFFFF000;
        memcpy(buf, reinterpret_cast<char *>(&map_) + off, size);
    }


private:
    static const int PNP_CONFIG_DEFAULT_BYTES = 16;

    typedef struct PnpConfigType {
        uint32_t xmask;
        uint32_t xaddr;
        uint16_t did;
        uint16_t vid;
        uint8_t size;
        uint8_t rsrv[3];
    } PnpConfigType;

    typedef struct pnp_map {
        volatile uint32_t hwid;         /// Read only HW ID
        volatile uint32_t fwid;         /// Read/Write Firmware ID
        volatile uint32_t tech;         /// Read only technology index
        volatile uint32_t rsrv1;        /// 
        volatile uint64_t idt;          /// 
        volatile uint64_t malloc_addr;  /// debuggind memalloc pointer
        volatile uint64_t malloc_size;  /// debugging memalloc size
        volatile uint64_t fwdbg1;       /// FW debug register
        volatile uint64_t rsrv[2];
        PnpConfigType slaves[64];
    } pnp_map;
    pnp_map map_;
};

