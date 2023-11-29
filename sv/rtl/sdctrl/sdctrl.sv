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
logic w_regs_err_clear;
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
logic w_cache_req_ready;
logic w_cache_resp_valid;
logic [63:0] wb_cache_resp_rdata;
logic w_cache_resp_err;
logic w_cache_resp_ready;
logic w_req_sdmem_valid;
logic w_req_sdmem_write;
logic [CFG_SDCACHE_ADDR_BITS-1:0] wb_req_sdmem_addr;
logic [SDCACHE_LINE_BITS-1:0] wb_req_sdmem_wdata;
logic w_regs_flush_valid;
logic w_cache_flush_end;
logic w_trx_cmd;
logic w_trx_cmd_dir;
logic w_trx_cmd_csn;
logic w_trx_wdog_ena;
logic w_trx_err_valid;
logic [3:0] wb_trx_err_setcode;
logic w_cmd_in;
logic w_cmd_req_ready;
logic w_cmd_resp_valid;
logic [5:0] wb_cmd_resp_cmd;
logic [31:0] wb_cmd_resp_reg;
logic [6:0] wb_cmd_resp_crc7_rx;
logic [6:0] wb_cmd_resp_crc7_calc;
logic [14:0] wb_cmd_resp_spistatus;
logic w_cmd_resp_ready;
logic [15:0] wb_crc16_0;
logic [15:0] wb_crc16_1;
logic [15:0] wb_crc16_2;
logic [15:0] wb_crc16_3;
logic w_wdog_trigger;
logic [3:0] wb_err_code;
logic w_err_pending;

// SPI-mode controller signals:
logic w_spi_dat;
logic w_spi_dat_csn;
logic w_spi_cmd_req_valid;
logic [5:0] wb_spi_cmd_req_cmd;
logic [31:0] wb_spi_cmd_req_arg;
logic [2:0] wb_spi_cmd_req_rn;
logic w_spi_req_sdmem_ready;
logic w_spi_resp_sdmem_valid;
logic [511:0] wb_spi_resp_sdmem_data;
logic w_spi_err_valid;
logic w_spi_err_clear;
logic [3:0] wb_spi_err_setcode;
logic w_spi_400kHz_ena;
logic [2:0] wb_spi_sdtype;
logic w_spi_wdog_ena;
logic w_spi_crc16_clear;
logic w_spi_crc16_next;

// SD-mode controller signals:
logic w_sd_dat0;
logic w_sd_dat0_dir;
logic w_sd_dat1;
logic w_sd_dat1_dir;
logic w_sd_dat2;
logic w_sd_dat2_dir;
logic w_sd_dat3;
logic w_sd_dat3_dir;
logic w_sd_cmd_req_valid;
logic [5:0] wb_sd_cmd_req_cmd;
logic [31:0] wb_sd_cmd_req_arg;
logic [2:0] wb_sd_cmd_req_rn;
logic w_sd_req_sdmem_ready;
logic w_sd_resp_sdmem_valid;
logic [511:0] wb_sd_resp_sdmem_data;
logic w_sd_err_valid;
logic w_sd_err_clear;
logic [3:0] wb_sd_err_setcode;
logic w_sd_400kHz_ena;
logic [2:0] wb_sd_sdtype;
logic w_sd_wdog_ena;
logic w_sd_crc16_clear;
logic w_sd_crc16_next;

