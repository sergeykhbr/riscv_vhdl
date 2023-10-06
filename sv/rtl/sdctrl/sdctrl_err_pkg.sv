// 
//  Copyright 2022 Sergey Khabarov, sergeykhbr@gmail.com
// 
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
// 
//      http://www.apache.org/licenses/LICENSE-2.0
// 
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
// 
package sdctrl_err_pkg;

import sdctrl_cfg_pkg::*;

typedef struct {
    logic [3:0] code;
} sdctrl_err_registers;

const sdctrl_err_registers sdctrl_err_r_reset = '{
    CMDERR_NONE                         // code
};

endpackage: sdctrl_err_pkg
