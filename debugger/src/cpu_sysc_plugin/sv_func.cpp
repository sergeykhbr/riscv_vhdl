/*
 *  Copyright 2023 Sergey Khabarov, sergeykhbr@gmail.com
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

#include "sv_func.h"
#include "api_core.h"

namespace debugger {

/** Read HEX-file using relaitve RTL simulation project path */
void SV_readmemh(const char *filename, sc_uint<32> *mem) {
    char tpath[4096];
    file_def *f;
    char buf;
    Reg64Type v64 = {0};
    int cnt = 0;
    int addr = 0;
    RISCV_sprintf(tpath, sizeof(tpath), "%s/../sv/prj/impl/asic_sim/%s", REPO_PATH, filename);
    f = RISCV_file_open(tpath, "rb");

    if (!f) {
        RISCV_printf(0, LOG_ERROR, "Cannot open file %s", filename);
        return;
    }

    while (RISCV_file_read(f, &buf, 1) > 0) {
        if (buf >= '0' && buf <= '9') {
            v64.val = (v64.val << 4) | static_cast<uint8_t>(buf - '0');
            cnt++;
        } else if (buf >= 'a' && buf <= 'f') {
            v64.val = (v64.val << 4) | (10 + static_cast<uint8_t>(buf - 'a'));
            cnt++;
        } else if (buf >= 'A' && buf <= 'F') {
            v64.val = (v64.val << 4) | (10 + static_cast<uint8_t>(buf - 'A'));
            cnt++;
        } else {
            if (cnt) {
                mem[addr++] = v64.buf32[0];
            }
            v64.val = 0;
            cnt = 0;
        }
    }

    RISCV_file_close(f);
}

} // namespace debugger
