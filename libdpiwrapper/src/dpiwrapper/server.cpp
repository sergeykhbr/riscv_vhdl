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
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

thread_return_t server_thread(void *args) {
    int cnt = 0;
	ServerDataType *data = (ServerDataType *)args;

    while (data->enable) {
        LIB_sleep_ms(1000);
        LIB_printf("test_thread %d: time=%.1f clkcnt=%d\n",
			cnt++,
			data->dpi_data.tm,
			data->dpi_data.clkcnt);
		data->txcnt++;

        if (data->txcnt > 1) {
            LIB_printf("No response from simulator. "
                "Increase simulation length reqcnt=%d\n", data->txcnt);
        }
    }
#if defined(_WIN32) || defined(__CYGWIN__)
    _endthreadex(0);
#else
#endif
    return 0;
}

