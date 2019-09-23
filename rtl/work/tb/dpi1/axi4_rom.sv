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

module axi4_rom #(
    parameter int memtech  = 0,
    parameter int async_reset = 0,
    parameter int xaddr    = 0,
    parameter int xmask    = 'hfffff,
    parameter int abits    = 17,
    parameter string init_file = ""
)(
    input nrst,
    input clk,
    //output axi4_slave_config_type cfg,
    input axi4_slave_in_type i,
    output axi4_slave_out_type o
);

parameter wb = 1 << (abits - 3);

typedef struct packed {
    logic [abits-4:0] raddr;
    logic [7:0] burst_cnt;
    logic r_valid;
} registers_t;

reg [CFG_SYSBUS_DATA_BITS-1:0] mem[0:wb-1];
registers_t v, r;

initial begin
    $readmemh(init_file, mem);
end

always_comb begin
    v = r;
    v.r_valid = 1'b0;
    if (r.burst_cnt != 0) begin
        v.r_valid = 1'b1;
        v.raddr = r.raddr + 1;
        v.burst_cnt = r.burst_cnt - 1;
    end else if (i.ar_valid == 1) begin
        v.raddr = i.ar_bits.addr[3 +: abits-3];
        v.r_valid = 1'b1;
        v.burst_cnt = i.ar_bits.len;
    end

    if (nrst == 0) begin
        v.raddr = 0;
        v.burst_cnt = 0;
        v.r_valid = 0;
    end
end

always_ff @ (posedge clk) begin
    r <= v;
end

assign o.ar_ready = 1'b1;
assign o.aw_ready = 1'b1;
assign o.r_valid = r.r_valid;
assign o.r_data = mem[r.raddr];

endmodule: axi4_rom
