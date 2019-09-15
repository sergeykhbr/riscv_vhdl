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

extern thread_return_t server_thread(void *args);

LibThreadType th1;
ServerDataType srvdata_;

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
	bool dpi_ok = true;
    LIB_printf("%", "Entering c_task_server_start\n");
    th1.func = reinterpret_cast<lib_thread_func>(server_thread);
	srvdata_.enable = 1;
    th1.args = &srvdata_;
    LIB_thread_create(&th1);
    dpi_sv_interface dpi;

	const double SIMULATION_TIME_NS = 20000.0;
	if (dpi_load_interface(&dpi) == 0) {
		while (srvdata_.dpi_data.tm < SIMULATION_TIME_NS) {
			if (srvdata_.txcnt == 0) {
				LIB_sleep_ms(1);
                continue;
            }
            srvdata_.request.req_type = REQ_TYPE_MOVE_CLOCK;
            srvdata_.request.param1 = 200;
			dpi.sv_task_process_request(&srvdata_.request);
			srvdata_.txcnt = 0;

            dpi.sv_func_get_data(&srvdata_.dpi_data);
		}
	}

    srvdata_.request.req_type = REQ_TYPE_STOP_SIM;
    if (dpi.sv_task_process_request) {
	    dpi.sv_task_process_request(&srvdata_.request);
    }

	srvdata_.enable = 0;
    LIB_thread_join(th1.Handle, INFINITE);
    LIB_printf("%s", "Thread joint\n");
}

