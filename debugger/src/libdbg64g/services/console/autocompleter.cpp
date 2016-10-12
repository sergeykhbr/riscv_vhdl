/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Auto Completer implementation.
 */

#include <string.h>
#include "autocompleter.h"
#include "coreservices/ithread.h"

namespace debugger {

/** Class registration in the Core */
REGISTER_CLASS(AutoCompleter)

AutoCompleter::AutoCompleter(const char *name) 
    : IService(name) {
    registerInterface(static_cast<IAutoComplete *>(this));
    registerAttribute("SocInfo", &socInfo_);
    registerAttribute("History", &history_);
    registerAttribute("HistorySize", &history_size_);

    socInfo_.make_string("");
    history_.make_list(0);
    history_size_.make_int64(4);
    history_idx_ = 0;

    symb_seq_msk_ = 0xFF;
    symb_seq_ = 0;
    cmdLine_ = "";
    carretPos_ = 0;
}

AutoCompleter::~AutoCompleter() {
}

void AutoCompleter::postinitService() {
    info_ = static_cast<ISocInfo *>
            (RISCV_get_service_iface(socInfo_.to_string(), IFACE_SOC_INFO));
    history_idx_ = history_.size();
}


bool AutoCompleter::processKey(uint8_t symb,
                                  AttributeType *cmd,
                                  AttributeType *cursor) {
    if (!convertToWinKey(symb)) {
        cmd->make_string(cmdLine_.c_str());
        return false;
    }
    return processKey(static_cast<int>(symb_seq_), cmd, cursor);
}

bool AutoCompleter::processKey(int key_sequence,
                                  AttributeType *cmd,
                                  AttributeType *cursor) {
    bool isNewLine = false;
    bool set_history_end = true;
    if (!cursor->is_list() || cursor->size() != 2) {
        cursor->make_list(2);
        (*cursor)[0u].make_int64(0);
        (*cursor)[1].make_int64(0);
    }
    uint8_t seq1 = static_cast<uint8_t>(key_sequence >> 8);
    uint8_t seq0 = static_cast<uint8_t>(key_sequence);
    if (seq1 == ARROW_PREFIX) {
        switch (seq0) {
        case KB_UP:
            set_history_end = false;
            if (history_idx_ == history_.size()) {
                unfinshedLine_ = cmdLine_;
            }
            if (history_idx_ > 0) {
                history_idx_--;
            }
            cmdLine_ = std::string(history_[history_idx_].to_string());
            break;
        case KB_DOWN:
            set_history_end = false;
            if (history_idx_ == (history_.size() - 1)) {
                history_idx_++;
                cmdLine_ = unfinshedLine_;
            } else if (history_idx_ < (history_.size() - 1)) {
                history_idx_++;
                cmdLine_ = std::string(history_[history_idx_].to_string());
            }
            break;
        case KB_LEFT:
            if (carretPos_ < cmdLine_.size()) {
                carretPos_++;
            }
            break;
        case KB_RIGHT:
            if (carretPos_ > 0) {
                carretPos_--;
            }
            break;
        default:;
        }
    } else {
        switch (seq0) {
        case 0:
            break;
        case '\b':// 1. Backspace button:
            if (cmdLine_.size() && carretPos_ == 0) {
                cmdLine_.erase(cmdLine_.size() - 1);
            } else if (cmdLine_.size() && carretPos_ < cmdLine_.size()) {
                std::string t1 = cmdLine_.substr(0, cmdLine_.size() - carretPos_ - 1);
                cmdLine_ = t1 + cmdLine_.substr(cmdLine_.size() - carretPos_, carretPos_);
            }
            break;
        case '\n':
        case '\r':// 2. Enter button:
            isNewLine = true;
            break;
        default:
            if (carretPos_ == 0) {
                cmdLine_ += static_cast<uint8_t>(key_sequence);
            } else if (carretPos_ == cmdLine_.size()) {
                std::string t1;
                t1 += static_cast<uint8_t>(key_sequence);
                cmdLine_ = t1 + cmdLine_;
            } else {
                std::string t1 = cmdLine_.substr(0, cmdLine_.size() - carretPos_);
                t1 += static_cast<uint8_t>(key_sequence);
                cmdLine_ = t1 + cmdLine_.substr(cmdLine_.size() - carretPos_, carretPos_);
            }
        }
    }

    cmd->make_string(cmdLine_.c_str());

    if (isNewLine) {
        symb_seq_ = 0;
        addToHistory(cmdLine_.c_str());
        cmdLine_.clear();
        carretPos_ = 0;
    }
    (*cursor)[0u].make_int64(carretPos_);
    (*cursor)[1].make_int64(carretPos_);

    if (set_history_end) {
        history_idx_ = history_.size();
    }
    return isNewLine;
}

bool AutoCompleter::convertToWinKey(uint8_t symb) {
    bool ret = true;
    symb_seq_ <<= 8;
    symb_seq_ |= symb;
    symb_seq_ &= symb_seq_msk_;

#if defined(_WIN32) || defined(__CYGWIN__)
    if (symb_seq_ == ARROW_PREFIX) {
        ret = false;
        symb_seq_msk_ = 0xFFFF;
    } else {
        symb_seq_msk_ = 0xFF;
    }
#else
    if (symb_seq_ == UNICODE_BACKSPACE) {
        symb_seq_ = '\b';
        symb_seq_msk_ = 0xFF;
    } else if (symb_seq_ == 0x1b) {
        symb_seq_msk_ = 0xFFFF;
        ret = false;
    } else if (symb_seq_ == 0x1b5b) {
        symb_seq_msk_ = 0xFFFFFF;
        ret = false;
    } else if (symb_seq_ == 0x1b5b41) {
        symb_seq_ = (ARROW_PREFIX << 8) | KB_UP;
        symb_seq_msk_ = 0xFF;
    } else if (symb_seq_ == 0x1b5b42) {
        symb_seq_ = (ARROW_PREFIX << 8) | KB_DOWN;
        symb_seq_msk_ = 0xFF;
    } else if (symb_seq_ == 0x1b5b43) {
        symb_seq_ = (ARROW_PREFIX << 8) | KB_RIGHT;
        symb_seq_msk_ = 0xFF;
        //ret = false;
    } else if (symb_seq_ == 0x1b5b44) {
        symb_seq_ = (ARROW_PREFIX << 8) | KB_LEFT;
        symb_seq_msk_ = 0xFF;
        //ret = false;
    } else {
        symb_seq_msk_ = 0xFF;
    }
#endif
    if (symb_seq_ == ((uint32_t('\r') << 8) | '\n')) {
        symb_seq_ = 0;
    }
    return ret;
}

void AutoCompleter::addToHistory(const char *cmd) {
    unsigned found = history_.size();
    for (unsigned i = 0; i < history_.size(); i++) {
        if (strcmp(cmd, history_[i].to_string()) == 0) {
            found = i;
            break;
        }
    }
    if (found  ==  history_.size()) {
        AttributeType t1;
        t1.make_string(cmd);
        history_.add_to_list(&t1);

        unsigned min_size = static_cast<unsigned>(history_size_.to_int64());
        if (history_.size() >= 2*min_size) {
            history_.trim_list(0, min_size);
        }
    } else if (found < (history_.size() - 1)) {
        history_.swap_list_item(found, history_.size() - 1);
    }
    history_idx_ = history_.size();
}


}  // namespace debugger
