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
logic w_regs_spi_mode;
logic w_regs_pcie_12V_support;
logic w_regs_pcie_available;
logic [3:0] wb_regs_voltage_supply;
logic [7:0] wb_regs_check_pattern;
logic w_mem_req_valid;
logic [CFG_SYSBUS_ADDR_BITS-1:0] wb_mem_req_addr;
logic [7:0] wb_mem_req_size;
logic w_mem_req_write;
logic [CFG_SYSBUS_DATA_BITS-1:0] wb_mem_req_wdata;
logic [CFG_SYSBUS_DATA_BYTES-1:0] wb_mem_req_wstrb;
logic w_mem_req_last;
logic w_mem_req_ready;
logic w_cache_req_ready;
logic w_cache_resp_valid;
logic [63:0] wb_cache_resp_rdata;
logic w_cache_resp_err;
logic w_cache_resp_ready;
logic w_req_sdmem_ready;
logic w_req_sdmem_valid;
logic w_req_sdmem_write;
logic [CFG_SDCACHE_ADDR_BITS-1:0] wb_req_sdmem_addr;
logic [SDCACHE_LINE_BITS-1:0] wb_req_sdmem_wdata;
logic [SDCACHE_LINE_BITS-1:0] wb_resp_sdmem_rdata;
logic w_resp_sdmem_err;
logic [CFG_SDCACHE_ADDR_BITS-1:0] wb_regs_flush_address;
logic w_regs_flush_valid;
logic w_cache_flush_end;
logic w_trx_cmd_dir;
logic w_trx_cmd_cs;
logic w_cmd_in;
logic w_cmd_req_ready;
logic w_cmd_resp_valid;
logic [5:0] wb_cmd_resp_cmd;
logic [31:0] wb_cmd_resp_reg;
logic [6:0] wb_cmd_resp_crc7_rx;
logic [6:0] wb_cmd_resp_crc7_calc;
logic [14:0] wb_cmd_resp_spistatus;
logic w_cmd_resp_ready;
logic [3:0] wb_trx_cmdstate;
logic [3:0] wb_trx_cmderr;
logic w_clear_cmderr;
logic w_400kHz_ena;
logic w_crc7_clear;
logic w_crc7_next;
logic w_crc7_dat;
logic [6:0] wb_crc7;
logic w_crc15_next;
logic [14:0] wb_crc15_0;
logic [14:0] wb_crc15_1;
logic [14:0] wb_crc15_2;
logic [14:0] wb_crc15_3;
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
    .i_resp_valid(w_cache_resp_valid),
    .i_resp_rdata(wb_cache_resp_rdata),
    .i_resp_err(w_cache_resp_err)
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
    .i_sd_cmd(i_cmd),
    .i_sd_dat0(i_dat0),
    .i_sd_dat1(i_dat1),
    .i_sd_dat2(i_dat2),
    .i_sd_dat3(i_cd_dat3),
    .o_sck(o_sclk),
    .o_sck_posedge(w_regs_sck_posedge),
    .o_sck_negedge(w_regs_sck),
    .o_watchdog(wb_regs_watchdog),
    .o_clear_cmderr(w_regs_clear_cmderr),
    .o_spi_mode(w_regs_spi_mode),
    .o_pcie_12V_support(w_regs_pcie_12V_support),
    .o_pcie_available(w_regs_pcie_available),
    .o_voltage_supply(wb_regs_voltage_supply),
    .o_check_pattern(wb_regs_check_pattern),
    .i_400khz_ena(w_400kHz_ena),
    .i_sdtype(r.sdtype),
    .i_sdstate(r.sdstate),
    .i_cmd_state(wb_trx_cmdstate),
    .i_cmd_err(wb_trx_cmderr),
    .i_cmd_req_valid(r.cmd_req_valid),
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


sdctrl_crc15 #(
    .async_reset(async_reset)
) crcdat0 (
    .i_clk(i_clk),
    .i_nrst(i_nrst),
    .i_clear(r.crc15_clear),
    .i_next(w_crc15_next),
    .i_dat(i_dat0),
    .o_crc15(wb_crc15_0)
);


