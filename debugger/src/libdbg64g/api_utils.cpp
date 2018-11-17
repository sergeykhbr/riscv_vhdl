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

#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <dirent.h>
#if defined(_WIN32) || defined(__CYGWIN__)
#else
    #include <dlfcn.h>
#endif
#include "api_types.h"
#include "api_utils.h"
#include "attribute.h"
#include "iservice.h"
#include "iclass.h"
#include "coreservices/irawlistener.h"
#include "coreservices/iclock.h"

namespace debugger {

/** Plugin Entry point type definition */
typedef void (*plugin_init_proc)();

/** Temporary buffer for the log messages. */
static char bufLog[1024*1024];
static int uniqueIdx_ = 0;

/** Redirect output to specified console. */
static AttributeType default_console(Attr_List);
mutex_def mutexDefaultConsoles_;

/** */
static FILE *logfile_ = 0;
static IClock *iclk_ = 0;
static AttributeType log_file(Attr_String);

/** Mutex to avoid concurency for the output stream among threads. */
mutex_def mutex_printf;

/** Core log message interface object. */
extern IFace *getInterface(const char *name);

extern "C" void RISCV_generate_name(AttributeType *name) {
    char str[256];
    RISCV_sprintf(str, sizeof(str), "obj_%08x", uniqueIdx_++);
    name->make_string(str);
}

extern "C" void RISCV_add_default_output(void *iout) {
    AttributeType lstn(static_cast<IRawListener *>(iout));
    RISCV_mutex_lock(&mutexDefaultConsoles_);
    default_console.add_to_list(&lstn);
    RISCV_mutex_unlock(&mutexDefaultConsoles_);
}

extern "C" void RISCV_remove_default_output(void *iout) {
    RISCV_mutex_lock(&mutexDefaultConsoles_);
    for (unsigned i = 0; i < default_console.size(); i++) {
        if (default_console[i].to_iface() == iout) {
            default_console.remove_from_list(i);
            break;
        }
    }
    RISCV_mutex_unlock(&mutexDefaultConsoles_);
}

extern "C" void RISCV_set_default_clock(void *iclk) {
    iclk_ = static_cast<IClock *>(iclk);
}

extern "C" int RISCV_enable_log(const char *filename) {
    if (logfile_) {
        fclose(logfile_);
        logfile_ = NULL;
    }
    logfile_ = fopen(filename, "w");
    if (!logfile_) {
        return 1;
    }
    return 0;
}

extern "C" void RISCV_disable_log() {
    if (logfile_) {
        fclose(logfile_);
    }
    logfile_ = 0;
}

extern "C" void RISCV_print_bin(int level, char *buf, int len) {
    std::cout.write(buf, len);
    //osLog.write(buf,len);
    //osLog.flush();

}

extern "C" int RISCV_printf(void *iface, int level, 
                            const char *fmt, ...) {
    int ret = 0;
    va_list arg;
    IFace *iout = reinterpret_cast<IFace *>(iface);
    uint64_t cur_t = 0;
    if (iclk_) {
        cur_t = iclk_->getStepCounter();
    }

    RISCV_mutex_lock(&mutex_printf);
    if (iout == NULL) {
        ret = RISCV_sprintf(bufLog, sizeof(bufLog),
                    "[%" RV_PRI64 "d, \'%s\', \'", cur_t, "unknown");
    } else if (strcmp(iout->getFaceName(), IFACE_SERVICE) == 0) {
        IService *iserv = static_cast<IService *>(iout);
        AttributeType *local_level = 
                static_cast<AttributeType *>(iserv->getAttribute("LogLevel"));
        if (level > static_cast<int>(local_level->to_int64())) {
            RISCV_mutex_unlock(&mutex_printf);
            return 0;
        }
        ret = RISCV_sprintf(bufLog, sizeof(bufLog),
                "[%" RV_PRI64 "d, \'%s\', \'", cur_t, iserv->getObjName());
    } else if (strcmp(iout->getFaceName(), IFACE_CLASS) == 0) {
        IClass *icls = static_cast<IClass *>(iout);
        ret = RISCV_sprintf(bufLog, sizeof(bufLog),
                "[%" RV_PRI64 "d, \'%s\', \'", cur_t, icls->getClassName());
    } else {
        ret = RISCV_sprintf(bufLog, sizeof(bufLog),
                "[%" RV_PRI64 "d, \'%s\', \'", cur_t, iout->getFaceName());
    }
    va_start(arg, fmt);
#if defined(_WIN32) || defined(__CYGWIN__)
    ret += vsprintf_s(&bufLog[ret], sizeof(bufLog) - ret, fmt, arg);
#else
    ret += vsprintf(&bufLog[ret], fmt, arg);
#endif
    va_end(arg);

    bufLog[ret++] = '\'';
    bufLog[ret++] = ']';
    bufLog[ret++] = '\n';
    bufLog[ret] = '\0';
    IRawListener *ilstn;
    RISCV_mutex_lock(&mutexDefaultConsoles_);
    for (unsigned i = 0; i < default_console.size(); i++) {
        ilstn = static_cast<IRawListener *>(default_console[i].to_iface());
        ilstn->updateData(bufLog, ret);
    }
    RISCV_mutex_unlock(&mutexDefaultConsoles_);
    if (logfile_) {
        fwrite(bufLog, ret, 1, logfile_);
        fflush(logfile_);
    }
    RISCV_mutex_unlock(&mutex_printf);
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
    int pid = RISCV_get_pid();
    char event_name[256];
    RISCV_sprintf(event_name, sizeof(event_name), "%s_%d", name, pid);
    ev->state = false;
    ev->cond = CreateEvent(
        NULL,               // default security attributes
        TRUE,               // manual-reset event
        FALSE,              // initial state is nonsignaled
        TEXT(event_name)    // object name
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
    ret = CreateFileMapping(
                 INVALID_HANDLE_VALUE,  // use paging file
                 NULL,                  // default security
                 PAGE_READWRITE,      // read/write access
                 0,                   // maximum object size (high-order DWORD)
                 sz,                  // maximum object size (low-order DWORD)
                 name);               // name of mapping object
#else
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
#endif
    if (ret == 0) {
        RISCV_error("Couldn't map view of file with size %d", sz);
    }
    return ret;
}

extern "C" void RISCV_memshare_unmap(void *buf) {
#if defined(_WIN32) || defined(__CYGWIN__)
    UnmapViewOfFile(buf);
#else
#endif
}

extern "C" void RISCV_memshare_delete(sharemem_def h) {
#if defined(_WIN32) || defined(__CYGWIN__)
    CloseHandle(h);
#else
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

extern "C" void RISCV_set_current_dir() {
#if defined(_WIN32) || defined(__CYGWIN__)
    HMODULE hMod = GetModuleHandle(NULL);
    char path[MAX_PATH] = "";
    GetModuleFileNameA(hMod, path, MAX_PATH);
#else         // Linux
    // Get path of executable.
    char path[1024];
    ssize_t n = readlink("/proc/self/exe", path, sizeof(path)/sizeof(path[0]) - 1);
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

/**
 * @brief   Loading all plugins from the 'plugins' sub-folder.
 * @details I suppose only one folders level so no itteration algorithm.
 */
void _load_plugins(AttributeType *list) {
#if defined(_WIN32) || defined(__CYGWIN__)
    HMODULE hlib;
#else
    void *hlib;
#endif
    DIR *dir;
    struct dirent *ent;
    plugin_init_proc plugin_init;
    char curdir[1024];
    std::string plugin_dir, plugin_lib;

    RISCV_get_core_folder(curdir, sizeof(curdir));

    plugin_dir = std::string(curdir) + "plugins/";
    dir = opendir(plugin_dir.c_str());

    if (dir == NULL) {
          RISCV_error("Plugins directory '%s' not found", curdir);
          return;
    }

    while ((ent = readdir(dir)) != NULL) {
        if (ent->d_type != DT_REG) {
            continue;
        }
        if ((strstr(ent->d_name, ".dll") == NULL)
         && (strstr(ent->d_name, ".so") == NULL)) {
            continue;
        }
        plugin_lib = plugin_dir + std::string(ent->d_name);
#if defined(_WIN32) || defined(__CYGWIN__)
        if ((hlib = LoadLibrary(plugin_lib.c_str())) == 0) {
            continue;
        }
        plugin_init = (plugin_init_proc)GetProcAddress(hlib, "plugin_init");
        if (!plugin_init) {
            FreeLibrary(hlib);
            continue;
        }
#else
        // reset errors
        dlerror();
        if ((hlib = dlopen(plugin_lib.c_str(), RTLD_LAZY)) == 0) {
            printf("Can't open library '%s': %s\n",
                       plugin_lib.c_str(), dlerror());
            continue;
        }
        plugin_init = (plugin_init_proc)dlsym(hlib, "plugin_init");
        if (dlerror()) {
            printf("Not found plugin_init() in file '%s'\n",
                       plugin_lib.c_str());
            dlclose(hlib);
            continue;
        }
#endif

        RISCV_info("Loading plugin file '%s'", plugin_lib.c_str());
        plugin_init();

        AttributeType item;
        item.make_list(2);
        item[0u] = AttributeType(plugin_lib.c_str());
        item[1] = AttributeType(Attr_UInteger, 
                                reinterpret_cast<uint64_t>(hlib));
        list->add_to_list(&item);
    }
    closedir(dir);
}

void _unload_plugins(AttributeType *list) {
    const char *plugin_name;
    for (unsigned i = 0; i < list->size(); i++) {
        if (!(*list)[i].is_list()) {
            RISCV_error("Can't free plugin[%d] library", i);
            continue;
        }
        plugin_name = (*list)[i][0u].to_string();
#if defined(_WIN32) || defined(__CYGWIN__)
        HMODULE hlib = reinterpret_cast<HMODULE>((*list)[i][1].to_uint64());
        FreeLibrary(hlib);
#else
        void *hlib = reinterpret_cast<void *>((*list)[i][1].to_uint64());
        if (strstr(plugin_name, "gui_plugin") == 0) {
            /** It's a workaround to avoid SIGSEGV on application exiting.
             *  It's a specific of dynamically linked QT-libraries. They 
             *  create and use global variable freeing on application
             *  closing.
             */
            dlclose(hlib);
        }
#endif
    }
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
    FILE *f = fopen(filename, "w");
    if (!f) {
        return;
    }
    writeAttrToFile(f, s, 0);
    fclose(f);
}

int RISCV_read_json_file(const char *filename, void *outattr) {
    AttributeType *out = reinterpret_cast<AttributeType *>(outattr);
    FILE *f = fopen(filename, "r");
    if (!f) {
        return 0;
    }
    fseek(f, 0, SEEK_END);
    int sz = ftell(f);
    out->make_data(sz);

    fseek(f, 0, SEEK_SET);
    fread(out->data(), sz, 1, f);
    return sz;
}

}  // namespace debugger
