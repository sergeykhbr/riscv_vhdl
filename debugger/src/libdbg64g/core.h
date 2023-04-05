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

#ifndef __SRC_LIBDBG64G_CORE_H__
#define __SRC_LIBDBG64G_CORE_H__

#include "api_core.h"
#include "iclass.h"
#include "iservice.h"
#include "ihap.h"
#include <iostream>

namespace debugger {

/** Plugin Entry point type definition */
typedef void (*plugin_init_proc)();

struct CoreTimerType {
    timer_callback_type cb;
    void *args;
    int interval;
    int delta;
    int single_shot;
};

class CoreService : public IService {
 public:
    explicit CoreService(const char *name);
    virtual ~CoreService();

    int isActive();
    void shutdown();
    bool isExiting();
    void setExiting();

    int setConfig(AttributeType *cfg);
    void getConfig(AttributeType *cfg);
    const AttributeType *getGlobalSettings();

    int createPlatformServices();
    void postinitPlatformServices();
    void predeletePlatformServices();

    void load_plugins();
    void unload_plugins();
    void registerClass(IFace *icls);
    void unregisterClass(const char *clsname);
    void registerService(IFace *isrv);
    void unregisterService(const char *srvname);
    void registerHap(IFace *ihap);
    void unregisterHap(IFace *ihap);
    void triggerHap(int type, uint64_t param, const char *descr);
    void registerConsole(IFace *iconsole);
    void unregisterConsole(IFace *iconsole);

    IFace *getClass(const char *name);
    IFace *getService(const char *name);
    void getServicesWithIFace(const char *iname, AttributeType *list);
    void getIFaceList(const char *iname, AttributeType *list);

    void lockPrintf();
    void unlockPrintf();
    int openLog(const char *filename);
    void closeLog();
    void outputLog(const char *buf, int sz);
    void outputConsole(const char *buf, int sz);

    void setTimestampClk(IFace *iclk) { iclk_ = iclk; }
    uint64_t getTimestamp();

    char *getpBufLog() { return bufLog_; }
    size_t sizeBufLog() { return sizeof(bufLog_); }
    void generateUniqueName(const char *prefix, char *out, size_t outsz);

 private:
    AttributeType Config_;
    AttributeType listPlugins_;
    AttributeType listClasses_;
    AttributeType listServices_;
    AttributeType listHap_;
    AttributeType listConsole_;

    int active_;
    event_def eventExiting_;
    mutex_def mutexLogFile_;
    mutex_def mutexPrintf_;
    mutex_def mutexDefaultConsoles_;

    IFace *iclk_;
    FILE *logFile_;

    /** Temporary buffer for the log messages. */
    char bufLog_[1024*1024];
    int uniqueIdx_;
};

}  // namespace debugger

#endif  // __SRC_LIBDBG64G_CORE_H__
