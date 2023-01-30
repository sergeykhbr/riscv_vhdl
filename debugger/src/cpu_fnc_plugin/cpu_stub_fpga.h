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

#ifndef __DEBUGGER_SRC_CPU_FNC_PLUGIN_CPU_STUB_FPGA_H__
#define __DEBUGGER_SRC_CPU_FNC_PLUGIN_CPU_STUB_FPGA_H__

#include <iservice.h>
#include "coreservices/icmdexec.h"
#include "generic/cpu_generic.h"

namespace debugger {

class CpuStubRiscVFpga : public IService {
 public:
    explicit CpuStubRiscVFpga(const char *name);

    /** IService interface */
    virtual void postinitService() override;
    virtual void predeleteService() override;

 private:
    AttributeType cmdexec_;
    AttributeType dmibar_;
    AttributeType tap_;

    ICmdExecutor *icmdexec_;
};

DECLARE_CLASS(CpuStubRiscVFpga)

}  // namespace debugger

#endif  // __DEBUGGER_SRC_CPU_FNC_PLUGIN_CPU_STUB_FPGA_H__
