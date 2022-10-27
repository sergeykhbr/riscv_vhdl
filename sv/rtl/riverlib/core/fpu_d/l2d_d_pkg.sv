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
package l2d_d_pkg;


typedef struct {
    logic busy;
    logic [2:0] ena;
    logic signA;
    logic [63:0] absA;
    logic [63:0] result;
    logic op_signed;
    logic [63:0] mantAlign;
    logic [5:0] lshift;
} Long2Double_registers;

const Long2Double_registers Long2Double_r_reset = '{
    1'b0,                               // busy
    '0,                                 // ena
    1'b0,                               // signA
    '0,                                 // absA
    '0,                                 // result
    1'b0,                               // op_signed
    '0,                                 // mantAlign
    '0                                  // lshift
};

endpackage: l2d_d_pkg
