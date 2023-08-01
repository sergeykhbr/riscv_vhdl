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

module vip_uart_top #(
    parameter bit async_reset = 1'b0,
    parameter int instnum = 0,
    parameter int baudrate = 115200,
    parameter int scaler = 8,
    parameter logpath = "uart"
)
(
    input logic i_nrst,
    input logic i_rx,
    output logic o_tx
);

import vip_uart_top_pkg::*;

localparam realtime pll_period = (1.0 / ((2 * scaler) * baudrate));

logic w_clk;
logic w_rx_rdy;
logic w_rx_rdy_clr;
logic w_tx_full;
logic [7:0] wb_rdata;
logic [7:0] wb_rdataz;
string outstr;
string outstrtmp;
string outfilename;                                         // formatted string name with instnum
int fl;
int fl_tmp;
vip_uart_top_registers r, rin;

function string U8ToString(
            input string istr,
            input logic [7:0] symb);
string ostr;
begin
    ostr = {istr, symb};
    return ostr;
end
endfunction: U8ToString

initial begin
    outfilename = $sformatf("%s_%1d.log",
            logpath,
            instnum);
    fl = $fopen(outfilename, "wb");
    assert (fl)
    else begin
        $warning("Cannot open log-file");
    end
    // Output string with each new symbol ended
    outfilename = $sformatf("%s_%1d.log.tmp",
            logpath,
            instnum);
    fl_tmp = $fopen(outfilename, "wb");
    assert (fl_tmp)
    else begin
        $warning("Cannot open log-file");
    end
end

vip_clk #(
    .period(pll_period)
) clk0 (
    .o_clk(w_clk)
);


vip_uart_receiver #(
    .async_reset(async_reset),
    .scaler(scaler)
) rx0 (
    .i_nrst(i_nrst),
    .i_clk(w_clk),
    .i_rx(i_rx),
    .o_rdy(w_rx_rdy),
    .i_rdy_clr(w_rx_rdy_clr),
    .o_data(wb_rdata)
);


vip_uart_transmitter #(
    .async_reset(async_reset),
    .scaler(scaler)
) tx0 (
    .i_nrst(i_nrst),
    .i_clk(w_clk),
    .i_we(w_rx_rdy),
    .i_wdata(wb_rdata),
    .o_full(w_tx_full),
    .o_tx(o_tx)
);


always_comb
begin: comb_proc
    vip_uart_top_registers v;
    v = r;

    w_rx_rdy_clr = w_rx_rdy;
    v.initdone = {r.initdone[0], 1'h1};

    if (~async_reset && i_nrst == 1'b0) begin
        v = vip_uart_top_r_reset;
    end

    rin = v;
end: comb_proc

generate
    if (async_reset) begin: async_rst_gen

        always_ff @(posedge w_clk, negedge i_nrst) begin: rg_proc
            if (i_nrst == 1'b0) begin
                r <= vip_uart_top_r_reset;
            end else begin
                r <= rin;
            end

            if (r.initdone[1] == 1'b0) begin
                outstrtmp = {""};
                outstrtmp = U8ToString(outstrtmp,
                        EOF_0x0D);
            end

            if (w_rx_rdy == 1'b1) begin
                if ((wb_rdata == 8'h0a) && (wb_rdataz != 8'h0d)) begin
                    // Create CR LF (0xd 0xa) instead of 0x0a:
                    outstr = U8ToString(outstr,
                            EOF_0x0D);
                end
                // Add symbol to string:
                outstr = U8ToString(outstr,
                        wb_rdata);
                outstrtmp = U8ToString(outstrtmp,
                        wb_rdata);

                if (wb_rdata == 8'h0a) begin
                    // Output simple string:
                    $display("%s", outstr);
                    $fwrite(fl, "%s", outstr);
                    $fflush(fl);
                end

                // Output string with the line ending symbol 0x0D first:
                $fwrite(fl_tmp, "%s", outstrtmp);
                $fflush(fl_tmp);

                // End-of-line
                if (wb_rdata == 8'h0a) begin
                    outstr = {""};
                    outstrtmp = {""};
                    outstrtmp = U8ToString(outstrtmp,
                            EOF_0x0D);
                end
                wb_rdataz = wb_rdata;
            end
        end: rg_proc


    end: async_rst_gen
    else begin: no_rst_gen

        always_ff @(posedge w_clk) begin: rg_proc
            r <= rin;

            if (r.initdone[1] == 1'b0) begin
                outstrtmp = {""};
                outstrtmp = U8ToString(outstrtmp,
                        EOF_0x0D);
            end

            if (w_rx_rdy == 1'b1) begin
                if ((wb_rdata == 8'h0a) && (wb_rdataz != 8'h0d)) begin
                    // Create CR LF (0xd 0xa) instead of 0x0a:
                    outstr = U8ToString(outstr,
                            EOF_0x0D);
                end
                // Add symbol to string:
                outstr = U8ToString(outstr,
                        wb_rdata);
                outstrtmp = U8ToString(outstrtmp,
                        wb_rdata);

                if (wb_rdata == 8'h0a) begin
                    // Output simple string:
                    $display("%s", outstr);
                    $fwrite(fl, "%s", outstr);
                    $fflush(fl);
                end

                // Output string with the line ending symbol 0x0D first:
                $fwrite(fl_tmp, "%s", outstrtmp);
                $fflush(fl_tmp);

                // End-of-line
                if (wb_rdata == 8'h0a) begin
                    outstr = {""};
                    outstrtmp = {""};
                    outstrtmp = U8ToString(outstrtmp,
                            EOF_0x0D);
                end
                wb_rdataz = wb_rdata;
            end
        end: rg_proc

    end: no_rst_gen
endgenerate

endmodule: vip_uart_top
