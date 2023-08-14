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

module sdctrl #(
    parameter bit async_reset = 1'b0
)
(
    input logic i_clk,                                      // CPU clock
    input logic i_nrst,                                     // Reset: active LOW
    input types_amba_pkg::mapinfo_type i_xmapinfo,          // APB interconnect slot information
    output types_pnp_pkg::dev_config_type o_xcfg,           // APB Device descriptor
    input types_amba_pkg::axi4_slave_in_type i_xslvi,       // AXI input interface to access SD-card memory
    output types_amba_pkg::axi4_slave_out_type o_xslvo,     // AXI output interface to access SD-card memory
    input types_amba_pkg::mapinfo_type i_pmapinfo,          // APB interconnect slot information
    output types_pnp_pkg::dev_config_type o_pcfg,           // APB sd-controller configuration registers descriptor
    input types_amba_pkg::apb_in_type i_apbi,               // APB Slave to Bridge interface
    output types_amba_pkg::apb_out_type o_apbo,             // APB Bridge to Slave interface
    output logic o_sclk,                                    // Clock up to 50 MHz
    input logic i_cmd,                                      // Command response;
    output logic o_cmd,                                     // Command request; DO in SPI mode
    output logic o_cmd_dir,                                 // Direction bit: 1=input; 0=output
    input logic i_dat0,                                     // Data Line[0] input; DI in SPI mode
    output logic o_dat0,                                    // Data Line[0] output
    output logic o_dat0_dir,                                // Direction bit: 1=input; 0=output
    input logic i_dat1,                                     // Data Line[1] input
    output logic o_dat1,                                    // Data Line[1] output
    output logic o_dat1_dir,                                // Direction bit: 1=input; 0=output
    input logic i_dat2,                                     // Data Line[2] input
    output logic o_dat2,                                    // Data Line[2] output
    output logic o_dat2_dir,                                // Direction bit: 1=input; 0=output
    input logic i_cd_dat3,                                  // Card Detect / Data Line[3] input
    output logic o_cd_dat3,                                 // Card Detect / Data Line[3] output; CS output in SPI mode
    output logic o_cd_dat3_dir,                             // Direction bit: 1=input; 0=output
    input logic i_detected,
    input logic i_protect
);

import types_amba_pkg::*;
import types_pnp_pkg::*;
import sdctrl_cfg_pkg::*;
import sdctrl_pkg::*;

logic w_regs_sck_posedge;
logic w_regs_sck;
logic w_regs_clear_cmderr;
logic [15:0] wb_regs_watchdog;
logic w_mem_req_valid;
logic [CFG_SYSBUS_ADDR_BITS-1:0] wb_mem_req_addr;
logic [7:0] wb_mem_req_size;
logic w_mem_req_write;
logic [CFG_SYSBUS_DATA_BITS-1:0] wb_mem_req_wdata;
logic [CFG_SYSBUS_DATA_BYTES-1:0] wb_mem_req_wstrb;
logic w_mem_req_last;
logic w_mem_req_ready;
logic w_mem_resp_valid;
logic [CFG_SYSBUS_DATA_BITS-1:0] wb_mem_resp_rdata;
logic wb_mem_resp_err;
logic w_cmd_req_ready;
logic w_cmd_resp_valid;
logic [5:0] wb_cmd_resp_cmd;
logic [31:0] wb_cmd_resp_reg;
logic [6:0] wb_cmd_resp_crc7_rx;
logic [6:0] wb_cmd_resp_crc7_calc;
logic w_cmd_resp_ready;
logic [3:0] wb_cmdstate;
logic [3:0] wb_cmderr;
logic w_crc7_clear;
logic w_crc7_next;
logic w_crc7_dat;
logic [6:0] wb_crc7;
logic w_crc16_next;
logic [3:0] wb_crc16_dat;
logic [15:0] wb_crc16;
sdctrl_registers r, rin;

axi_slv #(
    .async_reset(async_reset),
    .vid(VENDOR_OPTIMITECH),
    .did(OPTIMITECH_SDCTRL_MEM)
) xslv0 (
    .i_clk(i_clk),
    .i_nrst(i_nrst),
    .i_mapinfo(i_xmapinfo),
    .o_cfg(o_xcfg),
    .i_xslvi(i_xslvi),
    .o_xslvo(o_xslvo),
    .o_req_valid(w_mem_req_valid),
    .o_req_addr(wb_mem_req_addr),
    .o_req_size(wb_mem_req_size),
    .o_req_write(w_mem_req_write),
    .o_req_wdata(wb_mem_req_wdata),
    .o_req_wstrb(wb_mem_req_wstrb),
    .o_req_last(w_mem_req_last),
    .i_req_ready(w_mem_req_ready),
    .i_resp_valid(w_mem_resp_valid),
    .i_resp_rdata(wb_mem_resp_rdata),
    .i_resp_err(wb_mem_resp_err)
);


sdctrl_regs #(
    .async_reset(async_reset)
) regs0 (
    .i_clk(i_clk),
    .i_nrst(i_nrst),
    .i_pmapinfo(i_pmapinfo),
    .o_pcfg(o_pcfg),
    .i_apbi(i_apbi),
    .o_apbo(o_apbo),
    .o_sck(o_sclk),
    .o_sck_posedge(w_regs_sck_posedge),
    .o_sck_negedge(w_regs_sck),
    .o_watchdog(wb_regs_watchdog),
    .o_clear_cmderr(w_regs_clear_cmderr),
    .i_cmd_state(wb_cmdstate),
    .i_cmd_err(wb_cmderr),
    .i_cmd_req_valid(r.cmd_req_ena),
    .i_cmd_req_cmd(r.cmd_req_cmd),
    .i_cmd_resp_valid(w_cmd_resp_valid),
    .i_cmd_resp_cmd(wb_cmd_resp_cmd),
    .i_cmd_resp_reg(wb_cmd_resp_reg),
    .i_cmd_resp_crc7_rx(wb_cmd_resp_crc7_rx),
    .i_cmd_resp_crc7_calc(wb_cmd_resp_crc7_calc)
);


