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
#include "types.h"
#include "dpiwrapper/c_dpi.h"

int main(int argc, char *argv[]) {
#if defined(_WIN32) || defined(__CYGWIN__)
    HMODULE hlib1;
    HMODULE hlib2;
#else
    void *hlib;
#endif

    if ((hlib1 = LoadLibrary("libsv_stub.dll")) == 0) {
        printf("Can't open libsv_stub.dll\n");
        return -1;
    }

    if ((hlib2 = LoadLibrary("libdpiwrapper.dll")) == 0) {
        printf("Can't open libdpiwrapper.dll\n");
        return -1;
    }

    c_task_server_start_proc c_task_server_start = 
        (c_task_server_start_proc)GetProcAddress(hlib2,
        "c_task_server_start");
    if (!c_task_server_start) {
        printf("Can't find function dpi_c_task_server_start\n");
    } else {
        printf("Library libdpiwrapper.dll opened successfully\n");
        c_task_server_start();
    }

    FreeLibrary(hlib1);
    FreeLibrary(hlib2);

    return 0;
}