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

#pragma once

#include <inttypes.h>

typedef struct dpi_data_t {
	double tm;
	uint32_t clkcnt;
} dpi_data_t;

typedef void (*sv_func_get_data_proc)(dpi_data_t *d);

static const int REQ_TYPE_MOVE_CLOCK = 1;
static const int REQ_TYPE_MOVE_TIME  = 2;
static const int REQ_TYPE_STOP_SIM   = -1;

typedef struct request_t {
    int req_type;
    int param1;
} request_t;

typedef void (*sv_task_process_request_proc)(const request_t *req);

typedef struct dpi_sv_interface {
    sv_func_get_data_proc sv_func_get_data;
    sv_task_process_request_proc sv_task_process_request;
} dpi_sv_interface;


typedef void (*c_task_server_start_proc)();