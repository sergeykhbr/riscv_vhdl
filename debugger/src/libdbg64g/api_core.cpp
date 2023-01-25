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

#include "core.h"
#include "coreservices/ithread.h"
#include "generic/bus_generic.h"
#include "services/debug/serial_dbglink.h"
#include "services/debug/udp_dbglink.h"
#include "services/debug/edcl.h"
#include "services/debug/cpumonitor.h"
#include "services/debug/codecov_generic.h"
#include "services/debug/greth.h"
#include "services/debug/jtag.h"
#include "services/elfloader/elfreader.h"
#include "services/exec/cmdexec.h"
#include "services/mem/memlut.h"
#include "services/mem/memsim.h"
#include "services/mem/rmemsim.h"
#include "services/remote/tcpjtagbb.h"
#include "services/remote/tcpclient.h"
#include "services/remote/tcpserver.h"
#include "services/remote/dpiclient.h"
#include "services/comport/comport.h"
#include "services/console/autocompleter.h"
#include "services/console/console.h"
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <dirent.h>
#if defined(_WIN32) || defined(__CYGWIN__)
#else
    #include <dlfcn.h>
    #include <sys/mman.h>
    #include <sys/stat.h>
    #include <fcntl.h>
#endif

namespace debugger {

static const int TIMERS_MAX = 2;
static CoreTimerType timers_[TIMERS_MAX] = {{0}};

CoreService *pcore_ = NULL;

IFace *getInterface(const char *name) {
    return pcore_->getInterface(name);
}

extern "C" int RISCV_init() {
#if defined(_WIN32) || defined(__CYGWIN__)
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData)) {
        printf("error: %s\n", "Can't initialize sockets library");
    }
#endif
    pcore_ = new CoreService("core");

    REGISTER_CLASS_IDX(BusGeneric, 0);
    REGISTER_CLASS_IDX(SerialDbgService, 1);
    REGISTER_CLASS_IDX(UdpService, 2);
    REGISTER_CLASS_IDX(ElfReaderService, 3);
    REGISTER_CLASS_IDX(CmdExecutor, 4);
    REGISTER_CLASS_IDX(MemoryLUT, 5);
    REGISTER_CLASS_IDX(MemorySim, 6);
    REGISTER_CLASS_IDX(TcpClient, 7);
    REGISTER_CLASS_IDX(TcpServer, 8);
    REGISTER_CLASS_IDX(ComPortService, 9);
    REGISTER_CLASS_IDX(AutoCompleter, 10);
    REGISTER_CLASS_IDX(ConsoleService, 11);
    REGISTER_CLASS_IDX(EdclService, 12);
    REGISTER_CLASS_IDX(RegMemorySim, 13);
    REGISTER_CLASS_IDX(DpiClient, 14);
    REGISTER_CLASS_IDX(CpuMonitor, 15);
    REGISTER_CLASS_IDX(GenericCodeCoverage, 16);
    REGISTER_CLASS_IDX(Greth, 17)
    REGISTER_CLASS_IDX(TcpJtagBitBangClient, 18);
    REGISTER_CLASS_IDX(JTAG, 19);

    pcore_->load_plugins();
    return 0;
}

extern "C" void RISCV_cleanup() {
    pcore_->predeletePlatformServices();
    pcore_->unload_plugins();

#if defined(_WIN32) || defined(__CYGWIN__)
    WSACleanup();
#endif
    delete pcore_;
}

extern "C" int RISCV_set_configuration(AttributeType *cfg) {
    if (!pcore_) {
        printf("Core library wasn't initialized.\n");
        return -1;
    }
    if (pcore_->setConfig(cfg)) {
        printf("Wrong configuration.\n");
        return -1;
    }

    if (pcore_->createPlatformServices()) {
        return -1;
    }
    pcore_->postinitPlatformServices();

    RISCV_printf(getInterface(IFACE_SERVICE), 0, "%s",
    "\n****************************************************************\n"
    "  Universal System Emulator and Debugger\n"
    "  Copyright 2019 Sergey Khabarov, sergeykhbr@gmail.com.\n"
    "  Licensed under the Apache License, Version 2.0.\n"
    "******************************************************************");

    pcore_->triggerHap(HAP_ConfigDone, 0,
                      "Initial config done");
    return 0;
}

extern "C" void RISCV_get_configuration(AttributeType *cfg) {
    pcore_->getConfig(cfg);
}

