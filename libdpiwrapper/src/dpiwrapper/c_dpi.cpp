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

#include "utils.h"
#include "server.h"
#include "c_dpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

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
    AttributeType srv_request;
    AttributeType srv_response;
    AttributeType srv_config(Attr_Dict);

#if defined(_WIN32) || defined(__CYGWIN__)
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData)) {
        printf("error: %s\n", "Can't initialize sockets library");
    }
#endif

    srv_config["Timeout"].make_int64(500);
    srv_config["BlockingMode"].make_boolean(true);
    srv_config["HostIP"].make_string("");
    srv_config["HostPort"].make_uint64(8687);

    DpiServer server(srv_config);
    dpi_data_t sv_out;
    request_t sv_in;
    int err;

    LIB_printf("%", "Entering c_task_server_start\n");
    server.run();

	const double SIMULATION_TIME_NS = 20000.0;
	if (dpi_load_interface(&dpi) == 0) {
        /** Get initial sv data */
        dpi.sv_func_get_data(&sv_out);

		while (sv_out.tm < SIMULATION_TIME_NS) {
            /** Wait server's request */
            err = server.getRequest(srv_request);
            if (srv_request.is_equal("hartbeat")) {
                sv_in.req_type = REQ_TYPE_MOVE_CLOCK;
                sv_in.param1 = 200;
			    dpi.sv_task_process_request(&sv_in);
            }

            /** Read all signals from SystemVerilog */
            dpi.sv_func_get_data(&sv_out);

            /** Send response to external simulator */
            srv_response.make_dict();
            srv_response["Time"].make_floating(sv_out.tm);
            srv_response["clkcnt"].make_uint64(sv_out.clkcnt);
            server.sendResponse(srv_response);
		}
	}

    sv_in.req_type = REQ_TYPE_STOP_SIM;
    if (dpi.sv_task_process_request) {
	    dpi.sv_task_process_request(&sv_in);
    }

    server.stop();
    LIB_printf("%s", "Thread joint\n");
#if defined(_WIN32) || defined(__CYGWIN__)
    WSACleanup();
#endif
}

