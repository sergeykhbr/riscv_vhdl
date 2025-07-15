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
#include "../ambalib/types_pnp.h"
#include "../ambalib/apb_slv.h"
#include "api_core.h"

namespace debugger {

template<int log2_fifosz = 4>
SC_MODULE(apb_uart) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<bool> i_nrst;                                     // Reset: active LOW
    sc_in<mapinfo_type> i_mapinfo;                          // interconnect slot information
    sc_out<dev_config_type> o_cfg;                          // Device descriptor
    sc_in<apb_in_type> i_apbi;                              // APB  Slave to Bridge interface
    sc_out<apb_out_type> o_apbo;                            // APB Bridge to Slave interface
    sc_in<bool> i_rd;
    sc_out<bool> o_td;
    sc_out<bool> o_irq;

    void comb();
    void registers();

    apb_uart(sc_module_name name,
             bool async_reset,
             int sim_speedup_rate);
    virtual ~apb_uart();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;
    int sim_speedup_rate_;

    static const int fifosz = (1 << log2_fifosz);
    // Rx/Tx states
    static const uint8_t idle = 0;
    static const uint8_t startbit = 1;
    static const uint8_t data = 2;
    static const uint8_t parity = 3;
    static const uint8_t stopbit = 4;

    struct apb_uart_registers {
        sc_signal<sc_uint<32>> scaler;
        sc_signal<sc_uint<32>> scaler_cnt;
        sc_signal<bool> level;
        sc_signal<bool> err_parity;
        sc_signal<bool> err_stopbit;
        sc_signal<sc_uint<32>> fwcpuid;
        sc_signal<sc_uint<8>> rx_fifo[fifosz];
        sc_signal<sc_uint<3>> rx_state;
        sc_signal<bool> rx_ena;
        sc_signal<bool> rx_ie;
        sc_signal<bool> rx_ip;
        sc_signal<bool> rx_nstop;
        sc_signal<bool> rx_par;
        sc_signal<sc_uint<log2_fifosz>> rx_wr_cnt;
        sc_signal<sc_uint<log2_fifosz>> rx_rd_cnt;
        sc_signal<sc_uint<log2_fifosz>> rx_byte_cnt;
        sc_signal<sc_uint<log2_fifosz>> rx_irq_thresh;
        sc_signal<sc_uint<4>> rx_frame_cnt;
        sc_signal<bool> rx_stop_cnt;
        sc_signal<sc_uint<11>> rx_shift;
        sc_signal<sc_uint<8>> tx_fifo[fifosz];
        sc_signal<sc_uint<3>> tx_state;
        sc_signal<bool> tx_ena;
        sc_signal<bool> tx_ie;
        sc_signal<bool> tx_ip;
        sc_signal<bool> tx_nstop;
        sc_signal<bool> tx_par;
        sc_signal<sc_uint<log2_fifosz>> tx_wr_cnt;
        sc_signal<sc_uint<log2_fifosz>> tx_rd_cnt;
        sc_signal<sc_uint<log2_fifosz>> tx_byte_cnt;
        sc_signal<sc_uint<log2_fifosz>> tx_irq_thresh;
        sc_signal<sc_uint<4>> tx_frame_cnt;
        sc_signal<bool> tx_stop_cnt;
        sc_signal<sc_uint<11>> tx_shift;
        sc_signal<bool> tx_amo_guard;                       // AMO operation read-modify-write often hit on full flag border
        sc_signal<bool> resp_valid;
        sc_signal<sc_uint<32>> resp_rdata;
        sc_signal<bool> resp_err;
    };

