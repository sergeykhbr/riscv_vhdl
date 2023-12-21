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

#include "sdctrl_spimode.h"
#include "api_core.h"

namespace debugger {

sdctrl_spimode::sdctrl_spimode(sc_module_name name,
                               bool async_reset)
    : sc_module(name),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_posedge("i_posedge"),
    i_miso("i_miso"),
    o_mosi("o_mosi"),
    o_csn("o_csn"),
    i_detected("i_detected"),
    i_protect("i_protect"),
    i_cfg_pcie_12V_support("i_cfg_pcie_12V_support"),
    i_cfg_pcie_available("i_cfg_pcie_available"),
    i_cfg_voltage_supply("i_cfg_voltage_supply"),
    i_cfg_check_pattern("i_cfg_check_pattern"),
    i_cmd_req_ready("i_cmd_req_ready"),
    o_cmd_req_valid("o_cmd_req_valid"),
    o_cmd_req_cmd("o_cmd_req_cmd"),
    o_cmd_req_arg("o_cmd_req_arg"),
    o_cmd_req_rn("o_cmd_req_rn"),
    i_cmd_resp_valid("i_cmd_resp_valid"),
    i_cmd_resp_r1r2("i_cmd_resp_r1r2"),
    i_cmd_resp_arg32("i_cmd_resp_arg32"),
    o_data_req_ready("o_data_req_ready"),
    i_data_req_valid("i_data_req_valid"),
    i_data_req_write("i_data_req_write"),
    i_data_req_addr("i_data_req_addr"),
    i_data_req_wdata("i_data_req_wdata"),
    o_data_resp_valid("o_data_resp_valid"),
    o_data_resp_rdata("o_data_resp_rdata"),
    i_crc16_0("i_crc16_0"),
    o_crc16_clear("o_crc16_clear"),
    o_crc16_next("o_crc16_next"),
    o_wdog_ena("o_wdog_ena"),
    i_wdog_trigger("i_wdog_trigger"),
    i_err_code("i_err_code"),
    o_err_valid("o_err_valid"),
    o_err_clear("o_err_clear"),
    o_err_code("o_err_code"),
    o_400khz_ena("o_400khz_ena"),
    o_sdtype("o_sdtype") {

    async_reset_ = async_reset;

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_posedge;
    sensitive << i_miso;
    sensitive << i_detected;
    sensitive << i_protect;
    sensitive << i_cfg_pcie_12V_support;
    sensitive << i_cfg_pcie_available;
    sensitive << i_cfg_voltage_supply;
    sensitive << i_cfg_check_pattern;
    sensitive << i_cmd_req_ready;
    sensitive << i_cmd_resp_valid;
    sensitive << i_cmd_resp_r1r2;
    sensitive << i_cmd_resp_arg32;
    sensitive << i_data_req_valid;
    sensitive << i_data_req_write;
    sensitive << i_data_req_addr;
    sensitive << i_data_req_wdata;
    sensitive << i_crc16_0;
    sensitive << i_wdog_trigger;
    sensitive << i_err_code;
    sensitive << r.clkcnt;
    sensitive << r.cmd_req_valid;
    sensitive << r.cmd_req_cmd;
    sensitive << r.cmd_req_arg;
    sensitive << r.cmd_req_rn;
    sensitive << r.cmd_resp_cmd;
    sensitive << r.cmd_resp_arg32;
    sensitive << r.cmd_resp_r1;
    sensitive << r.cmd_resp_r2;
    sensitive << r.data_addr;
    sensitive << r.data_data;
    sensitive << r.data_resp_valid;
    sensitive << r.wdog_ena;
    sensitive << r.crc16_clear;
    sensitive << r.crc16_calc0;
    sensitive << r.crc16_rx0;
    sensitive << r.dat_csn;
    sensitive << r.dat_reading;
    sensitive << r.err_clear;
    sensitive << r.err_valid;
    sensitive << r.err_code;
    sensitive << r.sck_400khz_ena;
    sensitive << r.state;
    sensitive << r.wait_cmd_resp;
    sensitive << r.sdtype;
    sensitive << r.HCS;
    sensitive << r.S18;
    sensitive << r.OCR_VoltageWindow;
    sensitive << r.bitcnt;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
}

void sdctrl_spimode::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, i_posedge, i_posedge.name());
        sc_trace(o_vcd, i_miso, i_miso.name());
        sc_trace(o_vcd, o_mosi, o_mosi.name());
        sc_trace(o_vcd, o_csn, o_csn.name());
        sc_trace(o_vcd, i_detected, i_detected.name());
        sc_trace(o_vcd, i_protect, i_protect.name());
        sc_trace(o_vcd, i_cfg_pcie_12V_support, i_cfg_pcie_12V_support.name());
        sc_trace(o_vcd, i_cfg_pcie_available, i_cfg_pcie_available.name());
        sc_trace(o_vcd, i_cfg_voltage_supply, i_cfg_voltage_supply.name());
        sc_trace(o_vcd, i_cfg_check_pattern, i_cfg_check_pattern.name());
        sc_trace(o_vcd, i_cmd_req_ready, i_cmd_req_ready.name());
        sc_trace(o_vcd, o_cmd_req_valid, o_cmd_req_valid.name());
        sc_trace(o_vcd, o_cmd_req_cmd, o_cmd_req_cmd.name());
        sc_trace(o_vcd, o_cmd_req_arg, o_cmd_req_arg.name());
        sc_trace(o_vcd, o_cmd_req_rn, o_cmd_req_rn.name());
        sc_trace(o_vcd, i_cmd_resp_valid, i_cmd_resp_valid.name());
        sc_trace(o_vcd, i_cmd_resp_r1r2, i_cmd_resp_r1r2.name());
        sc_trace(o_vcd, i_cmd_resp_arg32, i_cmd_resp_arg32.name());
        sc_trace(o_vcd, o_data_req_ready, o_data_req_ready.name());
        sc_trace(o_vcd, i_data_req_valid, i_data_req_valid.name());
        sc_trace(o_vcd, i_data_req_write, i_data_req_write.name());
        sc_trace(o_vcd, i_data_req_addr, i_data_req_addr.name());
        sc_trace(o_vcd, i_data_req_wdata, i_data_req_wdata.name());
        sc_trace(o_vcd, o_data_resp_valid, o_data_resp_valid.name());
        sc_trace(o_vcd, o_data_resp_rdata, o_data_resp_rdata.name());
        sc_trace(o_vcd, i_crc16_0, i_crc16_0.name());
        sc_trace(o_vcd, o_crc16_clear, o_crc16_clear.name());
        sc_trace(o_vcd, o_crc16_next, o_crc16_next.name());
        sc_trace(o_vcd, o_wdog_ena, o_wdog_ena.name());
        sc_trace(o_vcd, i_wdog_trigger, i_wdog_trigger.name());
        sc_trace(o_vcd, i_err_code, i_err_code.name());
        sc_trace(o_vcd, o_err_valid, o_err_valid.name());
        sc_trace(o_vcd, o_err_clear, o_err_clear.name());
        sc_trace(o_vcd, o_err_code, o_err_code.name());
        sc_trace(o_vcd, o_400khz_ena, o_400khz_ena.name());
        sc_trace(o_vcd, o_sdtype, o_sdtype.name());
        sc_trace(o_vcd, r.clkcnt, pn + ".r_clkcnt");
        sc_trace(o_vcd, r.cmd_req_valid, pn + ".r_cmd_req_valid");
        sc_trace(o_vcd, r.cmd_req_cmd, pn + ".r_cmd_req_cmd");
        sc_trace(o_vcd, r.cmd_req_arg, pn + ".r_cmd_req_arg");
        sc_trace(o_vcd, r.cmd_req_rn, pn + ".r_cmd_req_rn");
        sc_trace(o_vcd, r.cmd_resp_cmd, pn + ".r_cmd_resp_cmd");
        sc_trace(o_vcd, r.cmd_resp_arg32, pn + ".r_cmd_resp_arg32");
        sc_trace(o_vcd, r.cmd_resp_r1, pn + ".r_cmd_resp_r1");
        sc_trace(o_vcd, r.cmd_resp_r2, pn + ".r_cmd_resp_r2");
        sc_trace(o_vcd, r.data_addr, pn + ".r_data_addr");
        sc_trace(o_vcd, r.data_data, pn + ".r_data_data");
        sc_trace(o_vcd, r.data_resp_valid, pn + ".r_data_resp_valid");
        sc_trace(o_vcd, r.wdog_ena, pn + ".r_wdog_ena");
        sc_trace(o_vcd, r.crc16_clear, pn + ".r_crc16_clear");
        sc_trace(o_vcd, r.crc16_calc0, pn + ".r_crc16_calc0");
        sc_trace(o_vcd, r.crc16_rx0, pn + ".r_crc16_rx0");
        sc_trace(o_vcd, r.dat_csn, pn + ".r_dat_csn");
        sc_trace(o_vcd, r.dat_reading, pn + ".r_dat_reading");
        sc_trace(o_vcd, r.err_clear, pn + ".r_err_clear");
        sc_trace(o_vcd, r.err_valid, pn + ".r_err_valid");
        sc_trace(o_vcd, r.err_code, pn + ".r_err_code");
        sc_trace(o_vcd, r.sck_400khz_ena, pn + ".r_sck_400khz_ena");
        sc_trace(o_vcd, r.state, pn + ".r_state");
        sc_trace(o_vcd, r.wait_cmd_resp, pn + ".r_wait_cmd_resp");
        sc_trace(o_vcd, r.sdtype, pn + ".r_sdtype");
        sc_trace(o_vcd, r.HCS, pn + ".r_HCS");
        sc_trace(o_vcd, r.S18, pn + ".r_S18");
        sc_trace(o_vcd, r.OCR_VoltageWindow, pn + ".r_OCR_VoltageWindow");
        sc_trace(o_vcd, r.bitcnt, pn + ".r_bitcnt");
    }

}

