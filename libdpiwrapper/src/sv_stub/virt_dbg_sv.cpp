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
uint64_t rdata = 0xcafef00ddeadbeedull;

extern "C" void sv_task_process_request(const sv_in_t *r) {
    switch (r->req_type) {
    case REQ_TYPE_SERVER_ERR:
        printf("Server error\n");
        break;
    case REQ_TYPE_STOP_SIM:
        printf("Stop simulation\n");
        break;
    case REQ_TYPE_INFO:
        printf("DPI: %s\n", r->descr);
        break;
    case REQ_TYPE_MOVE_CLOCK:
        timescale += r->param1 * 5.0;
        clkcnt += 10;
        break;
    case REQ_TYPE_MOVE_TIME:
        timescale += r->param1 * 5.0;
        clkcnt += 10;
        break;
    case REQ_TYPE_AXI4:
        break;
    default:
        printf("Unkown request %d\n", r->req_type);
    }
}

extern "C" void sv_func_get_data(sv_out_t *d) {
    d->tm = timescale;
    d->clkcnt = clkcnt;
    d->slvo.rdata[0] = rdata;
}

