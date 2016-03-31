/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Access to a hardware via Ethernet EDCL interface implementaion.
 */

#include "edcl_types.h"
#include "edcl.h"

namespace debugger {

/** Class registration in the Core */
REGISTER_CLASS(EdclService)

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
    UdpEdclCommonType req = {0};
    UdpEdclCommonType rsp;

    if (!itransport_) {
        RISCV_error("UDP transport not defined, addr=%x", addr);
        return 0;
    }

    int rd_bytes = 0;
    while (rd_bytes < bytes && rd_bytes != -1) {
        req.control.request.seqidx = 
                    static_cast<uint32_t>(seq_cnt_.to_uint64());
        req.control.request.write = 0;
        req.address = static_cast<uint32_t>(addr + rd_bytes);
        if ((bytes - rd_bytes) > EDCL_PAYLOAD_MAX_BYTES) {
            req.control.request.len = 
                    static_cast<uint32_t>(EDCL_PAYLOAD_MAX_BYTES);
        } else {
            req.control.request.len = static_cast<uint32_t>(bytes - rd_bytes);
        }

        off = write16(tx_buf_, 0, req.offset);
        off = write32(tx_buf_, off, req.control.word);
        off = write32(tx_buf_, off, req.address);

        off = itransport_->sendData(tx_buf_, off);
        if (off == -1) {
            RISCV_error("Data sending error", NULL);
            rd_bytes = -1;
            break;
        }

        off = itransport_->readData(rx_buf_, sizeof(rx_buf_));
        if (off == -1) {
            RISCV_error("Data receiving error", NULL);
            rd_bytes = -1;
            break;
        } 
        if (off == 0) {
            RISCV_error("No response. Break read transaction.", NULL);
            rd_bytes = -1;
            break;
        }

        rsp.control.word = read32(&rx_buf_[2]);

        const char *NAK[2] = {"ACK", "NAK"};
        RISCV_debug("EDCL read: %s[%d], len = %d", 
                    NAK[rsp.control.response.nak],
                    rsp.control.response.seqidx,
                    rsp.control.response.len);

        // Retry with new sequence counter.
        if (rsp.control.response.nak) {
            RISCV_info("Sequence counter detected %d. Re-sending transaction.",
                         rsp.control.response.seqidx);
            seq_cnt_.make_uint64(rsp.control.response.seqidx);
            continue;
        }

        memcpy(&obuf[rd_bytes], &rx_buf_[10], rsp.control.response.len);
        rd_bytes += rsp.control.response.len;
        seq_cnt_.make_uint64(seq_cnt_.to_uint64() + 1);
    }
    return rd_bytes;
}

int EdclService::write(uint64_t addr, int bytes, uint8_t *ibuf) {
    int off;
    UdpEdclCommonType req = {0};
    UdpEdclCommonType rsp;

    if (!itransport_) {
        RISCV_error("UDP transport not defined, addr=%x", addr);
        return 0;
    }

    int wr_bytes = 0;
    while (wr_bytes < bytes && wr_bytes != -1) {
        req.control.request.seqidx = 
                    static_cast<uint32_t>(seq_cnt_.to_uint64());
        req.control.request.write = 1;
        req.address = static_cast<uint32_t>(addr + wr_bytes);
        if ((bytes - wr_bytes) > EDCL_PAYLOAD_MAX_BYTES) {
            req.control.request.len = 
                    static_cast<uint32_t>(EDCL_PAYLOAD_MAX_BYTES);
        } else {
            req.control.request.len = static_cast<uint32_t>(bytes - wr_bytes);
        }

        off = write16(tx_buf_, 0, req.offset);
        off = write32(tx_buf_, off, req.control.word);
        off = write32(tx_buf_, off, req.address);
        memcpy(&tx_buf_[off], &ibuf[wr_bytes], req.control.request.len);


        off = itransport_->sendData(tx_buf_, off + req.control.request.len);
        if (off == -1) {
            RISCV_error("Data sending error", NULL);
            wr_bytes = -1;
            break;
        }

        off = itransport_->readData(rx_buf_, sizeof(rx_buf_));
        if (off == -1) {
            RISCV_error("Data receiving error", NULL);
            wr_bytes = -1;
            break;
        } 
        if (off == 0) {
            RISCV_error("No response. Break write transaction.", NULL);
            wr_bytes = -1;
            break;
        }

        rsp.control.word = read32(&rx_buf_[2]);

        // Warning:
        //   response length = 0;
        const char *NAK[2] = {"ACK", "NAK"};
        RISCV_debug("EDCL write: %s[%d], len = %d", 
                    NAK[rsp.control.response.nak],
                    rsp.control.response.seqidx,
                    req.control.request.len);

        // Retry with new sequence counter.
        if (rsp.control.response.nak) {
            RISCV_info("Sequence counter detected %d. Re-sending transaction.",
                         rsp.control.response.seqidx);
            seq_cnt_.make_uint64(rsp.control.response.seqidx);
            continue;
        }

        wr_bytes += req.control.request.len;
        seq_cnt_.make_uint64(seq_cnt_.to_uint64() + 1);
    }
    return wr_bytes;
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

uint32_t EdclService::read32(uint8_t *buf) {
    return (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | (buf[3] << 0);
}

}  // namespace debugger
