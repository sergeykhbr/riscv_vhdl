/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief	   FIFO implementation.
 */

#ifndef __DEBUGGER_FIFO_H__
#define __DEBUGGER_FIFO_H__

#include <inttypes.h>

namespace debugger {

template<class T> class TFifo {
public:
    TFifo(int sz) {
        arr_ = new T[size_ = sz];
        prd_ = &arr_[0];
        pwr_ = &arr_[1];
    }
    ~TFifo() {
        delete [] arr_;
    }
    bool isFull() {
        return pwr_ == prd_;
    }
    bool isEmpty() {
        if (pwr_ == (prd_ + 1)) {
            return true;
        }
        if ((prd_ - pwr_ + 1) == size_) {
            return true;
        }
        return false;
    }
    void get(T *out) {
        if (isEmpty()) {
            return;
        }
        if (prd_ >= &arr_[size_ - 1]) {
            *out = arr_[0];
            prd_ = arr_;
        } else {
            *out = prd_[1];
            prd_++;
        }
    }
    void put(T *in) {
        if (isFull()) {
            return;
        }
        *pwr_ = *in;
        if (pwr_ >= &arr_[size_ - 1]) {
            pwr_ = arr_;
        } else {
            pwr_++;
        }
    }
private:
    T *arr_;
    T *prd_;
    T *pwr_;
    int size_;
};

}  // namespace debugger

#endif  // __DEBUGGER_FIFO_H__
