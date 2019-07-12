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

#include <api_core.h>
#include "icache_func.h"

namespace debugger {

ICacheFunctional::ICacheFunctional(const char *name) : IService(name),
    ICommand(name, 0) {
    registerInterface(static_cast<IMemoryOperation *>(this));
    registerAttribute("SysBus", &sysBus_);
    registerAttribute("CmdExecutor", &cmdexec_);
    registerAttribute("TotalKBytes", &totalKBytes_);
    registerAttribute("LineBytes", &lineBytes_);
    registerAttribute("Ways", &ways_);
}

ICacheFunctional::~ICacheFunctional() {
}

void ICacheFunctional::postinitService() {
    isysbus_ = static_cast<IMemoryOperation *>(
        RISCV_get_service_iface(sysBus_.to_string(), IFACE_MEMORY_OPERATION));
    if (!isysbus_) {
        RISCV_error("System Bus interface '%s' not found",
                    sysBus_.to_string());
        return;
    }

    icmdexec_ = static_cast<ICmdExecutor *>(
        RISCV_get_service_iface(cmdexec_.to_string(), IFACE_CMD_EXECUTOR));
    if (!icmdexec_) {
        RISCV_error("ICmdExecutor interface '%s' not found", 
                    cmdexec_.to_string());
    } else {
        icmdexec_->registerCommand(static_cast<ICommand *>(this));
    }
}

void ICacheFunctional::predeleteService() {
    if (icmdexec_) {
        icmdexec_->unregisterCommand(static_cast<ICommand *>(this));
    }
}

ETransStatus ICacheFunctional::b_transport(Axi4TransactionType *trans) {
    uint32_t rcache;

    int hit = getCachedValue(trans->addr, rcache);
    if (hit == HIT_BOTH) {
        trans->rpayload.b64[0] = rcache;
        return TRANS_OK;
    }

    int lru = 0;
    WayMemType *waysel;
    if ((hit & HIT_WORD0) == 0) {
        if (getAdrOddEven(trans->addr) == WAY_EVEN) {
            waysel = &wayEven_[lru];
        } else {
            waysel = &wayOdd_[lru];
        }
        readLine(trans->addr, waysel);
    }

    uint64_t overlay_addr = trans->addr + 2;
    if (getAdrLine(trans->addr) != getAdrLine(overlay_addr)
        && (hit & HIT_WORD1) == 0) {
        if (getAdrOddEven(trans->addr) == WAY_EVEN) {
            waysel = &wayOdd_[lru];
        } else {
            waysel = &wayEven_[lru];
        }
        readLine(overlay_addr, waysel);
    }

    hit = getCachedValue(trans->addr, rcache);
    trans->rpayload.b64[0] = rcache;
    if (hit != HIT_BOTH) {
        RISCV_error("Something wrong");
    }
    return TRANS_OK;
}

int ICacheFunctional::getCachedValue(uint64_t addr, uint32_t &rdata) {
    uint64_t line_adr = getAdrLine(addr);
    uint64_t tag = getAdrTag(addr);

    EWays useWay = static_cast<EWays>(getAdrOddEven(addr));
    bool useOverlay = false;
    if ((addr & 0x1E) == 0x1E) {
        useOverlay = true;
    }

    uint64_t swapped_index[WAY_SubNum];
    uint64_t swapped_offset[WAY_SubNum];
    if (useWay == WAY_EVEN) {
        swapped_index[WAY_EVEN]  = getAdrIndex(addr);
        swapped_offset[WAY_EVEN] = getAdrOffset(addr);
        swapped_index[WAY_ODD]   = (getAdrIndex(addr) + 1) % LINES_PER_WAY;
        swapped_offset[WAY_ODD]  = 0;
    } else {
        swapped_index[WAY_EVEN]  = (getAdrIndex(addr) + 1) % LINES_PER_WAY;
        swapped_offset[WAY_EVEN] = 0;
        swapped_index[WAY_ODD]   = getAdrIndex(addr);
        swapped_offset[WAY_ODD]  = getAdrOffset(addr);
    }

    uint64_t cachetag[WAY_SubNum];
    uint32_t cacheval[WAY_SubNum];
    unsigned hit = 0;
    rdata = 0;

    for (unsigned n = 0; n < ICACHE_WAYS; n++) {
        wayEven_[n].readTagValue(swapped_index[WAY_EVEN],
                                 swapped_offset[WAY_EVEN],
                                 cachetag[WAY_EVEN],
                                 cacheval[WAY_EVEN]);

        wayOdd_[n].readTagValue(swapped_index[WAY_ODD],
                                swapped_offset[WAY_ODD],
                                cachetag[WAY_ODD],
                                cacheval[WAY_ODD]);


        if (useWay == WAY_EVEN) {
            if (tag == cachetag[WAY_EVEN]) {
                hit |= HIT_WORD0;
                rdata = cacheval[WAY_EVEN];
                if (!useOverlay) {
                    hit |= HIT_WORD1;
                } else if (tag == cachetag[WAY_ODD]) {
                    hit |= HIT_WORD1;
                    rdata &= ~0xFFFF0000;
                    rdata |= (cacheval[WAY_ODD] << 16);
                }
            }
        } else {
            if (tag == cachetag[WAY_ODD]) {
                hit |= HIT_WORD0;
                rdata = cacheval[WAY_ODD];
                if (!useOverlay) {
                    hit |= HIT_WORD1;
                } else if (tag == cachetag[WAY_EVEN]) {
                    hit |= HIT_WORD1;
                    rdata &= ~0xFFFF0000;
                    rdata |= (cacheval[WAY_EVEN] << 16);
                }
            }
        }
    }
    return hit;
}

void ICacheFunctional::readLine(uint64_t adr, WayMemType *way) {
    Axi4TransactionType tr;
    uint32_t index = getAdrIndex(adr);
    uint64_t tag = getAdrTag(adr);
    int burst_steps = ICACHE_LINE_BYTES / PAYLOAD_MAX_BYTES;
    int wstrb = 0x1;
    tr.action = MemAction_Read;
    tr.xsize = PAYLOAD_MAX_BYTES;
    tr.addr = adr & ~(ICACHE_LINE_BYTES - 1);
    tr.wstrb = 0;
    for (int burst = 0; burst < burst_steps; burst ++) {
        isysbus_->b_transport(&tr);
        way->writeLine(index, tag, wstrb, tr.rpayload.b64[0]);

        wstrb <<= 1;
        tr.addr += PAYLOAD_MAX_BYTES;
    }
}

uint64_t ICacheFunctional::getAdrLine(uint64_t adr) {
    uint64_t mask = (1ull << (BUS_ADDR_WIDTH)) - 1;
    uint64_t t = (adr & mask) >> OFFSET_WIDTH;
    return t << OFFSET_WIDTH;
}

// addr[31:14]
uint64_t ICacheFunctional::getAdrTag(uint64_t adr) {
    uint64_t mask = (1ull << (BUS_ADDR_WIDTH)) - 1;
    uint64_t t = (adr & mask) >> (OFFSET_WIDTH + ODDEVEN_WIDTH + INDEX_WIDTH);
    return t;
}

// add[13:6]
uint32_t ICacheFunctional::getAdrIndex(uint64_t adr) {
    uint64_t mask = (1ull << (OFFSET_WIDTH + ODDEVEN_WIDTH + INDEX_WIDTH)) - 1;
    uint64_t t = (adr & mask) >> (OFFSET_WIDTH + ODDEVEN_WIDTH);
    return static_cast<uint32_t>(t);
}

// addr[5]
uint64_t ICacheFunctional::getAdrOddEven(uint64_t adr) {
    uint64_t t = (adr >> OFFSET_WIDTH) & 0x1;
    return t;
}

// addr[4:0]
uint32_t ICacheFunctional::getAdrOffset(uint64_t adr) {
    uint64_t mask = (1ull << OFFSET_WIDTH) - 1;
    uint64_t t = adr & mask;
    return static_cast<uint32_t>(t);
}

int ICacheFunctional::isValid(AttributeType *args) {
    if (!cmdName_.is_equal((*args)[0u].to_string())) {
        return CMD_INVALID;
    }
    if (args->size() == 2 && (*args)[1].is_string()) {
        return CMD_VALID;
    }
    return CMD_WRONG_ARGS;
}

void ICacheFunctional::exec(AttributeType *args, AttributeType *res) {
    if ((*args)[1].is_equal("test")) {
        runTest();
    }
}

void ICacheFunctional::runTest() {
    Axi4TransactionType tr;
    tr.action = MemAction_Read;
    tr.source_idx = 0;
    tr.wstrb = 0;
    tr.xsize = 4;
    for (unsigned i = 0; i < 100; i++) {
        tr.addr = 0x1A + (i << 1);     // 2-bytes alignment (uint16_t)
        b_transport(&tr);
    }
}

}  // namespace debugger
