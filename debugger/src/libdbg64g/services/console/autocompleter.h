/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Auto-complete class declaration.
 */

#ifndef __DEBUGGER_AUTOCOMPLETER_H__
#define __DEBUGGER_AUTOCOMPLETER_H__

#include "iclass.h"
#include "iservice.h"
#include "coreservices/iautocomplete.h"
#include "coreservices/isocinfo.h"
#include <string>
#include <stdarg.h>

namespace debugger {

class AutoCompleter : public IService,
                      public IAutoComplete {
public:
    explicit AutoCompleter(const char *name);
    virtual ~AutoCompleter();

    /** IService interface */
    virtual void postinitService();

    /** IAutoComplete */
    virtual bool processKey(uint8_t symb, AttributeType *cmd, AttributeType *cursor);
    virtual bool processKey(int key_sequence, AttributeType *cmd, AttributeType *cursor);


private:
    bool convertToWinKey(uint8_t symb);
    void addToHistory(const char *cmd);

private:
    //AttributeType console_;
    AttributeType socInfo_;
    AttributeType history_;
    AttributeType history_size_;

    std::string cmdLine_;
    unsigned carretPos_;
    ISocInfo *info_;

    uint32_t symb_seq_;         // symbol sequence
    uint32_t symb_seq_msk_;
    // History switching
    std::string unfinshedLine_; // store the latest whe we look through history
    unsigned history_idx_;
};

DECLARE_CLASS(AutoCompleter)

}  // namespace debugger

#endif  // __DEBUGGER_AUTOCOMPLETER_H__
