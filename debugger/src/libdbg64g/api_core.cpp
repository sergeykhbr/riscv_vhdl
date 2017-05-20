/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Core API methods implementation.
 */

#include <string>
#include "api_core.h"
#include "api_types.h"
#include "iclass.h"
#include "ihap.h"
#include "coreservices/ithread.h"
#include "coreservices/iclock.h"

namespace debugger {

static AttributeType Config_;
static AttributeType listClasses_(Attr_List);
static AttributeType listHap_(Attr_List);
static AttributeType listPlugins_(Attr_List);
extern mutex_def mutex_printf;
extern mutex_def mutexDefaultConsoles_;

extern void _load_plugins(AttributeType *list);
extern void _unload_plugins(AttributeType *list);

class CoreService : public IService {
public:
    CoreService(const char *name) : IService("CoreService") {
        active_ = 1;
        RISCV_mutex_init(&mutex_printf);
        RISCV_mutex_init(&mutexDefaultConsoles_);
        RISCV_event_create(&mutexExiting_, "mutexExiting_");
        //logLevel_.make_int64(LOG_DEBUG);  // default = LOG_ERROR
    }
    virtual ~CoreService() {
        RISCV_event_close(&mutexExiting_);
    }

    int isActive() { return active_; }
    void shutdown() { active_ = 0; }
    bool isExiting() { return mutexExiting_.state; }
    void setExiting() { RISCV_event_set(&mutexExiting_); }
private:
    int active_;
    event_def mutexExiting_;
};
static CoreService core_("core");


IFace *getInterface(const char *name) {
    return core_.getInterface(name);
}


extern "C" int RISCV_init() {
#if defined(_WIN32) || defined(__CYGWIN__)
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData)) {
        RISCV_error("Can't initialize sockets library", NULL);
    }
#endif

    _load_plugins(&listPlugins_);
    return 0;
}

extern "C" void RISCV_cleanup() {
    IClass *icls;

    // Pre-deletion
    for (unsigned i = 0; i < listClasses_.size(); i++) {
        icls = static_cast<IClass *>(listClasses_[i].to_iface());
        icls->predeleteServices();
    }

#if defined(_WIN32) || defined(__CYGWIN__)
    WSACleanup();
#endif

    _unload_plugins(&listPlugins_);
    RISCV_mutex_lock(&mutex_printf);
    RISCV_mutex_destroy(&mutex_printf);
    RISCV_mutex_lock(&mutexDefaultConsoles_);
    RISCV_mutex_destroy(&mutexDefaultConsoles_);
    RISCV_disable_log();
}