// Mode multiplexed signals:
logic w_cmd_req_valid;
logic [5:0] wb_cmd_req_cmd;
logic [31:0] wb_cmd_req_arg;
logic [2:0] wb_cmd_req_rn;
logic w_req_sdmem_ready;
logic w_resp_sdmem_valid;
logic [511:0] wb_resp_sdmem_data;
logic w_err_valid;
logic w_err_clear;
logic [3:0] wb_err_setcode;
logic w_400kHz_ena;
logic [2:0] wb_sdtype;
logic w_wdog_ena;
logic w_crc16_clear;
logic w_crc16_next;
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
    .i_req_ready(w_cache_req_ready),
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
    .o_sck(o_sclk),
    .o_sck_posedge(w_regs_sck_posedge),
    .o_sck_negedge(w_regs_sck),
    .o_watchdog(wb_regs_watchdog),
    .o_err_clear(w_regs_err_clear),
    .o_spi_mode(w_regs_spi_mode),
    .o_pcie_12V_support(w_regs_pcie_12V_support),
    .o_pcie_available(w_regs_pcie_available),
    .o_voltage_supply(wb_regs_voltage_supply),
    .o_check_pattern(wb_regs_check_pattern),
    .i_400khz_ena(w_400kHz_ena),
    .i_sdtype(wb_sdtype),
    .i_sd_cmd(i_cmd),
    .i_sd_dat0(i_dat0),
    .i_sd_dat1(i_dat1),
    .i_sd_dat2(i_dat2),
    .i_sd_dat3(i_cd_dat3),
    .i_err_code(wb_err_code),
    .i_cmd_req_valid(w_cmd_req_valid),
    .i_cmd_req_cmd(wb_cmd_req_cmd),
    .i_cmd_resp_valid(w_cmd_resp_valid),
    .i_cmd_resp_cmd(wb_cmd_resp_cmd),
    .i_cmd_resp_reg(wb_cmd_resp_reg),
    .i_cmd_resp_crc7_rx(wb_cmd_resp_crc7_rx),
    .i_cmd_resp_crc7_calc(wb_cmd_resp_crc7_calc)
);

sdctrl_wdog #(
    .async_reset(async_reset)
) wdog0 (
    .i_clk(i_clk),
    .i_nrst(i_nrst),
    .i_ena(w_wdog_ena),
    .i_period(wb_regs_watchdog),
    .o_trigger(w_wdog_trigger)
);

sdctrl_err #(
    .async_reset(async_reset)
) err0 (
    .i_clk(i_clk),
    .i_nrst(i_nrst),
    .i_err_valid(w_err_valid),
    .i_err_code(wb_err_setcode),
    .i_err_clear(w_err_clear),
    .o_err_code(wb_err_code),
    .o_err_pending(w_err_pending)
);

sdctrl_crc16 #(
    .async_reset(async_reset)
) crcdat0 (
    .i_clk(i_clk),
    .i_nrst(i_nrst),
    .i_clear(w_crc16_clear),
    .i_next(w_crc16_next),
    .i_dat(i_dat0),
    .o_crc15(wb_crc16_0)
);

sdctrl_crc16 #(
    .async_reset(async_reset)
) crcdat1 (
    .i_clk(i_clk),
    .i_nrst(i_nrst),
    .i_clear(w_crc16_clear),
    .i_next(w_crc16_next),
    .i_dat(i_dat1),
    .o_crc15(wb_crc16_1)
);

sdctrl_crc16 #(
    .async_reset(async_reset)
) crcdat2 (
    .i_clk(i_clk),
    .i_nrst(i_nrst),
    .i_clear(w_crc16_clear),
    .i_next(w_crc16_next),
    .i_dat(i_dat2),
    .o_crc15(wb_crc16_2)
);

sdctrl_crc16 #(
    .async_reset(async_reset)
) crcdat3 (
    .i_clk(i_clk),
    .i_nrst(i_nrst),
    .i_clear(w_crc16_clear),
    .i_next(w_crc16_next),
    .i_dat(i_cd_dat3),
    .o_crc15(wb_crc16_3)
);

