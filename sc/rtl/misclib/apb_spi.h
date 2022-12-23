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
#pragma once

#include <systemc.h>
#include "../ambalib/types_amba.h"
#include "../ambalib/apb_slv.h"
#include "api_core.h"

namespace debugger {

template<int log2_fifosz = 9>
SC_MODULE(apb_spi) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<bool> i_nrst;                                     // Reset: active LOW
    sc_in<mapinfo_type> i_mapinfo;                          // interconnect slot information
    sc_out<dev_config_type> o_cfg;                          // Device descriptor
    sc_in<apb_in_type> i_apbi;                              // APB  Slave to Bridge interface
    sc_out<apb_out_type> o_apbo;                            // APB Bridge to Slave interface
    sc_out<bool> o_cs;
    sc_out<bool> o_sclk;
    sc_out<bool> o_miso;
    sc_in<bool> i_mosi;
    sc_in<bool> i_detected;
    sc_in<bool> i_protect;

    void comb();
    void registers();

    SC_HAS_PROCESS(apb_spi);

    apb_spi(sc_module_name name,
            bool async_reset);
    virtual ~apb_spi();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;

    static const int fifosz = (1 << log2_fifosz);
    // SPI states
    static const uint8_t idle = 0;
    static const uint8_t send_data = 1;
    static const uint8_t receive_data = 2;

    struct apb_spi_registers {
        sc_signal<sc_uint<32>> scaler;
        sc_signal<sc_uint<32>> scaler_cnt;
        sc_signal<bool> level;
        sc_signal<bool> cs;
        sc_signal<sc_uint<8>> rx_fifo[fifosz];
        sc_signal<sc_uint<8>> tx_fifo[fifosz];
        sc_signal<sc_uint<2>> state;
        sc_signal<sc_uint<log2_fifosz>> wr_cnt;
        sc_signal<sc_uint<log2_fifosz>> rd_cnt;
        sc_signal<sc_uint<3>> bit_cnt;
        sc_signal<sc_uint<8>> tx_shift;
        sc_signal<sc_uint<8>> rx_shift;
        sc_signal<sc_uint<8>> rx_shift;
        sc_signal<bool> resp_valid;
        sc_signal<sc_uint<32>> resp_rdata;
        sc_signal<bool> resp_err;
    } v, r;

    sc_signal<bool> w_req_valid;
    sc_signal<sc_uint<32>> wb_req_addr;
    sc_signal<bool> w_req_write;
    sc_signal<sc_uint<32>> wb_req_wdata;

    apb_slv *pslv0;

};

template<int log2_fifosz>
apb_spi<log2_fifosz>::apb_spi(sc_module_name name,
                              bool async_reset)
    : sc_module(name),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_mapinfo("i_mapinfo"),
    o_cfg("o_cfg"),
    i_apbi("i_apbi"),
    o_apbo("o_apbo"),
    o_cs("o_cs"),
    o_sclk("o_sclk"),
    o_miso("o_miso"),
    i_mosi("i_mosi"),
    i_detected("i_detected"),
    i_protect("i_protect") {

    async_reset_ = async_reset;
    pslv0 = 0;

    pslv0 = new apb_slv("pslv0", async_reset, VENDOR_OPTIMITECH, OPTIMITECH_UART);
    pslv0->i_clk(i_clk);
    pslv0->i_nrst(i_nrst);
    pslv0->i_mapinfo(i_mapinfo);
    pslv0->o_cfg(o_cfg);
    pslv0->i_apbi(i_apbi);
    pslv0->o_apbo(o_apbo);
    pslv0->o_req_valid(w_req_valid);
    pslv0->o_req_addr(wb_req_addr);
    pslv0->o_req_write(w_req_write);
    pslv0->o_req_wdata(wb_req_wdata);
    pslv0->i_resp_valid(r.resp_valid);
    pslv0->i_resp_rdata(r.resp_rdata);
    pslv0->i_resp_err(r.resp_err);



    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_mapinfo;
    sensitive << i_apbi;
    sensitive << i_mosi;
    sensitive << i_detected;
    sensitive << i_protect;
    sensitive << w_req_valid;
    sensitive << wb_req_addr;
    sensitive << w_req_write;
    sensitive << wb_req_wdata;
    sensitive << r.scaler;
    sensitive << r.scaler_cnt;
    sensitive << r.level;
    sensitive << r.cs;
    for (int i = 0; i < fifosz; i++) {
        sensitive << r.rx_fifo[i];
    }
    for (int i = 0; i < fifosz; i++) {
        sensitive << r.tx_fifo[i];
    }
    sensitive << r.state;
    sensitive << r.wr_cnt;
    sensitive << r.rd_cnt;
    sensitive << r.bit_cnt;
    sensitive << r.tx_shift;
    sensitive << r.rx_shift;
    sensitive << r.rx_shift;
    sensitive << r.resp_valid;
    sensitive << r.resp_rdata;
    sensitive << r.resp_err;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
}

