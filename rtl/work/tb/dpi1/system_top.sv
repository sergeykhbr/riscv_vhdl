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

import pkg_amba4::*;

module system_top();

realtime halfperiod = 2.5ns;  // 200 MHz

typedef struct packed {
    logic [31:0] clkcnt;
    logic r_valid;
    logic [63:0] r_data;
} registers_t;

registers_t v, r;

logic clk;
logic nrst;

axi4_slave_in_type slvi;
axi4_slave_out_type slvo;

initial begin
    clk <= 0;
    nrst <= 1'b0;
    #50 nrst <= 1'b1;
end

always begin
    #halfperiod clk <= ~clk; 
end

always_comb begin
    v = r;

    v = r.clkcnt + 1;
    v.r_valid = slvi.ar_valid;
    v.r_data = 0;
    if (slvi.ar_valid == 1) begin
        v.r_data = 'hfeedfacecafef00d;
    end

    if (nrst == 0) begin
        v.r_valid = 1'b0;
        v.r_data = 0;
        v.clkcnt = 0;
    end
end

assign slvo.ar_ready = 1'b1;
assign slvo.aw_ready = 1'b1;
assign slvo.r_valid = r.r_valid;
assign slvo.r_data = r.r_data;

virt_dbg virt_dbg_m(
    .nrst(nrst),
    .clk(clk),
    .clkcnt(r.clkcnt),
    .o_slvi(slvi),
    .i_slvo(slvo)
);

always_ff @ (posedge clk) begin
    r <= v;
end

endmodule: system_top