extern "C" const AttributeType *RISCV_get_global_settings() {
    return pcore_->getGlobalSettings();
}

extern "C" void RISCV_register_class(IFace *icls) {
    pcore_->registerClass(icls);
}

extern "C" void RISCV_unregister_class(const char *clsname) {
    pcore_->unregisterClass(clsname);
}

extern "C" void RISCV_register_hap(IFace *ihap) {
    pcore_->registerHap(ihap);
}

extern "C" void RISCV_unregister_hap(IFace *ihap) {
    pcore_->unregisterHap(ihap);
}

extern "C" void RISCV_trigger_hap(int type, uint64_t param,
                                  const char *descr) {
    pcore_->triggerHap(type, param, descr);
}

extern "C" IFace *RISCV_get_class(const char *name) {
    return pcore_->getClass(name);
}

extern "C" IFace *RISCV_create_service(IFace *iclass, const char *name, 
                                        AttributeType *args) {
    IClass *icls = static_cast<IClass *>(iclass);
    IService *iobj = icls->createService(".", name);
    iobj->initService(args);
    iobj->postinitService();
    return iobj;
}

extern "C" IFace *RISCV_get_service(const char *name) {
    return pcore_->getService(name);
}

extern "C" IFace *RISCV_get_service_iface(const char *servname,
                                          const char *facename) {
    IService *iserv = static_cast<IService *>(RISCV_get_service(servname));
    if (iserv == NULL) {
        RISCV_error("Service '%s' not found.", servname);
        return NULL;
    }
    return iserv->getInterface(facename);
}

extern "C" IFace *RISCV_get_service_port_iface(const char *servname,
                                               const char *portname,
                                               const char *facename) {
    IService *iserv = static_cast<IService *>(RISCV_get_service(servname));
    if (iserv == NULL) {
        RISCV_error("Service '%s' not found.", servname);
        return NULL;
    }
    return iserv->getPortInterface(portname, facename);
}

extern "C" void RISCV_get_services_with_iface(const char *iname,
                                             AttributeType *list) {
    pcore_->getServicesWithIFace(iname, list);
}

extern "C" void RISCV_get_iface_list(const char *iname,
                                     AttributeType *list) {
    pcore_->getIFaceList(iname, list);
}

extern "C" void RISCV_get_clock_services(AttributeType *list) {
    IService *iserv;
    RISCV_get_services_with_iface(IFACE_CLOCK, list);
    for (unsigned i = 0; i < list->size(); i++) {
        iserv = static_cast<IService *>((*list)[i].to_iface());
        (*list)[i].make_iface(iserv->getInterface(IFACE_CLOCK));
    }
}

static thread_return_t safe_exit_thread(void *args) {
    AttributeType t1, t2;
    IService *iserv;
    IThread *ith;
    RISCV_get_services_with_iface(IFACE_THREAD, &t1);

    for (unsigned i = 0; i < t1.size(); i++) {
        iserv = static_cast<IService *>(t1[i].to_iface());
        ith = static_cast<IThread *>(iserv->getInterface(IFACE_THREAD));
        printf("Stopping thread service '%s'. . .", iserv->getObjName());
        ith->stop();
        printf("Stopped\n");
    }

    pcore_->triggerHap(HAP_BreakSimulation,
                       0,
                       "Exiting");
    printf("All threads were stopped!\n");
    pcore_->shutdown();
    return 0;
}

extern "C" void RISCV_break_simulation() {
    if (pcore_->isExiting()) {
        return;
    }
    pcore_->setExiting();
    LibThreadType data;
    data.func = reinterpret_cast<lib_thread_func>(safe_exit_thread);
    data.args = 0;
    RISCV_thread_create(&data);
}


extern "C" void RISCV_dispatcher_start() {
    CoreTimerType *tmr;
    int sleep_interval = 20;
    int delta;
    while (pcore_->isActive()) {
        delta = 20;
        for (int i = 0; i < TIMERS_MAX; i++) {
            tmr = &timers_[i];
            if (!tmr->cb || !tmr->interval) {
                continue;
            }

            tmr->delta -= sleep_interval;
            if (tmr->delta <= 0) {
                tmr->cb(tmr->args);
                tmr->delta += tmr->interval;
                if (tmr->single_shot) {
                    RISCV_unregister_timer(tmr->cb);
                    continue;
                }
            }
            if (delta > tmr->delta) {
                delta = tmr->delta;
            }
        }
        sleep_interval = delta;
        RISCV_sleep_ms(sleep_interval);
    }
}