sdctrl_crc7 #(
    .async_reset(async_reset)
) crccmd0 (
    .i_clk(i_clk),
    .i_nrst(i_nrst),
    .i_clear(w_crc7_clear),
    .i_next(w_crc7_next),
    .i_dat(w_crc7_dat),
    .o_crc7(wb_crc7)
);


sdctrl_crc16 #(
    .async_reset(async_reset)
) crcdat0 (
    .i_clk(i_clk),
    .i_nrst(i_nrst),
    .i_clear(r.crc16_clear),
    .i_next(w_crc16_next),
    .i_dat(wb_crc16_dat),
    .o_crc16(wb_crc16)
);


sdctrl_cmd_transmitter #(
    .async_reset(async_reset)
) cmdtrx0 (
    .i_clk(i_clk),
    .i_nrst(i_nrst),
    .i_sclk_posedge(w_regs_sck_posedge),
    .i_sclk_negedge(w_regs_sck),
    .i_cmd(i_cmd),
    .o_cmd(o_cmd),
    .o_cmd_dir(o_cmd_dir),
    .i_watchdog(wb_regs_watchdog),
    .i_req_valid(r.cmd_req_ena),
    .i_req_cmd(r.cmd_req_cmd),
    .i_req_arg(r.cmd_req_arg),
    .i_req_rn(r.cmd_req_rn),
    .o_req_ready(w_cmd_req_ready),
    .i_crc7(wb_crc7),
    .o_crc7_clear(w_crc7_clear),
    .o_crc7_next(w_crc7_next),
    .o_crc7_dat(w_crc7_dat),
    .o_resp_valid(w_cmd_resp_valid),
    .o_resp_cmd(wb_cmd_resp_cmd),
    .o_resp_reg(wb_cmd_resp_reg),
    .o_resp_crc7_rx(wb_cmd_resp_crc7_rx),
    .o_resp_crc7_calc(wb_cmd_resp_crc7_calc),
    .i_resp_ready(w_cmd_resp_ready),
    .i_clear_cmderr(w_regs_clear_cmderr),
    .o_cmdstate(wb_cmdstate),
    .o_cmderr(wb_cmderr)
);


always_comb
begin: comb_proc
    sdctrl_registers v;
    logic v_crc16_next;

    v_crc16_next = 0;

    v = r;


    // SD-card global state:
    case (r.sdstate)
    SDSTATE_RESET: begin
        case (r.initstate)
        INITSTATE_CMD0: begin
            v.cmd_req_ena = 1'b1;
            v.cmd_req_cmd = CMD0;
            v.cmd_req_arg = 32'h00000000;
            v.cmd_req_rn = R1;
            v.initstate = (r.initstate + 1);
        end
        INITSTATE_CMD0_RESP: begin
            if (w_cmd_resp_valid == 1'b1) begin
                v.cmd_resp_r1 = wb_cmd_resp_cmd;
                v.cmd_resp_reg = wb_cmd_resp_reg;
                v.initstate = (r.initstate + 1);
            end
        end
        INITSTATE_CMD8: begin
        end
        INITSTATE_CMD41: begin
        end
        INITSTATE_CMD11: begin
        end
        INITSTATE_CMD2: begin
        end
        INITSTATE_CMD3: begin
        end
        INITSTATE_ERROR: begin
        end
        INITSTATE_DONE: begin
        end
        default: begin
        end
        endcase
    end
    default: begin
    end
    endcase

    if ((r.cmd_req_ena == 1'b1) && (w_cmd_req_ready == 1'b1)) begin
        v.cmd_req_ena = 1'b0;
    end

    if (~async_reset && i_nrst == 1'b0) begin
        v = sdctrl_r_reset;
    end

    w_cmd_resp_ready = 1'b1;
    w_crc16_next = v_crc16_next;

    o_cd_dat3 = r.dat[3];
    o_dat2 = r.dat[2];
    o_dat1 = r.dat[1];
    o_dat0 = r.dat[0];
    // Direction bits:
    o_dat0_dir = r.dat_dir;
    o_dat1_dir = r.dat_dir;
    o_dat2_dir = r.dat_dir;
    o_cd_dat3_dir = r.dat_dir;
    // Memory request:
    w_mem_req_ready = 1'b1;
    w_mem_resp_valid = 1'b1;
    wb_mem_resp_rdata = '1;
    wb_mem_resp_err = 1'b0;

    rin = v;
end: comb_proc

generate
    if (async_reset) begin: async_rst_gen

        always_ff @(posedge i_clk, negedge i_nrst) begin: rg_proc
            if (i_nrst == 1'b0) begin
                r <= sdctrl_r_reset;
            end else begin
                r <= rin;
            end
        end: rg_proc


    end: async_rst_gen
    else begin: no_rst_gen

        always_ff @(posedge i_clk) begin: rg_proc
            r <= rin;
        end: rg_proc

    end: no_rst_gen
endgenerate

endmodule: sdctrl
