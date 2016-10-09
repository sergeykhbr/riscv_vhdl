/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Autocompleter's interface.
 */

#ifndef __DEBUGGER_IAUTOCOMPLETE_H__
#define __DEBUGGER_IAUTOCOMPLETE_H__

#include "iface.h"
#include "attribute.h"

namespace debugger {

static const char *IFACE_AUTO_COMPLETE = "IAutoComplete";

static const uint8_t ARROW_PREFIX       = 0xe0;
static const uint8_t UNICODE_BACKSPACE  = 0x7f;
static const uint8_t KB_UP              = 0x48;
static const uint8_t KB_DOWN            = 80;
static const uint8_t KB_LEFT            = 75;
static const uint8_t KB_RIGHT           = 77;
static const uint8_t KB_ESCAPE          = 27;


class IAutoComplete : public IFace {
public:
    IAutoComplete() : IFace(IFACE_AUTO_COMPLETE) {}

    /**
     * @return New command ready flag
     */
    virtual bool processKey(uint8_t symb, AttributeType *cmd, AttributeType *cursor) =0;
    virtual bool processKey(int key_sequence, AttributeType *cmd, AttributeType *cursor) =0;
};

}  // namespace debugger

#endif  // __DEBUGGER_IAUTOCOMPLETE_H__
