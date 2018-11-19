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

#ifndef __DEBUGGER_IFACE_H__
#define __DEBUGGER_IFACE_H__

class IFace {
 public:
    explicit IFace(const char *name) : ifname_(name) {}
    virtual ~IFace() {}

    /** Get brief information. */
    virtual const char *getBrief() { return "Brief info not defined"; }

    /** Get detailed description. */
    virtual const char *getDetail() { return "Detail info not defined"; }

    /** Get interface name. */
    const char *getFaceName() { return ifname_; }

 protected:
    const char *ifname_;
};

#endif  // __DEBUGGER_IFACE_H__
