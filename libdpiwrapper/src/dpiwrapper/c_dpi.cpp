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

static TFifo<ICommand *> fifo_(10);

void dpi_put_fifo_request(void *iface) {
    ICommand *t1 = reinterpret_cast<ICommand *>(iface);
    fifo_.put(&t1);
}

int dpi_load_interface(dpi_sv_interface *iface) {
    int ret = 0;
    iface->sv_task_process_request = 
        (sv_task_process_request_proc)LIB_get_proc_addr("sv_task_process_request");
    if (!iface->sv_task_process_request) {
        ret = -1;
    }

    iface->sv_func_get_data = 
        (sv_func_get_data_proc)LIB_get_proc_addr("sv_func_get_data");
    if (!iface->sv_func_get_data) {
        ret = -1;
    }
    return ret;
}

extern "C" void c_task_server_start() {
    dpi_sv_interface dpi;
    AttributeType tcp_resp;
    AttributeType srv_config(Attr_Dict);
    LIB_init();

    // TODO: external JSON configuration
    srv_config["BlockingMode"].make_boolean(true);
    srv_config["HostIP"].make_string("");
    srv_config["HostPort"].make_uint64(8687);
    srv_config["ClientConfig"].make_dict();
    srv_config["ClientConfig"]["Timeout"].make_int64(500);

    DpiServer server(srv_config);
    sv_out_t sv_out;
    sv_in_t sv_in;
    int err;

    LIB_printf("%", "Entering c_task_server_start\n");
    server.run();

	const double SIMULATION_TIME_NS = 20000.0;
	if (dpi_load_interface(&dpi) == 0) {
        /** Get initial sv data */
        dpi.sv_func_get_data(&sv_out);

        while (sv_out.tm < SIMULATION_TIME_NS) {
            /** Wait any request */
            err = fifo_.wait(10000);
            if (err) {
                /** Timeout. Server should send hartbeat every 500 ms even
                    if there's no client connected. Something is wrong.
                */
                sv_in.req_type = REQ_TYPE_SERVER_ERR;
                LIB_sprintf(sv_in.descr, sizeof(sv_in.descr), "error: %s\n",
                            "server hartbeat timeout");
			    dpi.sv_task_process_request(&sv_in);
                break;
            } 

            ICommand *isrc;
            fifo_.get(&isrc);

            /** Parse external request */
            AttributeType req = isrc->getRequest();
            tcp_resp.make_list(Resp_ListSize);
            if(!req.is_list() || req.size() < Req_ListSize)  {
                tcp_resp[Resp_CmdType].make_string("Error");
                tcp_resp[Resp_Data].make_string("unsupported message format");
            } else if (req[Req_SourceName].is_equal("DpiServer")) {
                /** Managing requests: add/remove client, errors, status */
                if (req[Req_CmdType].is_equal("HartBeat")) {
                    if (req[Req_Data].to_uint32() == 0) {
                        sv_in.req_type = REQ_TYPE_INFO;
                        LIB_sprintf(sv_in.descr, sizeof(sv_in.descr), "%s",
                                    "Waiting simulator connection...");
                        dpi.sv_task_process_request(&sv_in);
                        dpi.sv_func_get_data(&sv_out);
                        tcp_resp[Resp_CmdType].make_string("Hartbeat");
                        tcp_resp[Resp_Data].make_floating(sv_out.tm);
                    }
                } else if (req[Req_CmdType].is_equal("ClientAdd")
                        || req[Req_CmdType].is_equal("ClientRemove")) {
                    sv_in.req_type = REQ_TYPE_INFO;
                    LIB_sprintf(sv_in.descr, sizeof(sv_in.descr), "%s",
                                req[Req_Data].to_string());
			        dpi.sv_task_process_request(&sv_in);
                }
            } else {
                if (req[Req_CmdType].is_equal("TimeSync")) {
                    sv_in.req_type = REQ_TYPE_MOVE_CLOCK;
                    sv_in.param1 = req[Req_Data].to_int();
                    dpi.sv_task_process_request(&sv_in);

                    // Output:
                    dpi.sv_func_get_data(&sv_out);
                    tcp_resp[Resp_CmdType].make_string("Time");
                    tcp_resp[Resp_Data].make_dict();
                    AttributeType &od = tcp_resp[Resp_Data];
                    od["Time"].make_floating(sv_out.tm);
                    od["clkcnt"].make_uint64(sv_out.clkcnt);

                    /** TODO: Check Master requests: interrupts, DMA */
                } else if (req[Req_CmdType].is_equal("AXI4")) {
                    const AttributeType &id = req[Req_Data];
                    sv_in.req_type = REQ_TYPE_AXI4;
                    sv_in.slvi.addr = id["addr"].to_uint64();
                    sv_in.slvi.we = id["we"].to_bool();
                    dpi.sv_task_process_request(&sv_in);

                    // Output:
                    dpi.sv_func_get_data(&sv_out);
                    tcp_resp[Resp_CmdType].make_string("Axi4Slave");
                    AttributeType &od = tcp_resp[Resp_Data];
                    od["rdata"].make_list(1);
                    od["rdata"][0u].make_uint64(sv_out.slvo.rdata[0]);
                } else {
                    dpi.sv_func_get_data(&sv_out);
                }
            }
            isrc->setResponse(tcp_resp);
		}
	}

    sv_in.req_type = REQ_TYPE_STOP_SIM;
    if (dpi.sv_task_process_request) {
	    dpi.sv_task_process_request(&sv_in);
    }

    server.stop();
    LIB_printf("%s", "Thread joint\n");
    LIB_cleanup();
}

