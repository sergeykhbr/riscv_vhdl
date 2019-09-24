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

extern "C" int c_task_server_start();
extern "C" int c_task_clk_posedge(const sv_out_t *sv2c, sv_in_t *c2sv);

extern "C" void sv_task_process_request(const sv_in_t *r) {
    switch (r->req_type) {
    case REQ_TYPE_SERVER_ERR:
        printf("Server error\n");
        break;
    case REQ_TYPE_STOP_SIM:
        printf("Stop simulation\n");
        break;
    default:
        printf("Unkown request %d\n", r->req_type);
    }
}

extern "C" void sv_func_info(const char *info) {
    printf("SV: %s\n", info);
}

enum EBusState {
    Bus_Idle,
    Bus_WaitResponse
};

extern "C" void simulation_start() {
    int clkcnt = 0;
    double timescale = 0.0;
    sv_in_t sv_in;
    sv_out_t sv_out;
    uint64_t rdata = 0x13;//0xcafef00ddeadbeedull;
    EBusState estate = Bus_Idle;

    c_task_server_start();

    while (1) {
        timescale += 5.0;
        clkcnt += 1;
        sv_out.tm = timescale;
        sv_out.clkcnt = clkcnt;
        sv_out.slvo.rdata[0] = 0;
        sv_out.req_ready = 0;
        sv_out.resp_valid = 0;

        switch (estate) {
        case Bus_Idle:
            sv_out.req_ready = 1;
            if (sv_in.req_type == REQ_TYPE_AXI4 && sv_in.req_valid) {
                estate = Bus_WaitResponse;
            }
            break;
        case Bus_WaitResponse:
            sv_out.resp_valid = 1;
            sv_out.slvo.rdata[0] = rdata;
            estate = Bus_Idle;
            break;
        default:;
        }

        c_task_clk_posedge(&sv_out, &sv_in);
    }
}

