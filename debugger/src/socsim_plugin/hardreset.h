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

#ifndef __SOCSIM_PLUGIN_HARDRESET_H__
#define __SOCSIM_PLUGIN_HARDRESET_H__

#include <iclass.h>
#include <iservice.h>
#include "coreservices/ireset.h"

namespace debugger {

class HardReset : public IService {
 public:
    HardReset(const char *name);

    /** IService interface */
    virtual void postinitService();

    /** TODO: command */
    virtual void powerOnPressed();
    virtual void powerOnReleased();

 private:
};

DECLARE_CLASS(HardReset)

}  // namespace debugger

#endif  // __SOCSIM_PLUGIN_HARDRESET_H__
