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

#include "iclass.h"
#include "ihap.h"
#include "core.h"
#include "coreservices/iclock.h"
#include "coreservices/irawlistener.h"
#include <string>
#include <stdlib.h>
#include <time.h>
#include <dirent.h>
#if defined(_WIN32) || defined(__CYGWIN__)
#else
    #include <dlfcn.h>
#endif

namespace debugger {

CoreService::CoreService(const char *name) : IService("CoreService") {
    active_ = 1;
    listPlugins_.make_list(0);
    listClasses_.make_list(0);
    listServices_.make_list(0);
    listHap_.make_list(0);
    listConsole_.make_list(0);

    RISCV_mutex_init(&mutexPrintf_);
    RISCV_mutex_init(&mutexDefaultConsoles_);
    RISCV_mutex_init(&mutexLogFile_);
    //logLevel_.make_int64(LOG_DEBUG);  // default = LOG_ERROR
    iclk_ = 0;
    uniqueIdx_ = 0;
    logFile_ = 0;
}

CoreService::~CoreService() {
    closeLog();
    RISCV_mutex_lock(&mutexPrintf_);
    RISCV_mutex_destroy(&mutexPrintf_);
    RISCV_mutex_lock(&mutexDefaultConsoles_);
    RISCV_mutex_destroy(&mutexDefaultConsoles_);
    RISCV_mutex_destroy(&mutexLogFile_);
    RISCV_event_close(&eventExiting_);
}

int CoreService::isActive() {
    return active_;
}

void CoreService::shutdown() {
    active_ = 0;
}

bool CoreService::isExiting() {
    return eventExiting_.state;
}

void CoreService::setExiting() {
    RISCV_event_set(&eventExiting_);
}

int CoreService::setConfig(AttributeType *cfg) {
    Config_.clone(cfg);
    if (!Config_.is_dict()) {
        return -1;
    }
    return 0;
}

void CoreService::getConfig(AttributeType *cfg) {
    IClass *icls;
    cfg->make_dict();
    (*cfg)["GlobalSettings"] = Config_["GlobalSettings"];
    (*cfg)["Services"].make_list(0);
    for (unsigned i = 0; i < listClasses_.size(); i++) {
        icls = static_cast<IClass *>(listClasses_[i].to_iface());
        AttributeType val = icls->getConfiguration();
        (*cfg)["Services"].add_to_list(&val);
    }
    cfg->to_config();
}

int CoreService::createPlatformServices() {
    IClass *icls;
    IService *iserv;
    AttributeType &Services = Config_["Services"];
    if (Services.is_list()) {
        for (unsigned i = 0; i < Services.size(); i++) {
            const char *clsname = Services[i]["Class"].to_string();
            icls = static_cast<IClass *>(RISCV_get_class(clsname));
            if (icls == NULL) {
                printf("Class %s not found\n", 
                                Services[i]["Class"].to_string());
                return -1;
            }
            /** Special global setting for the GUI class: */
            if (strcmp(icls->getClassName(), "GuiPluginClass") == 0) {
                if (!Config_["GlobalSettings"]["GUI"].to_bool()) {
                    RISCV_info("%s", "GUI disabled");
                    continue;
                }
            }

            AttributeType &Instances = Services[i]["Instances"];
            for (unsigned n = 0; n < Instances.size(); n++) {
                iserv =
                    icls->createService(Instances[n]["Name"].to_string());
                iserv->initService(&Instances[n]["Attr"]);
            }
        }
    }
    return 0;
}

void CoreService::postinitPlatformServices() {
    IService *isrv;
    for (unsigned i = 0; i < listServices_.size(); i++) {
        isrv = static_cast<IService *>(listServices_[i].to_iface());
        isrv->postinitService();
    }
}

void CoreService::predeletePlatformServices() {
    IService *isrv;
    for (unsigned i = 0; i < listServices_.size(); i++) {
        isrv = static_cast<IService *>(listServices_[i].to_iface());
        isrv->predeleteService();
    }
}

const AttributeType *CoreService::getGlobalSettings() {
    return &Config_["GlobalSettings"];
}

void CoreService::registerClass(IFace *icls) {
    IClass *it1, *it2;
    it1 = static_cast<IClass *>(icls);
    for (unsigned i = 0; i < listClasses_.size(); i++) {
        it2 = static_cast<IClass *>(listClasses_[i].to_iface());
        if (strcmp(it1->getClassName(), it2->getClassName()) == 0) {
            printf("Error: class %s already registerd\n", it1->getClassName());
            return;
        }
    }
    AttributeType item(icls);
    listClasses_.add_to_list(&item);
}

void CoreService::unregisterClass(const char *clsname) {
    IClass *icls;
    for (unsigned i = 0; i < listClasses_.size(); i++) {
        icls = static_cast<IClass *>(listClasses_[i].to_iface());
        if (strcmp(icls->getClassName(), clsname) == 0) {
            listClasses_.remove_from_list(i);
            break;
        }
    }
}

void CoreService::registerService(IFace *isrv) {
    IService *it1, *it2;
    it1 = static_cast<IService *>(isrv);
    for (unsigned i = 0; i < listServices_.size(); i++) {
        it2 = static_cast<IService *>(listServices_[i].to_iface());
        if (strcmp(it1->getObjName(), it2->getObjName()) == 0) {
            printf("Error: object %s already exists\n", it1->getObjName());
            return;
        }
    }
    AttributeType item(isrv);
    listServices_.add_to_list(&item);
}

void CoreService::unregisterService(const char *srvname) {
    IService *isrv;
    for (unsigned i = 0; i < listServices_.size(); i++) {
        isrv = static_cast<IService *>(listServices_[i].to_iface());
        if (strcmp(isrv->getObjName(), srvname) == 0) {
            listServices_.remove_from_list(i);
            break;
        }
    }
}

void CoreService::registerHap(IFace *ihap) {
    AttributeType item(ihap);
    listHap_.add_to_list(&item);
}

void CoreService::unregisterHap(IFace *ihap) {
    IFace *iface;
    for (unsigned i = 0; i < listHap_.size(); i++) {
        iface = listHap_[i].to_iface();
        if (ihap == iface) {
            listHap_.remove_from_list(i);
            break;
        }
    }
}

void CoreService::registerConsole(IFace *iconsole) {
    AttributeType item(iconsole);
    RISCV_mutex_lock(&mutexDefaultConsoles_);
    listConsole_.add_to_list(&item);
    RISCV_mutex_unlock(&mutexDefaultConsoles_);
}

void CoreService::unregisterConsole(IFace *iconsole) {
    RISCV_mutex_lock(&mutexDefaultConsoles_);
    for (unsigned i = 0; i < listConsole_.size(); i++) {
        if (listConsole_[i].to_iface() == iconsole) {
            listConsole_.remove_from_list(i);
            break;
        }
    }
    RISCV_mutex_unlock(&mutexDefaultConsoles_);
}

/**
 * @brief   Loading all plugins from the 'plugins' sub-folder.
 * @details I suppose only one folders level so no itteration algorithm.
 */
void CoreService::load_plugins() {
    std::string plugin_lib;
    plugin_init_proc plugin_init;

    RISCV_event_create(&eventExiting_, "eventExiting_");

#if defined(_WIN32) || defined(__CYGWIN__)
    HMODULE hlib;
    WDIR* dir;
    struct wdirent* ent;
    wchar_t curdir[4096];
    char strtmp[4096];
    std::wstring plugin_dirw;
    std::wstring plugin_libw;

    RISCV_get_core_folderw(curdir, sizeof(curdir));
    plugin_dirw = std::wstring(curdir) + L"plugins\\";

    dir = wopendir(plugin_dirw.c_str());
    if (dir == NULL) {
        wcstombs(strtmp, curdir, sizeof(curdir));
        RISCV_error("Plugins directory '%s' not found", strtmp);
        return;
    }

    while ((ent = wreaddir(dir)) != NULL) {
        if (ent->d_type != DT_REG) {
            continue;
        }
        if (wcsstr(ent->d_name, L".dll") == NULL) {
            continue;
        }
        plugin_libw = plugin_dirw + std::wstring(ent->d_name);
        if ((hlib = LoadLibraryW(plugin_libw.c_str())) == 0) {
            continue;
        }
        plugin_init = (plugin_init_proc)GetProcAddress(hlib, "plugin_init");
        if (!plugin_init) {
            FreeLibrary(hlib);
            continue;
        }

        wcstombs(strtmp, plugin_libw.c_str(), sizeof(strtmp));
        plugin_lib = std::string(strtmp);
        RISCV_info("Loading plugin file '%s'", plugin_lib.c_str());
        plugin_init();

        /** Save loaded plugin into local list attribute */
        AttributeType item;
        item.make_list(2);
        item[0u] = AttributeType(plugin_lib.c_str());
        item[1] = AttributeType(Attr_UInteger,
            reinterpret_cast<uint64_t>(hlib));
        listPlugins_.add_to_list(&item);
    }
    wclosedir(dir);
#else
    void *hlib;
    DIR* dir;
    struct dirent* ent;
    char curdir[4096];
    std::string plugin_dir;

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
        if (strstr(ent->d_name, ".so") == NULL) {
            continue;
        }

        plugin_lib = plugin_dir + std::string(ent->d_name);

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
        RISCV_info("Loading plugin file '%s'", plugin_lib.c_str());
        plugin_init();

        /** Save loaded plugin into local list attribute */
        AttributeType item;
        item.make_list(2);
        item[0u] = AttributeType(plugin_lib.c_str());
        item[1] = AttributeType(Attr_UInteger,
            reinterpret_cast<uint64_t>(hlib));
        listPlugins_.add_to_list(&item);
    }
    closedir(dir);
#endif
}

