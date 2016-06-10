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

#include <stdint.h>
#include <bits/size_t.h>

#ifdef _WIN32
extern void LIBH_write(uint64_t addr, uint8_t *buf, int size);
extern void LIBH_read(uint64_t addr, uint8_t *buf, int size);
#endif

uint16_t READ16(volatile uint16_t *addr)
{
#ifdef _WIN32
    uint16_t ret;
    LIBH_read((uint64_t)((size_t)addr), (uint8_t *)&ret, 2);
    return ret;
#else
    return *addr;
#endif
}

uint32_t READ32(volatile uint32_t *addr)
{
#ifdef _WIN32
    uint32_t ret;
    LIBH_read((uint64_t)((size_t)addr), (uint8_t *)&ret, 4);
    return ret;
#else
    return *addr;
#endif
}

uint64_t READ64(volatile uint64_t *addr)
{
#ifdef _WIN32
    uint64_t ret;
    LIBH_read((uint64_t)((size_t)addr), (uint8_t *)&ret, 8);
    return ret;
#else
    return *addr;
#endif
}

void WRITE32(volatile uint32_t *addr, uint32_t val)
{
#ifdef _WIN32
    LIBH_write((uint64_t)((size_t)addr), (uint8_t *)&val, 4);
#else
    *addr = val;
#endif
}

void WRITE64(volatile uint64_t *addr, uint64_t val)
{
#ifdef _WIN32
    LIBH_write((uint64_t)((size_t)addr), (uint8_t *)&val, 8);
#else
    *addr = val;
#endif
}
