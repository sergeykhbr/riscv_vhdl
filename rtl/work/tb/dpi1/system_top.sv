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

module system_top();

realtime halfperiod = 2.5ns;  // 200 MHz

logic clk;
logic [31:0] clkcnt = 32'd0;

initial begin
    clk <= 0;
    forever begin
        #halfperiod clk <= ~clk; 
    end
end

virt_dbg virt_dbg_m(
    .clk(clk),
    .clkcnt(clkcnt)
);

always @ (posedge clk) begin
    clkcnt <= clkcnt + 1;
end

endmodule: system_top
