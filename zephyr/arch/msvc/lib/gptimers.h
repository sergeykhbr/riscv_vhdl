/*
 * Copyright (c) 2016, GNSS Sensor Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once

#include "imapdev.h"

class GPTimers : public IMappedDevice {
public:
    GPTimers() {
    }

    virtual bool isAddrValid(uint64_t addr) {
        return (addr >= 0x80005000 && addr < 0x80006000);
    }
    virtual void write(uint64_t addr, uint8_t *buf, int size) {
        uint64_t off = addr - 0x80005000;
        switch (off>>2) {
        case 0x00:
            break;
        case 0x04:
            break;
        default:;
        }

    }
    virtual void read(uint64_t addr, uint8_t *buf, int size) {
        uint64_t off = addr - 0x80005000;
        switch (off>>2) {
        case 0x0:
            *(uint32_t *)buf = (uint32_t)getHighticks();
            break;
        case 0x04:
            *(uint32_t *)buf = (uint32_t)(getHighticks()>>32);
            break;
        default:;
        }
    }

private:
    // High-resolution timer.
    uint64_t getHighticks() {
      LARGE_INTEGER counter;
      QueryPerformanceCounter(&counter);

      double ticks = double(counter.QuadPart);
      return (uint64_t)(1000*ticks);
    }

private:
};

