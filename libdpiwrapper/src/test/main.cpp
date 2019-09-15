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
#include <string>
#include <cstring>
#include "types.h"
#include "dpiwrapper/c_dpi.h"

int main(int argc, char *argv[]) {
    c_task_server_start_proc c_task_server_start;
#if defined(_WIN32) || defined(__CYGWIN__)
    HMODULE hlib1, hlib2;

    if ((hlib1 = LoadLibrary("libsv_stub.dll")) == 0) {
        printf("Can't open libsv_stub.dll\n");
        return -1;
    }

    if ((hlib2 = LoadLibrary("libdpiwrapper.dll")) == 0) {
        printf("Can't open libdpiwrapper.dll\n");
        return -1;
    }

    c_task_server_start = (c_task_server_start_proc)GetProcAddress(hlib2,
                            "c_task_server_start");
#else
    void *hlib1, *hlib2;
    char fld[4096];

    Dl_info dl_info;
    dladdr((void *)main, &dl_info);
    sprintf(fld, "%s", dl_info.dli_fname);
    int n = (int)strlen(fld);
    while (n > 0 && fld[n] != '\\' && fld[n] != '/') n--;
    fld[n+1] = '\0';

    printf("Current folder %s\n", fld);

    dlerror();
    std::string f1 = std::string(fld) + std::string("libsv_stub.so");
    hlib1 = dlopen(f1.c_str(), RTLD_NOW);
    if (hlib1 == 0) {
        printf("Can't open libsv_stub.so, err=%s\n", dlerror());
        return -1;
    }

    dlerror();
    std::string f2 = std::string(fld) + std::string("libdpiwrapper.so");
    hlib2 = dlopen(f2.c_str(), RTLD_NOW);
    if (hlib2 == 0) {
        printf("Can't open libdpiwrapper.so, err=%s\n", dlerror());
        return -1;
    }

    c_task_server_start = (c_task_server_start_proc)dlsym(hlib2,
                            "c_task_server_start");
#endif

    if (!c_task_server_start) {
        printf("Can't find function dpi_c_task_server_start\n");
    } else {
        printf("Library libdpiwrapper.dll opened successfully\n");
        c_task_server_start();
    }

#if defined(_WIN32) || defined(__CYGWIN__)
    FreeLibrary(hlib1);
    FreeLibrary(hlib2);
#else
    dlclose(hlib1);
    dlclose(hlib2);
#endif
    return 0;
}