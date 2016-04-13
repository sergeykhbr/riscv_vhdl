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


BoardSim::BoardSim(const char *name)  : IService(name) {
    registerInterface(static_cast<IThread *>(this));
    registerInterface(static_cast<IBoardSim *>(this));
    registerInterface(static_cast<IRawListener *>(this));
    registerInterface(static_cast<IMemoryOperation *>(this));
    registerInterface(static_cast<IClockListener *>(this));
    registerAttribute("Enable", &isEnable_);
    registerAttribute("Map", &map_);
    registerAttribute("ClkSource", &clkSource_);
    registerAttribute("Transport", &transport_);

    isEnable_.make_boolean(true);
    map_.make_list(0);
    imap_.make_list(0);
    clkSource_.make_string("");
    transport_.make_string("");

    memset(txbuf_, 0, sizeof(txbuf_));
    seq_cnt_ = 35;
    fifo_to_ = new TFifo<FifoMessageType>(16);
    fifo_from_ = new TFifo<FifoMessageType>(16);
    ring_ = new RingBufferType(4096);
    RISCV_event_create(&event_tap_, "event_tap");
}

BoardSim::~BoardSim() {
    delete ring_;
    delete fifo_to_;
    delete fifo_from_;
    RISCV_event_close(&event_tap_);
}

void BoardSim::postinitService() {
    itransport_ = static_cast<IUdp *>(
        RISCV_get_service_iface(transport_.to_string(), IFACE_UDP));
    if (!itransport_) {
        RISCV_error("UDP interface '%s' not found",  transport_.to_string());
    }
    itransport_->registerListener(static_cast<IRawListener *>(this));

    iclk_ = static_cast<IClock *>(
        RISCV_get_service_iface(clkSource_.to_string(), IFACE_CLOCK));
    if (!iclk_) {
        RISCV_error("Clock interface '%s' not found", clkSource_.to_string());
    }

    for (unsigned i = 0; i < map_.size(); i++) {
        IMemoryOperation *imem = static_cast<IMemoryOperation *>(
                        RISCV_get_service_iface(map_[i].to_string(), 
                                                IFACE_MEMORY_OPERATION));
        if (!imem) {
            RISCV_error("Memory interface '%s' not found", 
                        map_[i].to_string());
            continue;
        }
        AttributeType t1(imem);
        imap_.add_to_list(&t1);
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

void BoardSim::transaction(Axi4TransactionType *payload) {
    IMemoryOperation *imem;
    bool unmapped = true;
    for (unsigned i = 0; i < imap_.size(); i++) {
        imem = static_cast<IMemoryOperation *>(imap_[i].to_iface());
        if (imem->getBaseAddress() <= payload->addr
            && payload->addr < (imem->getBaseAddress() + imem->getLength())) {
            imem->transaction(payload);
            unmapped = false;
            break;
            /// @todo Check memory overlapping
        }
    }

    if (unmapped) {
        RISCV_error("[%" RV_PRI64 "d] Access to unmapped address %08" RV_PRI64 "x", 
            iclk_->getStepCounter(), payload->addr);
        while (1) {}
    }

    const char *rw_str[2] = {"=>", "<="};
    uint32_t *pdata[2] = {payload->rpayload, payload->wpayload};
    RISCV_debug("[%08" RV_PRI64 "x] %s [%08x %08x %08x %08x]",
        payload->addr,
        rw_str[payload->rw],
        pdata[payload->rw][3], pdata[payload->rw][2], 
        pdata[payload->rw][1], pdata[payload->rw][0]);
}

void BoardSim::busyLoop() {
    int bytes;
    UdpEdclCommonType req;
    FifoMessageType msg;
    RISCV_info("Board Simulator was started", NULL);

    while (loopEnable_) {
        bytes = 
            itransport_->readData(rxbuf_, static_cast<int>(sizeof(rxbuf_)));

        if (bytes == 0) {
            continue;
        }

        req.control.word = read32(&rxbuf_[2]);
        req.address      = read32(&rxbuf_[6]);
        if (seq_cnt_ != req.control.request.seqidx) {
            req.control.response.nak = 1;
            req.control.response.seqidx = seq_cnt_;
            req.control.response.len = 0;
            write32(&txbuf_[2], req.control.word);
            write32(&txbuf_[6], req.address);
            bytes = sizeof(UdpEdclCommonType);
        } else {
            if (req.control.request.write == 0) {
                msg.rw = 0;
                msg.addr = req.address;
                msg.sz = req.control.request.len;
                msg.buf = &txbuf_[10];
                fifo_to_->put(&msg);
                bytes = sizeof(UdpEdclCommonType) + req.control.request.len;
            } else {
                msg.rw = 1;
                msg.addr = req.address;
                msg.sz = req.control.request.len;
                msg.buf = &rxbuf_[10];
                fifo_to_->put(&msg);
                bytes = sizeof(UdpEdclCommonType);
            }

            iclk_->registerStepCallback(static_cast<IClockListener *>(this),
                                        iclk_->getStepCounter() + 1);

            RISCV_event_wait(&event_tap_);
            RISCV_event_clear(&event_tap_);
            while (!fifo_from_->isEmpty()) {
                fifo_from_->get(&msg);
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
    threadInit_.Handle = 0;
}

void BoardSim::stepCallback(uint64_t t) {
    FifoMessageType msg;
    Axi4TransactionType memop;
    uint32_t *mem;
    //memop.bytes = 2;   // word32 AXI spec.
    memop.xsize = 4;
    while (!fifo_to_->isEmpty()) {
        fifo_to_->get(&msg);

        mem = reinterpret_cast<uint32_t *>(msg.buf);
        for (unsigned i = 0; i < msg.sz / 4u; i++) {
            memop.addr = msg.addr + 4*i;
            if (msg.rw == 0) {
                memop.rw = 0;
            } else {
                memop.rw = 1;
                memop.wstrb = 0x000f;
                memop.wpayload[0] = mem[i];
            }

            transaction(&memop);
            if (msg.rw == 0) {
                mem[i] = memop.rpayload[0];
            }
        }
        fifo_from_->put(&msg);
        RISCV_event_set(&event_tap_);
    }
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

