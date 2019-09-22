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

#include <types.h>
#include <utils.h>
#include "ithread.h"

IThread::IThread(const char *name) : IFace(IFACE_THREAD) {
    LIB_event_create(&loopEnable_, name);
    threadInit_.Handle = 0;
    threadListeners_.make_list(0);
}

bool IThread::run() {
    threadInit_.func = reinterpret_cast<lib_thread_func>(runThread);
    threadInit_.args = this;
    LIB_thread_create(&threadInit_);

    if (threadInit_.Handle) {
        LIB_event_set(&loopEnable_);
    }
    return loopEnable_.state;
}

void IThread::stop() {
    LIB_event_clear(&loopEnable_);
    if (threadInit_.Handle) {
        LIB_thread_join(threadInit_.Handle, 50000);
    }
    threadInit_.Handle = 0;
}

void IThread::runThread(void* arg) {
    IThread *th = reinterpret_cast<IThread *>(arg);
    th->busyLoop();
    th->exitEvent();
}

void IThread::registerThreadListener(IThreadListener *l) {
    AttributeType t1;
    t1.make_iface(l);
    threadListeners_.add_to_list(&t1);
}

void IThread::exitEvent() {
    IThreadListener *l;
    for (unsigned i = 0; i < threadListeners_.size(); i++) {
        l = static_cast<IThreadListener *>(threadListeners_[i].to_iface());
        l->threadExit(static_cast<IThread *>(this));
    }
#if defined(_WIN32) || defined(__CYGWIN__)
    _endthreadex(0);
#else
    pthread_exit(0);
#endif
}