extern "C" int RISCV_set_configuration(AttributeType *cfg) {
    IClass *icls;
    IService *iserv;

    Config_.clone(cfg);
    if (!Config_.is_dict()) {
        RISCV_error("Wrong configuration.", NULL);
        return -1;
    }

    AttributeType &Services = Config_["Services"];
    if (Services.is_list()) {
        for (unsigned i = 0; i < Services.size(); i++) {
            icls = static_cast<IClass *>(
                RISCV_get_class(Services[i]["Class"].to_string()));
            if (icls == NULL) {
                RISCV_error("Class %s not found", 
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
                iserv = icls->createService(Instances[n]["Name"].to_string());
                iserv->initService(&Instances[n]["Attr"]);
            }
        }
    }

    // Post initialization
    for (unsigned i = 0; i < listClasses_.size(); i++) {
        icls = static_cast<IClass *>(listClasses_[i].to_iface());
        icls->postinitServices();
    }

    RISCV_printf(getInterface(IFACE_SERVICE), 0, "%s",
    "\n**********************************************************\n"
    "  RISC-V debugger\n"
    "  Author: Sergey Khabarov - sergeykhbr@gmail.com\n"
    "  Copyright 2016 GNSS Sensor Ltd. All right reserved.\n"
    "**********************************************************");

    IHap *ihap;
    for (unsigned i = 0; i < listHap_.size(); i++) {
        ihap = static_cast<IHap *>(listHap_[i].to_iface());
        if (ihap->getType() == HAP_ConfigDone) {
            ihap->hapTriggered(getInterface(IFACE_SERVICE), 
                        HAP_ConfigDone, "Initial config done");
        }
    }
    return 0;
}

extern "C" const char *RISCV_get_configuration() {
    IClass *icls;
    AttributeType ret(Attr_Dict);
    ret["GlobalSettings"] = Config_["GlobalSettings"];
    ret["Services"].make_list(0);
    for (unsigned i = 0; i < listClasses_.size(); i++) {
        icls = static_cast<IClass *>(listClasses_[i].to_iface());
        AttributeType val = icls->getConfiguration();
        ret["Services"].add_to_list(&val);
    }
    return ret.to_config();
}

extern "C" const AttributeType *RISCV_get_global_settings() {
    return &Config_["GlobalSettings"];
}

extern "C" void RISCV_register_class(IFace *icls) {
    AttributeType item(icls);
    listClasses_.add_to_list(&item);
}

extern "C" void RISCV_register_hap(IFace *ihap) {
    AttributeType item(ihap);
    listHap_.add_to_list(&item);
}

extern "C" void RISCV_trigger_hap(IFace *isrc, int type, 
                                  const char *descr) {
    IHap *ihap;
    EHapType etype = static_cast<EHapType>(type);
    for (unsigned i = 0; i < listHap_.size(); i++) {
        ihap = static_cast<IHap *>(listHap_[i].to_iface());
        if (ihap->getType() == etype) {
            ihap->hapTriggered(isrc, etype, descr);
        }
    }
}


extern "C" IFace *RISCV_get_class(const char *name) {
    IClass *icls;
    for (unsigned i = 0; i < listClasses_.size(); i++) {
        icls = static_cast<IClass *>(listClasses_[i].to_iface());
        if (strcmp(name, icls->getClassName()) == 0) {
            return icls;
        }
    }
    return NULL;
}

extern "C" IFace *RISCV_create_service(IFace *iclass, const char *name, 
                                        AttributeType *args) {
    IClass *icls = static_cast<IClass *>(iclass);
    IService *iobj = icls->createService(name);
    iobj->initService(args);
    iobj->postinitService();
    return iobj;
}

extern "C" IFace *RISCV_get_service(const char *name) {
    IClass *icls;
    IService *iserv;
    for (unsigned i = 0; i < listClasses_.size(); i++) {
        icls = static_cast<IClass *>(listClasses_[i].to_iface());
        if ((iserv = icls->getInstance(name)) != NULL) {
            return iserv;
        }
    }
    return NULL;
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

extern "C" void RISCV_get_services_with_iface(const char *iname,  
                                             AttributeType *list) {
    IClass *icls;
    IService *iserv;
    IFace *iface;
    const AttributeType *tlist;
    list->make_list(0);
    
    for (unsigned i = 0; i < listClasses_.size(); i++) {
        icls = static_cast<IClass *>(listClasses_[i].to_iface());
        tlist = icls->getInstanceList();
        if (tlist->size()) {
            iserv = static_cast<IService *>((*tlist)[0u].to_iface());
            iface = iserv->getInterface(iname);
            if (iface) {
                AttributeType t1(iserv);
                list->add_to_list(&t1);
            }
        }
    }
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

    RISCV_trigger_hap(getInterface(IFACE_SERVICE),
                      HAP_BreakSimulation, "Exiting");
    printf("All threads were stopped!\n");
    core_.shutdown();
    return 0;
}

struct TimerType {
    timer_callback_type cb;
    void *args;
    int interval;
    int delta;
};

static const int TIMERS_MAX = 2;
static TimerType timers_[TIMERS_MAX] = {{0}};

extern "C" void RISCV_break_simulation() {
    if (core_.isExiting()) {
        return;
    }
    core_.setExiting();
    LibThreadType data;
    data.func = reinterpret_cast<lib_thread_func>(safe_exit_thread);
    data.args = 0;
    RISCV_thread_create(&data);
}


extern "C" void RISCV_dispatcher_start() {
    TimerType *tmr;
    int sleep_interval = 20;
    int delta;
    while (core_.isActive()) {
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
            }
            if (delta > tmr->delta) {
                delta = tmr->delta;
            }
        }
        sleep_interval = delta;
        RISCV_sleep_ms(sleep_interval);
    }
}

extern "C" void RISCV_register_timer(int msec,
                                    timer_callback_type cb, void *args) {
    if (!core_.isActive() || core_.isExiting()) {
        return;
    }
    TimerType *tmr = 0;
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
}

extern "C" void RISCV_unregister_timer(timer_callback_type cb) {
    for (int i = 0; i < TIMERS_MAX; i++) {
        if (timers_[i].cb == cb) {
            timers_[i].cb = 0;
            timers_[i].args = 0;
            timers_[i].interval = 0;
            timers_[i].delta = 0;
        }
    }
}

}  // namespace debugger
