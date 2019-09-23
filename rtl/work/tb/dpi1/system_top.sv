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

    if (nrst == 0) begin
        v.clkcnt = 0;
    end
end

virt_dbg virt_dbg_m(
    .nrst(nrst),
    .clk(clk),
    .clkcnt(r.clkcnt),
    .o_slvi(slvi),
    .i_slvo(slvo)
);

axi4_rom #(
    .abits(15),
    .init_file("../../../examples/bootrom_tests/linuxbuild/bin/bootrom_tests.hex")
)
axi_rom_m (
    .nrst(nrst),
    .clk(clk),
    //.cfg(cfg),
    .i(slvi),
    .o(slvo)
);

always_ff @ (posedge clk) begin
    r <= v;
end

endmodule: system_top
