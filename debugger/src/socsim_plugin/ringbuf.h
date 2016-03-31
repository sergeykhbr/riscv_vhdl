/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Ringbuffer implementation.
 */

#ifndef __DEBUGGER_RINGBUF_H__
#define __DEBUGGER_RINGBUF_H__

#include <inttypes.h>
#include <string>

namespace debugger {

class RingBufferType {
public:
    explicit RingBufferType(int sz) {
        size_ = sz + 1;
        rdpos_ = buf_ = new uint8_t[2 * size_];
        wrpos_ = rdpos_ + 1;
    }
    ~RingBufferType() {
        delete buf_;
    }

    uint8_t *put(uint8_t *buf, int sz) {
        uint8_t *ret = wrpos_;
        if (&buf_[size_] >= wrpos_ + sz) {
            memcpy(wrpos_, buf, sz);
            memcpy(wrpos_ + size_, buf, sz);
            wrpos_ += sz;
        } else {
            int part = static_cast<int>(&buf_[size_] - wrpos_);
            memcpy(wrpos_, buf, part);
            memcpy(wrpos_ + size_, buf, part);
            int part2 = sz - part;
            memcpy(buf_, &buf[part], part2);
            memcpy(buf_+ size_, &buf[part], part2);
            wrpos_ += (sz - size_);
        }
        return ret;
    }

    int get(uint8_t *obuf, int sz) {
        int ret = sz;
        if (ret > size()) {
            ret = size();
        }
        /// do not modificate rdpos_ pointer before data reading complete.
        if (rdpos_ == &buf_[size_ - 1]) {
            memcpy(obuf, buf_, ret);
        } else {
            memcpy(obuf, rdpos_ + 1, ret);
        }
        rdpos_ += sz;
        if (rdpos_ >= &buf_[size_]) {
            rdpos_ -= size_;
        }
        return ret;
    }

    int size() {
        int t1 = static_cast<int>(wrpos_ - rdpos_) - 1;
        if (t1 < 0) {
            t1 += size_;
        }
        return t1;
    }

private:
    uint8_t *buf_;
    uint8_t *wrpos_;
    uint8_t *rdpos_;
    int size_;
};

}  // namespace debugger

#endif  // __DEBUGGER_RINGBUF_H__
