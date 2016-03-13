/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Dynamically allocated buffer implementation.
 */

#include "api_core.h"
#include "autobuffer.h"
#include <cstdio>
#include <cstring>  // memcpy definition

namespace debugger {

AutoBuffer::AutoBuffer() {
    buf_ = NULL;
    buf_len_ = 0;
    buf_size_ = 0;
}

AutoBuffer::~AutoBuffer() {
    if (buf_) {
        delete [] buf_;
    }
}

void AutoBuffer::write_bin(const char *p, int sz) {
    if (buf_len_ + sz >= buf_size_) {
        if (buf_size_ == 0) {
            buf_size_ = 1024;
            buf_ = new char[buf_size_];
        } else {
            while (buf_len_ + sz >= buf_size_) {
                buf_size_ <<= 1;
            }
            char *t1 = new char[buf_size_];
            memcpy(t1, buf_, buf_len_);
            delete [] buf_;
            buf_ = t1;
        }
    }
    memcpy(&buf_[buf_len_], p, sz);
    buf_len_ += sz;
    buf_[buf_len_] = '\0';
}

void AutoBuffer::write_string(const char s) {
    write_bin(&s, 1);
}

void AutoBuffer::write_string(const char *s) {
    write_bin(s, static_cast<int>(strlen(s)));
}

void AutoBuffer::write_uint64(uint64_t v) {
    char tmp[128];
    int sz = RISCV_sprintf(tmp, sizeof(tmp),"0x%" RV_PRI64 "x", v);
    write_bin(tmp, sz);
}

void AutoBuffer::write_byte(uint8_t v) {
    char tmp[8];
    int sz = RISCV_sprintf(tmp, sizeof(tmp), "%02X", v);
    write_bin(tmp, sz);
}

}  // namespace debugger
