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

#include <inttypes.h>
#include <stdarg.h>

#ifndef __TEST_NORF_SRC_GENERAL_H__
#define __TEST_NORF_SRC_GENERAL_H__

#ifdef __cplusplus
extern "C" {
#endif

void print_uart(const char *buf, int sz);
void print_uart_hex(uint64_t val);
void printf_uart(const char *fmt, ... );

void print_pnp(void);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // __TEST_NORF_SRC_GENERAL_H__
