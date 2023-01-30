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

#include "codecov_generic.h"

namespace debugger {

int CoverageCmdType::isValid(AttributeType *args) {
    if (!(*args)[0u].is_equal("coverage")) {
        return CMD_INVALID;
    }
    return CMD_VALID;
}

void CoverageCmdType::exec(AttributeType *args, AttributeType *res) {
    GenericCodeCoverage *p = static_cast<GenericCodeCoverage *>(cmdParent_);
    if (args->size() == 2 && (*args)[1].is_string()) {
        if ((*args)[1].is_equal("detailed")) {
            p->getCoverageDetailed(res);
            return;
        }
    }
    res->make_floating(p->getCoverage());
}


GenericCodeCoverage::GenericCodeCoverage(const char *name) : IService(name) {
    registerInterface(static_cast<ICoverageTracker *>(this));
    registerAttribute("CmdExecutor", static_cast<IAttribute *>(&cmdexec_));
    registerAttribute("SourceCode", static_cast<IAttribute *>(&src_));
    registerAttribute("Paged", static_cast<IAttribute *>(&paged_));
    registerAttribute("Regions", static_cast<IAttribute *>(&regions_));
    track_sz_ = 0;
}

void GenericCodeCoverage::postinitService() {
    iexec_ = static_cast<ICmdExecutor *>
        (RISCV_get_service_iface(cmdexec_.to_string(), IFACE_CMD_EXECUTOR));
    if (!iexec_) {
        RISCV_error("Can't get ICmdExecutor interface %s",
                    cmdexec_.to_string());
        return;
    }

    isrc_ = static_cast<ISourceCode *>
        (RISCV_get_service_iface(src_.to_string(), IFACE_SOURCE_CODE));
    if (!isrc_) {
        RISCV_error("Can't get ISourceCode interface %s",
                    src_.to_string());
        return;
    }

    pcmd_ = new CoverageCmdType(static_cast<IService *>(this));
    iexec_->registerCommand(static_cast<ICommand *>(pcmd_));

    // Compute Total regions size
    if (!regions_.is_list()) {
        RISCV_error("Regions attribute of wrong format",
                    src_.to_string());
        return;
    }
    track_sz_ = 0;
    for (unsigned i = 0; i < regions_.size(); i++) {
        AttributeType &item = regions_[i];
        track_sz_ += item[1].to_uint64() - item[0u].to_uint64() + 1;
    }
    memset(flash_, 0, sizeof(flash_));
}

void GenericCodeCoverage::predeleteService() {
    if (iexec_) {
        iexec_->unregisterCommand(static_cast<ICommand *>(pcmd_));
    }
}

void GenericCodeCoverage::markAddress(uint64_t addr, uint8_t oplen) {
    addr = paged2flat(addr);
    if (addr < sizeof(flash_)) {
        flash_[addr] = oplen;
    }
}

uint64_t GenericCodeCoverage::paged2flat(uint64_t addr) {
    if (!paged_.to_bool()) {
        return addr;
    }
    return addr % sizeof(flash_);
}

double GenericCodeCoverage::getCoverage() {
    uint64_t used = 0;
    uint64_t off = 0;
    uint64_t inc_bytes;
    double ret;

    for (unsigned i = 0; i < regions_.size(); i++) {
        AttributeType &item = regions_[i];
        uint64_t sec_start = paged2flat(item[0u].to_uint32());
        uint64_t sec_end = paged2flat(item[1].to_uint32());
        uint64_t sec_sz = sec_end - sec_start + 1;
        off = 0;
        while (off < sec_sz) {
            if (!flash_[sec_start + off]) {
                off++;
            } else {
                inc_bytes = flash_[sec_start + off];
                if ((sec_start + off + inc_bytes) > sec_end) {
                    inc_bytes = 1 + sec_end - (sec_start + off);
                }
                used += inc_bytes;
                off += inc_bytes;
            }
        }
    }

    ret = 100.0*static_cast<double>(used)/track_sz_;
    return ret;
}

void GenericCodeCoverage::getCoverageDetailed(AttributeType *resp) {
    resp->attr_free();
    resp->make_list(0);
    unsigned off;
    uint8_t marked;
    AttributeType item;
    AttributeType symbol;
    bool split_section;
    enum EStage {
        SectionMarked,
        SectionCleared,
    } estage;
    char tstr[256];
    item.make_list(4);

    for (unsigned i = 0; i < regions_.size(); i++) {
        AttributeType &region = regions_[i];
        unsigned sec_start = region[0u].to_uint32();
        unsigned sec_end = region[1].to_uint32();
        unsigned sec_sz = sec_end - sec_start + 1;
        unsigned sec_cnt = 0;
        off = sec_start;
        marked = flash_[paged2flat(sec_start)];

        if (flash_[paged2flat(off)]) {
            item[0u].make_boolean(true);
            estage = SectionMarked;
        } else {
            item[0u].make_boolean(false);
            estage = SectionCleared;
        }
        item[1].make_uint64(sec_start);        // start addess initial value
        item[2].make_uint64(sec_start);        // end address initial value
        isrc_->addressToSymbol(sec_start, &symbol);
        RISCV_sprintf(tstr, sizeof(tstr), "%s+0x%x",
                        symbol[0u].to_string(), symbol[1].to_uint32());
        item[3].make_string(tstr);

        while (sec_cnt < sec_sz) {
            marked = flash_[paged2flat(off)];
            split_section = false;

            switch (estage) {
            case SectionMarked:
                if (!marked) {
                    estage = SectionCleared;
                    split_section = true;
                }
                break;
            case SectionCleared:
                if (marked) {
                    estage = SectionMarked;
                    split_section = true;
                }
                break;
            default:;
            }

            if (split_section) {
                resp->add_to_list(&item);

                // init attribute for the next sections
                item[0u].make_boolean(marked ? true: false);
                item[1].make_uint64(off);               // start address
                item[2].make_uint64(off);               // end address initial value
                isrc_->addressToSymbol(off, &symbol);
                RISCV_sprintf(tstr, sizeof(tstr), "%s+0x%x",
                              symbol[0u].to_string(), symbol[1].to_uint32());
                item[3].make_string(tstr);
            }

            if (marked) {
                for (uint8_t n = 0; n < marked; n++) {
                    if (sec_cnt < sec_sz) {
                        item[2].make_uint64(off);   // end address update
                    }
                    off++;
                    sec_cnt++;
                }
            } else {
                item[2].make_uint64(off);           // end address update
                off++;
                sec_cnt++;
            }
        }
        resp->add_to_list(&item);
    }
}

}  // namespace debugger
