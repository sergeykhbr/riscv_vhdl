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

static const int AXI4_BURST_LEN_MAX     = 4;

typedef struct axi4_slave_in_t {
    uint64_t addr;
    uint64_t wdata[AXI4_BURST_LEN_MAX];
    char we;        // 0 = read; 1 = write
    char wstrb;
    char burst;
    char len;
} axi4_slave_in_t;

typedef struct axi4_slave_out_t {
    uint64_t rdata[AXI4_BURST_LEN_MAX];
} axi4_slave_out_t;

typedef struct sv_out_t {
	double tm;
	uint32_t clkcnt;
    axi4_slave_out_t slvo;
} sv_out_t;

typedef void (*sv_func_get_data_proc)(sv_out_t *d);

static const int REQ_TYPE_SERVER_ERR    = -2;
static const int REQ_TYPE_STOP_SIM      = -1;
static const int REQ_TYPE_INFO          = 1;
static const int REQ_TYPE_MOVE_CLOCK    = 2;
static const int REQ_TYPE_MOVE_TIME     = 3;
static const int REQ_TYPE_AXI4          = 4;

typedef struct sv_in_t {
    int req_type;
    int param1;
    axi4_slave_in_t slvi;
    char descr[256];
} sv_in_t;

typedef void (*sv_task_process_request_proc)(const sv_in_t *req);

typedef struct dpi_sv_interface {
    sv_func_get_data_proc sv_func_get_data;
    sv_task_process_request_proc sv_task_process_request;
} dpi_sv_interface;

typedef void (*c_task_server_start_proc)();

/** Interface between clients and DPI context task */
enum EJsonRequestList {
    Req_SourceName,
    Req_CmdType,
    Req_Data,
    Req_ListSize
};

enum EJsonResponseList {
    Resp_CmdType,
    Resp_Data,
    Resp_ListSize
};

void dpi_put_fifo_request(void *iface);