sdctrl_spimode #(
    .async_reset(async_reset)
) spimode0 (
    .i_clk(i_clk),
    .i_nrst(r.nrst_spimode),
    .i_posedge(w_regs_sck_posedge),
    .i_miso(i_dat0),
    .o_mosi(w_spi_dat),
    .o_csn(w_spi_dat_csn),
    .i_detected(i_detected),
    .i_protect(i_protect),
    .i_cfg_pcie_12V_support(w_regs_pcie_12V_support),
    .i_cfg_pcie_available(w_regs_pcie_available),
    .i_cfg_voltage_supply(wb_regs_voltage_supply),
    .i_cfg_check_pattern(wb_regs_check_pattern),
    .i_cmd_req_ready(w_cmd_req_ready),
    .o_cmd_req_valid(w_spi_cmd_req_valid),
    .o_cmd_req_cmd(wb_spi_cmd_req_cmd),
    .o_cmd_req_arg(wb_spi_cmd_req_arg),
    .o_cmd_req_rn(wb_spi_cmd_req_rn),
    .i_cmd_resp_valid(w_cmd_resp_valid),
    .i_cmd_resp_r1r2(wb_cmd_resp_spistatus),
    .i_cmd_resp_arg32(wb_cmd_resp_reg),
    .o_data_req_ready(w_spi_req_sdmem_ready),
    .i_data_req_valid(w_req_sdmem_valid),
    .i_data_req_write(w_req_sdmem_write),
    .i_data_req_addr(wb_req_sdmem_addr),
    .i_data_req_wdata(wb_req_sdmem_wdata),
    .o_data_resp_valid(w_spi_resp_sdmem_valid),
    .o_data_resp_rdata(wb_spi_resp_sdmem_data),
    .i_crc16_0(wb_crc16_0),
    .o_crc16_clear(w_spi_crc16_clear),
    .o_crc16_next(w_spi_crc16_next),
    .o_wdog_ena(w_spi_wdog_ena),
    .i_wdog_trigger(w_wdog_trigger),
    .i_err_code(wb_err_code),
    .o_err_valid(w_spi_err_valid),
    .o_err_clear(w_spi_err_clear),
    .o_err_code(wb_spi_err_setcode),
    .o_400khz_ena(w_spi_400kHz_ena),
    .o_sdtype(wb_spi_sdtype)
);

sdctrl_sdmode #(
    .async_reset(async_reset)
) sdmode0 (
    .i_clk(i_clk),
    .i_nrst(r.nrst_sdmode),
    .i_posedge(w_regs_sck_posedge),
    .i_dat0(i_dat0),
    .o_dat0(w_sd_dat0),
    .o_dat0_dir(w_sd_dat0_dir),
    .i_dat1(i_dat1),
    .o_dat1(w_sd_dat1),
    .o_dat1_dir(w_sd_dat1_dir),
    .i_dat2(i_dat2),
    .o_dat2(w_sd_dat2),
    .o_dat2_dir(w_sd_dat2_dir),
    .i_cd_dat3(i_cd_dat3),
    .o_dat3(w_sd_dat3),
    .o_dat3_dir(w_sd_dat3_dir),
    .i_detected(i_detected),
    .i_protect(i_protect),
    .i_cfg_pcie_12V_support(w_regs_pcie_12V_support),
    .i_cfg_pcie_available(w_regs_pcie_available),
    .i_cfg_voltage_supply(wb_regs_voltage_supply),
    .i_cfg_check_pattern(wb_regs_check_pattern),
    .i_cmd_req_ready(w_cmd_req_ready),
    .o_cmd_req_valid(w_sd_cmd_req_valid),
    .o_cmd_req_cmd(wb_sd_cmd_req_cmd),
    .o_cmd_req_arg(wb_sd_cmd_req_arg),
    .o_cmd_req_rn(wb_sd_cmd_req_rn),
    .i_cmd_resp_valid(w_cmd_resp_valid),
    .i_cmd_resp_cmd(wb_cmd_resp_cmd),
    .i_cmd_resp_arg32(wb_cmd_resp_reg),
    .o_data_req_ready(w_sd_req_sdmem_ready),
    .i_data_req_valid(w_req_sdmem_valid),
    .i_data_req_write(w_req_sdmem_write),
    .i_data_req_addr(wb_req_sdmem_addr),
    .i_data_req_wdata(wb_req_sdmem_wdata),
    .o_data_resp_valid(w_sd_resp_sdmem_valid),
    .o_data_resp_rdata(wb_sd_resp_sdmem_data),
    .i_crc16_0(wb_crc16_0),
    .i_crc16_1(wb_crc16_1),
    .i_crc16_2(wb_crc16_2),
    .i_crc16_3(wb_crc16_3),
    .o_crc16_clear(w_sd_crc16_clear),
    .o_crc16_next(w_sd_crc16_next),
    .o_wdog_ena(w_sd_wdog_ena),
    .i_wdog_trigger(w_wdog_trigger),
    .i_err_code(wb_err_code),
    .o_err_valid(w_sd_err_valid),
    .o_err_clear(w_sd_err_clear),
    .o_err_code(wb_sd_err_setcode),
    .o_400khz_ena(w_sd_400kHz_ena),
    .o_sdtype(wb_sd_sdtype)
);

