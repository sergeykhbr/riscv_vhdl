/*
 *  Copyright 2019 Sergey Khabarov, sergeykhbr@gmail.com
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

#ifndef __DEBUGGER_SERVICES_REMOTE_JSONCMD_H__
#define __DEBUGGER_SERVICES_REMOTE_JSONCMD_H__

#include "tcpcmd_gen.h"

namespace debugger {

class JsonCommands : public TcpCommandsGen {
 public:
    explicit JsonCommands(IService *parent);

 protected:
    virtual int processCommand(const char *cmdbuf, int bufsz);
    virtual bool isStartMarker(char s) { return true; }
    virtual bool isEndMarker(const char *s, int sz) {
        return s[sz - 1] == '\0';
    }
};

}  // namespace debugger

#endif  // __DEBUGGER_SERVICES_REMOTE_JSONCMD_H__
