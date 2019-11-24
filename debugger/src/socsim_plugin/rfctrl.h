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

#ifndef __DEBUGGER_SOCSIM_PLUGIN_RFCTRL_H__
#define __DEBUGGER_SOCSIM_PLUGIN_RFCTRL_H__

#include "iclass.h"
#include "iservice.h"
#include "coreservices/imemop.h"

namespace debugger {

class RfController : public IService, 
                     public IMemoryOperation {
 public:
    explicit RfController(const char *name);
    virtual ~RfController();

    /** IService interface */
    virtual void postinitService();

    /** IMemoryOperation */
    virtual ETransStatus b_transport(Axi4TransactionType *trans);
    
 private:
    AttributeType subsystemConfig_;

    struct rfctrl_map {
        volatile uint32_t conf1;		// 0x00
        volatile uint32_t conf2;		// 0x04
        volatile uint32_t conf3;		// 0x08/
        volatile uint32_t pllconf;		// 0x0C/
        volatile uint32_t div;	        // 0x10
        volatile uint32_t fdiv;	        // 0x14
        volatile uint32_t strm;	        // 0x18
        volatile uint32_t clkdiv;		// 0x1C
        volatile uint32_t test1;		// 0x20
        volatile uint32_t test2;		// 0x24
        volatile uint32_t scale;		// 0x28
        volatile uint32_t run;		    // 0x2C
        volatile uint32_t reserved1[3];	// 0x30,0x34,0x38
        volatile uint32_t rw_ant_status;// 0x3C
        volatile uint32_t subsystem_config;// 0x40
    } regs_;
};

DECLARE_CLASS(RfController)

}  // namespace debugger

#endif  // __DEBUGGER_SOCSIM_PLUGIN_RFCTRL_H__