    void apb_uart_r_reset(apb_uart_registers& iv) {
        iv.scaler = 0;
        iv.scaler_cnt = 0;
        iv.level = 1;
        iv.err_parity = 0;
        iv.err_stopbit = 0;
        iv.fwcpuid = 0;
        for (int i = 0; i < fifosz; i++) {
            iv.rx_fifo[i] = 0;
        }
        iv.rx_state = idle;
        iv.rx_ena = 0;
        iv.rx_ie = 0;
        iv.rx_ip = 0;
        iv.rx_nstop = 0;
        iv.rx_par = 0;
        iv.rx_wr_cnt = 0;
        iv.rx_rd_cnt = 0;
        iv.rx_byte_cnt = 0;
        iv.rx_irq_thresh = 0;
        iv.rx_frame_cnt = 0;
        iv.rx_stop_cnt = 0;
        iv.rx_shift = 0;
        for (int i = 0; i < fifosz; i++) {
            iv.tx_fifo[i] = 0;
        }
        iv.tx_state = idle;
        iv.tx_ena = 0;
        iv.tx_ie = 0;
        iv.tx_ip = 0;
        iv.tx_nstop = 0;
        iv.tx_par = 0;
        iv.tx_wr_cnt = 0;
        iv.tx_rd_cnt = 0;
        iv.tx_byte_cnt = 0;
        iv.tx_irq_thresh = 0;
        iv.tx_frame_cnt = 0;
        iv.tx_stop_cnt = 0;
        iv.tx_shift = ~0ull;
        iv.tx_amo_guard = 0;
        iv.resp_valid = 0;
        iv.resp_rdata = 0;
        iv.resp_err = 0;
    }

    sc_signal<bool> w_req_valid;
    sc_signal<sc_uint<32>> wb_req_addr;
    sc_signal<bool> w_req_write;
    sc_signal<sc_uint<32>> wb_req_wdata;
    apb_uart_registers v;
    apb_uart_registers r;

    apb_slv *pslv0;

};

template<int log2_fifosz>
apb_uart<log2_fifosz>::apb_uart(sc_module_name name,
                                bool async_reset,
                                int sim_speedup_rate)
    : sc_module(name),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_mapinfo("i_mapinfo"),
    o_cfg("o_cfg"),
    i_apbi("i_apbi"),
    o_apbo("o_apbo"),
    i_rd("i_rd"),
    o_td("o_td"),
    o_irq("o_irq") {

    async_reset_ = async_reset;
    sim_speedup_rate_ = sim_speedup_rate;
    pslv0 = 0;

    pslv0 = new apb_slv("pslv0",
                         async_reset,
                         VENDOR_OPTIMITECH,
                         OPTIMITECH_UART);
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
    sensitive << i_rd;
    sensitive << w_req_valid;
    sensitive << wb_req_addr;
    sensitive << w_req_write;
    sensitive << wb_req_wdata;
    sensitive << r.scaler;
    sensitive << r.scaler_cnt;
    sensitive << r.level;
    sensitive << r.err_parity;
    sensitive << r.err_stopbit;
    sensitive << r.fwcpuid;
    for (int i = 0; i < fifosz; i++) {
        sensitive << r.rx_fifo[i];
    }
    sensitive << r.rx_state;
    sensitive << r.rx_ena;
    sensitive << r.rx_ie;
    sensitive << r.rx_ip;
    sensitive << r.rx_nstop;
    sensitive << r.rx_par;
    sensitive << r.rx_wr_cnt;
    sensitive << r.rx_rd_cnt;
    sensitive << r.rx_byte_cnt;
    sensitive << r.rx_irq_thresh;
    sensitive << r.rx_frame_cnt;
    sensitive << r.rx_stop_cnt;
    sensitive << r.rx_shift;
    for (int i = 0; i < fifosz; i++) {
        sensitive << r.tx_fifo[i];
    }
    sensitive << r.tx_state;
    sensitive << r.tx_ena;
    sensitive << r.tx_ie;
    sensitive << r.tx_ip;
    sensitive << r.tx_nstop;
    sensitive << r.tx_par;
    sensitive << r.tx_wr_cnt;
    sensitive << r.tx_rd_cnt;
    sensitive << r.tx_byte_cnt;
    sensitive << r.tx_irq_thresh;
    sensitive << r.tx_frame_cnt;
    sensitive << r.tx_stop_cnt;
    sensitive << r.tx_shift;
    sensitive << r.tx_amo_guard;
    sensitive << r.resp_valid;
    sensitive << r.resp_rdata;
    sensitive << r.resp_err;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
}