extern "C" void RISCV_register_timer(int msec, int single_shot,
                                     timer_callback_type cb, void *args) {
    CoreTimerType *tmr = 0;
    for (int i = 0; i < TIMERS_MAX; i++) {
        if (timers_[i].cb == 0) {
            tmr = &timers_[i];
            break;
        }
    }
    if (tmr == 0) {
        RISCV_error("%s", "No available timer slot");
        return;
    }
    tmr->cb = cb;
    tmr->args = args;
    tmr->interval = msec;
    tmr->delta = msec;
    tmr->single_shot = single_shot;
}

extern "C" void RISCV_unregister_timer(timer_callback_type cb) {
    for (int i = 0; i < TIMERS_MAX; i++) {
        if (timers_[i].cb == cb) {
            timers_[i].cb = 0;
            timers_[i].args = 0;
            timers_[i].interval = 0;
            timers_[i].delta = 0;
            timers_[i].single_shot = 0;
        }
    }
}

extern "C" void RISCV_generate_name(AttributeType *name) {
    char str[256];
    pcore_->generateUniqueName("obj", str, sizeof(str));
    name->make_string(str);
}

extern "C" void RISCV_add_default_output(void *iout) {
    pcore_->registerConsole(static_cast<IRawListener *>(iout));
}

extern "C" void RISCV_remove_default_output(void *iout) {
    pcore_->unregisterConsole(static_cast<IRawListener *>(iout));
}

extern "C" void RISCV_set_default_clock(void *iclk) {
    pcore_->setTimestampClk(static_cast<IFace *>(iclk));
}

extern "C" int RISCV_enable_log(const char *filename) {
    return pcore_->openLog(filename);
}

extern "C" void RISCV_disable_log() {
    pcore_->closeLog();
}

extern "C" int RISCV_printf(void *iface, int level, 
                            const char *fmt, ...) {
    int ret = 0;
    va_list arg;
    IFace *iout = reinterpret_cast<IFace *>(iface);
    uint64_t cur_t = pcore_->getTimestamp();

    char *buf = pcore_->getpBufLog();
    size_t buf_sz = pcore_->sizeBufLog();
    pcore_->lockPrintf();
    if (iout == NULL) {
        ret = RISCV_sprintf(buf, buf_sz,
                    "[%" RV_PRI64 "d, \"%s\", \"", cur_t, "unknown");
    } else if (strcmp(iout->getFaceName(), IFACE_SERVICE) == 0) {
        IService *iserv = static_cast<IService *>(iout);
        AttributeType *local_level = 
                static_cast<AttributeType *>(iserv->getAttribute("LogLevel"));
        if (level > static_cast<int>(local_level->to_int64())) {
            pcore_->unlockPrintf();
            return 0;
        }
        ret = RISCV_sprintf(buf, buf_sz,
                "[%" RV_PRI64 "d, \"%s\", \"", cur_t, iserv->getObjName());
    } else if (strcmp(iout->getFaceName(), IFACE_CLASS) == 0) {
        IClass *icls = static_cast<IClass *>(iout);
        ret = RISCV_sprintf(buf, buf_sz,
                "[%" RV_PRI64 "d, \"%s\", \"", cur_t, icls->getClassName());
    } else {
        ret = RISCV_sprintf(buf, buf_sz,
                "[%" RV_PRI64 "d, \"%s\", \"", cur_t, iout->getFaceName());
    }
    va_start(arg, fmt);
#if defined(_WIN32) || defined(__CYGWIN__)
    ret += vsprintf_s(&buf[ret], buf_sz - ret, fmt, arg);
#else
    ret += vsprintf(&buf[ret], fmt, arg);
#endif
    va_end(arg);

    buf[ret++] = '\"';
    buf[ret++] = ']';
    buf[ret++] = '\n';
    buf[ret] = '\0';

    pcore_->outputConsole(buf, ret);
    pcore_->outputLog(buf, ret);
    pcore_->unlockPrintf();
    return ret;
}

