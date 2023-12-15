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

#include "sdctrl.h"
#include "api_core.h"

namespace debugger {

sdctrl::sdctrl(sc_module_name name,
               bool async_reset)
    : sc_module(name),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_xmapinfo("i_xmapinfo"),
    o_xcfg("o_xcfg"),
    i_xslvi("i_xslvi"),
    o_xslvo("o_xslvo"),
    i_pmapinfo("i_pmapinfo"),
    o_pcfg("o_pcfg"),
    i_apbi("i_apbi"),
    o_apbo("o_apbo"),
    o_sclk("o_sclk"),
    i_cmd("i_cmd"),
    o_cmd("o_cmd"),
    o_cmd_dir("o_cmd_dir"),
    i_dat0("i_dat0"),
    o_dat0("o_dat0"),
    o_dat0_dir("o_dat0_dir"),
    i_dat1("i_dat1"),
    o_dat1("o_dat1"),
    o_dat1_dir("o_dat1_dir"),
    i_dat2("i_dat2"),
    o_dat2("o_dat2"),
    o_dat2_dir("o_dat2_dir"),
    i_cd_dat3("i_cd_dat3"),
    o_cd_dat3("o_cd_dat3"),
    o_cd_dat3_dir("o_cd_dat3_dir"),
    i_detected("i_detected"),
    i_protect("i_protect") {

    async_reset_ = async_reset;
    xslv0 = 0;
    regs0 = 0;
    err0 = 0;
    wdog0 = 0;
    crcdat0 = 0;
    crcdat1 = 0;
    crcdat2 = 0;
    crcdat3 = 0;
    spimode0 = 0;
    sdmode0 = 0;
    cmdtrx0 = 0;
    cache0 = 0;

    xslv0 = new axi_slv("xslv0", async_reset,
                         VENDOR_OPTIMITECH,
                         OPTIMITECH_SDCTRL_MEM);
    xslv0->i_clk(i_clk);
    xslv0->i_nrst(i_nrst);
    xslv0->i_mapinfo(i_xmapinfo);
    xslv0->o_cfg(o_xcfg);
    xslv0->i_xslvi(i_xslvi);
    xslv0->o_xslvo(o_xslvo);
    xslv0->o_req_valid(w_mem_req_valid);
    xslv0->o_req_addr(wb_mem_req_addr);
    xslv0->o_req_size(wb_mem_req_size);
    xslv0->o_req_write(w_mem_req_write);
    xslv0->o_req_wdata(wb_mem_req_wdata);
    xslv0->o_req_wstrb(wb_mem_req_wstrb);
    xslv0->o_req_last(w_mem_req_last);
    xslv0->i_req_ready(w_cache_req_ready);
    xslv0->i_resp_valid(w_cache_resp_valid);
    xslv0->i_resp_rdata(wb_cache_resp_rdata);
    xslv0->i_resp_err(w_cache_resp_err);

    regs0 = new sdctrl_regs("regs0", async_reset);
    regs0->i_clk(i_clk);
    regs0->i_nrst(i_nrst);
    regs0->i_pmapinfo(i_pmapinfo);
    regs0->o_pcfg(o_pcfg);
    regs0->i_apbi(i_apbi);
    regs0->o_apbo(o_apbo);
    regs0->o_sck(o_sclk);
    regs0->o_sck_posedge(w_regs_sck_posedge);
    regs0->o_sck_negedge(w_regs_sck);
    regs0->o_watchdog(wb_regs_watchdog);
    regs0->o_err_clear(w_regs_err_clear);
    regs0->o_spi_mode(w_regs_spi_mode);
    regs0->o_pcie_12V_support(w_regs_pcie_12V_support);
    regs0->o_pcie_available(w_regs_pcie_available);
    regs0->o_voltage_supply(wb_regs_voltage_supply);
    regs0->o_check_pattern(wb_regs_check_pattern);
    regs0->i_400khz_ena(w_400kHz_ena);
    regs0->i_sdtype(wb_sdtype);
    regs0->i_sd_cmd(i_cmd);
    regs0->i_sd_dat0(i_dat0);
    regs0->i_sd_dat1(i_dat1);
    regs0->i_sd_dat2(i_dat2);
    regs0->i_sd_dat3(i_cd_dat3);
    regs0->i_err_code(wb_err_code);
    regs0->i_cmd_req_valid(w_cmd_req_valid);
    regs0->i_cmd_req_cmd(wb_cmd_req_cmd);
    regs0->i_cmd_resp_valid(w_cmd_resp_valid);
    regs0->i_cmd_resp_cmd(wb_cmd_resp_cmd);
    regs0->i_cmd_resp_reg(wb_cmd_resp_reg);
    regs0->i_cmd_resp_crc7_rx(wb_cmd_resp_crc7_rx);
    regs0->i_cmd_resp_crc7_calc(wb_cmd_resp_crc7_calc);

    wdog0 = new sdctrl_wdog("wdog0", async_reset);
    wdog0->i_clk(i_clk);
    wdog0->i_nrst(i_nrst);
    wdog0->i_ena(w_wdog_ena);
    wdog0->i_period(wb_regs_watchdog);
    wdog0->o_trigger(w_wdog_trigger);

    err0 = new sdctrl_err("err0", async_reset);
    err0->i_clk(i_clk);
    err0->i_nrst(i_nrst);
    err0->i_err_valid(w_err_valid);
    err0->i_err_code(wb_err_setcode);
    err0->i_err_clear(w_err_clear);
    err0->o_err_code(wb_err_code);
    err0->o_err_pending(w_err_pending);

    crcdat0 = new sdctrl_crc16("crcdat0", async_reset);
    crcdat0->i_clk(i_clk);
    crcdat0->i_nrst(i_nrst);
    crcdat0->i_clear(w_crc16_clear);
    crcdat0->i_next(w_crc16_next);
    crcdat0->i_dat(i_dat0);
    crcdat0->o_crc15(wb_crc16_0);

    crcdat1 = new sdctrl_crc16("crcdat1", async_reset);
    crcdat1->i_clk(i_clk);
    crcdat1->i_nrst(i_nrst);
    crcdat1->i_clear(w_crc16_clear);
    crcdat1->i_next(w_crc16_next);
    crcdat1->i_dat(i_dat1);
    crcdat1->o_crc15(wb_crc16_1);

    crcdat2 = new sdctrl_crc16("crcdat2", async_reset);
    crcdat2->i_clk(i_clk);
    crcdat2->i_nrst(i_nrst);
    crcdat2->i_clear(w_crc16_clear);
    crcdat2->i_next(w_crc16_next);
    crcdat2->i_dat(i_dat2);
    crcdat2->o_crc15(wb_crc16_2);

    crcdat3 = new sdctrl_crc16("crcdat3", async_reset);
    crcdat3->i_clk(i_clk);
    crcdat3->i_nrst(i_nrst);
    crcdat3->i_clear(w_crc16_clear);
    crcdat3->i_next(w_crc16_next);
    crcdat3->i_dat(i_cd_dat3);
    crcdat3->o_crc15(wb_crc16_3);

    spimode0 = new sdctrl_spimode("spimode0", async_reset);
    spimode0->i_clk(i_clk);
    spimode0->i_nrst(r.nrst_spimode);
    spimode0->i_posedge(w_regs_sck_posedge);
    spimode0->i_miso(i_dat0);
    spimode0->o_mosi(w_spi_dat);
    spimode0->o_csn(w_spi_dat_csn);
    spimode0->i_detected(i_detected);
    spimode0->i_protect(i_protect);
    spimode0->i_cfg_pcie_12V_support(w_regs_pcie_12V_support);
    spimode0->i_cfg_pcie_available(w_regs_pcie_available);
    spimode0->i_cfg_voltage_supply(wb_regs_voltage_supply);
    spimode0->i_cfg_check_pattern(wb_regs_check_pattern);
    spimode0->i_cmd_req_ready(w_cmd_req_ready);
    spimode0->o_cmd_req_valid(w_spi_cmd_req_valid);
    spimode0->o_cmd_req_cmd(wb_spi_cmd_req_cmd);
    spimode0->o_cmd_req_arg(wb_spi_cmd_req_arg);
    spimode0->o_cmd_req_rn(wb_spi_cmd_req_rn);
    spimode0->i_cmd_resp_valid(w_cmd_resp_valid);
    spimode0->i_cmd_resp_r1r2(wb_cmd_resp_spistatus);
    spimode0->i_cmd_resp_arg32(wb_cmd_resp_reg);
    spimode0->o_data_req_ready(w_spi_req_sdmem_ready);
    spimode0->i_data_req_valid(w_req_sdmem_valid);
    spimode0->i_data_req_write(w_req_sdmem_write);
    spimode0->i_data_req_addr(wb_req_sdmem_addr);
    spimode0->i_data_req_wdata(wb_req_sdmem_wdata);
    spimode0->o_data_resp_valid(w_spi_resp_sdmem_valid);
    spimode0->o_data_resp_rdata(wb_spi_resp_sdmem_data);
    spimode0->i_crc16_0(wb_crc16_0);
    spimode0->o_crc16_clear(w_spi_crc16_clear);
    spimode0->o_crc16_next(w_spi_crc16_next);
    spimode0->o_wdog_ena(w_spi_wdog_ena);
    spimode0->i_wdog_trigger(w_wdog_trigger);
    spimode0->i_err_code(wb_err_code);
    spimode0->o_err_valid(w_spi_err_valid);
    spimode0->o_err_clear(w_spi_err_clear);
    spimode0->o_err_code(wb_spi_err_setcode);
    spimode0->o_400khz_ena(w_spi_400kHz_ena);
    spimode0->o_sdtype(wb_spi_sdtype);

    sdmode0 = new sdctrl_sdmode("sdmode0", async_reset);
    sdmode0->i_clk(i_clk);
    sdmode0->i_nrst(r.nrst_sdmode);
    sdmode0->i_posedge(w_regs_sck_posedge);
    sdmode0->i_dat0(i_dat0);
    sdmode0->o_dat0(w_sd_dat0);
    sdmode0->o_dat0_dir(w_sd_dat0_dir);
    sdmode0->i_dat1(i_dat1);
    sdmode0->o_dat1(w_sd_dat1);
    sdmode0->o_dat1_dir(w_sd_dat1_dir);
    sdmode0->i_dat2(i_dat2);
    sdmode0->o_dat2(w_sd_dat2);
    sdmode0->o_dat2_dir(w_sd_dat2_dir);
    sdmode0->i_cd_dat3(i_cd_dat3);
    sdmode0->o_dat3(w_sd_dat3);
    sdmode0->o_dat3_dir(w_sd_dat3_dir);
    sdmode0->i_detected(i_detected);
    sdmode0->i_protect(i_protect);
    sdmode0->i_cfg_pcie_12V_support(w_regs_pcie_12V_support);
    sdmode0->i_cfg_pcie_available(w_regs_pcie_available);
    sdmode0->i_cfg_voltage_supply(wb_regs_voltage_supply);
    sdmode0->i_cfg_check_pattern(wb_regs_check_pattern);
    sdmode0->i_cmd_req_ready(w_cmd_req_ready);
    sdmode0->o_cmd_req_valid(w_sd_cmd_req_valid);
    sdmode0->o_cmd_req_cmd(wb_sd_cmd_req_cmd);
    sdmode0->o_cmd_req_arg(wb_sd_cmd_req_arg);
    sdmode0->o_cmd_req_rn(wb_sd_cmd_req_rn);
    sdmode0->i_cmd_resp_valid(w_cmd_resp_valid);
    sdmode0->i_cmd_resp_cmd(wb_cmd_resp_cmd);
    sdmode0->i_cmd_resp_arg32(wb_cmd_resp_reg);
    sdmode0->o_data_req_ready(w_sd_req_sdmem_ready);
    sdmode0->i_data_req_valid(w_req_sdmem_valid);
    sdmode0->i_data_req_write(w_req_sdmem_write);
    sdmode0->i_data_req_addr(wb_req_sdmem_addr);
    sdmode0->i_data_req_wdata(wb_req_sdmem_wdata);
    sdmode0->o_data_resp_valid(w_sd_resp_sdmem_valid);
    sdmode0->o_data_resp_rdata(wb_sd_resp_sdmem_data);
    sdmode0->i_crc16_0(wb_crc16_0);
    sdmode0->i_crc16_1(wb_crc16_1);
    sdmode0->i_crc16_2(wb_crc16_2);
    sdmode0->i_crc16_3(wb_crc16_3);
    sdmode0->o_crc16_clear(w_sd_crc16_clear);
    sdmode0->o_crc16_next(w_sd_crc16_next);
    sdmode0->o_wdog_ena(w_sd_wdog_ena);
    sdmode0->i_wdog_trigger(w_wdog_trigger);
    sdmode0->i_err_code(wb_err_code);
    sdmode0->o_err_valid(w_sd_err_valid);
    sdmode0->o_err_clear(w_sd_err_clear);
    sdmode0->o_err_code(wb_sd_err_setcode);
    sdmode0->o_400khz_ena(w_sd_400kHz_ena);
    sdmode0->o_sdtype(wb_sd_sdtype);

    cmdtrx0 = new sdctrl_cmd_transmitter("cmdtrx0", async_reset);
    cmdtrx0->i_clk(i_clk);
    cmdtrx0->i_nrst(i_nrst);
    cmdtrx0->i_sclk_posedge(w_regs_sck_posedge);
    cmdtrx0->i_sclk_negedge(w_regs_sck);
    cmdtrx0->i_cmd(w_cmd_in);
    cmdtrx0->o_cmd(w_trx_cmd);
    cmdtrx0->o_cmd_dir(w_trx_cmd_dir);
    cmdtrx0->o_cmd_cs(w_trx_cmd_csn);
    cmdtrx0->i_spi_mode(w_regs_spi_mode);
    cmdtrx0->i_err_code(wb_err_code);
    cmdtrx0->i_wdog_trigger(w_wdog_trigger);
    cmdtrx0->i_cmd_set_low(r.cmd_set_low);
    cmdtrx0->i_req_valid(w_cmd_req_valid);
    cmdtrx0->i_req_cmd(wb_cmd_req_cmd);
    cmdtrx0->i_req_arg(wb_cmd_req_arg);
    cmdtrx0->i_req_rn(wb_cmd_req_rn);
    cmdtrx0->o_req_ready(w_cmd_req_ready);
    cmdtrx0->o_resp_valid(w_cmd_resp_valid);
    cmdtrx0->o_resp_cmd(wb_cmd_resp_cmd);
    cmdtrx0->o_resp_reg(wb_cmd_resp_reg);
    cmdtrx0->o_resp_crc7_rx(wb_cmd_resp_crc7_rx);
    cmdtrx0->o_resp_crc7_calc(wb_cmd_resp_crc7_calc);
    cmdtrx0->o_resp_spistatus(wb_cmd_resp_spistatus);
    cmdtrx0->i_resp_ready(w_cmd_resp_ready);
    cmdtrx0->o_wdog_ena(w_trx_wdog_ena);
    cmdtrx0->o_err_valid(w_trx_err_valid);
    cmdtrx0->o_err_setcode(wb_trx_err_setcode);

    cache0 = new sdctrl_cache("cache0", async_reset);
    cache0->i_clk(i_clk);
    cache0->i_nrst(i_nrst);
    cache0->i_req_valid(w_mem_req_valid);
    cache0->i_req_write(w_mem_req_write);
    cache0->i_req_addr(wb_mem_req_addr);
    cache0->i_req_wdata(wb_mem_req_wdata);
    cache0->i_req_wstrb(wb_mem_req_wstrb);
    cache0->o_req_ready(w_cache_req_ready);
    cache0->o_resp_valid(w_cache_resp_valid);
    cache0->o_resp_data(wb_cache_resp_rdata);
    cache0->o_resp_err(w_cache_resp_err);
    cache0->i_resp_ready(w_cache_resp_ready);
    cache0->i_req_mem_ready(w_req_sdmem_ready);
    cache0->o_req_mem_valid(w_req_sdmem_valid);
    cache0->o_req_mem_write(w_req_sdmem_write);
    cache0->o_req_mem_addr(wb_req_sdmem_addr);
    cache0->o_req_mem_data(wb_req_sdmem_wdata);
    cache0->i_mem_data_valid(w_resp_sdmem_valid);
    cache0->i_mem_data(wb_resp_sdmem_data);
    cache0->i_mem_fault(w_err_pending);
    cache0->i_flush_valid(w_regs_flush_valid);
    cache0->o_flush_end(w_cache_flush_end);

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_xmapinfo;
    sensitive << i_xslvi;
    sensitive << i_pmapinfo;
    sensitive << i_apbi;
    sensitive << i_cmd;
    sensitive << i_dat0;
    sensitive << i_dat1;
    sensitive << i_dat2;
    sensitive << i_cd_dat3;
    sensitive << i_detected;
    sensitive << i_protect;
    sensitive << w_regs_sck_posedge;
    sensitive << w_regs_sck;
    sensitive << w_regs_err_clear;
    sensitive << wb_regs_watchdog;
    sensitive << w_regs_spi_mode;
    sensitive << w_regs_pcie_12V_support;
    sensitive << w_regs_pcie_available;
    sensitive << wb_regs_voltage_supply;
    sensitive << wb_regs_check_pattern;
    sensitive << w_mem_req_valid;
    sensitive << wb_mem_req_addr;
    sensitive << wb_mem_req_size;
    sensitive << w_mem_req_write;
    sensitive << wb_mem_req_wdata;
    sensitive << wb_mem_req_wstrb;
    sensitive << w_mem_req_last;
    sensitive << w_cache_req_ready;
    sensitive << w_cache_resp_valid;
    sensitive << wb_cache_resp_rdata;
    sensitive << w_cache_resp_err;
    sensitive << w_cache_resp_ready;
    sensitive << w_req_sdmem_valid;
    sensitive << w_req_sdmem_write;
    sensitive << wb_req_sdmem_addr;
    sensitive << wb_req_sdmem_wdata;
    sensitive << w_regs_flush_valid;
    sensitive << w_cache_flush_end;
    sensitive << w_trx_cmd;
    sensitive << w_trx_cmd_dir;
    sensitive << w_trx_cmd_csn;
    sensitive << w_trx_wdog_ena;
    sensitive << w_trx_err_valid;
    sensitive << wb_trx_err_setcode;
    sensitive << w_cmd_in;
    sensitive << w_cmd_req_ready;
    sensitive << w_cmd_resp_valid;
    sensitive << wb_cmd_resp_cmd;
    sensitive << wb_cmd_resp_reg;
    sensitive << wb_cmd_resp_crc7_rx;
    sensitive << wb_cmd_resp_crc7_calc;
    sensitive << wb_cmd_resp_spistatus;
    sensitive << w_cmd_resp_ready;
    sensitive << wb_crc16_0;
    sensitive << wb_crc16_1;
    sensitive << wb_crc16_2;
    sensitive << wb_crc16_3;
    sensitive << w_wdog_trigger;
    sensitive << wb_err_code;
    sensitive << w_err_pending;
    sensitive << w_spi_dat;
    sensitive << w_spi_dat_csn;
    sensitive << w_spi_cmd_req_valid;
    sensitive << wb_spi_cmd_req_cmd;
    sensitive << wb_spi_cmd_req_arg;
    sensitive << wb_spi_cmd_req_rn;
    sensitive << w_spi_req_sdmem_ready;
    sensitive << w_spi_resp_sdmem_valid;
    sensitive << wb_spi_resp_sdmem_data;
    sensitive << w_spi_err_valid;
    sensitive << w_spi_err_clear;
    sensitive << wb_spi_err_setcode;
    sensitive << w_spi_400kHz_ena;
    sensitive << wb_spi_sdtype;
    sensitive << w_spi_wdog_ena;
    sensitive << w_spi_crc16_clear;
    sensitive << w_spi_crc16_next;
    sensitive << w_sd_dat0;
    sensitive << w_sd_dat0_dir;
    sensitive << w_sd_dat1;
    sensitive << w_sd_dat1_dir;
    sensitive << w_sd_dat2;
    sensitive << w_sd_dat2_dir;
    sensitive << w_sd_dat3;
    sensitive << w_sd_dat3_dir;
    sensitive << w_sd_cmd_req_valid;
    sensitive << wb_sd_cmd_req_cmd;
    sensitive << wb_sd_cmd_req_arg;
    sensitive << wb_sd_cmd_req_rn;
    sensitive << w_sd_req_sdmem_ready;
    sensitive << w_sd_resp_sdmem_valid;
    sensitive << wb_sd_resp_sdmem_data;
    sensitive << w_sd_err_valid;
    sensitive << w_sd_err_clear;
    sensitive << wb_sd_err_setcode;
    sensitive << w_sd_400kHz_ena;
    sensitive << wb_sd_sdtype;
    sensitive << w_sd_wdog_ena;
    sensitive << w_sd_crc16_clear;
    sensitive << w_sd_crc16_next;
    sensitive << w_cmd_req_valid;
    sensitive << wb_cmd_req_cmd;
    sensitive << wb_cmd_req_arg;
    sensitive << wb_cmd_req_rn;
    sensitive << w_req_sdmem_ready;
    sensitive << w_resp_sdmem_valid;
    sensitive << wb_resp_sdmem_data;
    sensitive << w_err_valid;
    sensitive << w_err_clear;
    sensitive << wb_err_setcode;
    sensitive << w_400kHz_ena;
    sensitive << wb_sdtype;
    sensitive << w_wdog_ena;
    sensitive << w_crc16_clear;
    sensitive << w_crc16_next;
    sensitive << r.nrst_spimode;
    sensitive << r.nrst_sdmode;
    sensitive << r.clkcnt;
    sensitive << r.cmd_set_low;
    sensitive << r.mode;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
}

sdctrl::~sdctrl() {
    if (xslv0) {
        delete xslv0;
    }
    if (regs0) {
        delete regs0;
    }
    if (err0) {
        delete err0;
    }
    if (wdog0) {
        delete wdog0;
    }
    if (crcdat0) {
        delete crcdat0;
    }
    if (crcdat1) {
        delete crcdat1;
    }
    if (crcdat2) {
        delete crcdat2;
    }
    if (crcdat3) {
        delete crcdat3;
    }
    if (spimode0) {
        delete spimode0;
    }
    if (sdmode0) {
        delete sdmode0;
    }
    if (cmdtrx0) {
        delete cmdtrx0;
    }
    if (cache0) {
        delete cache0;
    }
}

void sdctrl::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, i_xslvi, i_xslvi.name());
        sc_trace(o_vcd, o_xslvo, o_xslvo.name());
        sc_trace(o_vcd, i_apbi, i_apbi.name());
        sc_trace(o_vcd, o_apbo, o_apbo.name());
        sc_trace(o_vcd, o_sclk, o_sclk.name());
        sc_trace(o_vcd, i_cmd, i_cmd.name());
        sc_trace(o_vcd, o_cmd, o_cmd.name());
        sc_trace(o_vcd, o_cmd_dir, o_cmd_dir.name());
        sc_trace(o_vcd, i_dat0, i_dat0.name());
        sc_trace(o_vcd, o_dat0, o_dat0.name());
        sc_trace(o_vcd, o_dat0_dir, o_dat0_dir.name());
        sc_trace(o_vcd, i_dat1, i_dat1.name());
        sc_trace(o_vcd, o_dat1, o_dat1.name());
        sc_trace(o_vcd, o_dat1_dir, o_dat1_dir.name());
        sc_trace(o_vcd, i_dat2, i_dat2.name());
        sc_trace(o_vcd, o_dat2, o_dat2.name());
        sc_trace(o_vcd, o_dat2_dir, o_dat2_dir.name());
        sc_trace(o_vcd, i_cd_dat3, i_cd_dat3.name());
        sc_trace(o_vcd, o_cd_dat3, o_cd_dat3.name());
        sc_trace(o_vcd, o_cd_dat3_dir, o_cd_dat3_dir.name());
        sc_trace(o_vcd, i_detected, i_detected.name());
        sc_trace(o_vcd, i_protect, i_protect.name());
        sc_trace(o_vcd, r.nrst_spimode, pn + ".r_nrst_spimode");
        sc_trace(o_vcd, r.nrst_sdmode, pn + ".r_nrst_sdmode");
        sc_trace(o_vcd, r.clkcnt, pn + ".r_clkcnt");
        sc_trace(o_vcd, r.cmd_set_low, pn + ".r_cmd_set_low");
        sc_trace(o_vcd, r.mode, pn + ".r_mode");
    }

    if (xslv0) {
        xslv0->generateVCD(i_vcd, o_vcd);
    }
    if (regs0) {
        regs0->generateVCD(i_vcd, o_vcd);
    }
    if (err0) {
        err0->generateVCD(i_vcd, o_vcd);
    }
    if (wdog0) {
        wdog0->generateVCD(i_vcd, o_vcd);
    }
    if (crcdat0) {
        crcdat0->generateVCD(i_vcd, o_vcd);
    }
    if (crcdat1) {
        crcdat1->generateVCD(i_vcd, o_vcd);
    }
    if (crcdat2) {
        crcdat2->generateVCD(i_vcd, o_vcd);
    }
    if (crcdat3) {
        crcdat3->generateVCD(i_vcd, o_vcd);
    }
    if (spimode0) {
        spimode0->generateVCD(i_vcd, o_vcd);
    }
    if (sdmode0) {
        sdmode0->generateVCD(i_vcd, o_vcd);
    }
    if (cmdtrx0) {
        cmdtrx0->generateVCD(i_vcd, o_vcd);
    }
    if (cache0) {
        cache0->generateVCD(i_vcd, o_vcd);
    }
}

