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
    ICommand(this, name) {
    registerInterface(static_cast<IMemoryOperation *>(this));
    registerAttribute("SysBus", &sysBus_);
    registerAttribute("CmdExecutor", &cmdexec_);
    registerAttribute("TotalKBytes", &totalKBytes_);
    registerAttribute("LineBytes", &lineBytes_);
    registerAttribute("Ways", &ways_);

    memset(tblLruEven_.row, 0xe4, sizeof(tblLruEven_.row)); // 0x3, 0x2, 0x1, 0x0
    memset(tblLruOdd_.row, 0xe4, sizeof(tblLruOdd_.row)); // 0x3, 0x2, 0x1, 0x0
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
    Reg64Type rcache;
    uint8_t hitidx[2];
    Axi4TransactionType ref;

    ref = *trans;
    isysbus_->b_transport(&ref);

    uint64_t addr5 = getAdrOddEven(trans->addr);

    getCachedValue(trans->addr, rcache, hitidx);

    int lru = 0;
    uint32_t index;
    if (hitidx[0] == ICACHE_WAYS) {  // cache miss
        index = getAdrIndex(trans->addr);
        if (addr5 == 0) {
            lru = tblLruEven_.row[index].n0;
            tblLruEven_.buf[index] >>= 2;
            tblLruEven_.row[index].n3 = lru;
            readLine(trans->addr, &wayEven_[lru]);
        } else {
            lru = tblLruOdd_.row[index].n0;
            tblLruOdd_.buf[index] >>= 2;
            tblLruOdd_.row[index].n3 = lru;
            readLine(trans->addr, &wayOdd_[lru]);
        }
    }

    uint64_t overlay_addr = trans->addr + 2;
    if (hitidx[1] == ICACHE_WAYS
        && getAdrLine(trans->addr) != getAdrLine(overlay_addr)) {
        index = getAdrIndex(overlay_addr);
        if (addr5 == 1) {
            lru = tblLruEven_.row[index].n0;
            tblLruEven_.buf[index] >>= 2;
            tblLruEven_.row[index].n3 = lru;
            readLine(overlay_addr, &wayEven_[lru]);
        } else {
            lru = tblLruOdd_.row[index].n0;
            tblLruOdd_.buf[index] >>= 2;
            tblLruOdd_.row[index].n3 = lru;
            readLine(overlay_addr, &wayOdd_[lru]);
        }
    }

    getCachedValue(trans->addr, rcache, hitidx);
    trans->rpayload.b16[0] = rcache.buf16[0];
    trans->rpayload.b16[1] = rcache.buf16[1];
    if (hitidx[0] == ICACHE_WAYS || hitidx[1] == ICACHE_WAYS) {
        RISCV_error("Something wrong: [%08" RV_PRI64 "x] not cached",
                     trans->addr);
    }
    if (ref.rpayload.b32[0] != trans->rpayload.b32[0]) {
        RISCV_error("Wrong caching data at [%08" RV_PRI64 "x]",
                    trans->addr);
    }
    return TRANS_OK;
}

