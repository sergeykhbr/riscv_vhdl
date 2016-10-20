/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Asynchronous queue with time markers.
 */

#include "api_utils.h"
#include "async_tqueue.h"

namespace debugger {

AsyncTQueueType::AsyncTQueueType() {
    preLen_ = 0;
    curLen_ = 0;
    stepPreQueued_.make_list(0);
    stepQueue_.make_list(16);   /** it will be auto reallocated if needed */
    item_.make_list(Queue_Total);
    RISCV_mutex_init(&mutex_);
}

AsyncTQueueType::~AsyncTQueueType() {
    RISCV_mutex_destroy(&mutex_);
}

void AsyncTQueueType::put(uint64_t time, IFace *cb) {
    RISCV_mutex_lock(&mutex_);
    item_[Queue_Time].make_uint64(time);
    item_[Queue_IFace].make_iface(cb);
    if (preLen_ == stepPreQueued_.size()) {
        unsigned new_sz = 2 * stepPreQueued_.size();
        if (new_sz == 0) {
            new_sz = 1;
        }
        stepPreQueued_.realloc_list(new_sz);
    }
    stepPreQueued_[preLen_].attr_free();
    stepPreQueued_[preLen_] = item_;
    preLen_++;
    RISCV_mutex_unlock(&mutex_);
}
void AsyncTQueueType::pushPreQueued() {
    if (preLen_ == 0) {
        return;
    }
    RISCV_mutex_lock(&mutex_);
    for (unsigned i = 0; i < preLen_; i++) {
        if (curLen_ < stepQueue_.size()) {
            stepQueue_[curLen_].attr_free();
            stepQueue_[curLen_] = stepPreQueued_[i];
        } else {
            stepQueue_.add_to_list(&stepPreQueued_[i]);
        }
        curLen_++;
    }
    preLen_= 0;
    RISCV_mutex_unlock(&mutex_);
}

void AsyncTQueueType::initProc() {
    curIdx_ = 0;
}

IFace *AsyncTQueueType::getNext(uint64_t step_cnt) {
    IFace *ret = 0;
    if (curIdx_ >= curLen_) {
        return ret;
    }
    for (unsigned i = curIdx_; i < curLen_; i++) {
        uint64_t ev_time = stepQueue_[i][Queue_Time].to_uint64();

        if (step_cnt < ev_time) {
            continue;
        }
        ret = stepQueue_[i][Queue_IFace].to_iface();

        // remove item from list using swap function to avoid usage
        // of allocation/deallocation calls.
        stepQueue_.swap_list_item(i, curLen_ - 1);
        curLen_--;
        curIdx_--;
        break;
    }
    return ret;
}

}  // namespace debugger

