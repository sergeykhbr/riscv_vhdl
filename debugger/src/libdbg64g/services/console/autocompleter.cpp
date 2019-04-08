/*
 *  Copyright 2018 Sergey Khabarov, sergeykhbr@gmail.com
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
 */

#include <string.h>
#include "autocompleter.h"
#include "coreservices/ithread.h"

namespace debugger {

AutoCompleter::AutoCompleter(const char *name) 
    : IService(name) {
    registerInterface(static_cast<IAutoComplete *>(this));
    registerAttribute("History", &history_);
    registerAttribute("HistorySize", &history_size_);
    registerAttribute("HistoryFile", &historyFile_);

    history_.make_list(0);
    history_size_.make_int64(4);
    historyFile_.make_string("");
    history_idx_ = 0;

    cmdLine_ = "";
    carretPos_ = 0;
}

AutoCompleter::~AutoCompleter() {
}

void AutoCompleter::postinitService() {
    if (historyFile_.size()) {
        AttributeType historyData;
        AttributeType historyConfig;
        RISCV_read_json_file(historyFile_.to_string(), &historyData);
        if (historyData.is_data() && historyData.size()) {
            historyConfig.from_config(historyData.to_string());
            if (historyConfig.is_list()) {
                for (unsigned i = 0; i < historyConfig.size(); i++) {
                    addToHistory(historyConfig[i].to_string());
                }
            }
        }
    }
    history_idx_ = history_.size();
}

void AutoCompleter::predeleteService() {
    if (!historyFile_.size()) {
        return;
    }
    AttributeType t1;
    t1.clone(&history_);
    t1.to_config();
    RISCV_write_json_file(historyFile_.to_string(),  t1.to_string());
}

bool AutoCompleter::processKey(uint32_t qt_key,
                                AttributeType *cmd,
                                AttributeType *cursor) {
    bool add_symbol = false;
    bool isNewLine = false;
    bool set_history_end = true;
    if (!cursor->is_list() || cursor->size() != 2) {
        cursor->make_list(2);
        (*cursor)[0u].make_int64(0);
        (*cursor)[1].make_int64(0);
    }
    switch (qt_key) {
    case KB_Up:
        set_history_end = false;
        if (history_idx_ == history_.size()) {
            unfinshedLine_ = cmdLine_;
        }
        if (history_idx_ > 0) {
            history_idx_--;
        }
        cmdLine_ = std::string(history_[history_idx_].to_string());
        break;
    case KB_Down:
        set_history_end = false;
        if (history_idx_ == (history_.size() - 1)) {
            history_idx_++;
            cmdLine_ = unfinshedLine_;
        } else if (history_idx_ < (history_.size() - 1)) {
            history_idx_++;
            cmdLine_ = std::string(history_[history_idx_].to_string());
        }
        break;
    case KB_Left:
        if (carretPos_ < cmdLine_.size()) {
            carretPos_++;
        }
        break;
    case KB_Right:
        if (carretPos_ > 0) {
            carretPos_--;
        }
        break;
    case KB_Backspace:// 1. Backspace button:
        if (cmdLine_.size() && carretPos_ == 0) {
            cmdLine_.erase(cmdLine_.size() - 1);
        } else if (cmdLine_.size() && carretPos_ < cmdLine_.size()) {
            std::string t1 = cmdLine_.substr(0, cmdLine_.size() - carretPos_ - 1);
            cmdLine_ = t1 + cmdLine_.substr(cmdLine_.size() - carretPos_, carretPos_);
        }
        break;
    case KB_Return:// 2. Enter button:
        isNewLine = true;
        break;
    case 0:
        break;
    case KB_Shift:
    case KB_Control:
    case KB_Alt:
        break;
    case KB_Dot:
        qt_key = '.';
        add_symbol = true;
        break;
    default:
        add_symbol = true;
    }

    if (add_symbol) {
        if (carretPos_ == 0) {
            cmdLine_ += static_cast<uint8_t>(qt_key);
        } else if (carretPos_ == cmdLine_.size()) {
            std::string t1;
            t1 += static_cast<uint8_t>(qt_key);
            cmdLine_ = t1 + cmdLine_;
        } else {
            std::string t1 = cmdLine_.substr(0, cmdLine_.size() - carretPos_);
            t1 += static_cast<uint8_t>(qt_key);
            cmdLine_ = t1 + cmdLine_.substr(cmdLine_.size() - carretPos_, carretPos_);
        }
    }

    cmd->make_string(cmdLine_.c_str());

    if (isNewLine) {
        if (cmdLine_.size()) {
            addToHistory(cmdLine_.c_str());
            cmdLine_.clear();
        }
        carretPos_ = 0;
    }
    (*cursor)[0u].make_int64(carretPos_);
    (*cursor)[1].make_int64(carretPos_);

    if (set_history_end) {
        history_idx_ = history_.size();
    }
    return isNewLine;
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
