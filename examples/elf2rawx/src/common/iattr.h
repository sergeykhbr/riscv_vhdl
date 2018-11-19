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

#ifndef __DEBUGGER_IATTRIBUTE_H__
#define __DEBUGGER_IATTRIBUTE_H__

#include "iface.h"

static const char *const IFACE_ATTRIBUTE = "IAttribute";

class IAttribute : public IFace {
 public:
    IAttribute() :
        IFace(IFACE_ATTRIBUTE), attr_name_(NULL), attr_descr_(NULL) {}

    virtual void allocAttrName(const char *name) = 0;
    virtual void freeAttrName() = 0;
    virtual const char *getAttrName() { return attr_name_; }

    virtual void allocAttrDescription(const char *descr) = 0;
    virtual void freeAttrDescription() = 0;
    virtual const char *getAttrDescription() { return attr_descr_; }

    virtual void postinitAttribute() {}

 protected:
    char *attr_name_;
    char *attr_descr_;
};

#endif  // __DEBUGGER_IATTRIBUTE_H__
