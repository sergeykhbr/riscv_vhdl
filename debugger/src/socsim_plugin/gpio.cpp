/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      GPIO functional model.
 */

#include "api_core.h"
#include "gpio.h"

namespace debugger {

GPIO::GPIO(const char *name) : RegMemBankGeneric(name),
    input_val(static_cast<IService *>(this), "input_val", 0x00),
    input_en(static_cast<IService *>(this), "input_en", 0x04),
    output_en(static_cast<IService *>(this), "output_en", 0x08),
    output_val(static_cast<IService *>(this), "output_val", 0x0C),
    pue(static_cast<IService *>(this), "pue", 0x10),
    ds(static_cast<IService *>(this), "ds", 0x14),
    rise_ie(static_cast<IService *>(this), "rise_ie", 0x18),
    rise_ip(static_cast<IService *>(this), "rise_ip", 0x1C),
    fall_ie(static_cast<IService *>(this), "fall_ie", 0x20),
    fall_ip(static_cast<IService *>(this), "fall_ip", 0x24),
    high_ie(static_cast<IService *>(this), "high_ie", 0x28),
    high_ip(static_cast<IService *>(this), "high_ip", 0x2C),
    low_ie(static_cast<IService *>(this), "low_ie", 0x30),
    low_ip(static_cast<IService *>(this), "low_ip", 0x34),
    iof_en(static_cast<IService *>(this), "iof_en", 0x38),
    iof_sel(static_cast<IService *>(this), "iof_sel", 0x3C),
    out_xor(static_cast<IService *>(this), "out_xor", 0x40) {
    registerAttribute("IrqController", &irqctrl_);
    registerAttribute("IrqId", &irqid_);
    registerAttribute("DIP", &dip_);
}

void GPIO::postinitService() {
    RegMemBankGeneric::postinitService();

}

uint32_t GPIO::readInputs() {
    uint32_t i = input_en.getValue().buf16[0];  // 16 IOs total
    i &= (dip_.to_uint32() & 0xFFFF);
    return i;
}

void GPIO::requestInterrupt(int pinidx) {
    iirq_->requestInterrupt(static_cast<IService *>(this),
                            irqid_.to_int() + pinidx);
}

uint32_t GPIO::GPIO_INPUT_VAL_TYPE::aboutToRead(uint32_t cur_val) {
    GPIO *p = static_cast<GPIO *>(parent_);
    cur_val = p->readInputs();
    return cur_val;
}

}  // namespace debugger
