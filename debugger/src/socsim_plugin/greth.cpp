/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Ethernet MAC device functional model.
 */

#include "api_core.h"
#include "greth.h"

namespace debugger {

/** Class registration in the Core */
REGISTER_CLASS(Greth)

Greth::Greth(const char *name) 
    : IService(name) {
    registerInterface(static_cast<IThread *>(this));
    registerInterface(static_cast<IMemoryOperation *>(this));
    registerInterface(static_cast<IClockListener *>(this));
    registerAttribute("BaseAddress", &baseAddress_);
    registerAttribute("Length", &length_);
    registerAttribute("IP", &ip_);
    registerAttribute("MAC", &mac_);
    registerAttribute("Bus", &bus_);
    registerAttribute("Transport", &transport_);

    baseAddress_.make_uint64(0);
    length_.make_uint64(0);
    ip_.make_uint64(0);
    mac_.make_uint64(0);
    bus_.make_string("");
    transport_.make_string("");

    memset(txbuf_, 0, sizeof(txbuf_));
    seq_cnt_ = 35;
    fifo_to_ = new TFifo<FifoMessageType>(16);
    fifo_from_ = new TFifo<FifoMessageType>(16);
    RISCV_event_create(&event_tap_, "event_tap");
}
Greth::~Greth() {
    delete fifo_to_;
    delete fifo_from_;
    RISCV_event_close(&event_tap_);
}

void Greth::postinitService() {
    ibus_ = static_cast<IBus *>(
       RISCV_get_service_iface(bus_.to_string(), IFACE_BUS));

    if (!ibus_) {
        RISCV_error("Bus interface '%s' not found", 
                    bus_.to_string());
        return;
    }

    itransport_ = static_cast<IUdp *>(
        RISCV_get_service_iface(transport_.to_string(), IFACE_UDP));

    if (!itransport_) {
        RISCV_error("UDP interface '%s' not found", 
                    bus_.to_string());
        return;
    }

    AttributeType clks;
    RISCV_get_clock_services(&clks);
    if (clks.size()) {
        iclk0_ = static_cast<IClock *>(clks[0u].to_iface());
    } else {
        RISCV_error("CPUs not found", NULL);
    }

    // Get global settings:
    const AttributeType *glb = RISCV_get_global_settings();
    if ((*glb)["SimEnable"].to_bool()) {
        if (!run()) {
            RISCV_error("Can't create thread.", NULL);
            return;
        }
    }
}

void Greth::predeleteService() {
    stop();
}

void Greth::busyLoop() {
    int bytes;
    UdpEdclCommonType req;
    FifoMessageType msg;
    RISCV_info("Ethernet thread was started", NULL);

    while (isEnabled()) {
        bytes = 
            itransport_->readData(rxbuf_, static_cast<int>(sizeof(rxbuf_)));

        if (bytes == 0) {
            continue;
        }

        req.control.word = read32(&rxbuf_[2]);
        req.address      = read32(&rxbuf_[6]);
        if (seq_cnt_ != req.control.request.seqidx) {
            sendNAK(&req);
            continue;
        }

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

        iclk0_->registerStepCallback(static_cast<IClockListener *>(this),
                                    iclk0_->getStepCounter() + 1);

        RISCV_event_wait(&event_tap_);
        RISCV_event_clear(&event_tap_);
        while (!fifo_from_->isEmpty()) {
            fifo_from_->get(&msg);
        }

        req.control.response.nak = 0;
        req.control.response.seqidx = seq_cnt_;
        write32(&txbuf_[2], req.control.word);

        seq_cnt_++;
        itransport_->sendData(txbuf_, bytes);
    }
    loopEnable_ = false;
    threadInit_.Handle = 0;
}

void Greth::stepCallback(uint64_t t) {
    FifoMessageType msg;
    uint8_t *buf;
    uint64_t addr;
    while (!fifo_to_->isEmpty()) {
        fifo_to_->get(&msg);

        addr = msg.addr;
        buf = msg.buf;
        for (unsigned i = 0; i < msg.sz / 4u; i++) {
            if (msg.rw == 0) {
                ibus_->read(addr, buf, 4);
            } else {
                ibus_->write(addr, buf, 4);
            }
            buf += 4;
            addr += 4;
        }
        fifo_from_->put(&msg);
        RISCV_event_set(&event_tap_);
    }
}

void Greth::transaction(Axi4TransactionType *payload) {
}

void Greth::sendNAK(UdpEdclCommonType *req) {
    req->control.response.nak = 1;
    req->control.response.seqidx = seq_cnt_;
    req->control.response.len = 0;
    write32(&txbuf_[2], req->control.word);
    write32(&txbuf_[6], req->address);

    itransport_->sendData(txbuf_, sizeof(UdpEdclCommonType));
}

uint32_t Greth::read32(uint8_t *buf) {
    return (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | (buf[3] << 0);
}

void Greth::write32(uint8_t *buf, uint32_t v) {
    buf[0] = (char)((v >> 24) & 0xFF);
    buf[1] = (char)((v >> 16) & 0xFF);
    buf[2] = (char)((v >> 8) & 0xFF);
    buf[3] = (char)(v & 0xFF);
}

}  // namespace debugger
