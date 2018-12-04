/*
 *  Copyright 2018 Sergey Khabarov, sergeykhbr@gmail.com
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

#pragma once

#include <inttypes.h>
#include <vector>
#include "elftypes.h"
#include "common/attribute.h"

class ElfReader {
 public:
    explicit ElfReader(const char *file_name);

    virtual unsigned loadableSectionTotal() {
        return loadSectionList_.size();
    }

    virtual const char *sectionName(unsigned idx) {
        return loadSectionList_[idx][LoadSh_name].to_string();
    }

    virtual uint64_t sectionAddress(unsigned idx)  {
        return loadSectionList_[idx][LoadSh_addr].to_uint64();
    }

    virtual uint64_t sectionSize(unsigned idx)  {
        return loadSectionList_[idx][LoadSh_size].to_uint64();
    }

    virtual uint8_t *sectionData(unsigned idx)  {
        return loadSectionList_[idx][LoadSh_data].data();
    }


    void writeRawImage(AttributeType *ofiles, uint32_t fixed_size = 0);
    void writeRomHexArray(AttributeType *ofiles,
                          int bytes_per_line,
                          int fixed_size = 0);

private:
    int readElfHeader();
    int loadSections();
    void processDebugSymbol(SectionHeaderType *sh);

private:
    enum ELoadSectionItem {
        LoadSh_name,
        LoadSh_addr,
        LoadSh_size,
        LoadSh_data,
        LoadSh_Total,
    };

    enum ESymbolInfoListItem {
        Symbol_Name,
        Symbol_Addr,
        Symbol_Size,
        Symbol_Type,
        Symbol_Total
    };

    enum ESymbolType {
        SYMBOL_TYPE_FILE     = 0x01,
        SYMBOL_TYPE_FUNCTION = 0x02,
        SYMBOL_TYPE_DATA     = 0x04
    };

    enum EMode {
        Mode_32bits,
        Mode_64bits
    } emode_;

    AttributeType sourceProc_;
    AttributeType symbolList_;
    AttributeType loadSectionList_;

    uint8_t *image_;
    ElfHeaderType *header_;
    SectionHeaderType **sh_tbl_;
    char *sectionNames_;
    char *symbolNames_;

    uint64_t lastLoadAdr_;
};
