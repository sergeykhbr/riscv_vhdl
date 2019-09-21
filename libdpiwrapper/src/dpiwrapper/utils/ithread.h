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

#include <types.h>
#include "iface.h"
#include "attribute.h"

static const char *const IFACE_THREAD = "IThread";

class IThread : public IFace {
 public:
    IThread(const char *name);

    /** create and start seperate thread */
    virtual bool run();

    /** @brief Stop and join thread */
    virtual void stop();

    /** check thread status */
    virtual bool isEnabled() { return loopEnable_.state; }

 protected:
    static void runThread(void *arg);

    /** working cycle function */
    virtual void busyLoop() = 0;

 protected:
    event_def loopEnable_;
    LibThreadType threadInit_;
};