extern "C" int RISCV_sprintf(char *s, size_t len, const char *fmt, ...) {
    int ret;
    va_list arg;
    va_start(arg, fmt);
#if defined(_WIN32) || defined(__CYGWIN__)
    ret = vsprintf_s(s, len, fmt, arg);
#else
    ret = vsprintf(s, fmt, arg);
#endif
    va_end(arg);
    return ret;
}

extern "C" int RISCV_sscanf(const char *s, const char *fmt, ...) {
    int ret;
    va_list arg;
    va_start(arg, fmt);
#if defined(_WIN32) || defined(__CYGWIN__)
    ret = vsscanf(s, fmt, arg);
#else
    ret = vsscanf(fmt, s, arg);
#endif
    va_end(arg);
    return ret;
}

/* Suspend thread on certain number of milliseconds */
extern "C" void RISCV_sleep_ms(int ms) {
#if defined(_WIN32) || defined(__CYGWIN__)
    Sleep(ms);
#else
    usleep(1000*ms);
#endif
}

extern "C" uint64_t RISCV_get_time_ms() {
#if defined(_WIN32) || defined(__CYGWIN__)
    return 1000*(clock()/CLOCKS_PER_SEC);
#else
    struct timeval tc;
    gettimeofday(&tc, NULL);
    return 1000*tc.tv_sec + tc.tv_usec/1000;
#endif
}

extern "C" int RISCV_get_pid() {
#if defined(_WIN32) || defined(__CYGWIN__)
    return _getpid();
#else
    return getpid();
#endif
}

extern "C" void RISCV_memory_barrier() {
#if defined(_WIN32) || defined(__CYGWIN__)
    MemoryBarrier();
#else
    __sync_synchronize();
#endif
}

extern "C" void RISCV_thread_create(void *data) {
    LibThreadType *p = (LibThreadType *)data;
#if defined(_WIN32) || defined(__CYGWIN__)
    p->Handle = (thread_def)_beginthreadex(0, 0, p->func, p->args, 0, 0);
#else
    pthread_create(&p->Handle, 0, p->func, p->args);
#endif
}

extern "C" uint64_t RISCV_thread_id() {
	  uint64_t r;
#if defined(_WIN32) || defined(__CYGWIN__)
	  r = GetCurrentThreadId();
#else
	  r = pthread_self();
#endif
	  return r;
}

extern "C" void RISCV_thread_join(thread_def th, int ms) {
#if defined(_WIN32) || defined(__CYGWIN__)
    WaitForSingleObject(th, ms);
#else
    pthread_join(th, 0);
#endif
}

extern "C" void RISCV_event_create(event_def *ev, const char *name) {
#if defined(_WIN32) || defined(__CYGWIN__)
    char event_name[256];
    wchar_t wevent_name[256];
    size_t converted;
    pcore_->generateUniqueName("", event_name, sizeof(event_name));
    mbstowcs_s(&converted, wevent_name, event_name, sizeof(event_name));
    ev->state = false;
    ev->cond = CreateEventW(
        NULL,               // default security attributes
        TRUE,               // manual-reset event
        FALSE,              // initial state is nonsignaled
        wevent_name         // object name
        );
#else
    pthread_mutex_init(&ev->mut, NULL);
    pthread_cond_init(&ev->cond, NULL);
    ev->state = false;
#endif
}

extern "C" void RISCV_event_close(event_def *ev) {
#if defined(_WIN32) || defined(__CYGWIN__)
    CloseHandle(ev->cond);
#else
    pthread_mutex_destroy(&ev->mut);
    pthread_cond_destroy(&ev->cond);
#endif
}

extern "C" void RISCV_event_set(event_def *ev) {
#if defined(_WIN32) || defined(__CYGWIN__)
    ev->state = true;
    SetEvent(ev->cond);
#else
    pthread_mutex_lock(&ev->mut);
    ev->state = true;
    pthread_mutex_unlock(&ev->mut);
    pthread_cond_signal(&ev->cond);
#endif
}

extern "C" int RISCV_event_is_set(event_def *ev) {
    return ev->state ? 1: 0;
}

extern "C" void RISCV_event_clear(event_def *ev) {
#if defined(_WIN32) || defined(__CYGWIN__)
    ev->state = false;
    ResetEvent(ev->cond);
#else
    ev->state = false;
#endif
}

