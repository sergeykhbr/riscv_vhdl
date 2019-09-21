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

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <tfifo.h>
#include <icommand.h>
#include "utils.h"
#include "server.h"
#include "c_dpi.h"

static dpi_sv_interface dpi_ = {0};
static TFifo<ICommand *> fifo_(10);
static DpiServer server_;
static char descr_[256];

void dpi_put_fifo_request(void *iface) {
    ICommand *t1 = reinterpret_cast<ICommand *>(iface);
    fifo_.put(&t1);
}

int dpi_load_interface(dpi_sv_interface *iface) {
    int ret = 0;
    iface->sv_task_process_request = 
        (sv_task_process_request_proc)LIB_get_proc_addr("sv_task_process_request");
    if (!iface->sv_task_process_request) {
        LIB_printf("Can't find %s\n", "sv_task_process_request");
        ret = -1;
    }

    iface->sv_func_get_data = 
        (sv_func_get_data_proc)LIB_get_proc_addr("sv_func_get_data");
    if (!iface->sv_func_get_data) {
        LIB_printf("Can't find %s\n", "sv_func_get_data");
        ret = -1;
    }
    return ret;
}

extern "C" int c_task_server_start() {
    AttributeType tcp_resp;
    AttributeType srv_config(Attr_Dict);
    LIB_init();

    // TODO: external JSON configuration
    srv_config["BlockingMode"].make_boolean(true);
    srv_config["HostIP"].make_string("");
    srv_config["HostPort"].make_uint64(8687);
    srv_config["ClientConfig"].make_dict();
    srv_config["ClientConfig"]["Timeout"].make_int64(500);

    if (dpi_load_interface(&dpi_)) {
        return -1;
    }
    sv_in_t sv_in;
    sv_in.info = descr_;

    server_.postinit(srv_config);

    LIB_printf("%s", "Entering c_task_server_start\n");
    sv_in.req_type = REQ_TYPE_INFO;
    if (server_.run()) {
        LIB_sprintf(descr_, sizeof(descr_), "c_task_server_start: %s",
            "TCP server run successfully. See dpilib.log file for more "
            "details");
    } else {
        LIB_sprintf(descr_, sizeof(descr_), "%s",
                    "c_task_server_start: TCP server start failed");
    }
    if (dpi_.sv_task_process_request) {
	    dpi_.sv_task_process_request(&sv_in);
    }

    //server.stop();
    LIB_printf("c_task_server_start: %s\n", "end");
    //LIB_cleanup();
    return 0;
}

extern "C" int c_task_clk_posedge(const sv_out_t *d) {
    AttributeType tcp_resp;
    ICommand* isrc;
    sv_in_t sv_in;
    sv_in.info = descr_;
    if (!dpi_.sv_task_process_request) {
        return -1;
    }
    if (fifo_.is_empty()) {
#if 1  // delme
        sv_in.req_type = REQ_TYPE_INFO;
        LIB_sprintf(descr_, sizeof(descr_), "dpi posedge: %d", d->clkcnt);
        if (d->clkcnt % 1000 == 999) {
            dpi_.sv_task_process_request(&sv_in);
        }
#endif
        return 0;
    }

    fifo_.get(&isrc);

    /** Parse external request */
    AttributeType req = isrc->getRequest();
    tcp_resp.make_list(Resp_ListSize);
    if (!req.is_list() || req.size() < Req_ListSize) {
        tcp_resp[Resp_CmdType].make_string("Error");
        tcp_resp[Resp_Data].make_string("unsupported message format");
    } else if (req[Req_SourceName].is_equal("DpiServer")) {
        /** Managing requests: add/remove client, errors, status */
        if (req[Req_CmdType].is_equal("HartBeat")) {
            if (req[Req_Data].to_uint32() == 0) {
                sv_in.req_type = REQ_TYPE_INFO;
                LIB_sprintf(descr_, sizeof(descr_), "%s",
                    "Waiting simulator connection...");
                dpi_.sv_task_process_request(&sv_in);
                tcp_resp[Resp_CmdType].make_string("Hartbeat");
                tcp_resp[Resp_Data].make_floating(d->tm);
            }
        } else if (req[Req_CmdType].is_equal("ClientAdd")
            || req[Req_CmdType].is_equal("ClientRemove")) {
            sv_in.req_type = REQ_TYPE_INFO;
            LIB_sprintf(descr_, sizeof(descr_), "%s",
                req[Req_Data].to_string());
            dpi_.sv_task_process_request(&sv_in);
        }
    } else {
        if (req[Req_CmdType].is_equal("TimeSync")) {
            sv_in.req_type = REQ_TYPE_MOVE_CLOCK;
            sv_in.param1 = req[Req_Data].to_int();
            dpi_.sv_task_process_request(&sv_in);

            // Output:
            tcp_resp[Resp_CmdType].make_string("Time");
            tcp_resp[Resp_Data].make_dict();
            AttributeType& od = tcp_resp[Resp_Data];
            od["Time"].make_floating(d->tm);
            od["clkcnt"].make_uint64(d->clkcnt);

            /** TODO: Check Master requests: interrupts, DMA */
        } else if (req[Req_CmdType].is_equal("AXI4")) {
            const AttributeType& id = req[Req_Data];
            sv_in.req_type = REQ_TYPE_AXI4;
            sv_in.slvi.addr = id["addr"].to_uint64();
            sv_in.slvi.we = id["we"].to_bool();
            dpi_.sv_task_process_request(&sv_in);

            // Output:
            tcp_resp[Resp_CmdType].make_string("Axi4Slave");
            AttributeType& od = tcp_resp[Resp_Data];
            od["rdata"].make_list(1);
            od["rdata"][0u].make_uint64(d->slvo.rdata[0]);
        }
    }
    isrc->setResponse(tcp_resp);
    return 0;
}

