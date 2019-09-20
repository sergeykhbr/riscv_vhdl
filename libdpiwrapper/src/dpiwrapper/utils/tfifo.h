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

#pragma once

#include <types.h>
#include <utils.h>
#include <attribute.h>

template<class T> class TFifo {
 public:
    explicit TFifo(int size) {
        LIB_event_create(&event_, "tfifo");
        buf_ = new T[size];
        size_ = size;
        wcnt_ = 1;
        rcnt_ = 0;
    }
    ~TFifo() {
        delete [] buf_;
        LIB_event_close(&event_);
    }

    /** Thread safe method of the callbacks registration */
    void put(T *item) {
        if (is_full()) {
            LIB_event_set(&event_);
            return;
        }
        buf_[wcnt_] = *item;
        if (++wcnt_ >= size_) {
            wcnt_ = 0;
        }
        LIB_event_set(&event_);
    }

    void get(T *item) {
        if (is_empty()) {
            return;
        }
        if (++rcnt_ >= size_) {
            rcnt_ = 0;
        }
        *item = buf_[rcnt_];
    }

    /** Wait data with timeout */
    int wait(int ms) {
        LIB_event_clear(&event_);
        return LIB_event_wait_ms(&event_, ms);
    }

 protected:
    bool is_full() {
        return wcnt_ == rcnt_;
    }
    bool is_empty() {
        int ret = wcnt_ - rcnt_;
        if (ret < 0) {
            ret += size_;
        }
        return ret == 1;
    }

 protected:
    event_def event_;
    T *buf_;
    int wcnt_;
    int rcnt_;
    int size_;
};

