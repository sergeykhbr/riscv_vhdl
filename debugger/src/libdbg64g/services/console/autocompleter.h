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

#ifndef __DEBUGGER_AUTOCOMPLETER_H__
#define __DEBUGGER_AUTOCOMPLETER_H__

#include "iclass.h"
#include "iservice.h"
#include "coreservices/iautocomplete.h"
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
    virtual void predeleteService();

    /** IAutoComplete */
    virtual bool processKey(uint32_t qt_key, AttributeType *cmd,
                            AttributeType *cursor);


 private:
    void addToHistory(const char *cmd);

 private:
    AttributeType history_;
    AttributeType history_size_;
    AttributeType historyFile_;

    std::string cmdLine_;
    unsigned carretPos_;

    // History switching
    std::string unfinshedLine_; // store the latest whe we look through history
    unsigned history_idx_;
};

DECLARE_CLASS(AutoCompleter)

}  // namespace debugger

#endif  // __DEBUGGER_AUTOCOMPLETER_H__
