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

`timescale 1ns/10ps

module zeroenc #(
    parameter int iwidth = 105,                             // Input bus width
    parameter int shiftwidth = 7                            // Encoded value width
)
(
    input logic [iwidth-1:0] i_value,                       // Input value to encode
    output logic [shiftwidth-1:0] o_shift                   // First non-zero bit
);

logic [shiftwidth-1:0] wb_muxind[0: (iwidth + 1) - 1];

assign wb_muxind[iwidth] = '0;
for (genvar i = (iwidth - 1); i >= 0; i--) begin: shftgen
    assign wb_muxind[i] = (i_value[i] == 1'b1) ? i : wb_muxind[(i + 1)];
end: shftgen
assign o_shift = wb_muxind[0];

endmodule: zeroenc