template<int log2_fifosz>
apb_uart<log2_fifosz>::~apb_uart() {
    if (pslv0) {
        delete pslv0;
    }
}

template<int log2_fifosz>
void apb_uart<log2_fifosz>::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, i_apbi, i_apbi.name());
        sc_trace(o_vcd, o_apbo, o_apbo.name());
        sc_trace(o_vcd, i_rd, i_rd.name());
        sc_trace(o_vcd, o_td, o_td.name());
        sc_trace(o_vcd, o_irq, o_irq.name());
        sc_trace(o_vcd, r.scaler, pn + ".r.scaler");
        sc_trace(o_vcd, r.scaler_cnt, pn + ".r.scaler_cnt");
        sc_trace(o_vcd, r.level, pn + ".r.level");
        sc_trace(o_vcd, r.err_parity, pn + ".r.err_parity");
        sc_trace(o_vcd, r.err_stopbit, pn + ".r.err_stopbit");
        sc_trace(o_vcd, r.fwcpuid, pn + ".r.fwcpuid");
        for (int i = 0; i < fifosz; i++) {
            sc_trace(o_vcd, r.rx_fifo[i], pn + ".r.rx_fifo(" + std::to_string(i) + ")");
        }
        sc_trace(o_vcd, r.rx_state, pn + ".r.rx_state");
        sc_trace(o_vcd, r.rx_ena, pn + ".r.rx_ena");
        sc_trace(o_vcd, r.rx_ie, pn + ".r.rx_ie");
        sc_trace(o_vcd, r.rx_ip, pn + ".r.rx_ip");
        sc_trace(o_vcd, r.rx_nstop, pn + ".r.rx_nstop");
        sc_trace(o_vcd, r.rx_par, pn + ".r.rx_par");
        sc_trace(o_vcd, r.rx_wr_cnt, pn + ".r.rx_wr_cnt");
        sc_trace(o_vcd, r.rx_rd_cnt, pn + ".r.rx_rd_cnt");
        sc_trace(o_vcd, r.rx_byte_cnt, pn + ".r.rx_byte_cnt");
        sc_trace(o_vcd, r.rx_irq_thresh, pn + ".r.rx_irq_thresh");
        sc_trace(o_vcd, r.rx_frame_cnt, pn + ".r.rx_frame_cnt");
        sc_trace(o_vcd, r.rx_stop_cnt, pn + ".r.rx_stop_cnt");
        sc_trace(o_vcd, r.rx_shift, pn + ".r.rx_shift");
        for (int i = 0; i < fifosz; i++) {
            sc_trace(o_vcd, r.tx_fifo[i], pn + ".r.tx_fifo(" + std::to_string(i) + ")");
        }
        sc_trace(o_vcd, r.tx_state, pn + ".r.tx_state");
        sc_trace(o_vcd, r.tx_ena, pn + ".r.tx_ena");
        sc_trace(o_vcd, r.tx_ie, pn + ".r.tx_ie");
        sc_trace(o_vcd, r.tx_ip, pn + ".r.tx_ip");
        sc_trace(o_vcd, r.tx_nstop, pn + ".r.tx_nstop");
        sc_trace(o_vcd, r.tx_par, pn + ".r.tx_par");
        sc_trace(o_vcd, r.tx_wr_cnt, pn + ".r.tx_wr_cnt");
        sc_trace(o_vcd, r.tx_rd_cnt, pn + ".r.tx_rd_cnt");
        sc_trace(o_vcd, r.tx_byte_cnt, pn + ".r.tx_byte_cnt");
        sc_trace(o_vcd, r.tx_irq_thresh, pn + ".r.tx_irq_thresh");
        sc_trace(o_vcd, r.tx_frame_cnt, pn + ".r.tx_frame_cnt");
        sc_trace(o_vcd, r.tx_stop_cnt, pn + ".r.tx_stop_cnt");
        sc_trace(o_vcd, r.tx_shift, pn + ".r.tx_shift");
        sc_trace(o_vcd, r.tx_amo_guard, pn + ".r.tx_amo_guard");
        sc_trace(o_vcd, r.resp_valid, pn + ".r.resp_valid");
        sc_trace(o_vcd, r.resp_rdata, pn + ".r.resp_rdata");
        sc_trace(o_vcd, r.resp_err, pn + ".r.resp_err");
    }

    if (pslv0) {
        pslv0->generateVCD(i_vcd, o_vcd);
    }
}

