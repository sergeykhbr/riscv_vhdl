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

#ifndef __DEBUGGER_ELF_LOADER_H__
#define __DEBUGGER_ELF_LOADER_H__

#include "iclass.h"
#include "iservice.h"
#include "coreservices/itap.h"
#include "coreservices/ielfreader.h"
#include "coreservices/isrccode.h"
#include "elf_types.h"

namespace debugger {

class ElfReaderService : public IService,
                         public IElfReader {
public:
    explicit ElfReaderService(const char *name);
    virtual ~ElfReaderService();

    /** IService interface */
    virtual void postinitService();

    /** IElfReader interface */
    virtual int readFile(const char *filename);

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

    enum EMode {
        Mode_32bits,
        Mode_64bits
    } emode_;

    AttributeType sourceProc_;
    AttributeType symbolList_;
    AttributeType loadSectionList_;

    ISourceCode *isrc_;
    uint8_t *image_;
    ElfHeaderType *header_;
    SectionHeaderType **sh_tbl_;
    char *sectionNames_;
    char *symbolNames_;
};

DECLARE_CLASS(ElfReaderService)

}  // namespace debugger

#endif  // __DEBUGGER_ELF_LOADER_H__