sdctrl_crc15 #(
    .async_reset(async_reset)
) crcdat1 (
    .i_clk(i_clk),
    .i_nrst(i_nrst),
    .i_clear(r.crc15_clear),
    .i_next(w_crc15_next),
    .i_dat(i_dat1),
    .o_crc15(wb_crc15_1)
);


sdctrl_crc15 #(
    .async_reset(async_reset)
) crcdat2 (
    .i_clk(i_clk),
    .i_nrst(i_nrst),
    .i_clear(r.crc15_clear),
    .i_next(w_crc15_next),
    .i_dat(i_dat2),
    .o_crc15(wb_crc15_2)
);


sdctrl_crc15 #(
    .async_reset(async_reset)
) crcdat3 (
    .i_clk(i_clk),
    .i_nrst(i_nrst),
    .i_clear(r.crc15_clear),
    .i_next(w_crc15_next),
    .i_dat(i_cd_dat3),
    .o_crc15(wb_crc15_3)
);


sdctrl_cmd_transmitter #(
    .async_reset(async_reset)
) cmdtrx0 (
    .i_clk(i_clk),
    .i_nrst(i_nrst),
    .i_sclk_posedge(w_regs_sck_posedge),
    .i_sclk_negedge(w_regs_sck),
    .i_cmd(w_cmd_in),
    .o_cmd(o_cmd),
    .o_cmd_dir(w_trx_cmd_dir),
    .o_cmd_cs(w_trx_cmd_cs),
    .i_spi_mode(w_regs_spi_mode),
    .i_watchdog(wb_regs_watchdog),
    .i_cmd_set_low(r.cmd_set_low),
    .i_req_valid(r.cmd_req_valid),
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
    .o_resp_spistatus(wb_cmd_resp_spistatus),
    .i_resp_ready(w_cmd_resp_ready),
    .i_clear_cmderr(w_clear_cmderr),
    .o_cmdstate(wb_trx_cmdstate),
    .o_cmderr(wb_trx_cmderr)
);


sdctrl_cache #(
    .async_reset(async_reset),
    .ibits(CFG_LOG2_SDCACHE_LINEBITS)
) cache0 (
    .i_clk(i_clk),
    .i_nrst(i_nrst),
    .i_req_valid(r.cache_req_valid),
    .i_req_write(r.cache_req_write),
    .i_req_addr(r.cache_req_addr),
    .i_req_wdata(r.cache_req_wdata),
    .i_req_wstrb(r.cache_req_wstrb),
    .o_req_ready(w_cache_req_ready),
    .o_resp_valid(w_cache_resp_valid),
    .o_resp_data(wb_cache_resp_rdata),
    .o_resp_err(w_cache_resp_err),
    .i_resp_ready(w_cache_resp_ready),
    .i_req_mem_ready(w_req_sdmem_ready),
    .o_req_mem_valid(w_req_sdmem_valid),
    .o_req_mem_write(w_req_sdmem_write),
    .o_req_mem_addr(wb_req_sdmem_addr),
    .o_req_mem_data(wb_req_sdmem_wdata),
    .i_mem_data_valid(r.sdmem_valid),
    .i_mem_data(wb_resp_sdmem_rdata),
    .i_mem_fault(w_resp_sdmem_err),
    .i_flush_address(wb_regs_flush_address),
    .i_flush_valid(w_regs_flush_valid),
    .o_flush_end(w_cache_flush_end)
);


