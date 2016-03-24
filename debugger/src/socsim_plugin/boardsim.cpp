/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Simulator of the FPGA board with Ethernet UDP/EDCL interface.
 */

#include "api_core.h"
#include "boardsim.h"

namespace debugger {

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


/** Class registration in the Core */
static BoardSimClass local_class_;

BoardSim::BoardSim(const char *name)  : IService(name) {
registerInterface(static_cast<IThread *>(this));
    registerInterface(static_cast<IBoardSim *>(this));
    registerInterface(static_cast<IRawListener *>(this));
    registerAttribute("Enable", &isEnable_);
    registerAttribute("Transport", &transport_);

    isEnable_.make_boolean(true);
    transport_.make_string("");

    memset(txbuf_, 0, sizeof(txbuf_));
    seq_cnt_ = 35;
    for (unsigned i = 0; i < sizeof(SRAM_)/4; i++) {
        (reinterpret_cast<uint32_t *>(SRAM_))[i] = 
                    static_cast<uint32_t>(0xcafe0000 + i);
    }

#ifdef USE_VERILATED_CORE
    core = new Vtop();
#endif
}

BoardSim::~BoardSim() {
#ifdef USE_VERILATED_CORE
    delete core;
#endif
}

void BoardSim::postinitService() {
    itransport_ = static_cast<IUdp *>(
        RISCV_get_service_iface(transport_.to_string(), IFACE_UDP));
    if (itransport_) {
        itransport_->registerListener(static_cast<IRawListener *>(this));
    } else {
        RISCV_error("UDP interface '%s' not found", 
                    transport_.to_string());
        return;
    }

    if (isEnable_.to_bool()) {
        if (!run()) {
            RISCV_error("Can't create thread.", NULL);
            return;
        }
    }
}

void BoardSim::predeleteService() {
    stop();
}

void BoardSim::busyLoop() {
    int bytes;
    UdpEdclCommonType req;
    RISCV_info("Board Simulator was started", NULL);

    while (loopEnable_) {
        bytes = 
            itransport_->readData(rxbuf_, static_cast<int>(sizeof(rxbuf_)));

        if (bytes == 0) {
            continue;
        }

        req.control.word = read32(&rxbuf_[2]);
        req.address      = read32(&rxbuf_[6]);
        //req.control.request.seqidx
        if (seq_cnt_ != req.control.request.seqidx) {
            req.control.response.nak = 1;
            req.control.response.seqidx = seq_cnt_;
            req.control.response.len = 0;
            write32(&txbuf_[2], req.control.word);
            write32(&txbuf_[6], req.address);
            bytes = sizeof(UdpEdclCommonType);
        } else {
            if (req.control.request.write == 0) {
                memcpy(&txbuf_[10], &SRAM_[req.address & (sizeof(SRAM_) - 1)], 
                                        req.control.request.len);
                bytes = sizeof(UdpEdclCommonType) + req.control.request.len;

                RISCV_debug("Read [%08x]; size = %d; %08x..",
                    req.address,
                    req.control.request.len,
                    *(reinterpret_cast<uint32_t *>(&txbuf_[10])));
            } else {
                memcpy(&SRAM_[req.address & (sizeof(SRAM_) - 1)], &rxbuf_[10],
                                        req.control.request.len);
                bytes = sizeof(UdpEdclCommonType);

                RISCV_debug("Write [%08x]; size = %d; %08x..",
                    req.address,
                    req.control.request.len,
                    *(reinterpret_cast<uint32_t *>(&rxbuf_[10])));
            }
            req.control.response.nak = 0;
            req.control.response.seqidx = seq_cnt_;
            write32(&txbuf_[2], req.control.word);

            seq_cnt_++;
        }

        if (bytes != 0) {
            itransport_->sendData(txbuf_, bytes);
        }
    }
    loopEnable_ = false;
    threadInit_.Handle = NULL;
}

uint32_t BoardSim::read32(uint8_t *buf) {
    return (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | (buf[3] << 0);
}

void BoardSim::write32(uint8_t *buf, uint32_t v) {
    buf[0] = (char)((v >> 24) & 0xFF);
    buf[1] = (char)((v >> 16) & 0xFF);
    buf[2] = (char)((v >> 8) & 0xFF);
    buf[3] = (char)(v & 0xFF);
}

}  // namespace debugger