template<int log2_fifosz>
apb_spi<log2_fifosz>::~apb_spi() {
    if (pslv0) {
        delete pslv0;
    }
}

template<int log2_fifosz>
void apb_spi<log2_fifosz>::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, i_mapinfo, i_mapinfo.name());
        sc_trace(o_vcd, o_cfg, o_cfg.name());
        sc_trace(o_vcd, i_apbi, i_apbi.name());
        sc_trace(o_vcd, o_apbo, o_apbo.name());
        sc_trace(o_vcd, o_cs, o_cs.name());
        sc_trace(o_vcd, o_sclk, o_sclk.name());
        sc_trace(o_vcd, o_miso, o_miso.name());
        sc_trace(o_vcd, i_mosi, i_mosi.name());
        sc_trace(o_vcd, i_detected, i_detected.name());
        sc_trace(o_vcd, i_protect, i_protect.name());
        sc_trace(o_vcd, r.scaler, pn + ".r_scaler");
        sc_trace(o_vcd, r.scaler_cnt, pn + ".r_scaler_cnt");
        sc_trace(o_vcd, r.level, pn + ".r_level");
        sc_trace(o_vcd, r.cs, pn + ".r_cs");
        for (int i = 0; i < fifosz; i++) {
            char tstr[1024];
            RISCV_sprintf(tstr, sizeof(tstr), "%s.r_rx_fifo%d", pn.c_str(), i);
            sc_trace(o_vcd, r.rx_fifo[i], tstr);
        }
        for (int i = 0; i < fifosz; i++) {
            char tstr[1024];
            RISCV_sprintf(tstr, sizeof(tstr), "%s.r_tx_fifo%d", pn.c_str(), i);
            sc_trace(o_vcd, r.tx_fifo[i], tstr);
        }
        sc_trace(o_vcd, r.state, pn + ".r_state");
        sc_trace(o_vcd, r.wr_cnt, pn + ".r_wr_cnt");
        sc_trace(o_vcd, r.rd_cnt, pn + ".r_rd_cnt");
        sc_trace(o_vcd, r.bit_cnt, pn + ".r_bit_cnt");
        sc_trace(o_vcd, r.tx_shift, pn + ".r_tx_shift");
        sc_trace(o_vcd, r.rx_shift, pn + ".r_rx_shift");
        sc_trace(o_vcd, r.rx_shift, pn + ".r_rx_shift");
        sc_trace(o_vcd, r.resp_valid, pn + ".r_resp_valid");
        sc_trace(o_vcd, r.resp_rdata, pn + ".r_resp_rdata");
        sc_trace(o_vcd, r.resp_err, pn + ".r_resp_err");
    }

    if (pslv0) {
        pslv0->generateVCD(i_vcd, o_vcd);
    }
}

