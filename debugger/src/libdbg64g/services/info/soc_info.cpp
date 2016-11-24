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
    DsuMapType *dsu = reinterpret_cast<DsuMapType *>(dsuBase_.to_uint64());
    AttributeType id(name);
    for (unsigned i = 0; i < listCSR_.size(); i++) {
        if (strcmp(id.to_upper(), listCSR_[i][0u].to_string()) == 0) {
            return reinterpret_cast<uint64_t>(
                &dsu->csr[listCSR_[i][2].to_uint64()]);
        }
    }
    return REG_ADDR_ERROR;
}

uint64_t SocInfo::reg2addr(const char *name) {
    DsuMapType *dsu = reinterpret_cast<DsuMapType *>(dsuBase_.to_uint64());
    for (unsigned i = 0; i < listRegs_.size(); i++) {
        if (strcmp(name, listRegs_[i][0u].to_string()) == 0) {
            return reinterpret_cast<uint64_t>(
                &dsu->ureg.v.iregs[listRegs_[i][2].to_uint64()]);
        }
    }
    return REG_ADDR_ERROR;
}

uint64_t SocInfo::addressPlugAndPlay() {
    return pnpBase_.to_uint64();
}

uint64_t SocInfo::addressGpio() {
    return gpioBase_.to_uint64();
}


uint64_t SocInfo::addressRunControl() {
    DsuMapType *dsu = reinterpret_cast<DsuMapType *>(dsuBase_.to_uint64());
    return reinterpret_cast<uint64_t>(&dsu->udbg.v.control);
}

uint64_t SocInfo::addressStepCounter() {
    DsuMapType *dsu = reinterpret_cast<DsuMapType *>(dsuBase_.to_uint64());
    return reinterpret_cast<uint64_t>(&dsu->udbg.v.step_cnt);
}

uint64_t SocInfo::addressBreakCreate() {
    DsuMapType *dsu = reinterpret_cast<DsuMapType *>(dsuBase_.to_uint64());
    return reinterpret_cast<uint64_t>(&dsu->udbg.v.add_breakpoint);
}

uint64_t SocInfo::addressBreakRemove() {
    DsuMapType *dsu = reinterpret_cast<DsuMapType *>(dsuBase_.to_uint64());
    return reinterpret_cast<uint64_t>(&dsu->udbg.v.remove_breakpoint);
}

uint64_t SocInfo::valueHalt() {
    DsuMapType::udbg_type::debug_region_type::control_reg ctrl;
    ctrl.val = 0x0;
    ctrl.bits.core_id = 0;
    ctrl.bits.halt    = 1;
    return ctrl.val;
}

uint64_t SocInfo::valueRun() {
    DsuMapType::udbg_type::debug_region_type::control_reg ctrl;
    ctrl.val = 0x0;
    return ctrl.val;
}

uint64_t SocInfo::valueRunStepping() {
    DsuMapType::udbg_type::debug_region_type::control_reg ctrl;
    ctrl.val = 0x0;
    ctrl.bits.stepping = 1;
    return ctrl.val;
}

}  // namespace debugger
