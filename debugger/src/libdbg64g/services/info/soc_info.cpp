#include "soc_info.h"

namespace debugger {

/** Class registration in the Core */
REGISTER_CLASS(SocInfo)

SocInfo::SocInfo(const char *name) 
    : IService(name) {
    registerInterface(static_cast<ISocInfo *>(this));
    registerAttribute("PnpBaseAddress", &pnpBase_);
    registerAttribute("DsuBaseAddress", &dsuBase_);
    registerAttribute("GpioBaseAddress", &gpioBase_);
    registerAttribute("ListCSR", &listCSR_);
    registerAttribute("ListRegs", &listRegs_);

    dsuBase_.make_uint64(0);
    listCSR_.make_list(0);
    listRegs_.make_list(0);
}

void SocInfo::postinitService() {
}

unsigned SocInfo::getRegsTotal() {
    return listRegs_.size();
}

void SocInfo::getRegsList(AttributeType *lst) {
    lst->make_list(listRegs_.size());
    for (unsigned i = 0; i < listRegs_.size(); i++) {
        (*lst)[i] = listRegs_[i][0u];
    }
}

unsigned SocInfo::getCsrTotal() {
    return listCSR_.size();
}

void SocInfo::getCsrList(AttributeType *lst) {
    lst->make_list(listCSR_.size());
    for (unsigned i = 0; i < listCSR_.size(); i++) {
        (*lst)[i] = listCSR_[i][0u];
    }
}

uint64_t SocInfo::csr2addr(const char *name) {
    AttributeType id(name);
    for (unsigned i = 0; i < listCSR_.size(); i++) {
        if (strcmp(id.to_upper(), listCSR_[i][0u].to_string()) == 0) {
            return dsuBase_.to_uint64() + (listCSR_[i][2].to_uint64() << 4);
        }
    }
    return ~0;
}

uint64_t SocInfo::reg2addr(const char *name) {
    uint64_t REG_BASE_ADDR = dsuBase_.to_uint64() + 0x10000 + 64*8;
    for (unsigned i = 0; i < listRegs_.size(); i++) {
        if (strcmp(name, listRegs_[i][0u].to_string()) == 0) {
            return REG_BASE_ADDR + 8 * listRegs_[i][2].to_uint64();
        }
    }
    return ~0;
}

uint64_t SocInfo::addressPlugAndPlay() {
    return pnpBase_.to_uint64();
}

uint64_t SocInfo::addressGpio() {
    return gpioBase_.to_uint64();
}


uint64_t SocInfo::addressRunControl() {
    return dsuBase_.to_uint64() + 0x10000;
}

uint64_t SocInfo::addressStepCounter() {
    return dsuBase_.to_uint64() + 0x10000 + 8;
}

uint64_t SocInfo::addressBreakCreate() {
    return dsuBase_.to_uint64() + 0x10000 + 16;
}

uint64_t SocInfo::addressBreakRemove() {
    return dsuBase_.to_uint64() + 0x10000 + 24;
}

uint64_t SocInfo::valueHalt() {
    DsuRunControlRegType ctrl;
    ctrl.val = 0x0;
    ctrl.bits.core_id = 0;
    ctrl.bits.halt    = 1;
    return ctrl.val;
}

uint64_t SocInfo::valueRun() {
    DsuRunControlRegType ctrl;
    ctrl.val = 0x0;
    return ctrl.val;
}

uint64_t SocInfo::valueRunStepping() {
    DsuRunControlRegType ctrl;
    ctrl.val = 0x0;
    ctrl.bits.stepping = 1;
    return ctrl.val;
}

}  // namespace debugger
