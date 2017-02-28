/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      elf-file loader class declaration.
 */

#ifndef __DEBUGGER_ELF_LOADER_H__
#define __DEBUGGER_ELF_LOADER_H__

#include "iclass.h"
#include "iservice.h"
#include "coreservices/itap.h"
#include "coreservices/ielfreader.h"
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

    virtual void getSymbols(AttributeType *list) { *list = symbolList_; }

    virtual void addressToSymbol(uint64_t addr, AttributeType *name);

private:
    int readElfHeader();
    int loadSections();
    void processStringTable(SectionHeaderType *sh);
    void processDebugSymbol(SectionHeaderType *sh);

private:
    enum ELoadSectionItem {
        LoadSh_name,
        LoadSh_addr,
        LoadSh_size,
        LoadSh_data,
        LoadSh_Total,
    };

    AttributeType symbolList_;
    AttributeType loadSectionList_;

    uint8_t *image_;
    ElfHeaderType *header_;
    SectionHeaderType *sh_tbl_;
    char *sectionNames_;
    char *symbolNames_;
};

DECLARE_CLASS(ElfReaderService)

}  // namespace debugger

#endif  // __DEBUGGER_ELF_LOADER_H__
