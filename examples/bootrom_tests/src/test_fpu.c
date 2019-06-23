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

#include <string.h>
#include "fw_api.h"

void test_fpu(void) { 
#ifdef FPU_ENABLED
    printf_uart("%s", "HW_FPU . . . . .");
#else
    printf_uart("%s", "SOFT_FPU . . . .");
#endif

    double x1 = 10.323;
    double x2 = -5.3333;
    double x3;

    x3 = x1 * x2;
    if (x3 != (10.323 * -5.3333)) {
        print_uart("FAIL (DMUL)\r\n", 12);
        return;
    }

    x3 = x1 / x2;
    if (x3 != (10.323 / -5.3333)) {
        print_uart("FAIL (DDIV)\r\n", 12);
        return;
    }

    x3 = x1 + x2;
    // It supposed to work only with optimization -O0
    if (x3 != (10.323 - 5.3333)) {
        print_uart("FAIL (DADD)\r\n", 12);
        return;
    }

    x3 = x1 - x2;
    if (x3 != (10.323 + 5.3333)) {
        print_uart("FAIL (DSUB)\r\n", 12);
        return;
    }

    print_uart("PASS\r\n", 6);
}