template<int log2_fifosz>
void apb_spi<log2_fifosz>::comb() {
    sc_uint<32> vb_rdata;
    sc_uint<log2_fifosz> vb_tx_wr_cnt_next;
    bool v_tx_fifo_full;
    bool v_tx_fifo_empty;
    sc_uint<8> vb_tx_fifo_rdata;
    bool v_tx_fifo_we;
    sc_uint<log2_fifosz> vb_rx_wr_cnt_next;
    bool v_rx_fifo_full;
    bool v_rx_fifo_empty;
    sc_uint<8> vb_rx_fifo_rdata;
    bool v_rx_fifo_we;
    bool v_rx_fifo_re;
    bool v_negedge_flag;
    bool v_posedge_flag;
    bool par;

    vb_rdata = 0;
    vb_tx_wr_cnt_next = 0;
    v_tx_fifo_full = 0;
    v_tx_fifo_empty = 0;
    vb_tx_fifo_rdata = 0;
    v_tx_fifo_we = 0;
    vb_rx_wr_cnt_next = 0;
    v_rx_fifo_full = 0;
    v_rx_fifo_empty = 0;
    vb_rx_fifo_rdata = 0;
    v_rx_fifo_we = 0;
    v_rx_fifo_re = 0;
    v_negedge_flag = 0;
    v_posedge_flag = 0;
    par = 0;

    v.scaler = r.scaler;
    v.scaler_cnt = r.scaler_cnt;
    v.level = r.level;
    v.cs = r.cs;
    for (int i = 0; i < fifosz; i++) {
        v.rx_fifo[i] = r.rx_fifo[i];
    }
    for (int i = 0; i < fifosz; i++) {
        v.tx_fifo[i] = r.tx_fifo[i];
    }
    v.state = r.state;
    v.wr_cnt = r.wr_cnt;
    v.rd_cnt = r.rd_cnt;
    v.bit_cnt = r.bit_cnt;
    v.tx_shift = r.tx_shift;
    v.rx_shift = r.rx_shift;
    v.rx_shift = r.rx_shift;
    v.resp_valid = r.resp_valid;
    v.resp_rdata = r.resp_rdata;
    v.resp_err = r.resp_err;


    // Transmitter's FIFO:
    vb_tx_wr_cnt_next = (r.wr_cnt.read() + 1);
    if (vb_tx_wr_cnt_next == r.rd_cnt.read()) {
        v_tx_fifo_full = 1;
    }

    if (r.rd_cnt.read() == r.wr_cnt.read()) {
        v_tx_fifo_empty = 1;
        v.bit_cnt = 0;
    }
    // Receiver's FIFO:

    // system bus clock scaler to baudrate:
    if (r.scaler.read().or_reduce() == 1) {
        if (r.scaler_cnt.read() == (r.scaler.read() - 1)) {
            v.scaler_cnt = 0;
            v.level = (!r.level);
            v_posedge_flag = (!r.level);
            v_negedge_flag = r.level;
        } else {
            v.scaler_cnt = (r.scaler_cnt.read() + 1);
        }
    }

    // Transmitter's state machine:
    if (v_posedge_flag == 1) {
        switch (r.state.read()) {
        case idle:
            if (v_tx_fifo_empty == 0) {
                v.tx_shift = vb_tx_fifo_rdata;
                v.state = send_data;
                v.rd_cnt = (r.rd_cnt.read() + 1);
                v.bit_cnt = 7;
                v.cs = 1;
            } else {
                v.tx_shift = ~0ull;
            }
            break;
        case send_data:
            if (r.bit_cnt.read().or_reduce() == 0) {
                v.state = receive_data;
                v.bit_cnt = 7;
            }
            break;
        case receive_data:
            if (r.bit_cnt.read()(2, 0).or_reduce() == 0) {
                v.state = idle;
                v.rx_shift = r.rx_shift;
            }
            break;
        default:
            break;
        }

        if (r.state.read() != idle) {
            v.bit_cnt = (r.bit_cnt.read() + 1);
            v.tx_shift = ((r.tx_shift.read()(7, 1) << 1) | 1);
        }
    }

    // Registers access:
    switch (wb_req_addr.read()(11, 2)) {
    case 0:                                                 // 0x00: txdata
        vb_rdata[31] = v_tx_fifo_full;
        if (w_req_valid.read() == 1) {
            if (w_req_write.read() == 1) {
                v_tx_fifo_we = (~v_tx_fifo_full);
            }
        }
        break;
    case 1:                                                 // 0x04: rxdata
        vb_rdata[31] = v_rx_fifo_empty;
        vb_rdata(7, 0) = vb_rx_fifo_rdata;
        if (w_req_valid.read() == 1) {
            if (w_req_write.read() == 1) {
                // do nothing:
            } else {
                v_rx_fifo_re = (!v_rx_fifo_empty);
            }
        }
        break;
    case 2:                                                 // 0x08: txctrl
        vb_rdata[0] = i_detected.read();                    // [0] sd card inserted
        vb_rdata[1] = i_protect.read();                     // [1] write protect
        vb_rdata(5, 4) = r.state;                           // [5:4] state machine
        break;
    case 6:                                                 // 0x18: scaler
        vb_rdata = r.scaler;
        if ((w_req_valid.read() == 1) && (w_req_write.read() == 1)) {
            v.scaler = wb_req_wdata.read()(30, 0);
            v.scaler_cnt = 0;
        }
        break;
    default:
        break;
    }

    if (v_rx_fifo_we == 1) {
    } else if (v_rx_fifo_re == 1) {
    }
    if (v_tx_fifo_we == 1) {
        v.wr_cnt = (r.wr_cnt.read() + 1);
        v.tx_fifo[r.wr_cnt.read().to_int()] = wb_req_wdata.read()(7, 0);
    }

    v.resp_valid = w_req_valid;
    v.resp_rdata = vb_rdata;
    v.resp_err = 0;

    if (!async_reset_ && i_nrst.read() == 0) {
        v.scaler = 0;
        v.scaler_cnt = 0;
        v.level = 1;
        v.cs = 0;
        for (int i = 0; i < fifosz; i++) {
            v.rx_fifo[i] = 0;
        }
        for (int i = 0; i < fifosz; i++) {
            v.tx_fifo[i] = 0;
        }
        v.state = idle;
        v.wr_cnt = 0;
        v.rd_cnt = 0;
        v.bit_cnt = 0;
        v.tx_shift = ~0ul;
        v.rx_shift = 0;
        v.rx_shift = 0;
        v.resp_valid = 0;
        v.resp_rdata = 0;
        v.resp_err = 0;
    }

    o_sclk = r.level;
    o_miso = r.tx_shift.read()[7];
}

