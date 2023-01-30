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
 */

#ifndef __DEBUGGER_CME_PLUGIN_GENERIC_KEY_GEN1_H__
#define __DEBUGGER_CME_PLUGIN_GENERIC_KEY_GEN1_H__

#include "iservice.h"
#include "coreservices/icmdexec.h"
#include "coreservices/ikeyboard.h"
#include "coreservices/ireset.h"
#include "generic/iotypes.h"

namespace debugger {

class KeyGeneric : public ICommand,
                   public IResetListener {
 public:
    KeyGeneric(IService *parent, const char *keyname);

 public:
    /** ICommand */
    virtual int isValid(AttributeType *args);
    virtual void exec(AttributeType *args, AttributeType *res);

    /** IResetListener */
    virtual void reset(IFace *isource);

 protected:
    IFace *getInterface(const char *name) {
        return cmdParent_->getInterface(name);
    }

    // Common
    virtual void press();
    virtual void release();

 protected:
    bool pressed_;
    bool power_on_;
    AttributeType keyName_;
    IKeyboard *ikb_;
};

class KeyGeneric8 : public KeyGeneric,
                    public IIOPortListener8 {
 public:
    explicit KeyGeneric8(IService *parent, AttributeType &cfg) :
        KeyGeneric(parent, cfg[0u].to_string()) {
        inverse_ = cfg[1].to_bool();
        port_ = cfg[2];
        row_ = cfg[3].to_int();
        power_on_ = cfg[4].to_bool();
    }

    /** Common method */
    virtual void postinit();

    /** IIOPortListener8 */
    virtual void readData(uint8_t *val, uint8_t mask);
    virtual void writeData(uint8_t val, uint8_t mask) {}
    virtual void latch() {}

 protected:
    bool inverse_;
    AttributeType port_;
    int row_;
};

class KeyGeneric32 : public KeyGeneric,
                     public IIOPortListener32 {
 public:
    KeyGeneric32(IService *parent, AttributeType &cfg) :
        KeyGeneric(parent, cfg[0u].to_string()) {
        inverse_ = cfg[1].to_bool();
        port_ = cfg[2];
        row_ = cfg[3].to_int();
        power_on_ = cfg[4].to_bool();
    }

    /** Common method */
    virtual void postinit();

    /** IIOPortListener32 */
    virtual void readData(uint32_t *val, uint32_t mask);
    virtual void writeData(uint32_t val, uint32_t mask) {}
    virtual void latch() {}

 protected:
    bool inverse_;
    AttributeType port_;
    int row_;
};

class KeyPOW : public KeyGeneric {
 public:
    KeyPOW(IService *parent, const char *name) : KeyGeneric(parent, name) {
        power_on_ = true;
    }
};

}  // namespace debugger

#endif  // __DEBUGGER_CME_PLUGIN_GENERIC_KEY_GEN1_H__