always_comb
begin: comb_proc
    sdctrl_registers v;
    logic v_crc15_next;
    logic [31:0] vb_cmd_req_arg;
    logic v_cmd_resp_ready;
    logic v_cmd_dir;
    logic v_cmd_in;
    logic v_dat0_dir;
    logic v_dat3_dir;
    logic v_dat3_out;
    logic v_clear_cmderr;
    logic v_mem_req_ready;
    logic v_req_sdmem_ready;
    logic v_cache_resp_ready;

    v_crc15_next = 0;
    vb_cmd_req_arg = 0;
    v_cmd_resp_ready = 0;
    v_cmd_dir = 0;
    v_cmd_in = 0;
    v_dat0_dir = 0;
    v_dat3_dir = 0;
    v_dat3_out = 0;
    v_clear_cmderr = 0;
    v_mem_req_ready = 0;
    v_req_sdmem_ready = 0;
    v_cache_resp_ready = 0;

    v = r;

    vb_cmd_req_arg = r.cmd_req_arg;

    if (w_regs_spi_mode == 1'b1) begin
        v_dat3_dir = DIR_OUTPUT;
        v_dat3_out = w_trx_cmd_cs;
        v_cmd_dir = DIR_OUTPUT;
        v_dat0_dir = DIR_INPUT;
        v_cmd_in = i_dat0;
        if (w_regs_sck_posedge) begin
            // Not a full block 4096 bits just a cache line:
            v.sdmem_data = {r.sdmem_data[510: 0], i_dat0};
            v.bitcnt = (r.bitcnt + 1);
        end
    end else begin
        v_dat3_dir = r.dat3_dir;
        v_dat3_out = r.dat[3];
        v_cmd_dir = w_trx_cmd_dir;
        v_dat0_dir = r.dat_dir;
        v_cmd_in = i_cmd;
    end

    if (r.wait_cmd_resp == 1'b1) begin
        v_cmd_resp_ready = 1'b1;
        if (w_cmd_resp_valid == 1'b1) begin
            v.wait_cmd_resp = 1'b0;
            v.cmd_resp_r1 = wb_cmd_resp_cmd;
            v.cmd_resp_reg = wb_cmd_resp_reg;
            v.cmd_resp_spistatus = wb_cmd_resp_spistatus;

            if ((r.cmd_req_cmd == CMD8)
                    && (wb_trx_cmderr == CMDERR_NO_RESPONSE)) begin
                v.sdtype = SDCARD_VER1X;
                v.HCS = 1'b0;                               // Standard Capacity only
                v.initstate = IDLESTATE_CMD55;
                v_clear_cmderr = 1'b1;
            end else if (wb_trx_cmderr != CMDERR_NONE) begin
                if (r.cmd_req_cmd == CMD0) begin
                    // Re-send CMD0
                    v.initstate = IDLESTATE_CMD0;
                    v_clear_cmderr = 1'b1;
                end else begin
                    v.sdstate = SDSTATE_INA;
                    v.sdtype = SDCARD_UNUSABLE;
                end
            end else begin
                // Parse Rx response:
                case (r.cmd_req_rn)
                R1: begin
                end
                R3: begin
                    // Table 5-1: OCR Register definition, page 246
                    //     [23:0]  Voltage window can be requested by CMD58
                    //     [24]    Switching to 1.8V accepted (S18A)
                    //     [27]    Over 2TB support status (CO2T)
                    //     [29]    UHS-II Card status
                    //     [30]    Card Capacity Status (CCS)
                    //     [31]    Card power-up status (busy is LOW if the card not finished the power-up routine)
                    if (wb_cmd_resp_reg[31] == 1'b1) begin
                        v.OCR_VoltageWindow = wb_cmd_resp_reg[23: 0];
                        v.HCS = wb_cmd_resp_reg[30];
                        v.S18 = wb_cmd_resp_reg[24];
                    end
                end
                R6: begin
                    v.RCA = {wb_cmd_resp_reg[31: 16], 16'h0000};
                end
                default: begin
                end
                endcase
            end
        end
    end else if (r.cmd_req_valid == 1'b1) begin
        // Do nothing wait to accept
    end else begin
        // SD-card global state:
        case (r.sdstate)
        SDSTATE_PRE_INIT: begin
            // Page 222, Fig.4-96 State Diagram (Pre-Init mode)
            // 1. No commands were sent to the card after POW (except CMD0):
            //     CMD line held High for at least 1 ms (by SW), then SDCLK supplied
            //     at least 74 clocks with keeping CMD line High
            // 2. CMD High to Low transition && CMD=Low < 74 clocks then go idle,
            //     if Low >= 74 clocks then Fast boot in CV-mode
            if (w_regs_sck_posedge == 1'b1) begin
                v.clkcnt = (r.clkcnt + 1);
            end
            if (r.clkcnt >= 7'h49) begin
                v.sdstate = SDSTATE_IDLE;
            end
            if (r.clkcnt <= 7'h3f) begin
            end else begin
                v.cmd_set_low = 1'b0;
            end
        end
        SDSTATE_IDLE: begin
            case (r.initstate)
            IDLESTATE_CMD0: begin
                v.sdtype = SDCARD_UNKNOWN;
                v.HCS = 1'b1;
                v.S18 = 1'b0;
                v.RCA = '0;
                v.cmd_req_valid = 1'b1;
                v.cmd_req_cmd = CMD0;
                v.cmd_req_rn = R1;
                vb_cmd_req_arg = '0;
                v.initstate = IDLESTATE_CMD8;
            end
            IDLESTATE_CMD8: begin
                // See page 113. 4.3.13 Send Interface Condition Command
                //   [39:22] reserved 00000h
                //   [21]    PCIe 1.2V support 0
                //   [20]    PCIe availability 0
                //   [19:16] Voltage Supply (VHS) 0001b: 2.7-3.6V
                //   [15:8]  Check Pattern 55h
                v.cmd_req_valid = 1'b1;
                v.cmd_req_cmd = CMD8;
                v.cmd_req_rn = R7;
                vb_cmd_req_arg = '0;
                vb_cmd_req_arg[13] = w_regs_pcie_12V_support;
                vb_cmd_req_arg[12] = w_regs_pcie_available;
                vb_cmd_req_arg[11: 8] = wb_regs_voltage_supply;
                vb_cmd_req_arg[7: 0] = wb_regs_check_pattern;
                v.initstate = IDLESTATE_CMD55;
            end
            IDLESTATE_CMD55: begin
                // Page 64: APP_CMD (CMD55) shall always precede ACMD41.
                //   [31:16] RCA (Relative Adrress should be set 0)
                //   [15:0] stuff bits
                v.cmd_req_valid = 1'b1;
                v.cmd_req_cmd = CMD55;
                v.cmd_req_rn = R1;
                vb_cmd_req_arg = '0;
                v.initstate = IDLESTATE_ACMD41;
            end
            IDLESTATE_ACMD41: begin
                // Page 131: SD_SEND_OP_COND. 
                //   [31] reserved bit
                //   [30] HCS (high capacity support)
                //   [29] reserved for eSD
                //   [28] XPC (maximum power in default speed)
                //   [27:25] reserved bits
                //   [24] S18R Send request to switch to 1.8V
                //   [23:0] VDD voltage window (OCR[23:0])
                v.cmd_req_valid = 1'b1;
                v.cmd_req_cmd = ACMD41;
                vb_cmd_req_arg = '0;
                vb_cmd_req_arg[30] = r.HCS;
                vb_cmd_req_arg[23: 0] = r.OCR_VoltageWindow;
                if (w_regs_spi_mode == 1'b0) begin
                    // SD mode:
                    vb_cmd_req_arg[24] = r.S18;
                    v.cmd_req_rn = R3;
                    v.initstate = IDLESTATE_CARD_IDENTIFICATION;
                end else begin
                    // SPI mode:
                    v.cmd_req_rn = R1;
                    v.initstate = IDLESTATE_CMD58;
                end
            end
            IDLESTATE_CMD58: begin
                // READ_OCR: Reads OCR register. Used in SPI mode only.
                //   [31] reserved bit
                //   [30] HCS (high capacity support)
                //   [29:0] reserved
                //   SPI R1 response always in upper bits [14:8]
                if (r.cmd_resp_spistatus[14: 8] != 7'h01) begin
                    // SD card not in idle state
                    v.initstate = IDLESTATE_CMD55;
                end else begin
                    v.cmd_req_valid = 1'b1;
                    v.cmd_req_cmd = CMD58;
                    vb_cmd_req_arg = '0;
                    v.cmd_req_rn = R3;
                    v.initstate = IDLESTATE_CARD_IDENTIFICATION;
                end
            end
            IDLESTATE_CARD_IDENTIFICATION: begin
                if (r.HCS == 1'b1) begin
                    v.sdtype = SDCARD_VER2X_HC;
                end else if (r.sdtype == SDCARD_UNKNOWN) begin
                    v.sdtype = SDCARD_VER2X_SC;
                end
                if (w_regs_spi_mode == 1'b0) begin
                    // SD mode:
                    if (r.cmd_resp_reg[31] == 1'b0) begin
                        // LOW if the card has not finished power-up routine
                        v.initstate = IDLESTATE_CMD55;
                    end else if (r.S18 == 1'b1) begin
                        // Voltage switch command to change 3.3V to 1.8V
                        v.readystate = READYSTATE_CMD11;
                    end else begin
                        v.readystate = READYSTATE_CMD2;
                    end
                    v.sdstate = SDSTATE_READY;
                end else begin
                    // SPI mode:
                    v.sdstate = SDSTATE_SPI_DATA;
                end
            end
            default: begin
                v.initstate = IDLESTATE_CMD0;
            end
            endcase
        end
        SDSTATE_READY: begin
            case (r.readystate)
            READYSTATE_CMD11: begin
                // CMD11: VOLTAGE_SWITCH siwtch to 1.8V bus signaling.
                //   [31:0] reserved all zeros
                v.cmd_req_valid = 1'b1;
                v.cmd_req_cmd = CMD11;
                v.cmd_req_rn = R1;
                vb_cmd_req_arg = '0;
                v.readystate = READYSTATE_CMD2;
            end
            READYSTATE_CMD2: begin
                // CMD2: ALL_SEND_CID ask to send CID number.
                //   [31:0] stuff bits
                v.cmd_req_valid = 1'b1;
                v.cmd_req_cmd = CMD2;
                v.cmd_req_rn = R2;
                vb_cmd_req_arg = '0;
                v.readystate = READYSTATE_CHECK_CID;
            end
            READYSTATE_CHECK_CID: begin
                v.sdstate = SDSTATE_IDENT;
                v.identstate = IDENTSTATE_CMD3;
            end
            default: begin
            end
            endcase
        end
        SDSTATE_IDENT: begin
            case (r.identstate)
            IDENTSTATE_CMD3: begin
                // CMD3: SEND_RELATIVE_ADDR ask card to publish a new relative address (RCA).
                //   [31:0] stuff bits
                v.cmd_req_valid = 1'b1;
                v.cmd_req_cmd = CMD3;
                v.cmd_req_rn = R6;
                vb_cmd_req_arg = '0;
                v.identstate = IDENTSTATE_CHECK_RCA;
            end
            IDENTSTATE_CHECK_RCA: begin
                v.sdstate = SDSTATE_STBY;
            end
            default: begin
            end
            endcase
        end
        SDSTATE_SPI_DATA: begin
            if (r.spidatastate == SPIDATASTATE_CACHE_REQ) begin
                if (w_cache_req_ready == 1'b1) begin
                    v.cache_req_valid = 1'b0;
                    v.spidatastate = SPIDATASTATE_CACHE_WAIT_RESP;
                end
            end else if (r.spidatastate == SPIDATASTATE_CACHE_WAIT_RESP) begin
                v_req_sdmem_ready = 1'b1;
                v_cache_resp_ready = 1'b1;
                v.sdmem_addr = wb_req_sdmem_addr[(CFG_SDCACHE_ADDR_BITS - 1): 9];
                if (w_req_sdmem_valid == 1'b1) begin
                    if (w_req_sdmem_write == 1'b0) begin
                        v.spidatastate = SPIDATASTATE_CMD17_READ_SINGLE_BLOCK;
                    end else begin
                        v.spidatastate = SPIDATASTATE_CMD24_WRITE_SINGLE_BLOCK;
                    end
                end else if (w_cache_resp_valid == 1'b1) begin
                    v.spidatastate = SPIDATASTATE_WAIT_MEM_REQ;
                end
            end else if (r.spidatastate == SPIDATASTATE_CMD17_READ_SINGLE_BLOCK) begin
                // CMD17: READ_SINGLE_BLOCK. Reads a block of the size SET_BLOCKLEN
                //   [31:0] data address
                v.cmd_req_valid = 1'b1;
                v.cmd_req_cmd = CMD17;
                v.cmd_req_rn = R1;
                vb_cmd_req_arg = r.sdmem_addr;
                v.spidatastate = SPIDATASTATE_WAIT_DATA_START;
                v.bitcnt = '0;
            end else if (r.spidatastate == SPIDATASTATE_CMD24_WRITE_SINGLE_BLOCK) begin
            end else if (r.spidatastate == SPIDATASTATE_WAIT_DATA_START) begin
                if (r.bitcnt[7: 0] == 8'hfe) begin
                    v.spidatastate = SPIDATASTATE_READING_DATA;
                    v.bitcnt = '0;
                end else if ((&r.bitcnt) == 1'b1) begin
                    // TODO: set errmode, no data response
                end
            end else if (r.spidatastate == SPIDATASTATE_READING_DATA) begin
                if (w_regs_sck_posedge == 1'b1) begin
                    if ((&r.bitcnt) == 1'b1) begin
                        v.spidatastate = SPIDATASTATE_READING_CRC15;
                    end
                end
            end else if (r.spidatastate == SPIDATASTATE_READING_CRC15) begin
                v.spidatastate = SPIDATASTATE_READING_END;
            end else if (r.spidatastate == SPIDATASTATE_READING_END) begin
                v.spidatastate = SPIDATASTATE_CACHE_WAIT_RESP;
            end else begin
                // Wait memory request:
                v_mem_req_ready = 1'b1;
                if (w_mem_req_valid == 1'b1) begin
                    v.spidatastate = SPIDATASTATE_CACHE_REQ;
                    v.cache_req_valid = 1'b1;
                    v.cache_req_addr = (wb_mem_req_addr - i_xmapinfo.addr_start);
                    v.cache_req_write = w_mem_req_write;
                    v.cache_req_wdata = wb_mem_req_wdata;
                    v.cache_req_wstrb = wb_mem_req_wstrb;
                end
            end
        end
        default: begin
        end
        endcase
    end
    v.cmd_req_arg = vb_cmd_req_arg;

    if ((r.cmd_req_valid == 1'b1) && (w_cmd_req_ready == 1'b1)) begin
        v.cmd_req_valid = 1'b0;
        v.wait_cmd_resp = 1'b1;
    end

    v.sdmem_valid = 1'b0;
    if ((r.spidatastate == SPIDATASTATE_READING_DATA)
            && ((&r.bitcnt[8: 0]) == 1'b1)
            && (w_regs_sck_posedge == 1'b1)) begin
        v.sdmem_valid = 1'b1;
    end

    if (~async_reset && i_nrst == 1'b0) begin
        v = sdctrl_r_reset;
    end

    w_cmd_resp_ready = v_cmd_resp_ready;
    w_crc15_next = v_crc15_next;
    // Page 222, Table 4-81 Overview of Card States vs Operation Modes table
    if ((r.sdstate <= SDSTATE_IDENT)
            || (r.sdstate == SDSTATE_INA)
            || (r.sdstate == SDSTATE_PRE_INIT)) begin
        w_400kHz_ena = 1'b1;
    end else begin
        // data transfer mode:
        // Stand-By, Transfer, Sending, Receive, Programming, Disconnect states
        w_400kHz_ena = 1'b0;
    end

    w_cmd_in = v_cmd_in;
    o_cd_dat3 = v_dat3_out;
    o_dat2 = r.dat[2];
    o_dat1 = r.dat[1];
    o_dat0 = r.dat[0];
    // Direction bits:
    o_cmd_dir = v_cmd_dir;
    o_dat0_dir = v_dat0_dir;
    o_dat1_dir = r.dat_dir;
    o_dat2_dir = r.dat_dir;
    o_cd_dat3_dir = v_dat3_dir;
    // Memory request:
    w_mem_req_ready = v_mem_req_ready;
    w_cache_resp_ready = v_cache_resp_ready;
    w_clear_cmderr = (w_regs_clear_cmderr || v_clear_cmderr);
    // Cache to SD card requests:
    w_req_sdmem_ready = v_req_sdmem_ready;
    w_regs_flush_valid = 1'b0;

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