void CoreService::unload_plugins() {
    const char *plugin_name;
    for (unsigned i = 0; i < listPlugins_.size(); i++) {
        if (!listPlugins_[i].is_list()) {
            printf("Can't free plugin[%d] library\n", i);
            continue;
        }
        plugin_name = listPlugins_[i][0u].to_string();
#if defined(_WIN32) || defined(__CYGWIN__)
        HMODULE hlib =
            reinterpret_cast<HMODULE>(listPlugins_[i][1].to_uint64());
        FreeLibrary(hlib);
#else
        void *hlib = reinterpret_cast<void *>(listPlugins_[i][1].to_uint64());
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

void CoreService::triggerHap(int type, uint64_t param, const char *descr) {
    IHap *ihap;
    EHapType etype = static_cast<EHapType>(type);
    AttributeType haplist = listHap_;
    // Hap handler can unregister itself so we need to las valid list
    for (unsigned i = 0; i < haplist.size(); i++) {
        ihap = static_cast<IHap *>(haplist[i].to_iface());
        if (ihap->getType() == HAP_All || ihap->getType() == etype) {
            ihap->hapTriggered(etype, param, descr);
        }
    }
    haplist.attr_free();
}

IFace *CoreService::getClass(const char *name) {
    IClass *icls;
    for (unsigned i = 0; i < listClasses_.size(); i++) {
        icls = static_cast<IClass *>(listClasses_[i].to_iface());
        if (strcmp(name, icls->getClassName()) == 0) {
            return icls;
        }
    }
    return NULL;
}

IFace *CoreService::getService(const char *name) {
    IService *isrv;
    for (unsigned i = 0; i < listServices_.size(); i++) {
        isrv = static_cast<IService *>(listServices_[i].to_iface());
        if (strcmp(isrv->getObjName(), name) == 0) {
            return isrv;
        }
    }
    return NULL;
}

void CoreService::getServicesWithIFace(const char *iname,
                                       AttributeType *list) {
    IService *iserv;
    IFace *iface;
    list->make_list(0);
    
    for (unsigned i = 0; i < listServices_.size(); i++) {
        iserv = static_cast<IService *>(listServices_[i].to_iface());
        iface = iserv->getInterface(iname);
        if (iface) {
            AttributeType t1(iserv);
            list->add_to_list(&t1);
        }
    }
}

void CoreService::getIFaceList(const char *iname,
                               AttributeType *list) {
    IService *iserv;
    IFace *iface;
    const AttributeType *tports;
    list->make_list(0);
    
    for (unsigned i = 0; i < listServices_.size(); i++) {
        iserv = static_cast<IService *>(listServices_[i].to_iface());
        iface = iserv->getInterface(iname);
        if (iface) {
            AttributeType t1(iface);
            list->add_to_list(&t1);
        }
        tports = iserv->getPortList();
        for (unsigned k = 0; k < tports->size(); k++) {
            const AttributeType &prt = (*tports)[k];
            // [0] port name; [1] port interface
            iface = prt[1].to_iface();
            if (strcmp(iname, iface->getFaceName()) == 0) {
                AttributeType t1(iface);
                list->add_to_list(&t1);
            }
        }
    }
}

void CoreService::lockPrintf() {
    RISCV_mutex_lock(&mutexPrintf_);
}

void CoreService::unlockPrintf() {
    RISCV_mutex_unlock(&mutexPrintf_);
}

int CoreService::openLog(const char *filename) {
    if (logFile_) {
        fclose(logFile_);
        logFile_ = NULL;
    }
    logFile_ = fopen(filename, "wb");
    if (!logFile_) {
        return 1;
    }
    return 0;
}

void CoreService::closeLog() {
    if (logFile_) {
        fclose(logFile_);
    }
    logFile_ = 0;
}

void CoreService::outputLog(const char *buf, int sz) {
    if (!logFile_) {
        return;
    }
    RISCV_mutex_lock(&mutexLogFile_);
    fwrite(buf, sz, 1, logFile_);
    fflush(logFile_);
    RISCV_mutex_unlock(&mutexLogFile_);
}

void CoreService::outputConsole(const char *buf, int sz) {
    IRawListener *ilstn;
    RISCV_mutex_lock(&mutexDefaultConsoles_);
    for (unsigned i = 0; i < listConsole_.size(); i++) {
        ilstn = static_cast<IRawListener *>(listConsole_[i].to_iface());
        ilstn->updateData(buf, sz);
    }
    RISCV_mutex_unlock(&mutexDefaultConsoles_);
}

void CoreService::generateUniqueName(const char *prefix,
                                     char *out, size_t outsz) {
    RISCV_sprintf(out, outsz, "%s_%d_%08x",
                prefix, RISCV_get_pid(), uniqueIdx_++);
}

uint64_t CoreService::getTimestamp() {
    if (!iclk_) {
        return 0;
    }
    IClock *i = static_cast<IClock *>(iclk_);
    return i->getStepCounter();
}

}  // namespace debugger
