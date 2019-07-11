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

static const int BUS_ADDR_WIDTH = 32;
static const int OFFSET_WIDTH = 5;   // [4:0]   offset: 32 bytes per line
static const int INDEX_WIDTH  = 9;   // [13:5]  index: 16 KB per way
static const int TAG_WIDTH    = 18;  // [31:14] tag

class Mem16 {
 public:
    Mem16(uint64_t sz) {
        size_ = sz;
        mem_ = new uint16_t[size_];
    }
    ~Mem16() {
        delete [] mem_;
    }
    void read(uint64_t addr, uint16_t *data) {
        addr %= size_;
        *data = mem_[addr];
    }
    void write(uint64_t addr, uint16_t data) {
        addr %= size_;
        mem_[addr] = data;
    }
   
 protected:
    uint16_t *mem_;
    uint64_t size_;
};

class MemTag {
 public:
    MemTag(uint64_t sz) {
        size_ = sz;
        // [21:4] tag value
        // [3:0]  qword is valid
        mem_ = new uint32_t[size_];
    }
    ~MemTag() {
        delete [] mem_;
    }
    void read(uint64_t addr, uint32_t *data) {
        addr %= size_;
        *data = mem_[addr];
    }
    void write(uint64_t addr, uint32_t data) {
        addr %= size_;
        mem_[addr] = data;
    }
   
 protected:
    uint32_t *mem_;
    uint64_t size_;
};

class MemWay {
    MemWay() {
        totsz_ = (1 << 14);
        linesz_ = 32;
        linetot_ = totsz_ / linesz_;
        memtag_ = new MemTag(linetot_);
        for (uint64_t i = 0; i < linesz_ / sizeof(uint16_t); i++) {
            mem16_[i] = new Mem16(linetot_);
        }
    }
    ~MemWay() {
        for (uint64_t i = 0; i < linesz_ / sizeof(uint16_t); i++) {
            delete mem16_[i];
        }
        delete memtag_;
    }
    void read(uint64_t addr, uint32_t *data) {
        //addr %= size_;
        //*data = mem_[addr];
    }
    void write(uint64_t addr, uint32_t data) {
        //addr %= size_;
        //mem_[addr] = data;
    }
   
 protected:
    uint64_t totsz_;
    uint64_t linesz_;
    uint64_t linetot_;

    MemTag *memtag_;
    Mem16 *mem16_[16];
};

MemWay memway_;

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

struct CacheLineData {
    uint64_t tag;
    Reg16Type mem16[16];
    uint32_t valid;     // 16-bits bit field
};

class WayMemType {
 public:
    WayMemType() {
    }
    ~WayMemType() {
    }

    void getValue(uint32_t index, CacheLineData *data) {
    }
};

ETransStatus ICacheFunctional::b_transport(Axi4TransactionType *trans) {
    Axi4TransactionType tr;
    for (unsigned i = 0; i < 2; i++) {
        req_.adr[i].tag = getAdrTag(trans->addr + i*sizeof(uint16_t));
        req_.adr[i].index = getAdrIndex(trans->addr + i*sizeof(uint16_t));
        req_.adr[i].offset = getAdrOffset(trans->addr + i*sizeof(uint16_t));
        req_.adr[i].hit = false;
    }

    bool even_odd_swap = false;
    if (trans->addr & 0x2) {
        even_odd_swap = true;
    } else {
        even_odd_swap = false;
    }

    CacheLineData data;
    WayMemType memWay[4];
    Reg64Type resp;
    unsigned hit_way[2] = {~0u, ~0u};
    for (unsigned i = 0; i < 2; i++) {
        for (unsigned n = 0; n < ways_.to_uint32(); n++) {
            memWay[n].getValue(req_.adr[i].index, &data);
            if (data.tag == req_.adr[i].tag) {
                hit_way[i] = n;
                resp.buf16[i] = data.mem16[req_.adr[i].offset >> 1].word;
            }
        }
    }

    for (unsigned i = 0; i < 2; i++) {
        if (hit_way[i] != ~0u) {
            continue;
        }

    }
    return TRANS_OK;
}

uint64_t ICacheFunctional::getAdrTag(uint64_t adr) {
    uint64_t mask = (1ull << (BUS_ADDR_WIDTH)) - 1;
    uint64_t t = (adr & mask) >> (OFFSET_WIDTH + INDEX_WIDTH);
    return t;
}

uint32_t ICacheFunctional::getAdrIndex(uint64_t adr) {
    uint64_t mask = (1ull << (OFFSET_WIDTH + INDEX_WIDTH)) - 1;
    uint64_t t = (adr & mask) >> OFFSET_WIDTH;
    return static_cast<uint32_t>(t);
}

uint32_t ICacheFunctional::getAdrOffset(uint64_t adr) {
    uint64_t mask = (1ull << OFFSET_WIDTH) - 1;
    uint64_t t = (adr & mask);
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
        tr.addr = i << 1;     // 2-bytes alignment (uint16_t)
        b_transport(&tr);
    }
}

}  // namespace debugger
