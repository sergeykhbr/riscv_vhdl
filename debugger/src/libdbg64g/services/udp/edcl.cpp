/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Access to a hardware via Ethernet EDCL interface implementaion.
 */

#include "edcl.h"

namespace debugger {

/** Class registration in the Core */
static EdclServiceClass local_class_;


struct UdpEdclRdType {
    uint16_t offset;
    uint32_t control;
    uint32_t address;
    //uint32 data; // 0 to 242 words
};

struct EdclResponseType {
    uint16_t offset;
    // 32 bits fields:
    uint32_t unused : 7;
    uint32_t len    : 10;
    uint32_t nak    : 1;    // ACK = 0; NAK = 1
    uint32_t seqidx : 14;   // sequence id
    //uint32 data; // 0 to 242 words
};

EdclService::EdclService(const char *name) : IService(name) {
    registerInterface(static_cast<ITap *>(this));
    seq_cnt_ = 2;
    itransport_ = 0;
}

void EdclService::postinitService() {
    if (!config_.is_dict()) {
        RISCV_error("Wrong configuration", NULL);
        return;
    }
    const char *transport_name = config_["Transport"].to_string();
    IService *iserv = 
        static_cast<IService *>(RISCV_get_service(transport_name));
    if (!iserv) {
        RISCV_error("Transport service '%'s not found", transport_name);
    }
    itransport_ = static_cast<IUdp *>(iserv->getInterface(IFACE_UDP));
    if (!itransport_) {
        RISCV_error("UDP interface '%s' not found", transport_name);
    }
}

AttributeType EdclService::getConnectionSettings() {
    AttributeType ret;
    if (!itransport_) {
        RISCV_error("Transport not defined", NULL);
        return ret;
    }
    return itransport_->getConnectionSettings();
}

void EdclService::setTargetSettings(const AttributeType *target) {
    if (!itransport_) {
        RISCV_error("Transport not defined", NULL);
        return;
    }
    itransport_->setTargetSettings(target);
}

int EdclService::read(uint64_t addr, int bytes, uint8_t *obuf) {
    int off;
    UdpEdclRdType req;
    req.offset = 0;
    req.control = (seq_cnt_ << 18) 
                | (0 << 17) 
                | (static_cast<uint32_t>(bytes) << 7);
    req.address = static_cast<uint32_t>(addr);

    off = write16(datagram_, 0, req.offset);
    off = write32(datagram_, off, req.control);
    off = write32(datagram_, off, req.address);

    if (!itransport_) {
        RISCV_error("UDP transport not defined, addr=%x", addr);
        return 0;
    }

    itransport_->sendData(reinterpret_cast<const char *>(datagram_), off);

    off = itransport_->readData(reinterpret_cast<const char *>(obuf), 
                    static_cast<int>(sizeof(EdclResponseType) + bytes + 3));
    seq_cnt_++;
    return off;
}

int EdclService::write16(uint8_t *buf, int off, uint16_t v) {
    buf[off++] = (uint8_t)((v >> 8) & 0xFF);
    buf[off++] = (uint8_t)(v & 0xFF);
    return off;
}

int EdclService::write32(uint8_t *buf, int off, uint32_t v) {
    buf[off++] = (uint8_t)((v >> 24) & 0xFF);
    buf[off++] = (uint8_t)((v >> 16) & 0xFF);
    buf[off++] = (uint8_t)((v >> 8) & 0xFF);
    buf[off++] = (uint8_t)(v & 0xFF);
    return off;
}

}  // namespace debugger