void sdctrl::comb() {
    bool v_cmd_dir;
    bool v_cmd_in;
    bool v_cmd_out;
    bool v_dat0_dir;
    bool v_dat0_out;
    bool v_dat1_dir;
    bool v_dat1_out;
    bool v_dat2_dir;
    bool v_dat2_out;
    bool v_dat3_dir;
    bool v_dat3_out;
    bool v_cmd_req_valid;
    sc_uint<6> vb_cmd_req_cmd;
    sc_uint<32> vb_cmd_req_arg;
    sc_uint<3> vb_cmd_req_rn;
    bool v_req_sdmem_ready;
    bool v_resp_sdmem_valid;
    sc_biguint<512> vb_resp_sdmem_data;
    bool v_err_valid;
    bool v_err_clear;
    sc_uint<4> vb_err_setcode;
    bool v_400kHz_ena;
    sc_uint<3> vb_sdtype;
    bool v_wdog_ena;
    bool v_crc16_clear;
    bool v_crc16_next;

    v_cmd_dir = DIR_OUTPUT;
    v_cmd_in = 0;
    v_cmd_out = 1;
    v_dat0_dir = DIR_OUTPUT;
    v_dat0_out = 1;
    v_dat1_dir = DIR_OUTPUT;
    v_dat1_out = 1;
    v_dat2_dir = DIR_OUTPUT;
    v_dat2_out = 1;
    v_dat3_dir = DIR_OUTPUT;
    v_dat3_out = 1;
    v_cmd_req_valid = 0;
    vb_cmd_req_cmd = 0;
    vb_cmd_req_arg = 0;
    vb_cmd_req_rn = 0;
    v_req_sdmem_ready = 0;
    v_resp_sdmem_valid = 0;
    vb_resp_sdmem_data = 0;
    v_err_valid = 0;
    v_err_clear = 0;
    vb_err_setcode = 0;
    v_400kHz_ena = 1;
    vb_sdtype = 0;
    v_wdog_ena = 0;
    v_crc16_clear = 0;
    v_crc16_next = 0;

    v = r;


    if (r.mode.read() == MODE_PRE_INIT) {
        // Page 222, Fig.4-96 State Diagram (Pre-Init mode)
        // 1. No commands were sent to the card after POW (except CMD0):
        //     CMD line held High for at least 1 ms (by SW), then SDCLK supplied
        //     at least 74 clocks with keeping CMD line High
        // 2. CMD High to Low transition && CMD=Low < 74 clocks then go idle,
        //     if Low >= 74 clocks then Fast boot in CV-mode
        if (w_regs_sck_posedge.read() == 1) {
            v.clkcnt = (r.clkcnt.read() + 1);
        }
        if (r.clkcnt.read() >= 73) {
            if (w_regs_spi_mode.read() == 1) {
                v.mode = MODE_SPI;
                v.nrst_spimode = 1;
            } else {
                v.mode = MODE_SD;
                v.nrst_sdmode = 1;
            }
        }
    } else if (r.mode.read() == MODE_SPI) {
        // SPI MOSI:
        v_cmd_dir = DIR_OUTPUT;
        v_cmd_out = (!(((!w_trx_cmd.read()) && (!w_trx_cmd_csn.read()))
                || ((!w_spi_dat.read()) && (!w_spi_dat_csn.read()))));
        // SPI MISO:
        v_dat0_dir = DIR_INPUT;
        v_cmd_in = i_dat0;
        // SPI CSn:
        v_dat3_dir = DIR_OUTPUT;
        v_dat3_out = (w_trx_cmd_csn.read() && w_spi_dat_csn.read());
        // Unused in SPI mode:
        v_dat2_dir = DIR_OUTPUT;
        v_dat2_out = 1;
        v_dat1_dir = DIR_OUTPUT;
        v_dat1_out = 1;

        v_cmd_req_valid = w_spi_cmd_req_valid;
        vb_cmd_req_cmd = wb_spi_cmd_req_cmd;
        vb_cmd_req_arg = wb_spi_cmd_req_arg;
        vb_cmd_req_rn = wb_spi_cmd_req_rn;
        v_req_sdmem_ready = w_spi_req_sdmem_ready;
        v_resp_sdmem_valid = w_spi_resp_sdmem_valid;
        vb_resp_sdmem_data = wb_spi_resp_sdmem_data;
        v_err_valid = w_spi_err_valid;
        v_err_clear = (w_regs_err_clear.read() || w_spi_err_clear.read());
        vb_err_setcode = wb_spi_err_setcode;
        v_400kHz_ena = w_spi_400kHz_ena;
        vb_sdtype = wb_spi_sdtype;
        v_wdog_ena = (w_spi_wdog_ena.read() || w_trx_wdog_ena.read());
        v_crc16_clear = w_spi_crc16_clear;
        v_crc16_next = w_spi_crc16_next;
    } else {
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
        v_err_clear = (w_regs_err_clear.read() || w_sd_err_clear.read());
        vb_err_setcode = wb_sd_err_setcode;
        v_400kHz_ena = w_sd_400kHz_ena;
        vb_sdtype = wb_sd_sdtype;
        v_wdog_ena = (w_sd_wdog_ena.read() || w_trx_wdog_ena.read());
        v_crc16_clear = w_sd_crc16_clear;
        v_crc16_next = w_sd_crc16_next;
    }

    if (!async_reset_ && i_nrst.read() == 0) {
        sdctrl_r_reset(v);
    }

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
    w_cmd_resp_ready = 1;
    w_cache_resp_ready = 1;
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
}

void sdctrl::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        sdctrl_r_reset(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

