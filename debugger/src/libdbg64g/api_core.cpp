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

class CoreService : public IService {
public:
    CoreService(const char *name) : IService("CoreService") {}
};

static CoreService core_("core");
static AttributeType Config_;
static AttributeType listClasses_(Attr_List);
static AttributeType listHap_(Attr_List);
static AttributeType listPlugins_(Attr_List);
extern mutex_def mutex_printf;

extern void _load_plugins(AttributeType *list);
extern void _unload_plugins(AttributeType *list);


IFace *getInterface(const char *name) {
    return core_.getInterface(name);
}


extern "C" int RISCV_init() {
    RISCV_mutex_init(&mutex_printf);

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
    IService *iserv;
    const AttributeType *objs;

    // Pre-deletion
    for (unsigned i = 0; i < listClasses_.size(); i++) {
        icls = static_cast<IClass *>(listClasses_[i].to_iface());
        objs = (icls->getInstanceList());
        for (unsigned n = 0; n < objs->size(); n++) {
            iserv = static_cast<IService *>((*objs)[n].to_iface());
            icls->predeleteServices(iserv);
        }
    }

#if defined(_WIN32) || defined(__CYGWIN__)
    WSACleanup();
#endif

    _unload_plugins(&listPlugins_);
    RISCV_mutex_lock(&mutex_printf);
    RISCV_mutex_destroy(&mutex_printf);
}

extern "C" void RISCV_set_configuration(AttributeType *cfg) {
    IClass *icls;
    IService *iserv;

    Config_.clone(cfg);
    if (!Config_.is_dict()) {
        RISCV_error("Wrong configuration.", NULL);
        return;
    }

    AttributeType &Services = Config_["Services"];
    if (Services.is_list()) {
        for (unsigned i = 0; i < Services.size(); i++) {
            icls = static_cast<IClass *>(
                RISCV_get_class(Services[i]["Class"].to_string()));
            if (icls == NULL) {
                RISCV_error("Class %s not found", 
                             Services[i]["Class"].to_string());
                continue;
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
            ihap->hapTriggered(HAP_ConfigDone);
        }
    }
}

extern "C" const char *RISCV_get_configuration() {
    IClass *icls;
    AttributeType ret(Attr_Dict);
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
                AttributeType t1(iface);
                list->add_to_list(&t1);
            }
        }
    }
}

extern "C" void RISCV_get_clock_services(AttributeType *list) {
    RISCV_get_services_with_iface(IFACE_CLOCK, list);
}

extern "C" void RISCV_break_simulation() {
    AttributeType t1;
    IThread *ith;
    RISCV_get_services_with_iface(IFACE_THREAD, &t1);

    for (unsigned i = 0; i < t1.size(); i++) {
        ith = static_cast<IThread *>(static_cast<IThread *>(t1[i].to_iface()));
        ith->breakSignal();
    }

    IHap *ihap;
    for (unsigned i = 0; i < listHap_.size(); i++) {
        ihap = static_cast<IHap *>(listHap_[i].to_iface());
        if (ihap->getType() == HAP_BreakSimulation) {
            ihap->hapTriggered(HAP_BreakSimulation);
        }
    }
}

}  // namespace debugger
