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

#ifndef __DEBUGGER_EDCL_H__
#define __DEBUGGER_EDCL_H__

#include "iclass.h"
#include "iservice.h"
#include "coreservices/itap.h"
#include "coreservices/ilink.h"
#include <inttypes.h>

namespace debugger {

class EdclService : public IService,
                    public ITap {
public:
    EdclService(const char *name);

    /** IService interface */
    virtual void postinitService();

    /** ITap interface */
    virtual int read(uint64_t addr, int bytes, uint8_t *obuf);
    virtual int write(uint64_t addr, int bytes, uint8_t *ibuf);

private:
    int write16(uint8_t *buf, int off, uint16_t v);
    int write32(uint8_t *buf, int off, uint32_t v);
    uint32_t read32(uint8_t *buf);

private:
    /** This is limitation of the MAC fifo. Protocol allows increase the
     * following value up to 242 words. */
    static const int EDCL_PAYLOAD_MAX_WORDS32 = 8;
    static const int EDCL_PAYLOAD_MAX_BYTES  = 4*EDCL_PAYLOAD_MAX_WORDS32;

    uint8_t tx_buf_[4096];
    uint8_t rx_buf_[4096];
    ILink *itransport_;
    AttributeType transport_;
    AttributeType seq_cnt_;

    int dbgRdTRansactionCnt_;
};

DECLARE_CLASS(EdclService)

}  // namespace debugger

#endif  // __DEBUGGER_EDCL_H__
