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

#ifndef __DEBUGGER_SRC_STM32L4XX_STM32L4_ST7789V_H__
#define __DEBUGGER_SRC_STM32L4XX_STM32L4_ST7789V_H__

#include <iclass.h>
#include <iservice.h>
#include "coreservices/ireset.h"
#include "coreservices/idisplay.h"
#include "coreservices/icmdexec.h"
#include "coreservices/iioport.h"
#include "generic/iotypes.h"

namespace debugger {

static const int ST7789V_WIDTH  = 320;
static const int ST7789V_HEIGHT = 240;

class ST7789VCmdType : public GenericDisplayCmdType {
 public:
    ST7789VCmdType(IService *parent, const char *name)
        : GenericDisplayCmdType(parent, name), last_pixel_(-1) {}

 protected:
    virtual int getWidth() { return ST7789V_WIDTH; }
    virtual int getHeight() { return ST7789V_HEIGHT; }
    virtual uint32_t getBkgColor() { return 0; }
    virtual void getFrame(AttributeType *res, bool diff);

 protected:
    int last_pixel_;
};

class RD_PinType : public IOPinType32 {
 public:
    RD_PinType(IService *parent) : IOPinType32(parent, "L_RD") {}

 protected:
    virtual void posedge() override;
};

class WR_PinType : public IOPinType32 {
 public:
    WR_PinType(IService *parent) : IOPinType32(parent, "L_WR") {}

 protected:
    virtual void posedge() override;
};

class DC_PinType : public IOPinType32 {
 public:
    DC_PinType(IService *parent) : IOPinType32(parent, "L_DC") {}
};

class CS_PinType : public IOPinType32 {
 public:
    CS_PinType(IService *parent) : IOPinType32(parent, "L_CS") {}
};

class DataBusType : public IIOPortListener32 {
 public:
    DataBusType(IService *parent) : parent_(parent), data_(0) {
        parent->registerPortInterface("L_D",
                static_cast<IIOPortListener32 *>(this));
    }

    uint16_t getData()             { return data_; }
    void    setData(uint16_t data) { data = data_; }

    /** IIOPortListener32 interface */
    virtual void readData(uint32_t *val, uint32_t mask) {}
    virtual void writeData(uint32_t val, uint32_t mask) { data_ = val; }
    virtual void latch() {}

 private:
    IService *parent_;
    uint16_t data_;     // port size 16 bits (not 32)
};

class ST7789V : public IService,
                public IResetListener {
 friend class ST7789VCmdType;
 public:
    ST7789V(const char *name);

    /** IService interface */
    virtual void postinitService();
    virtual void predeleteService();

    /** IResetListener interface */
    virtual void reset(IFace *isource);

    /** Common Registers methods: */
    void posedgeRD();
    void posedgeWR();

 protected:
    void connectListener(IIOPortListener32 *iface, const AttributeType &cfg);

 private:
    /** Commands list */
    enum cmd_t {
        CMD_CASET = 0x2A,
        CMD_RASET = 0x2B,
        CMD_RAMWR = 0x2C,
        CMD_MADCTL = 0x36
    };

    /** Display internal state data */
    struct {
        struct {
            uint16_t xs;
            uint16_t xe;
        } caset;

        struct {
            uint16_t ys;
            uint16_t ye;
        } raset;

        struct {
            uint8_t res0 : 2;
            uint8_t MH : 1;
            uint8_t RGB : 1;
            uint8_t ML : 1;
            uint8_t MV : 1;
            uint8_t MX : 1;
            uint8_t MY : 1;
        } madctl;
    } m_state;

    void processCommand();

    void iled_setpixel(uint32_t rgb);

    /** Actions */
    void caset();
    void raset();
    void ramwr();
    void madctl();

    uint16_t m_x;
    uint16_t m_y;

    AttributeType cmdexec_;
    AttributeType ioData_;    // 'portname'
    AttributeType ioRD_;       // ['portname',pinidx]
    AttributeType ioWR_;       // ['portname',pinidx]
    AttributeType ioDC_;       // ['portname',pinidx]
    AttributeType ioReset_;    // ['portname',pinidx]
    AttributeType ioCS_;       // ['portname',pinidx]

    ICmdExecutor *iexec_;
    ST7789VCmdType *pcmd_;

    RD_PinType pinRD_;
    WR_PinType pinWR_;
    DC_PinType pinDC_;
    CS_PinType pinCS_;
    DataBusType busData_;

    uint16_t cmdBuf_[8];
    uint8_t cmdBufPos_;
    uint32_t frame_[ST7789V_HEIGHT * ST7789V_WIDTH];
    int last_modified_pixel_;
};
/*----------------------------------------------------------------------------*/

DECLARE_CLASS(ST7789V)

}  // namespace debugger

#endif  // __DEBUGGER_SRC_STM32L4XX_STM32L4_ST7789V_H__