template<int log2_fifosz>
void apb_uart<log2_fifosz>::comb() {
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

    v.scaler = r.scaler.read();
    v.scaler_cnt = r.scaler_cnt.read();
    v.level = r.level.read();
    v.err_parity = r.err_parity.read();
    v.err_stopbit = r.err_stopbit.read();
    v.fwcpuid = r.fwcpuid.read();
    for (int i = 0; i < fifosz; i++) {
        v.rx_fifo[i] = r.rx_fifo[i].read();
    }
    v.rx_state = r.rx_state.read();
    v.rx_ena = r.rx_ena.read();
    v.rx_ie = r.rx_ie.read();
    v.rx_ip = r.rx_ip.read();
    v.rx_nstop = r.rx_nstop.read();
    v.rx_par = r.rx_par.read();
    v.rx_wr_cnt = r.rx_wr_cnt.read();
    v.rx_rd_cnt = r.rx_rd_cnt.read();
    v.rx_byte_cnt = r.rx_byte_cnt.read();
    v.rx_irq_thresh = r.rx_irq_thresh.read();
    v.rx_frame_cnt = r.rx_frame_cnt.read();
    v.rx_stop_cnt = r.rx_stop_cnt.read();
    v.rx_shift = r.rx_shift.read();
    for (int i = 0; i < fifosz; i++) {
        v.tx_fifo[i] = r.tx_fifo[i].read();
    }
    v.tx_state = r.tx_state.read();
    v.tx_ena = r.tx_ena.read();
    v.tx_ie = r.tx_ie.read();
    v.tx_ip = r.tx_ip.read();
    v.tx_nstop = r.tx_nstop.read();
    v.tx_par = r.tx_par.read();
    v.tx_wr_cnt = r.tx_wr_cnt.read();
    v.tx_rd_cnt = r.tx_rd_cnt.read();
    v.tx_byte_cnt = r.tx_byte_cnt.read();
    v.tx_irq_thresh = r.tx_irq_thresh.read();
    v.tx_frame_cnt = r.tx_frame_cnt.read();
    v.tx_stop_cnt = r.tx_stop_cnt.read();
    v.tx_shift = r.tx_shift.read();
    v.tx_amo_guard = r.tx_amo_guard.read();
    v.resp_valid = r.resp_valid.read();
    v.resp_rdata = r.resp_rdata.read();
    v.resp_err = r.resp_err.read();
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

    vb_rx_fifo_rdata = r.rx_fifo[r.rx_rd_cnt.read().to_int()].read();
    vb_tx_fifo_rdata = r.tx_fifo[r.tx_rd_cnt.read().to_int()].read();

    // Check FIFOs counters with thresholds:
    if (r.tx_byte_cnt.read() < r.tx_irq_thresh.read()) {
        v.tx_ip = r.tx_ie.read();
    }

    if (r.rx_byte_cnt.read() > r.rx_irq_thresh.read()) {
        v.rx_ip = r.rx_ie.read();
    }

    // Transmitter's FIFO:
    vb_tx_wr_cnt_next = (r.tx_wr_cnt.read() + 1);
    if (vb_tx_wr_cnt_next == r.tx_rd_cnt.read()) {
        v_tx_fifo_full = 1;
    }

    if (r.tx_rd_cnt.read() == r.tx_wr_cnt.read()) {
        v_tx_fifo_empty = 1;
        v.tx_byte_cnt = 0;
    }
    // Receiver's FIFO:
    vb_rx_wr_cnt_next = (r.rx_wr_cnt.read() + 1);
    if (vb_rx_wr_cnt_next == r.rx_rd_cnt.read()) {
        v_rx_fifo_full = 1;
    }

    if (r.rx_rd_cnt.read() == r.rx_wr_cnt.read()) {
        v_rx_fifo_empty = 1;
        v.rx_byte_cnt = 0;
    }

    // system bus clock scaler to baudrate:
    if (r.scaler.read().or_reduce() == 1) {
        if (r.scaler_cnt.read() == (r.scaler.read() - 1)) {
            v.scaler_cnt = 0;
            v.level = (!r.level.read());
            v_posedge_flag = (!r.level.read());
            v_negedge_flag = r.level.read();
        } else {
            v.scaler_cnt = (r.scaler_cnt.read() + 1);
        }

        if (((r.rx_state.read() == idle) && (i_rd.read() == 1))
                && ((r.tx_state.read() == idle) && (v_tx_fifo_empty == 1))) {
            v.scaler_cnt = 0;
            v.level = 1;
        }
    }

    // Transmitter's state machine:
    if (v_posedge_flag == 1) {
        switch (r.tx_state.read()) {
        case idle:
            if ((v_tx_fifo_empty == 0) && (r.tx_ena.read() == 1)) {
                // stopbit=1,parity=xor,data[7:0],startbit=0
                if (r.tx_par.read() == 1) {
                    par = (vb_tx_fifo_rdata[7]
                            ^ vb_tx_fifo_rdata[6]
                            ^ vb_tx_fifo_rdata[5]
                            ^ vb_tx_fifo_rdata[4]
                            ^ vb_tx_fifo_rdata[3]
                            ^ vb_tx_fifo_rdata[2]
                            ^ vb_tx_fifo_rdata[1]
                            ^ vb_tx_fifo_rdata[0]);
                    v.tx_shift = (1, par, vb_tx_fifo_rdata, 0);
                } else {
                    v.tx_shift = (3, vb_tx_fifo_rdata, 0);
                }

                v.tx_state = startbit;
                v.tx_rd_cnt = (r.tx_rd_cnt.read() + 1);
                v.tx_byte_cnt = (r.tx_byte_cnt.read() - 1);
                v.tx_frame_cnt = 0;
            } else {
                v.tx_shift = ~0ull;
            }
            break;
        case startbit:
            v.tx_state = data;
            break;
        case data:
            if (r.tx_frame_cnt.read() == 8) {
                if (r.tx_par.read() == 1) {
                    v.tx_state = parity;
                } else {
                    v.tx_state = stopbit;
                    v.tx_stop_cnt = r.tx_nstop.read();
                }
            }
            break;
        case parity:
            v.tx_state = stopbit;
            break;
        case stopbit:
            if (r.tx_stop_cnt.read() == 0) {
                v.tx_state = idle;
                v.tx_shift = ~0ull;
            } else {
                v.tx_stop_cnt = 0;
            }
            break;
        default:
            break;
        }

        if ((r.tx_state.read() != idle) && (r.tx_state.read() != stopbit)) {
            v.tx_frame_cnt = (r.tx_frame_cnt.read() + 1);
            v.tx_shift = (1, r.tx_shift.read()(10, 1));
        }
    }

    // Receiver's state machine:
    if (v_negedge_flag == 1) {
        switch (r.rx_state.read()) {
        case idle:
            if ((i_rd.read() == 0) && (r.rx_ena.read() == 1)) {
                v.rx_state = data;
                v.rx_shift = 0;
                v.rx_frame_cnt = 0;
            }
            break;
        case data:
            v.rx_shift = (i_rd.read(), r.rx_shift.read()(7, 1));
            if (r.rx_frame_cnt.read() == 7) {
                if (r.rx_par.read() == 1) {
                    v.rx_state = parity;
                } else {
                    v.rx_state = stopbit;
                    v.rx_stop_cnt = r.rx_nstop.read();
                }
            } else {
                v.rx_frame_cnt = (r.rx_frame_cnt.read() + 1);
            }
            break;
        case parity:
            par = (r.rx_shift.read()[7]
                    ^ r.rx_shift.read()[6]
                    ^ r.rx_shift.read()[5]
                    ^ r.rx_shift.read()[4]
                    ^ r.rx_shift.read()[3]
                    ^ r.rx_shift.read()[2]
                    ^ r.rx_shift.read()[1]
                    ^ r.rx_shift.read()[0]);
            if (par == i_rd.read()) {
                v.err_parity = 0;
            } else {
                v.err_parity = 1;
            }

            v.rx_state = stopbit;
            break;
        case stopbit:
            if (i_rd.read() == 0) {
                v.err_stopbit = 1;
            } else {
                v.err_stopbit = 0;
            }

            if (r.rx_stop_cnt.read() == 0) {
                if (v_rx_fifo_full == 0) {
                    v_rx_fifo_we = 1;
                }
                v.rx_state = idle;
            } else {
                v.rx_stop_cnt = 0;
            }
            break;
        default:
            break;
        }
    }

    // Registers access:
    switch (wb_req_addr.read()(11, 2)) {
    case 0:                                                 // 0x00: txdata
        vb_rdata[31] = v_tx_fifo_full;
        if (w_req_valid.read() == 1) {
            if (w_req_write.read() == 1) {
                v_tx_fifo_we = ((!v_tx_fifo_full) & (!r.tx_amo_guard.read()));
            } else {
                v.tx_amo_guard = v_tx_fifo_full;            // skip next write
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
        vb_rdata[0] = r.tx_ena.read();                      // [0] tx ena
        vb_rdata[1] = r.tx_nstop.read();                    // [1] Number of stop bits
        vb_rdata[2] = r.tx_par.read();                      // [2] parity bit enable
        vb_rdata(18, 16) = r.tx_irq_thresh.read()(2, 0);    // [18:16] FIFO threshold to raise irq
        if ((w_req_valid.read() == 1) && (w_req_write.read() == 1)) {
            v.tx_ena = wb_req_wdata.read()[0];
            v.tx_nstop = wb_req_wdata.read()[1];
            v.tx_par = wb_req_wdata.read()[2];
            v.tx_irq_thresh = wb_req_wdata.read()(18, 16);
        }
        break;
    case 3:                                                 // 0x0C: rxctrl
        vb_rdata[0] = r.rx_ena.read();                      // [0] txena
        vb_rdata[1] = r.rx_nstop.read();                    // [1] Number of stop bits
        vb_rdata[2] = r.rx_par.read();
        vb_rdata(18, 16) = r.rx_irq_thresh.read()(2, 0);
        if ((w_req_valid.read() == 1) && (w_req_write.read() == 1)) {
            v.rx_ena = wb_req_wdata.read()[0];
            v.rx_nstop = wb_req_wdata.read()[1];
            v.rx_par = wb_req_wdata.read()[2];
            v.rx_irq_thresh = wb_req_wdata.read()(18, 16);
        }
        break;
    case 4:                                                 // 0x10: ie
        vb_rdata[0] = r.tx_ie.read();
        vb_rdata[1] = r.rx_ie.read();
        if ((w_req_valid.read() == 1) && (w_req_write.read() == 1)) {
            v.tx_ie = wb_req_wdata.read()[0];
            v.rx_ie = wb_req_wdata.read()[1];
        }
        break;
    case 5:                                                 // 0x14: ip
        vb_rdata[0] = r.tx_ip.read();
        vb_rdata[1] = r.rx_ip.read();
        if ((w_req_valid.read() == 1) && (w_req_write.read() == 1)) {
            v.tx_ip = wb_req_wdata.read()[0];
            v.rx_ip = wb_req_wdata.read()[1];
        }
        break;
    case 6:                                                 // 0x18: scaler
        vb_rdata = r.scaler.read();
        if ((w_req_valid.read() == 1) && (w_req_write.read() == 1)) {
            v.scaler = wb_req_wdata.read()(30, sim_speedup_rate_);
            v.scaler_cnt = 0;
        }
        break;
    case 7:                                                 // 0x1C: fwcpuid
        vb_rdata = r.fwcpuid.read();
        if ((w_req_valid.read() == 1) && (w_req_write.read() == 1)) {
            if ((r.fwcpuid.read().or_reduce() == 0) || (wb_req_wdata.read().or_reduce() == 0)) {
                v.fwcpuid = wb_req_wdata.read();
            }
        }
        break;
    default:
        break;
    }

    if (v_rx_fifo_we == 1) {
        v.rx_wr_cnt = (r.rx_wr_cnt.read() + 1);
        v.rx_byte_cnt = (r.rx_byte_cnt.read() + 1);
        v.rx_fifo[r.rx_wr_cnt.read().to_int()] = r.rx_shift.read()(7, 0);
    } else if (v_rx_fifo_re == 1) {
        v.rx_rd_cnt = (r.rx_rd_cnt.read() + 1);
        v.rx_byte_cnt = (r.rx_byte_cnt.read() - 1);
    }
    if (v_tx_fifo_we == 1) {
        v.tx_wr_cnt = (r.tx_wr_cnt.read() + 1);
        v.tx_byte_cnt = (r.tx_byte_cnt.read() + 1);
        v.tx_fifo[r.tx_wr_cnt.read().to_int()] = wb_req_wdata.read()(7, 0);
    }

    v.resp_valid = w_req_valid.read();
    v.resp_rdata = vb_rdata;
    v.resp_err = 0;

    if ((!async_reset_) && (i_nrst.read() == 0)) {
        apb_uart_r_reset(v);
    }

    o_td = r.tx_shift.read()[0];
    o_irq = (r.tx_ip.read() | r.rx_ip.read());
}

template<int log2_fifosz>
void apb_uart<log2_fifosz>::registers() {
    if ((async_reset_ == 1) && (i_nrst.read() == 0)) {
        apb_uart_r_reset(r);
    } else {
        r.scaler = v.scaler.read();
        r.scaler_cnt = v.scaler_cnt.read();
        r.level = v.level.read();
        r.err_parity = v.err_parity.read();
        r.err_stopbit = v.err_stopbit.read();
        r.fwcpuid = v.fwcpuid.read();
        for (int i = 0; i < fifosz; i++) {
            r.rx_fifo[i] = v.rx_fifo[i].read();
        }
        r.rx_state = v.rx_state.read();
        r.rx_ena = v.rx_ena.read();
        r.rx_ie = v.rx_ie.read();
        r.rx_ip = v.rx_ip.read();
        r.rx_nstop = v.rx_nstop.read();
        r.rx_par = v.rx_par.read();
        r.rx_wr_cnt = v.rx_wr_cnt.read();
        r.rx_rd_cnt = v.rx_rd_cnt.read();
        r.rx_byte_cnt = v.rx_byte_cnt.read();
        r.rx_irq_thresh = v.rx_irq_thresh.read();
        r.rx_frame_cnt = v.rx_frame_cnt.read();
        r.rx_stop_cnt = v.rx_stop_cnt.read();
        r.rx_shift = v.rx_shift.read();
        for (int i = 0; i < fifosz; i++) {
            r.tx_fifo[i] = v.tx_fifo[i].read();
        }
        r.tx_state = v.tx_state.read();
        r.tx_ena = v.tx_ena.read();
        r.tx_ie = v.tx_ie.read();
        r.tx_ip = v.tx_ip.read();
        r.tx_nstop = v.tx_nstop.read();
        r.tx_par = v.tx_par.read();
        r.tx_wr_cnt = v.tx_wr_cnt.read();
        r.tx_rd_cnt = v.tx_rd_cnt.read();
        r.tx_byte_cnt = v.tx_byte_cnt.read();
        r.tx_irq_thresh = v.tx_irq_thresh.read();
        r.tx_frame_cnt = v.tx_frame_cnt.read();
        r.tx_stop_cnt = v.tx_stop_cnt.read();
        r.tx_shift = v.tx_shift.read();
        r.tx_amo_guard = v.tx_amo_guard.read();
        r.resp_valid = v.resp_valid.read();
        r.resp_rdata = v.resp_rdata.read();
        r.resp_err = v.resp_err.read();
    }
}

}  // namespace debugger

