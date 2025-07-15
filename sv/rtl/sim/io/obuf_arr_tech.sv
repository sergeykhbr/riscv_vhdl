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

module obuf_arr_tech #(
    parameter int width = 8                                 // Bus width
)
(
    input logic [width-1:0] i,                              // Input signal
    output logic [width-1:0] o                              // Output signal
);
 
  genvar n;
  generate 
    for (n = 0; n < width; n++) begin: obufx
      obuf_tech obx(
         .o(o[n]),
         .i(i[n])
      );
    end
  endgenerate

endmodule: obuf_arr_tech
