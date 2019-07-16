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

#ifndef __DEBUGGER_SRC_CPU_FNC_PLUGIN_ICACHE_FUNC_H__
#define __DEBUGGER_SRC_CPU_FNC_PLUGIN_ICACHE_FUNC_H__

#include <iclass.h>
#include <iservice.h>
#include "coreservices/imemop.h"
#include "coreservices/icmdexec.h"

namespace debugger {

static const int BUS_ADDR_WIDTH = 32;
static const int OFFSET_WIDTH   = 5;    // [4:0]  log2(ICACHE_LINE_BYTES)
static const int ODDEVEN_WIDTH  = 1;    // [5]    0=even; 1=odd
// [13:6]  8: index: 8 KB per odd/even ways (64 KB icache)
// [12:6]  7: index: 4 KB per odd/even ways (32 KB icache)
// [11:6]  6: index: 2 KB per odd/even ways (16 KB icache)
static const int INDEX_WIDTH    = 6;    // log2(LINES_PER_WAY)
// [31:14] tag when 64 KB
// [31:13] tag when 32 KB
// [31:12] tag when 16 KB
static const int TAG_WIDTH      =
    BUS_ADDR_WIDTH - (OFFSET_WIDTH + ODDEVEN_WIDTH + INDEX_WIDTH);


static const int ICACHE_LINE_BYTES  = 1 << OFFSET_WIDTH;
static const int ICACHE_WAYS        = 4;    // 4 odds, 4 even
// 64  => 16 KB, 4-way cache
// 128 => 32 KB, 4-way cache
// 256 => 64 KB, 4-way cache
static const int LINES_PER_WAY      = 1 << INDEX_WIDTH;
static const int BYTES_PER_WAY      = LINES_PER_WAY * ICACHE_LINE_BYTES;
static const int ICACHE_TOTAL_BYTES = (2 * ICACHE_WAYS) * BYTES_PER_WAY;

static const int HIT_WORD0  = 0x1;
static const int HIT_WORD1  = 0x2;
static const int HIT_BOTH   = HIT_WORD0 | HIT_WORD1;

struct TagMemInType {
    uint64_t tag;
    uint64_t index;
    uint64_t offset;
};

struct TagMemOutType {
    uint64_t tag;
    uint16_t buf16[2];
};

struct CacheLineData {
    uint64_t tag;
    Reg64Type mem64[ICACHE_LINE_BYTES / sizeof(uint64_t)];
};

class WayMemType {
 public:
    WayMemType() {}
    ~WayMemType() {}

    void writeLine(uint32_t index, uint64_t tag, int wstrb, uint64_t wdata) {
        for (int i = 0; i < 4; i++) {
            if (wstrb & (1 << i)) {
                lines_[index].tag = tag;
                lines_[index].mem64[i].val = wdata;
            }
        }
    }
    void readTagValue(uint64_t index, uint64_t off,
                      TagMemOutType &out) {
        out.tag = lines_[index].tag;

        // Two-bytes alignment with wrapping
        int wcnt = (off >> 3) & 0x3;
        int bcnt = (off >> 1) & 0x3;
        out.buf16[0] = lines_[index].mem64[wcnt].buf16[bcnt];

        off += 2;
        wcnt = (off >> 3) & 0x3;
        bcnt = (off >> 1) & 0x3;
        out.buf16[1] = lines_[index].mem64[wcnt].buf16[bcnt];
    }

 private:
    CacheLineData lines_[LINES_PER_WAY];
};

class ICacheFunctional : public IService,
                         public IMemoryOperation,
                         public ICommand {
 public:
    explicit ICacheFunctional(const char *name);
    virtual ~ICacheFunctional();

    /** IService interface */
    virtual void postinitService() override;
    virtual void predeleteService() override;

    /** IMemoryOperation */
    virtual ETransStatus b_transport(Axi4TransactionType *trans);

    /** ICommand */
    virtual int isValid(AttributeType *args);
    virtual void exec(AttributeType *args, AttributeType *res);

 private:
    uint64_t getAdrLine(uint64_t adr);
    uint64_t getAdrOddEven(uint64_t adr);
    uint64_t getAdrTag(uint64_t adr);
    uint32_t getAdrIndex(uint64_t adr);
    uint32_t getAdrOffset(uint64_t adr);

    void getCachedValue(uint64_t adr, Reg64Type &rdata, uint8_t *hit);
    void readLine(uint64_t adr, WayMemType *way);
    void runTest();

 private:
    AttributeType sysBus_;
    AttributeType cmdexec_;
    AttributeType totalKBytes_;  // default 64 KB
    AttributeType lineBytes_;    // default 32 B
    AttributeType ways_;         // default 4

    IMemoryOperation *isysbus_;
    ICmdExecutor *icmdexec_;

    struct LruEntryType {
        uint8_t n0 : 2;
        uint8_t n1 : 2;
        uint8_t n2 : 2;
        uint8_t n3 : 2;
    };

    union LruTableType {
        LruEntryType row[LINES_PER_WAY];
        uint8_t buf[LINES_PER_WAY];
    };
    LruTableType tblLruEven_;
    LruTableType tblLruOdd_;

    enum EWays {
        WAY_EVEN,
        WAY_ODD,
        WAY_SubNum
    };
    WayMemType wayOdd_[ICACHE_WAYS];
    WayMemType wayEven_[ICACHE_WAYS];

    /** By default suppose using:
            16 memory banks with 16 bits width to provide C-instruction
            alignment.
     */
    uint16_t **banks_;
};

DECLARE_CLASS(ICacheFunctional)

}  // namespace debugger

#endif  // __DEBUGGER_SRC_CPU_FNC_PLUGIN_ICACHE_FUNC_H__
