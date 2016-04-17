/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Thread interface declaration.
 */

#ifndef __DEBUGGER_ITHREAD_H__
#define __DEBUGGER_ITHREAD_H__

#include "iface.h"
#include "api_core.h"

namespace debugger {

static const char *const IFACE_THREAD = "IThread";

class IThread : public IFace {
public:
    IThread() : IFace(IFACE_THREAD) {
        loopEnable_ = false;
        interrupt_ = false;
        threadInit_.Handle = 0;
    }

    /** create and start seperate thread */
    virtual bool run() {
        loopEnable_ = false;
        threadInit_.func = (lib_thread_func)runThread;
        threadInit_.args = this;
        RISCV_thread_create(&threadInit_);

        if (threadInit_.Handle) {
            loopEnable_ = true;
        }
        return loopEnable_;
    }

    /** @brief Stop and join thread */
    virtual void stop() {
        loopEnable_ = false;
        if (threadInit_.Handle) {
            RISCV_thread_join(threadInit_.Handle, 50000);
        }
    }

    /** Cannot stop thread from itself, so use this interrupt method */
    virtual void breakSignal() {
        interrupt_ = true;
    }
    
    /** check thread status */
    virtual bool isEnabled() { return loopEnable_ && !interrupt_; }

protected:
    /** working cycle function */
    virtual void busyLoop() =0;

    static void runThread(void *arg) {
        reinterpret_cast<IThread *>(arg)->busyLoop();
    }

protected:
    volatile bool loopEnable_;
    volatile bool interrupt_;
    LibThreadType threadInit_;
};

}  // namespace debugger

#endif  // __DEBUGGER_ITHREAD_H__