void sdctrl_spimode::comb() {
    bool v_dat;
    sc_uint<32> vb_cmd_req_arg;
    bool v_data_req_ready;
    bool v_crc16_next;

    v_dat = 0;
    vb_cmd_req_arg = 0;
    v_data_req_ready = 0;
    v_crc16_next = 0;

    v = r;

    v.err_clear = 0;
    v.err_valid = 0;
    v.err_code = 0;
    v.data_resp_valid = 0;
    vb_cmd_req_arg = r.cmd_req_arg;

    if (i_posedge) {
        // Not a full block 4096 bits just a cache line (dat_csn is active LOW):
        v.data_data = (r.data_data.read()(510, 0), (i_miso.read() || r.dat_csn.read()));
        v.bitcnt = (r.bitcnt.read() + 1);
    }

    if (r.cmd_req_valid.read() == 1) {
        if (i_cmd_req_ready.read() == 1) {
            v.cmd_req_valid = 0;
            v.wait_cmd_resp = 1;
        }
    } else if ((i_cmd_resp_valid.read() == 1) && (r.wait_cmd_resp.read() == 1)) {
        // Parse Rx response:
        v.wait_cmd_resp = 0;
        v.cmd_resp_r1 = i_cmd_resp_r1r2.read()(14, 8);
        v.cmd_resp_arg32 = i_cmd_resp_arg32;
        switch (r.cmd_req_rn.read()) {
        case R2:
            v.cmd_resp_r2 = i_cmd_resp_r1r2.read()(7, 0);
            break;
        case R3:
            // Table 5-1: OCR Register definition, page 246
            //     [23:0]  Voltage window can be requested by CMD58
            //     [24]    Switching to 1.8V accepted (S18A)
            //     [27]    Over 2TB support status (CO2T)
            //     [29]    UHS-II Card status
            //     [30]    Card Capacity Status (CCS)
            //     [31]    Card power-up status (busy is LOW if the card not finished the power-up routine)
            if (i_cmd_resp_arg32.read()[31] == 1) {
                v.OCR_VoltageWindow = i_cmd_resp_arg32.read()(23, 0);
                v.HCS = i_cmd_resp_arg32.read()[30];
                if (i_cmd_resp_arg32.read()[30] == 1) {
                    v.sdtype = SDCARD_VER2X_HC;
                } else if (r.sdtype.read() == SDCARD_UNKNOWN) {
                    v.sdtype = SDCARD_VER2X_SC;
                }
                v.S18 = i_cmd_resp_arg32.read()[24];
            }
            break;
        case R6:
            // Relative Address (RCA):
            v.data_addr = (i_cmd_resp_arg32.read()(31, 16) << 16);
            break;
        default:
            break;
        }
    } else if (r.wait_cmd_resp.read() == 1) {
        // do nothing
    } else if (r.state.read() == STATE_CMD0) {
        v.sdtype = SDCARD_UNKNOWN;
        v.HCS = 1;
        v.S18 = 0;
        v.data_addr = 0;
        v.OCR_VoltageWindow = 0x0000000000FF8000;
        v.cmd_req_valid = 1;
        v.cmd_req_cmd = CMD0;
        v.cmd_req_rn = R1;
        vb_cmd_req_arg = 0;
        v.state = STATE_CMD8;
    } else if (r.state.read() == STATE_CMD8) {
        // See page 113. 4.3.13 Send Interface Condition Command
        //   [39:22] reserved 00000h
        //   [21]    PCIe 1.2V support 0
        //   [20]    PCIe availability 0
        //   [19:16] Voltage Supply (VHS) 0001b: 2.7-3.6V
        //   [15:8]  Check Pattern 55h
        if (i_err_code.read().or_reduce() == 0) {
            v.cmd_req_valid = 1;
        } else {
            v.state = STATE_CMD0;
            v.err_clear = 1;
        }
        v.cmd_req_cmd = CMD8;
        v.cmd_req_rn = R7;
        vb_cmd_req_arg = 0;
        vb_cmd_req_arg[13] = i_cfg_pcie_12V_support;
        vb_cmd_req_arg[12] = i_cfg_pcie_available;
        vb_cmd_req_arg(11, 8) = i_cfg_voltage_supply;
        vb_cmd_req_arg(7, 0) = i_cfg_check_pattern;
        v.state = STATE_CMD55;
    } else if (r.state.read() == STATE_CMD55) {
        // Page 64: APP_CMD (CMD55) shall always precede ACMD41.
        //   [31:16] RCA (Relative Adrress should be set 0)
        //   [15:0] stuff bits
        if (i_err_code.read() == CMDERR_NO_RESPONSE) {
            v.sdtype = SDCARD_VER1X;
            v.HCS = 0;                                      // Standard Capacity only
            v.err_clear = 1;
        }
        v.cmd_req_valid = 1;
        v.cmd_req_cmd = CMD55;
        v.cmd_req_rn = R1;
        vb_cmd_req_arg = 0;
        v.state = STATE_ACMD41;
    } else if (r.state.read() == STATE_ACMD41) {
        // Page 131: SD_SEND_OP_COND. 
        //   [31] reserved bit
        //   [30] HCS (high capacity support)
        //   [29] reserved for eSD
        //   [28] XPC (maximum power in default speed)
        //   [27:25] reserved bits
        //   [24] S18R Send request to switch to 1.8V
        //   [23:0] VDD voltage window (OCR[23:0])
        v.cmd_req_valid = 1;
        v.cmd_req_cmd = ACMD41;
        vb_cmd_req_arg = 0;
        vb_cmd_req_arg[30] = r.HCS;
        vb_cmd_req_arg(23, 0) = r.OCR_VoltageWindow;
        v.cmd_req_rn = R1;
        v.state = STATE_CMD58;
    } else if (r.state.read() == STATE_CMD58) {
        // READ_OCR: Reads OCR register. Used in SPI mode only.
        //   [31] reserved bit
        //   [30] HCS (high capacity support)
        //   [29:0] reserved
        if (r.cmd_resp_r1.read() != 0x01) {
            // SD card not in idle state
            v.state = STATE_CMD55;
        } else {
            v.cmd_req_valid = 1;
            v.cmd_req_cmd = CMD58;
            vb_cmd_req_arg = 0;
            v.cmd_req_rn = R3;
            v.state = STATE_WAIT_DATA_REQ;
            v.sck_400khz_ena = 0;
        }
    } else if (r.state.read() == STATE_WAIT_DATA_REQ) {
        v.wdog_ena = 0;
        v_data_req_ready = 1;
        if (i_data_req_valid.read() == 1) {
            v.data_addr = i_data_req_addr.read()((CFG_SDCACHE_ADDR_BITS - 1), 9);
            v.data_data = i_data_req_wdata;
            if (i_data_req_write.read() == 1) {
                v.state = STATE_CMD24_WRITE_SINGLE_BLOCK;
            } else {
                v.state = STATE_CMD17_READ_SINGLE_BLOCK;
            }
        }
    } else if (r.state.read() == STATE_CMD17_READ_SINGLE_BLOCK) {
        // CMD17: READ_SINGLE_BLOCK. Reads a block of the size SET_BLOCKLEN
        //   [31:0] data address
        v.cmd_req_valid = 1;
        v.cmd_req_cmd = CMD17;
        v.cmd_req_rn = R1;
        vb_cmd_req_arg = r.data_addr;
        v.state = STATE_WAIT_DATA_START;
        v.bitcnt = 0;
    } else if (r.state.read() == STATE_CMD24_WRITE_SINGLE_BLOCK) {
        // TODO:
    } else if (r.state.read() == STATE_WAIT_DATA_START) {
        v.dat_csn = 0;
        v.dat_reading = 1;
        v.crc16_clear = 1;
        v.wdog_ena = 1;
        if (i_err_code.read() != CMDERR_NONE) {
            v.state = STATE_WAIT_DATA_REQ;
        } else if (r.data_data.read()(7, 0) == 0xFE) {
            v.state = STATE_READING_DATA;
            v.bitcnt = 0;
            v.crc16_clear = 0;
        } else if (i_wdog_trigger.read() == 1) {
            v.state = STATE_WAIT_DATA_REQ;
            v.err_valid = 1;
            v.err_code = CMDERR_NO_RESPONSE;
        }
    } else if (r.state.read() == STATE_READING_DATA) {
        if (i_posedge.read() == 1) {
            v_crc16_next = 1;
            if (r.bitcnt.read().and_reduce() == 1) {
                v.state = STATE_READING_CRC15;
                v.crc16_calc0 = i_crc16_0;
            }
            if (r.bitcnt.read()(8, 0).and_reduce() == 1) {
                v.data_resp_valid = 1;
            }
        }
    } else if (r.state.read() == STATE_READING_CRC15) {
        if (i_posedge.read() == 1) {
            if (r.bitcnt.read()(3, 0).and_reduce() == 1) {
                v.state = STATE_READING_END;
                v.dat_csn = 1;
                v.dat_reading = 0;
            }
        }
    } else if (r.state.read() == STATE_READING_END) {
        v.crc16_rx0 = r.data_data.read()(15, 0).to_uint();
        v.state = STATE_WAIT_DATA_REQ;
    }

    v.cmd_req_arg = vb_cmd_req_arg;
    v_dat = (r.dat_reading.read() || r.data_data.read()[511]);

    if (!async_reset_ && i_nrst.read() == 0) {
        sdctrl_spimode_r_reset(v);
    }

    o_mosi = v_dat;
    o_cmd_req_valid = r.cmd_req_valid;
    o_cmd_req_cmd = r.cmd_req_cmd;
    o_cmd_req_arg = r.cmd_req_arg;
    o_cmd_req_rn = r.cmd_req_rn;
    o_data_req_ready = v_data_req_ready;
    o_data_resp_valid = r.data_resp_valid;
    o_data_resp_rdata = r.data_data;
    o_csn = r.dat_csn;
    o_crc16_clear = r.crc16_clear;
    o_crc16_next = v_crc16_next;
    o_wdog_ena = r.wdog_ena;
    o_err_valid = r.err_valid;
    o_err_clear = r.err_clear;
    o_err_code = r.err_code;
    o_400khz_ena = r.sck_400khz_ena;
    o_sdtype = r.sdtype;
}

void sdctrl_spimode::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        sdctrl_spimode_r_reset(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

