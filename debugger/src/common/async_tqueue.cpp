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

#include <api_core.h>
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
ClockAsyncTQueueType::ClockAsyncTQueueType() {
    size_ = 1;
    queue_ = new StepQueueItemType[size_];

    RISCV_mutex_init(&mutex_);
    hardReset();
}

ClockAsyncTQueueType::~ClockAsyncTQueueType() {
    RISCV_mutex_destroy(&mutex_);
    delete [] queue_;
}

void ClockAsyncTQueueType::hardReset() {
    item_total_ = 0;
    precnt_ = 0;
    item_cnt_ = 0;
}


void ClockAsyncTQueueType::put(uint64_t time, IFace *cb) {
    RISCV_mutex_lock(&mutex_);
    if (precnt_ >= 1024) {
        RISCV_mutex_unlock(&mutex_);
        RISCV_printf(0, 0, "clock pre-queue overflow: %d", precnt_);
        return;
    }
    prequeue_[precnt_].left = 0;
    prequeue_[precnt_].right = 0;
    prequeue_[precnt_].time = time;
    prequeue_[precnt_++].iface = cb;
    RISCV_mutex_unlock(&mutex_);
}

bool ClockAsyncTQueueType::move(IFace *cb, uint64_t time) {
    RISCV_mutex_lock(&mutex_);
    for (int i = 0; i < precnt_; i++) {
        if (prequeue_[i].iface == cb) {
            prequeue_[i].time = time;
            RISCV_mutex_unlock(&mutex_);
            return true;
        }
    }
    for (int i = 0; i < item_total_; i++) {
        if (queue_[i].iface == cb) {
            queue_[i].time = time;
            RISCV_mutex_unlock(&mutex_);
            return true;
        }
    }
    RISCV_mutex_unlock(&mutex_);
    return false;
}

void ClockAsyncTQueueType::pushPreQueued() {
    if (precnt_ == 0) {
        return;
    }
    RISCV_mutex_lock(&mutex_);
    while ((precnt_ + item_total_) > size_) {
        int t1 = 2*size_;
        StepQueueItemType *p1 = new StepQueueItemType[t1];
        memcpy(p1, queue_, item_total_*sizeof(StepQueueItemType));
        delete [] queue_;
        queue_ = p1;
        size_ = t1;
    }
    memcpy(&queue_[item_total_], prequeue_, precnt_*sizeof(StepQueueItemType));
    item_total_ += precnt_;
    precnt_ = 0;
    RISCV_mutex_unlock(&mutex_);
}


IFace *ClockAsyncTQueueType::getNext(uint64_t step_cnt) {
    IFace *ret = 0;
    if (item_cnt_ >= item_total_) {
        return ret;
    }
    for (; item_cnt_ < item_total_; item_cnt_++) {
        if (step_cnt < queue_[item_cnt_].time) {
            continue;
        }
        ret = queue_[item_cnt_].iface;

        // remove item from list using swap function to avoid usage
        // of allocation/deallocation calls.
        if (item_cnt_ < (item_total_ - 1)) {
            queue_[item_cnt_] = queue_[item_total_ - 1];
        }
        item_total_--;
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

