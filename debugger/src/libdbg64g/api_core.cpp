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

namespace debugger {

class CoreService : public IService {
public:
    CoreService(const char *name) : IService("CoreService") {}
};

static CoreService core_("core");
static AttributeType Config_;
static AttributeType listClasses_(Attr_List);
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
    for (unsigned i = 0; i < listClasses_.size(); i++) {
        icls = static_cast<IClass *>(listClasses_[i].to_iface());
        objs = (icls->getInstanceList());
        for (unsigned n = 0; n < objs->size(); n++) {
            iserv = static_cast<IService *>((*objs)[n].to_iface());
            icls->deleteServices(iserv);
        }
    }
#if defined(_WIN32) || defined(__CYGWIN__)
    WSACleanup();
#endif

    _unload_plugins(&listPlugins_);
    RISCV_mutex_lock(&mutex_printf);
    RISCV_mutex_destroy(&mutex_printf);
}

extern "C" void RISCV_set_configuration(const char *config) {
    IClass *icls;
    IService *iserv;

    Config_.from_config(config);
    if (!Config_.is_dict()) {
        RISCV_error("Wrong configuration string '%s'.", config);
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

extern "C" void RISCV_register_class(IFace *icls) {
    AttributeType item(icls);
    listClasses_.add_to_list(&item);
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

}  // namespace debugger
