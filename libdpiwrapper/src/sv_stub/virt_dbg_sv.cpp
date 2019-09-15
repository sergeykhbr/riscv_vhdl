/*
 *  Copyright 2019 Sergey Khabarov, sergeykhbr@gmail.com
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

#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "dpiwrapper/c_dpi.h"

int clkcnt = 0;
double timescale = 0.0;

extern "C" void sv_task_process_request(const request_t *r) {
    timescale += r->param1 * 5.0;
    clkcnt += 10;
}

extern "C" void sv_func_get_data(dpi_data_t *d) {
    d->tm = timescale;
    d->clkcnt = clkcnt;
}