extern "C" void RISCV_event_wait(event_def *ev) {
#if defined(_WIN32) || defined(__CYGWIN__)
    WaitForSingleObject(ev->cond, INFINITE);
#else
    int result = 0;
    pthread_mutex_lock(&ev->mut);
    while (result == 0 && !ev->state) {
        result = pthread_cond_wait(&ev->cond, &ev->mut);
    } 
    pthread_mutex_unlock(&ev->mut);
#endif
}

extern "C" int RISCV_event_wait_ms(event_def *ev, int ms) {
#if defined(_WIN32) || defined(__CYGWIN__)
    DWORD wait_ms = ms;
    if (ms == 0) {
        wait_ms = INFINITE;
    }
    if (WAIT_TIMEOUT == WaitForSingleObject(ev->cond, wait_ms)) {
        return 1;
    }
    return 0;
#else
    struct timeval tc;
    struct timespec ts;
    int next_us;
    int result = 0;
    gettimeofday(&tc, NULL);
    next_us = tc.tv_usec + 1000 * ms;
    ts.tv_sec = tc.tv_sec + (next_us / 1000000);
    next_us -= 1000000 * (next_us / 1000000);
    ts.tv_nsec = 1000 * next_us;

    pthread_mutex_lock(&ev->mut);
    while (result == 0 && !ev->state) {
        result = pthread_cond_timedwait(&ev->cond, &ev->mut, &ts);
    }
    pthread_mutex_unlock(&ev->mut);
    if (ETIMEDOUT == result) {
        return 1;
    }
    return 0;
#endif
}

extern "C" sharemem_def RISCV_memshare_create(const char *name, int sz) {
    sharemem_def ret = 0;
#if defined(_WIN32) || defined(__CYGWIN__)
    wchar_t w_name[1024];
    size_t bytes_converted;
    mbstowcs_s(&bytes_converted, w_name, name, 256);

    ret = CreateFileMappingW(
                 INVALID_HANDLE_VALUE,  // use paging file
                 NULL,                  // default security
                 PAGE_READWRITE,      // read/write access
                 0,                   // maximum object size (high-order DWORD)
                 sz,                  // maximum object size (low-order DWORD)
                 w_name);               // name of mapping object
#else
    ret = shm_open(name, O_CREAT | O_RDWR, 0777);
    if (ret > 0) {
        if ( ftruncate(ret, sz + 1) == -1 ) {
            RISCV_error("Couldn't set mem size %s", name);
            ret = 0;
        }
    } else {
        ret = 0;
    }
#endif
    if (ret == 0) {
        RISCV_error("Couldn't create map object %s", name);
    }
    return ret;
}

extern "C" void* RISCV_memshare_map(sharemem_def h, int sz) {
    void *ret = 0;
#if defined(_WIN32) || defined(__CYGWIN__)
    ret = MapViewOfFile(h,   // handle to map object
                        FILE_MAP_ALL_ACCESS, // read/write permission
                        0,
                        0,
                        sz);
#else
    ret = mmap(NULL, sz + 1, PROT_READ|PROT_WRITE, MAP_SHARED, h, 0);
    if (ret < 0) {
        ret = 0;
    }
#endif
    if (ret == 0) {
        RISCV_error("Couldn't map view of file with size %d", sz);
    }
    return ret;
}

extern "C" void RISCV_memshare_unmap(void *buf, int sz) {
#if defined(_WIN32) || defined(__CYGWIN__)
    UnmapViewOfFile(buf);
#else
    munmap(buf, sz + 1);
#endif
}

extern "C" void RISCV_memshare_delete(sharemem_def h) {
#if defined(_WIN32) || defined(__CYGWIN__)
    CloseHandle(h);
#else
    close(h);
#endif
}

extern "C" int RISCV_mutex_init(mutex_def *mutex) {
#if defined(_WIN32) || defined(__CYGWIN__)
    InitializeCriticalSection(mutex);
#else
    pthread_mutexattr_t attr;

    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(mutex, &attr);
#endif
    return 0;
}

extern "C" int RISCV_mutex_lock(mutex_def *mutex) {
#if defined(_WIN32) || defined(__CYGWIN__)
    EnterCriticalSection(mutex);
#else
    pthread_mutex_lock(mutex) ;
#endif
    return 0;
}

extern "C" int RISCV_mutex_unlock(mutex_def * mutex) {
#if defined(_WIN32) || defined(__CYGWIN__)
    LeaveCriticalSection(mutex);
#else
    pthread_mutex_unlock(mutex);
#endif
    return 0;
}

