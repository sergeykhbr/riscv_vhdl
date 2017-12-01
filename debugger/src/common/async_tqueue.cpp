/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Asynchronous queue with time markers.
 */

#include <api_utils.h>
#include <async_tqueue.h>

namespace debugger {

AsyncTQueueType::AsyncTQueueType() {
    stepPreQueued_.make_list(0);
    stepQueue_.make_list(16);   /** it will be auto reallocated if needed */
    RISCV_mutex_init(&mutex_);
    hardReset();
}

AsyncTQueueType::~AsyncTQueueType() {
    RISCV_mutex_destroy(&mutex_);
}

void AsyncTQueueType::hardReset() {
    preLen_ = 0;
    curLen_ = 0;
    curIdx_ = 0;
}

void AsyncTQueueType::put(AttributeType *item) {
    if (preLen_ == stepPreQueued_.size()) {
        unsigned new_sz = 2 * stepPreQueued_.size();
        if (new_sz == 0) {
            new_sz = 1;
        }
        stepPreQueued_.realloc_list(new_sz);
    }
    stepPreQueued_[preLen_].attr_free();
    stepPreQueued_[preLen_] = *item;
    preLen_++;
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
    preLen_ = 0;
    RISCV_mutex_unlock(&mutex_);
}

void AsyncTQueueType::initProc() {
    curIdx_ = 0;
}

/** Clock queue */
ClockAsyncTQueueType::ClockAsyncTQueueType() : AsyncTQueueType() {
    item_.make_list(Queue_Total);
}

void ClockAsyncTQueueType::put(uint64_t time, IFace *cb) {
    RISCV_mutex_lock(&mutex_);
    item_[Queue_Time].make_uint64(time);
    item_[Queue_IFace].make_iface(cb);

    AsyncTQueueType::put(&item_);
    RISCV_mutex_unlock(&mutex_);
}

IFace *ClockAsyncTQueueType::getNext(uint64_t step_cnt) {
    IFace *ret = 0;
    if (curIdx_ >= curLen_) {
        return ret;
    }
    for (unsigned i = curIdx_; i < curLen_; i++) {
        uint64_t ev_time = stepQueue_[i][Queue_Time].to_uint64();

        curIdx_ = i;
        if (step_cnt < ev_time) {
            continue;
        }
        ret = stepQueue_[i][Queue_IFace].to_iface();

        // remove item from list using swap function to avoid usage
        // of allocation/deallocation calls.
        if (i < (curLen_ - 1)) {
            stepQueue_.swap_list_item(i, curLen_ - 1);
        }
        curLen_--;
        break;
    }
    return ret;
}


/** GUI queue */
GuiAsyncTQueueType::GuiAsyncTQueueType() : AsyncTQueueType() {
    item_.make_list(Queue_Total);
    dbg_cnt_ = 0;
}

void GuiAsyncTQueueType::put(IFace *src, AttributeType *cmd, bool silent) {
    RISCV_mutex_lock(&mutex_);
    item_[Queue_Source].make_iface(src);
    item_[Queue_Command] = *cmd;
    item_[Queue_Silent].make_boolean(silent);

    AsyncTQueueType::put(&item_);
    dbg_cnt_++;
    RISCV_mutex_unlock(&mutex_);
}

bool GuiAsyncTQueueType::getNext(IFace **src, AttributeType &cmd,
                                 bool &silent) {
    *src = 0;
    if (curIdx_ >= curLen_) {
        curIdx_ = 0;
        curLen_ = 0;
        return false;
    }
    AttributeType &item = stepQueue_[curIdx_++];
    dbg_cnt_--;

    *src = item[Queue_Source].to_iface();
    cmd = item[Queue_Command];
    silent = item[Queue_Silent].to_bool();
    return true;
}

void GuiAsyncTQueueType::remove(IFace *src) {
    RISCV_mutex_lock(&mutex_);
    for (unsigned i = 0; i < preLen_; i++) {
        AttributeType &item = stepPreQueued_[i];
        if (item[Queue_Source].to_iface() == src) {
            item[Queue_Source].make_iface(0);
        }
    }
    for (unsigned i = 0; i < curLen_; i++) {
        AttributeType &item = stepQueue_[i];
        if (item[Queue_Source].to_iface() == src) {
            item[Queue_Source].make_iface(0);
        }
    }
    RISCV_mutex_unlock(&mutex_);
}

}  // namespace debugger