sdctrl_cmd_transmitter #(
    .async_reset(async_reset)
) cmdtrx0 (
    .i_clk(i_clk),
    .i_nrst(i_nrst),
    .i_sclk_posedge(w_regs_sck_posedge),
    .i_sclk_negedge(w_regs_sck),
    .i_cmd(w_cmd_in),
    .o_cmd(w_trx_cmd),
    .o_cmd_dir(w_trx_cmd_dir),
    .o_cmd_cs(w_trx_cmd_csn),
    .i_spi_mode(w_regs_spi_mode),
    .i_err_code(wb_err_code),
    .i_wdog_trigger(w_wdog_trigger),
    .i_cmd_set_low(r.cmd_set_low),
    .i_req_valid(w_cmd_req_valid),
    .i_req_cmd(wb_cmd_req_cmd),
    .i_req_arg(wb_cmd_req_arg),
    .i_req_rn(wb_cmd_req_rn),
    .o_req_ready(w_cmd_req_ready),
    .o_resp_valid(w_cmd_resp_valid),
    .o_resp_cmd(wb_cmd_resp_cmd),
    .o_resp_reg(wb_cmd_resp_reg),
    .o_resp_crc7_rx(wb_cmd_resp_crc7_rx),
    .o_resp_crc7_calc(wb_cmd_resp_crc7_calc),
    .o_resp_spistatus(wb_cmd_resp_spistatus),
    .i_resp_ready(w_cmd_resp_ready),
    .o_wdog_ena(w_trx_wdog_ena),
    .o_err_valid(w_trx_err_valid),
    .o_err_setcode(wb_trx_err_setcode)
);

sdctrl_cache #(
    .async_reset(async_reset)
) cache0 (
    .i_clk(i_clk),
    .i_nrst(i_nrst),
    .i_req_valid(w_mem_req_valid),
    .i_req_write(w_mem_req_write),
    .i_req_addr(wb_mem_req_addr),
    .i_req_wdata(wb_mem_req_wdata),
    .i_req_wstrb(wb_mem_req_wstrb),
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
    .i_mem_data_valid(w_resp_sdmem_valid),
    .i_mem_data(wb_resp_sdmem_data),
    .i_mem_fault(w_err_pending),
    .i_flush_valid(w_regs_flush_valid),
    .o_flush_end(w_cache_flush_end)
);