template<int log2_fifosz>
void apb_spi<log2_fifosz>::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        r.scaler = 0;
        r.scaler_cnt = 0;
        r.level = 1;
        r.cs = 0;
        for (int i = 0; i < fifosz; i++) {
            r.rx_fifo[i] = 0;
        }
        for (int i = 0; i < fifosz; i++) {
            r.tx_fifo[i] = 0;
        }
        r.state = idle;
        r.wr_cnt = 0;
        r.rd_cnt = 0;
        r.bit_cnt = 0;
        r.tx_shift = ~0ul;
        r.rx_shift = 0;
        r.rx_shift = 0;
        r.resp_valid = 0;
        r.resp_rdata = 0;
        r.resp_err = 0;
    } else {
        r.scaler = v.scaler;
        r.scaler_cnt = v.scaler_cnt;
        r.level = v.level;
        r.cs = v.cs;
        for (int i = 0; i < fifosz; i++) {
            r.rx_fifo[i] = v.rx_fifo[i];
        }
        for (int i = 0; i < fifosz; i++) {
            r.tx_fifo[i] = v.tx_fifo[i];
        }
        r.state = v.state;
        r.wr_cnt = v.wr_cnt;
        r.rd_cnt = v.rd_cnt;
        r.bit_cnt = v.bit_cnt;
        r.tx_shift = v.tx_shift;
        r.rx_shift = v.rx_shift;
        r.rx_shift = v.rx_shift;
        r.resp_valid = v.resp_valid;
        r.resp_rdata = v.resp_rdata;
        r.resp_err = v.resp_err;
    }
}

}  // namespace debugger

