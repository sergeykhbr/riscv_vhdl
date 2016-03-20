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



struct EdclControlRequestType {
    // 32 bits fields:
    uint32_t unused : 7;
    uint32_t len    : 10;
    uint32_t write  : 1;    // read = 0; write = 1
    uint32_t seqidx : 14;   // sequence id
    //uint32 data; // 0 to 242 words
};


struct EdclControlResponseType {
    // 32 bits fields:
    uint32_t unused : 7;
    uint32_t len    : 10;
    uint32_t nak    : 1;    // ACK = 0; NAK = 1
    uint32_t seqidx : 14;   // sequence id
    //uint32 data; // 0 to 242 words
};

#pragma pack(1)
struct UdpEdclCommonType {
    uint16_t offset;
    union ControlType {
        uint32_t word;
        EdclControlRequestType request;
        EdclControlResponseType response;
    } control;
    uint32_t address;
    //uint32 data; // 0 to 242 words
};
#pragma pack()

EdclService::EdclService(const char *name) : IService(name) {
    registerInterface(static_cast<ITap *>(this));
    registerAttribute("Transport", &transport_);
    registerAttribute("seq_cnt", &seq_cnt_);
    seq_cnt_.make_uint64(0);
    itransport_ = 0;
}

void EdclService::postinitService() {
    IService *iserv = 
        static_cast<IService *>(RISCV_get_service(transport_.to_string()));
    if (!iserv) {
        RISCV_error("Transport service '%'s not found", 
                    transport_.to_string());
    }
    itransport_ = static_cast<IUdp *>(iserv->getInterface(IFACE_UDP));
    if (!itransport_) {
        RISCV_error("UDP interface '%s' not found", 
                     transport_.to_string());
    }
}

int EdclService::read(uint64_t addr, int bytes, uint8_t *obuf) {
    int off;
    UdpEdclCommonType req;
    req.offset = 0;
    /*req.control = static_cast<uint32_t>(seq_cnt_.to_uint64() << 18)
                | (0 << 17) 
                | (static_cast<uint32_t>(bytes) << 7);*/
    req.control.request.seqidx = static_cast<uint32_t>(seq_cnt_.to_uint64());
    req.control.request.write = 0;
    req.control.request.len = static_cast<uint32_t>(bytes);
    req.address = static_cast<uint32_t>(addr);

    off = write16(datagram_, 0, req.offset);
    off = write32(datagram_, off, req.control.word);
    off = write32(datagram_, off, req.address);

    if (!itransport_) {
        RISCV_error("UDP transport not defined, addr=%x", addr);
        return 0;
    }

    off = itransport_->sendData(reinterpret_cast<const char *>(datagram_), off);

    if (off == -1) {
        return off;
    }

    off = itransport_->readData(rx_buf_, sizeof(rx_buf_));
    if (off) {
        UdpEdclCommonType rsp;
        rsp.control.word = (rx_buf_[2] << 24) | (rx_buf_[3] << 16)
              | (rx_buf_[4] << 8) | (rx_buf_[5] << 0);

        RISCV_info("Response: seq_idx = %d", rsp.control.response.seqidx);
        RISCV_info("Response: NAK     = %d", rsp.control.response.nak);
        RISCV_info("Response: len     = %d", rsp.control.response.len);
        if (off > sizeof(UdpEdclCommonType)) {
            //memcpy(obuf, rx_buf_, 
        }
        if (rsp.control.response.nak) {
            seq_cnt_.make_uint64(rsp.control.response.seqidx);
        } else {
            seq_cnt_.make_uint64(seq_cnt_.to_uint64() + 1);
        }
    } else {
        RISCV_error("No data received", NULL);
    }


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
