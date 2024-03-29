/*
 *  Copyright 2023 Sergey Khabarov, sergeykhbr@gmail.com
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
#include <string.h>
#include <stdio.h>
#include <utils.h>

int check_global_init = 0x7;

// Wait about 8-10 ms in RTL simulation before output (spent on .data segment coping without memcpy):
int main() {
    char ss[256];
    int ss_len;
    ss_len = sprintf(ss, "Hello World - %d!!!!\n", check_global_init);

    // printf() function requries more than 64 KB of ROM (see cpp_demo example):
    for (int i = 0; i < ss_len; i++) {
        utils_uart_putc(ss[i]);
    }
    return 0;
}