extern "C" int RISCV_mutex_destroy(mutex_def *mutex) {
#if defined(_WIN32) || defined(__CYGWIN__)
    DeleteCriticalSection(mutex);
#else
    pthread_mutex_destroy(mutex);
#endif
    return 0;
}

extern "C" void *RISCV_malloc(uint64_t sz) {
    return malloc((size_t)sz);
}

extern "C" void RISCV_free(void *p) {
    if (p) {
        free(p);
    }
}

extern "C" int RISCV_get_core_folder(char *out, int sz) {
#if defined(_WIN32) || defined(__CYGWIN__)
    HMODULE hm = NULL;
    if (!GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
            GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            (LPCSTR)&RISCV_get_core_folder,
            &hm)) {
        
        return GetLastError();
    }
    GetModuleFileNameA(hm, out, sz);
#else
    Dl_info dl_info;
    dladdr((void *)RISCV_get_core_folder, &dl_info);
    RISCV_sprintf(out, sz, "%s", dl_info.dli_fname);
#endif
    int n = (int)strlen(out);
    while (n > 0 && out[n] != '\\' && out[n] != '/') n--;

    out[n+1] = '\0';
    return 0;
}

extern "C" int RISCV_get_core_folderw(wchar_t* out, int sz) {
#if defined(_WIN32) || defined(__CYGWIN__)
    GetModuleFileNameW(NULL, out, sz);
#else
    Dl_info dl_info;
    dladdr((void*)RISCV_get_core_folder, &dl_info);
    mbstowcs(out, dl_info.dli_fname, 4096);
#endif
    int n = (int)wcslen(out);
    while (n > 0 && out[n] != L'\\' && out[n] != L'/') {
        n--;
    }

    out[n + 1] = 0;
    return 0;
}

extern "C" void RISCV_set_current_dir() {
#if defined(_WIN32) || defined(__CYGWIN__)
    HMODULE hMod = GetModuleHandle(NULL);
    char path[MAX_PATH] = "";
    GetModuleFileNameA(hMod, path, MAX_PATH);
#else         // Linux
    // Get path of executable.
    char path[1024];
    ssize_t n = readlink("/proc/self/exe", path,
                         sizeof(path)/sizeof(path[0]) - 1);
    if (n == -1) {
        return;
    }
    path[n] = 0;
#endif

    size_t i;
    for(i = strlen(path) - 1; i > 0 && path[i] != '/' && path[i] != '\\'; --i);
    path[i] = '\0';

#if defined(_WIN32) || defined(__CYGWIN__)
    SetCurrentDirectoryA(path);
#else         // Linux
    chdir(path);
#endif
}

void put_char(char s, FILE *f) {
    fputc(s, f);
    fflush(f);
}

// check if this attribute like list with items <= 2: [a,b]
bool is_single_line(const char *s) {
    bool ret = false;
    if (*s == '[') {
        const char *pcheck = s + 1;
        int item_cnt = 0;
        int symbol_cnt = 0;
        while (*pcheck != ']') {
            symbol_cnt++;
            if (*pcheck == ',') {
                item_cnt++;
            } else if (*pcheck == '[' || *pcheck == '{') {
                item_cnt = 100;
                break;
            }
            ++pcheck;
        }
        if (item_cnt <= 2 && symbol_cnt < 80) {
            ret = true;
        }
    }
    return ret;
}

const char *writeAttrToFile(FILE *f, const char *s, int depth) {
    const char *ret = s;
    bool end_attr = false;
    bool do_single_line = false;

    if ((*s) == 0) {
        return ret;
    }

    put_char('\n', f);
    for (int i = 0; i < depth; i++) {
        put_char(' ', f);
        put_char(' ', f);
    }
    

    while ((*ret) && !end_attr) {
        put_char(*ret, f);
        if ((*ret) == ',') {
            if (!do_single_line) {
                put_char('\n', f);
                for (int i = 0; i < depth; i++) {
                    put_char(' ', f);
                    put_char(' ', f);
                }
            }
        } else if ((*ret) == ']' || (*ret) == '}') {
            if (do_single_line) {
                do_single_line = false;
            } else {
                return ret;
            }
        } else if ((*ret) == '[' || (*ret) == '{') {
            do_single_line = is_single_line(ret);
            if (!do_single_line) {
                ret = writeAttrToFile(f, ret + 1, depth + 1);
            }
        }
        ++ret;
    }
    
    return ret;
}

