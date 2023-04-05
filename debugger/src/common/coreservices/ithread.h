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

#ifndef __DEBUGGER_ITHREAD_H__
#define __DEBUGGER_ITHREAD_H__

#include <iface.h>
#include <api_core.h>

namespace debugger {

static const char *const IFACE_THREAD = "IThread";

class IThread : public IFace {
 public:
    IThread() : IFace(IFACE_THREAD) {
        AttributeType t1;
        RISCV_generate_name(&t1);
        RISCV_event_create(&loopEnable_, t1.to_string());
        threadInit_.Handle = 0;
    }

    /** create and start seperate thread */
    virtual bool run() {
        threadInit_.func = reinterpret_cast<lib_thread_func>(runThread);
        threadInit_.args = this;
        RISCV_thread_create(&threadInit_);

        if (threadInit_.Handle) {
            RISCV_event_set(&loopEnable_);
        }
        return loopEnable_.state;
    }

    /** @brief Request to stop thread */
    virtual void stop() {
        RISCV_event_clear(&loopEnable_);
    }

    virtual void join(int ms) {
        if (threadInit_.Handle) {
            RISCV_thread_join(threadInit_.Handle, ms);
        }
        threadInit_.Handle = 0;
    }

    /** check thread status */
    virtual bool isEnabled() { return loopEnable_.state; }

 protected:
    /** working cycle function */
    virtual void busyLoop() =0;

    static void runThread(void *arg) {
        reinterpret_cast<IThread *>(arg)->busyLoop();
    }

 protected:
    event_def loopEnable_;
    LibThreadType threadInit_;
};

}  // namespace debugger

#endif  // __DEBUGGER_ITHREAD_H__
