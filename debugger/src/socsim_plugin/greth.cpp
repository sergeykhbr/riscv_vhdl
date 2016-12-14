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
    registerInterface(static_cast<IAxi4NbResponse *>(this));
    registerAttribute("BaseAddress", &baseAddress_);
    registerAttribute("Length", &length_);
    registerAttribute("IrqLine", &irqLine_);
    registerAttribute("IrqControl", &irqctrl_);
    registerAttribute("IP", &ip_);
    registerAttribute("MAC", &mac_);
    registerAttribute("Bus", &bus_);
    registerAttribute("Transport", &transport_);

    baseAddress_.make_uint64(0);
    length_.make_uint64(0);
    irqLine_.make_uint64(0);
    irqctrl_.make_string("");
    ip_.make_uint64(0);
    mac_.make_uint64(0);
    bus_.make_string("");
    transport_.make_string("");

    memset(txbuf_, 0, sizeof(txbuf_));
    seq_cnt_ = 35;
    RISCV_event_create(&event_tap_, "event_tap");
}
Greth::~Greth() {
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

    iwire_ = static_cast<IWire *>(
        RISCV_get_service_iface(irqctrl_.to_string(), IFACE_WIRE));
    if (!iwire_) {
        RISCV_error("Can't find IWire interface %s", irqctrl_.to_string());
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
    uint32_t *tbuf;
    UdpEdclCommonType req;
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

        trans_.addr = req.address;
        trans_.xsize = 4;
        if (req.control.request.write == 0) {
            trans_.action = MemAction_Read;
            tbuf = reinterpret_cast<uint32_t *>(&txbuf_[10]);
            bytes = sizeof(UdpEdclCommonType) + req.control.request.len;
        } else {
            trans_.action = MemAction_Write;
            trans_.wstrb = (1 << trans_.xsize) - 1;
            tbuf = reinterpret_cast<uint32_t *>(&rxbuf_[10]);
            bytes = sizeof(UdpEdclCommonType);
        }
        for (unsigned i = 0; i < req.control.request.len / 4u; i++) {
            if (trans_.action == MemAction_Write) {
                trans_.wpayload.b32[0] = *tbuf;
            }
            RISCV_event_clear(&event_tap_);
            ibus_->nb_transport(&trans_, this);
            if (RISCV_event_wait_ms(&event_tap_, 500) != 0) {
                RISCV_error("CPU queue callback timeout", NULL);
            } else if (trans_.action == MemAction_Read) {
                *tbuf = trans_.rpayload.b32[0];
            }
            tbuf++;
            trans_.addr += 4;
        }

        req.control.response.nak = 0;
        req.control.response.seqidx = seq_cnt_;
        write32(&txbuf_[2], req.control.word);

        seq_cnt_++;
        itransport_->sendData(txbuf_, bytes);
    }
}

void Greth::nb_response(Axi4TransactionType *trans) {
    RISCV_event_set(&event_tap_);
}

void Greth::b_transport(Axi4TransactionType *trans) {
    RISCV_error("ETH Slave registers not implemented", NULL);
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
