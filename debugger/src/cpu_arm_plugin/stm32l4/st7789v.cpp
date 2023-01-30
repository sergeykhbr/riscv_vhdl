/*
 *  Copyright 2020 Sergey Khabarov, sergeykhbr@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 * ST7789V, single-chip controller/driver for 262K-color, graphic type TFT-LCD 
 * (on-chip display data RAM 240x320x18 bits) display functional model.
 */

#include "api_core.h"
#include "st7789v.h"

namespace debugger {

void ST7789VCmdType::getFrame(AttributeType *res, bool diff) {
    ST7789V *p = static_cast<ST7789V *>(cmdParent_);
    int t1 = p->last_modified_pixel_;
    //if (diff && t1 == last_pixel_) {
    //    res->make_nil();
    //    return;
    //}
    last_pixel_ = t1;
    RISCV_memory_barrier();
    if (!res->is_data() || res->size() != sizeof(p->frame_)) {
        res->make_data(sizeof(p->frame_));
    }
    memcpy(res->data(), p->frame_, sizeof(p->frame_));
}

ST7789V::ST7789V(const char *name) :
    IService(name),
    pinRD_(this),
    pinWR_(this),
    pinDC_(this),
    pinCS_(this),
    busData_(this) {
    registerInterface(static_cast<IIOPortListener32 *>(&busData_));
    registerInterface(static_cast<IResetListener *>(this));

    registerAttribute("CmdExecutor", static_cast<IAttribute *>(&cmdexec_));
    registerAttribute("IOData", static_cast<IAttribute *>(&ioData_));
    registerAttribute("IORD", static_cast<IAttribute *>(&ioRD_));
    registerAttribute("IOWR", static_cast<IAttribute *>(&ioWR_));
    registerAttribute("IODC", static_cast<IAttribute *>(&ioDC_));
    registerAttribute("IOCS", static_cast<IAttribute *>(&ioCS_));
    registerAttribute("IOReset", static_cast<IAttribute *>(&ioReset_));

    memset(frame_, 0, sizeof(frame_));
    m_x = 0;
    m_y = 0;
    last_modified_pixel_ = 0;
}

void ST7789V::postinitService() {
    iexec_ = static_cast<ICmdExecutor *>
        (RISCV_get_service_iface(cmdexec_.to_string(), IFACE_CMD_EXECUTOR));
    if (!iexec_) {
        RISCV_error("Can't get ICmdExecutor interface %s",
                    cmdexec_.to_string());
        return;
    }
    pcmd_ = new ST7789VCmdType(static_cast<IService *>(this),
                               getObjName());
    iexec_->registerCommand(static_cast<ICommand *>(pcmd_));

    connectListener(static_cast<IIOPortListener32 *>(&busData_), ioData_);
    connectListener(static_cast<IIOPortListener32 *>(&pinRD_), ioRD_[0u]);
    pinRD_.setPinIdx(ioRD_[1].to_int());
    connectListener(static_cast<IIOPortListener32 *>(&pinWR_), ioWR_[0u]);
    pinWR_.setPinIdx(ioWR_[1].to_int());
    connectListener(static_cast<IIOPortListener32 *>(&pinDC_), ioDC_[0u]);
    pinDC_.setPinIdx(ioDC_[1].to_int());
    connectListener(static_cast<IIOPortListener32 *>(&pinCS_), ioCS_[0u]);
    pinCS_.setPinIdx(ioCS_[1].to_int());
}

void ST7789V::predeleteService() {
    if (iexec_) {
        iexec_->unregisterCommand(static_cast<ICommand *>(pcmd_));
        delete pcmd_;
    }
}

void ST7789V::connectListener(IIOPortListener32 *iface,
                              const AttributeType &cfg) {
    IIOPort *iport = 0;
    if (cfg.is_string()) {
        iport = static_cast<IIOPort *>(RISCV_get_service_iface(
            cfg.to_string(), IFACE_IOPORT));
    } else if (cfg.is_list()) {
        iport = static_cast<IIOPort *>(RISCV_get_service_port_iface(
            cfg[0u].to_string(), cfg[1].to_string(), IFACE_IOPORT));
    }

    if (iport == 0) {
        RISCV_error("Can't get IIOPort interface %s", cfg.to_string());
        return;
    }
    iport->registerPortListener(iface);
}


void ST7789V::reset(IFace *isource) {
}

void ST7789V::posedgeRD() {
}

void ST7789V::posedgeWR() {
    if (pinDC_.getLevel()) {
        cmdBuf_[cmdBufPos_++] = busData_.getData();
        if (cmdBufPos_ >= 8) {
            cmdBufPos_ = 7;
        }
    } else {
        cmdBuf_[0] = busData_.getData();
        cmdBufPos_ = 1;
    }

    processCommand();
}

void ST7789V::processCommand() {
    switch (cmdBuf_[0]) {
    case CMD_CASET:
        if (cmdBufPos_ == 5)
            caset();
        break;
    case CMD_RASET:
        if (cmdBufPos_ == 5)
            raset();
        break;
    case CMD_MADCTL:
        if (cmdBufPos_ == 2)
            madctl();
        break;
    case CMD_RAMWR:
        if (cmdBufPos_ == 2)
            ramwr();
        break;
    }
}

void ST7789V::caset() {
    m_state.caset.xs = cmdBuf_[1] << 8;
    m_state.caset.xs |= static_cast<uint8_t>(cmdBuf_[2]);
    m_state.caset.xe = cmdBuf_[3] << 8;
    m_state.caset.xe |= static_cast<uint8_t>(cmdBuf_[4]);

    m_x = m_state.caset.xs;
}

void ST7789V::raset() {
    m_state.raset.ys = cmdBuf_[1] << 8;
    m_state.raset.ys |= static_cast<uint8_t>(cmdBuf_[2]);
    m_state.raset.ye = cmdBuf_[3] << 8;
    m_state.raset.ye |= static_cast<uint8_t>(cmdBuf_[4]);

    m_y = m_state.raset.ys;
}

void ST7789V::ramwr() {
    uint8_t r, g, b;
    uint32_t rgb;

    r = (cmdBuf_[1] >> 8) & 0xF8;
    g = (cmdBuf_[1] >> 3) & 0xFC;
    b = (cmdBuf_[1] << 3);
    rgb = (r << 16) | (g << 8) | b;

    iled_setpixel(rgb);

    m_x++;
    if (m_x > m_state.caset.xe) {
        m_x = m_state.caset.xs;

        m_y++;
        if (m_y > m_state.raset.ye) {
            m_y = m_state.raset.ys;
        }
    }

    cmdBufPos_ = 1;
}

void ST7789V::madctl() {
    memcpy(&m_state.madctl, &cmdBuf_[1], sizeof(m_state.madctl));
}

void ST7789V::iled_setpixel(uint32_t rgb) {
    int pix_idx = m_y*ST7789V_HEIGHT + ST7789V_HEIGHT-m_x-1;
    if (frame_[pix_idx] != rgb) {
        last_modified_pixel_ = pix_idx;
    }
    frame_[pix_idx] = rgb;
}


void RD_PinType::posedge() {
    static_cast<ST7789V *>(parent_)->posedgeRD();
}

void WR_PinType::posedge() {
    static_cast<ST7789V *>(parent_)->posedgeWR();
}

}  // namespace debugger

