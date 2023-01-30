/*
 *  Copyright 2023 Sergey Khabarov, sergeykhbr@gmail.com
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

#include <iclass.h>
#include <iservice.h>
#include "coreservices/icmdexec.h"
#include "coreservices/isrccode.h"
#include "coreservices/icoveragetracker.h"

namespace debugger {

class CoverageCmdType : public ICommand {
 public:
    CoverageCmdType(IService *parent) : ICommand(parent, "coverage") {
        briefDescr_.make_string("Get code usage information.");
        detailedDescr_.make_string(
            "Description:\n"
            "    This command returns brief or detailed information about\n"
            "    code usage (coverage) in precentage.\n"
            "Usage:\n"
            "    1. Read double value with the brief information in precentage:\n"
            "        coverage\n"
            "    2. Read list with used/unused address ranges:\n"
            "        coverage ranges\n"
            "    3. Read list with detailed information and symbol names:\n"
            "        coverage detailed\n"
            "Example:\n"
            "    coverage\n"
            "    coverage detailed");
    }

    /** ICommand */
    virtual int isValid(AttributeType *args);
    virtual void exec(AttributeType *args, AttributeType *res);
};


class GenericCodeCoverage : public IService,
                            public ICoverageTracker {
 public:
    explicit GenericCodeCoverage(const char *name);
 
    /** IService interface */
    virtual void postinitService();
    virtual void predeleteService();

    /** ICoverageTracker */
    virtual void markAddress(uint64_t addr, uint8_t oplen);

    /** Common commands access methods */
    virtual double getCoverage();
    virtual void getCoverageDetailed(AttributeType *resp);

 protected:
    uint64_t paged2flat(uint64_t addr);

 protected:
    AttributeType cmdexec_;
    AttributeType src_;
    AttributeType regions_;
    AttributeType paged_;

    ICmdExecutor *iexec_;
    ISourceCode *isrc_;
    CoverageCmdType *pcmd_;

    uint64_t track_sz_;
    uint8_t flash_[128*1024];
};

DECLARE_CLASS(GenericCodeCoverage)

}  // namespace debugger