void RISCV_write_json_file(const char *filename, const char *s) {
    FILE *f = fopen(filename, "wb");
    if (!f) {
        return;
    }
    writeAttrToFile(f, s, 0);
    fclose(f);
}

int json_file_readline(FILE *f, char *buf, int &sz) {
    int c = getc(f);
    int ret = sz;
    while (c != EOF) {
        if (ret == sz && (c == '\n' || c == '\r')) {
            // empty line
            c = getc(f);
            continue;
        }
        buf[ret++] = static_cast<char>(c);
        buf[ret] = '\0';
        if (c == '\n') {
            break;
        }
        c = getc(f);
    }
    if (c == EOF && sz != ret) {
        c = buf[ret - 1];   // EOF but new line is available
    }
    sz = ret;
    return c;
}

void make_path_relative_newname(const char *fullname, char *newname) {
    AttributeType tnewname(newname);
    size_t tsz = strlen(fullname);
    while (tsz > 0 && fullname[tsz - 1] != '\\' && fullname[tsz - 1] != '/') {
        tsz--;
    }
    memcpy(newname, fullname, tsz);
    memcpy(&newname[tsz], tnewname.to_string(), tnewname.size());
    newname[tsz + tnewname.size()] = '\0';
}

int json_file_readall(const char *filename, char **pout) {
    FILE *f = fopen(filename, "rb");
    char *incdata;
    int textcnt;
    int incsz;
    int fsz;
    int ret = 0;
#ifdef REPO_PATH
    const char* repo_path = REPO_PATH;
#else
    const char *repo_path = ".";
#endif
    if (!f) {
        return 0;
    }
    fseek(f, 0, SEEK_END);
    fsz = ftell(f);
    fseek(f, 0, SEEK_SET);

    (*pout) = new char[fsz];
    memset((*pout), 0, fsz);

    textcnt = 0;
    char *psub1;
    while (json_file_readline(f, (*pout), textcnt) != EOF) {
        psub1 = strstr(&(*pout)[ret], "${REPO_PATH}");
        if (psub1 != 0) {
            int fsznew = fsz + static_cast<int>(strlen(repo_path));
            char *pnew = new char[fsznew];
            size_t foundpos = psub1 - (*pout);
            memset(pnew, 0, fsznew);
            memcpy(pnew, *pout, foundpos);
            strcpy(&pnew[foundpos], repo_path);
            memcpy(&pnew[foundpos + strlen(repo_path)],
                   &(*pout)[foundpos + strlen("${REPO_PATH}")],
                   fsz - (foundpos + strlen("${REPO_PATH}")));

            fsz = fsznew;
            delete [] (*pout);
            (*pout) = pnew;
            textcnt += static_cast<int>(
                strlen(repo_path) - strlen("${REPO_PATH}"));
        }
        psub1 = strstr(&(*pout)[ret], "#include");
        if (psub1 != 0) {
            char fname[4096];
            char *pf = fname;
            psub1 += 8;     // skip #include
            while (*psub1 == ' ') {
                psub1++;
            }
            if (*psub1 == '"' || *psub1 == '\'') {
                psub1++;
            }
            while (*psub1 && *psub1 != '"' && *psub1 != '\'') {
                *pf++ = *psub1++;
                *pf = '\0';
            }
            // todo:  read file
            make_path_relative_newname(filename, fname);
            incsz = json_file_readall(fname, &incdata);
            if (incsz) {
                // realloc buffer:
                char *t1 = new char [fsz + incsz];
                memset(t1, 0, fsz + incsz);
                memcpy(t1, (*pout), ret);
                memcpy(&t1[ret], incdata, incsz);
                delete [] incdata;
                delete [] (*pout);
                (*pout) = t1;
                fsz += incsz;
                textcnt = ret + incsz;
            }
        }
        ret = textcnt;
    }
    fclose(f);
    return ret;
}

int RISCV_read_json_file(const char *filename, void *outattr) {
    AttributeType *out = reinterpret_cast<AttributeType *>(outattr);
    char *tbuf;
    int sz = json_file_readall(filename, &tbuf);
    if (sz) {
        out->make_data(sz, tbuf);
        delete [] tbuf;
    }
    return sz;
}

}  // namespace debugger