always_comb
begin: comb_proc
    sdctrl_registers v;
    logic v_cmd_dir;
    logic v_cmd_in;
    logic v_cmd_out;
    logic v_dat0_dir;
    logic v_dat0_out;
    logic v_dat1_dir;
    logic v_dat1_out;
    logic v_dat2_dir;
    logic v_dat2_out;
    logic v_dat3_dir;
    logic v_dat3_out;
    logic v_cmd_req_valid;
    logic [5:0] vb_cmd_req_cmd;
    logic [31:0] vb_cmd_req_arg;
    logic [2:0] vb_cmd_req_rn;
    logic v_req_sdmem_ready;
    logic v_resp_sdmem_valid;
    logic [511:0] vb_resp_sdmem_data;
    logic v_err_valid;
    logic v_err_clear;
    logic [3:0] vb_err_setcode;
    logic v_400kHz_ena;
    logic [2:0] vb_sdtype;
    logic v_wdog_ena;
    logic v_crc16_clear;
    logic v_crc16_next;

    v_cmd_dir = DIR_OUTPUT;
    v_cmd_in = 1'b0;
    v_cmd_out = 1'b1;
    v_dat0_dir = DIR_OUTPUT;
    v_dat0_out = 1'b1;
    v_dat1_dir = DIR_OUTPUT;
    v_dat1_out = 1'b1;
    v_dat2_dir = DIR_OUTPUT;
    v_dat2_out = 1'b1;
    v_dat3_dir = DIR_OUTPUT;
    v_dat3_out = 1'b1;
    v_cmd_req_valid = 1'b0;
    vb_cmd_req_cmd = '0;
    vb_cmd_req_arg = '0;
    vb_cmd_req_rn = '0;
    v_req_sdmem_ready = 1'b0;
    v_resp_sdmem_valid = 1'b0;
    vb_resp_sdmem_data = '0;
    v_err_valid = 1'b0;
    v_err_clear = 1'b0;
    vb_err_setcode = '0;
    v_400kHz_ena = 1'b1;
    vb_sdtype = '0;
    v_wdog_ena = 1'b0;
    v_crc16_clear = 1'b0;
    v_crc16_next = 1'b0;

    v = r;


    if (r.mode == MODE_PRE_INIT) begin
        // Page 222, Fig.4-96 State Diagram (Pre-Init mode)
        // 1. No commands were sent to the card after POW (except CMD0):
        //     CMD line held High for at least 1 ms (by SW), then SDCLK supplied
        //     at least 74 clocks with keeping CMD line High
        // 2. CMD High to Low transition && CMD=Low < 74 clocks then go idle,
        //     if Low >= 74 clocks then Fast boot in CV-mode
        if (w_regs_sck_posedge == 1'b1) begin
            v.clkcnt = (r.clkcnt + 1);
        end
        if (r.clkcnt >= 7'd73) begin
            if (w_regs_spi_mode == 1'b1) begin
                v.mode = MODE_SPI;
                v.nrst_spimode = 1'b1;
            end else begin
                v.mode = MODE_SD;
                v.nrst_sdmode = 1'b1;
            end
        end
    end else if (r.mode == MODE_SPI) begin
        // SPI MOSI:
        v_cmd_dir = DIR_OUTPUT;
        v_cmd_out = (~(((~w_trx_cmd) && (~w_trx_cmd_csn))
                || ((~w_spi_dat) && (~w_spi_dat_csn))));
        // SPI MISO:
        v_dat0_dir = DIR_INPUT;
        v_cmd_in = i_dat0;
        // SPI CSn:
        v_dat3_dir = DIR_OUTPUT;
        v_dat3_out = (w_trx_cmd_csn && w_spi_dat_csn);
        // Unused in SPI mode:
        v_dat2_dir = DIR_OUTPUT;
        v_dat2_out = 1'b1;
        v_dat1_dir = DIR_OUTPUT;
        v_dat1_out = 1'b1;

        v_cmd_req_valid = w_spi_cmd_req_valid;
        vb_cmd_req_cmd = wb_spi_cmd_req_cmd;
        vb_cmd_req_arg = wb_spi_cmd_req_arg;
        vb_cmd_req_rn = wb_spi_cmd_req_rn;
        v_req_sdmem_ready = w_spi_req_sdmem_ready;
        v_resp_sdmem_valid = w_spi_resp_sdmem_valid;
        vb_resp_sdmem_data = wb_spi_resp_sdmem_data;
        v_err_valid = w_spi_err_valid;
        v_err_clear = (w_regs_err_clear || w_spi_err_clear);
        vb_err_setcode = wb_spi_err_setcode;
        v_400kHz_ena = w_spi_400kHz_ena;
        vb_sdtype = wb_spi_sdtype;
        v_wdog_ena = (w_spi_wdog_ena || w_trx_wdog_ena);
        v_crc16_clear = w_spi_crc16_clear;
        v_crc16_next = w_spi_crc16_next;
    end else begin
        v_cmd_dir = w_trx_cmd_dir;
        v_cmd_in = i_cmd;
        v_cmd_out = w_trx_cmd;
        v_dat0_dir = w_sd_dat0_dir;
        v_dat0_out = w_sd_dat0;
        v_dat1_dir = w_sd_dat1_dir;
        v_dat1_out = w_sd_dat1;
        v_dat2_dir = w_sd_dat2_dir;
        v_dat2_out = w_sd_dat2;
        v_dat3_dir = w_sd_dat3_dir;
        v_dat3_out = w_sd_dat3;

        v_cmd_req_valid = w_sd_cmd_req_valid;
        vb_cmd_req_cmd = wb_sd_cmd_req_cmd;
        vb_cmd_req_arg = wb_sd_cmd_req_arg;
        vb_cmd_req_rn = wb_sd_cmd_req_rn;
        v_req_sdmem_ready = w_sd_req_sdmem_ready;
        v_resp_sdmem_valid = w_sd_resp_sdmem_valid;
        vb_resp_sdmem_data = wb_sd_resp_sdmem_data;
        v_err_valid = w_sd_err_valid;
        v_err_clear = (w_regs_err_clear || w_sd_err_clear);
        vb_err_setcode = wb_sd_err_setcode;
        v_400kHz_ena = w_sd_400kHz_ena;
        vb_sdtype = wb_sd_sdtype;
        v_wdog_ena = (w_sd_wdog_ena || w_trx_wdog_ena);
        v_crc16_clear = w_sd_crc16_clear;
        v_crc16_next = w_sd_crc16_next;
    end

    if (~async_reset && i_nrst == 1'b0) begin
        v = sdctrl_r_reset;
    end

    w_cmd_in = v_cmd_in;
    o_cmd = v_cmd_out;
    o_cmd_dir = v_cmd_dir;
    o_cd_dat3 = v_dat3_out;
    o_cd_dat3_dir = v_dat3_dir;
    o_dat2 = v_dat2_out;
    o_dat2_dir = v_dat2_dir;
    o_dat1 = v_dat1_out;
    o_dat1_dir = v_dat1_dir;
    o_dat0 = v_dat0_out;
    o_dat0_dir = v_dat0_dir;
    w_cmd_req_valid = v_cmd_req_valid;
    wb_cmd_req_cmd = vb_cmd_req_cmd;
    wb_cmd_req_arg = vb_cmd_req_arg;
    wb_cmd_req_rn = vb_cmd_req_rn;
    w_cmd_resp_ready = 1'b1;
    w_cache_resp_ready = 1'b1;
    w_req_sdmem_ready = v_req_sdmem_ready;
    w_resp_sdmem_valid = v_resp_sdmem_valid;
    wb_resp_sdmem_data = vb_resp_sdmem_data;
    w_err_valid = v_err_valid;
    w_err_clear = v_err_clear;
    wb_err_setcode = vb_err_setcode;
    w_400kHz_ena = v_400kHz_ena;
    wb_sdtype = vb_sdtype;
    w_wdog_ena = v_wdog_ena;
    w_crc16_clear = v_crc16_clear;
    w_crc16_next = v_crc16_next;

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