void ICacheFunctional::getCachedValue(uint64_t addr,
                                      Reg64Type &rdata,
                                      uint8_t *hit) {
    TagMemInType swapin[WAY_SubNum];
    TagMemOutType memEven[ICACHE_WAYS];
    TagMemOutType memOdd[ICACHE_WAYS];

    uint64_t line_adr = getAdrLine(addr);
    uint64_t line_adr_overlay = getAdrLine(addr + (1 << OFFSET_WIDTH));

    uint64_t addr5 = getAdrOddEven(addr);
    bool useOverlay = false;
    if ((addr & 0x1E) == 0x1E) {
        useOverlay = true;
    }

    if (addr5 == 0) {
        swapin[WAY_EVEN].tag    = getAdrTag(line_adr);
        swapin[WAY_EVEN].index  = getAdrIndex(line_adr);
        swapin[WAY_EVEN].offset = getAdrOffset(addr);
        swapin[WAY_ODD].tag     = getAdrTag(line_adr_overlay);
        swapin[WAY_ODD].index   = getAdrIndex(line_adr_overlay);
        swapin[WAY_ODD].offset  = 0;
    } else {
        swapin[WAY_EVEN].tag    = getAdrTag(line_adr_overlay);
        swapin[WAY_EVEN].index  = getAdrIndex(line_adr_overlay);
        swapin[WAY_EVEN].offset = 0;
        swapin[WAY_ODD].tag     = getAdrTag(line_adr);
        swapin[WAY_ODD].index   = getAdrIndex(line_adr);
        swapin[WAY_ODD].offset  = getAdrOffset(addr);
    }

    hit[WAY_EVEN] = ICACHE_WAYS;
    hit[WAY_ODD] = ICACHE_WAYS;
    rdata.val = 0;

    // Memory banks
    for (int n = 0; n < ICACHE_WAYS; n++) {
        wayEven_[n].readTagValue(swapin[WAY_EVEN].index,
                                 swapin[WAY_EVEN].offset,
                                 memEven[n]);

        wayOdd_[n].readTagValue(swapin[WAY_ODD].index,
                                swapin[WAY_ODD].offset,
                                memOdd[n]);
    }

    struct WayMuxType {
        uint8_t hit;
        uint16_t rdata[2];
    } waysel[WAY_SubNum];

    // Check read tag and select way
    waysel[WAY_EVEN].hit = ICACHE_WAYS;
    waysel[WAY_EVEN].rdata[0] = 0;
    waysel[WAY_EVEN].rdata[1] = 0;
    waysel[WAY_ODD].hit = ICACHE_WAYS;
    waysel[WAY_ODD].rdata[0] = 0;
    waysel[WAY_ODD].rdata[1] = 0;
    for (uint8_t n = 0; n < static_cast<uint8_t>(ICACHE_WAYS); n++) {
        if (memEven[n].tag == swapin[WAY_EVEN].tag) {
            waysel[WAY_EVEN].hit = n;
            waysel[WAY_EVEN].rdata[0] = memEven[n].buf16[0];
            waysel[WAY_EVEN].rdata[1] = memEven[n].buf16[1];
        }

        if (memOdd[n].tag == swapin[WAY_ODD].tag) {
            waysel[WAY_ODD].hit = n;
            waysel[WAY_ODD].rdata[0] = memOdd[n].buf16[0];
            waysel[WAY_ODD].rdata[1] = memOdd[n].buf16[1];
        }
    }

    // Swap back rdata
    if (addr5 == 0) {
        if (useOverlay == 0) {
            hit[0] = waysel[WAY_EVEN].hit;
            hit[1] = waysel[WAY_EVEN].hit;
            rdata.buf16[0] = waysel[WAY_EVEN].rdata[0];
            rdata.buf16[1] = waysel[WAY_EVEN].rdata[1];
        } else {
            hit[0] = waysel[WAY_EVEN].hit;
            hit[1] = waysel[WAY_ODD].hit;
            rdata.buf16[0] = waysel[WAY_EVEN].rdata[0];
            rdata.buf16[1] = waysel[WAY_ODD].rdata[0];
        }
    } else {
        if (useOverlay == 0) {
            hit[0] = waysel[WAY_ODD].hit;
            hit[1] = waysel[WAY_ODD].hit;
            rdata.buf16[0] = waysel[WAY_ODD].rdata[0];
            rdata.buf16[1] = waysel[WAY_ODD].rdata[1];
        } else {
            hit[0] = waysel[WAY_ODD].hit;
            hit[1] = waysel[WAY_EVEN].hit;
            rdata.buf16[0] = waysel[WAY_ODD].rdata[0];
            rdata.buf16[1] = waysel[WAY_EVEN].rdata[0];
        }
    }
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

// addr[31:5]
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

        tr.addr = 0x2000 + 0x1A + (i << 1);     // 2-bytes alignment (uint16_t)
        b_transport(&tr);

        // fwimage rom
        tr.addr = 0x00100000 + 0x1A + (i << 1);     // 2-bytes alignment (uint16_t)
        b_transport(&tr);

        tr.addr = 0x00102000 + 0x1A + (i << 1);     // 2-bytes alignment (uint16_t)
        b_transport(&tr);

        tr.addr = 0x00104000 + 0x1A + (i << 1);     // 2-bytes alignment (uint16_t)
        b_transport(&tr);
    }
}

}  // namespace debugger
