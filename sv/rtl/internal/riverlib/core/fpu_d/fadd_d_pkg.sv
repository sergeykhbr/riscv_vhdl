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
package fadd_d_pkg;


typedef struct {
    logic busy;
    logic [7:0] ena;
    logic [63:0] a;
    logic [63:0] b;
    logic [63:0] result;
    logic illegal_op;
    logic overflow;
    logic add;
    logic sub;
    logic eq;
    logic lt;
    logic le;
    logic max;
    logic min;
    logic flMore;
    logic flEqual;
    logic flLess;
    logic [11:0] preShift;
    logic signOpMore;
    logic [10:0] expMore;
    logic [52:0] mantMore;
    logic [52:0] mantLess;
    logic [104:0] mantLessScale;
    logic [105:0] mantSum;
    logic [6:0] lshift;
    logic [104:0] mantAlign;
    logic [11:0] expPostScale;
    logic [11:0] expPostScaleInv;
    logic [104:0] mantPostScale;
} DoubleAdd_registers;

const DoubleAdd_registers DoubleAdd_r_reset = '{
    1'b0,                               // busy
    '0,                                 // ena
    '0,                                 // a
    '0,                                 // b
    '0,                                 // result
    1'b0,                               // illegal_op
    1'b0,                               // overflow
    1'b0,                               // add
    1'b0,                               // sub
    1'b0,                               // eq
    1'b0,                               // lt
    1'b0,                               // le
    1'b0,                               // max
    1'b0,                               // min
    1'b0,                               // flMore
    1'b0,                               // flEqual
    1'b0,                               // flLess
    '0,                                 // preShift
    1'b0,                               // signOpMore
    '0,                                 // expMore
    '0,                                 // mantMore
    '0,                                 // mantLess
    '0,                                 // mantLessScale
    '0,                                 // mantSum
    '0,                                 // lshift
    '0,                                 // mantAlign
    '0,                                 // expPostScale
    '0,                                 // expPostScaleInv
    '0                                  // mantPostScale
};

endpackage: fadd_d_pkg
