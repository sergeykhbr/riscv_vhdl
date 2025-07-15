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
#include "sfifo.h"
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
    sc_out<bool> o_mosi;
    sc_in<bool> i_miso;
    sc_in<bool> i_detected;
    sc_in<bool> i_protect;

    void comb();
    void registers();

    apb_spi(sc_module_name name,
            bool async_reset);
    virtual ~apb_spi();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;

    static const int fifo_dbits = 8;
    // SPI states
    static const uint8_t idle = 0;
    static const uint8_t wait_edge = 1;
    static const uint8_t send_data = 2;
    static const uint8_t recv_data = 3;
    static const uint8_t recv_sync = 4;
    static const uint8_t ending = 5;

    struct apb_spi_registers {
        sc_signal<sc_uint<32>> scaler;
        sc_signal<sc_uint<32>> scaler_cnt;
        sc_signal<sc_uint<16>> wdog;
        sc_signal<sc_uint<16>> wdog_cnt;
        sc_signal<bool> generate_crc;
        sc_signal<bool> rx_ena;
        sc_signal<bool> rx_synced;
        sc_signal<bool> rx_data_block;                      // Wait 0xFE start data block marker
        sc_signal<bool> level;
        sc_signal<bool> cs;
        sc_signal<sc_uint<3>> state;
        sc_signal<sc_uint<8>> shiftreg;
        sc_signal<sc_uint<16>> ena_byte_cnt;
        sc_signal<sc_uint<3>> bit_cnt;
        sc_signal<sc_uint<8>> tx_val;
        sc_signal<sc_uint<8>> rx_val;
        sc_signal<bool> rx_ready;
        sc_signal<sc_uint<7>> crc7;
        sc_signal<sc_uint<16>> crc16;
        sc_signal<sc_uint<8>> spi_resp;
        sc_signal<sc_uint<log2_fifosz>> txmark;
        sc_signal<sc_uint<log2_fifosz>> rxmark;
        sc_signal<bool> resp_valid;
        sc_signal<sc_uint<32>> resp_rdata;
        sc_signal<bool> resp_err;
    };

    void apb_spi_r_reset(apb_spi_registers& iv) {
        iv.scaler = 0;
        iv.scaler_cnt = 0;
        iv.wdog = 0;
        iv.wdog_cnt = 0;
        iv.generate_crc = 0;
        iv.rx_ena = 0;
        iv.rx_synced = 0;
        iv.rx_data_block = 0;
        iv.level = 1;
        iv.cs = 0;
        iv.state = idle;
        iv.shiftreg = ~0ull;
        iv.ena_byte_cnt = 0;
        iv.bit_cnt = 0;
        iv.tx_val = 0;
        iv.rx_val = 0;
        iv.rx_ready = 0;
        iv.crc7 = 0;
        iv.crc16 = 0;
        iv.spi_resp = 0;
        iv.txmark = 0;
        iv.rxmark = 0;
        iv.resp_valid = 0;
        iv.resp_rdata = 0;
        iv.resp_err = 0;
    }

    sc_signal<bool> w_req_valid;
    sc_signal<sc_uint<32>> wb_req_addr;
    sc_signal<bool> w_req_write;
    sc_signal<sc_uint<32>> wb_req_wdata;
    // Rx FIFO signals:
    sc_signal<bool> w_rxfifo_we;
    sc_signal<sc_uint<8>> wb_rxfifo_wdata;
    sc_signal<bool> w_rxfifo_re;
    sc_signal<sc_uint<8>> wb_rxfifo_rdata;
    sc_signal<sc_uint<(log2_fifosz + 1)>> wb_rxfifo_count;
    // Tx FIFO signals:
    sc_signal<bool> w_txfifo_we;
    sc_signal<sc_uint<8>> wb_txfifo_wdata;
    sc_signal<bool> w_txfifo_re;
    sc_signal<sc_uint<8>> wb_txfifo_rdata;
    sc_signal<sc_uint<(log2_fifosz + 1)>> wb_txfifo_count;
    apb_spi_registers v;
    apb_spi_registers r;

    apb_slv *pslv0;
    sfifo<fifo_dbits, log2_fifosz> *rxfifo;
    sfifo<fifo_dbits, log2_fifosz> *txfifo;

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
    o_mosi("o_mosi"),
    i_miso("i_miso"),
    i_detected("i_detected"),
    i_protect("i_protect") {

    async_reset_ = async_reset;
    pslv0 = 0;
    rxfifo = 0;
    txfifo = 0;

    pslv0 = new apb_slv("pslv0",
                         async_reset,
                         VENDOR_OPTIMITECH,
                         OPTIMITECH_SDCTRL_REG);
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

    rxfifo = new sfifo<fifo_dbits,
                       log2_fifosz>("rxfifo",
                                    async_reset);
    rxfifo->i_clk(i_clk);
    rxfifo->i_nrst(i_nrst);
    rxfifo->i_we(w_rxfifo_we);
    rxfifo->i_wdata(wb_rxfifo_wdata);
    rxfifo->i_re(w_rxfifo_re);
    rxfifo->o_rdata(wb_rxfifo_rdata);
    rxfifo->o_count(wb_rxfifo_count);

    txfifo = new sfifo<fifo_dbits,
                       log2_fifosz>("txfifo",
                                    async_reset);
    txfifo->i_clk(i_clk);
    txfifo->i_nrst(i_nrst);
    txfifo->i_we(w_txfifo_we);
    txfifo->i_wdata(wb_txfifo_wdata);
    txfifo->i_re(w_txfifo_re);
    txfifo->o_rdata(wb_txfifo_rdata);
    txfifo->o_count(wb_txfifo_count);

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_mapinfo;
    sensitive << i_apbi;
    sensitive << i_miso;
    sensitive << i_detected;
    sensitive << i_protect;
    sensitive << w_req_valid;
    sensitive << wb_req_addr;
    sensitive << w_req_write;
    sensitive << wb_req_wdata;
    sensitive << w_rxfifo_we;
    sensitive << wb_rxfifo_wdata;
    sensitive << w_rxfifo_re;
    sensitive << wb_rxfifo_rdata;
    sensitive << wb_rxfifo_count;
    sensitive << w_txfifo_we;
    sensitive << wb_txfifo_wdata;
    sensitive << w_txfifo_re;
    sensitive << wb_txfifo_rdata;
    sensitive << wb_txfifo_count;
    sensitive << r.scaler;
    sensitive << r.scaler_cnt;
    sensitive << r.wdog;
    sensitive << r.wdog_cnt;
    sensitive << r.generate_crc;
    sensitive << r.rx_ena;
    sensitive << r.rx_synced;
    sensitive << r.rx_data_block;
    sensitive << r.level;
    sensitive << r.cs;
    sensitive << r.state;
    sensitive << r.shiftreg;
    sensitive << r.ena_byte_cnt;
    sensitive << r.bit_cnt;
    sensitive << r.tx_val;
    sensitive << r.rx_val;
    sensitive << r.rx_ready;
    sensitive << r.crc7;
    sensitive << r.crc16;
    sensitive << r.spi_resp;
    sensitive << r.txmark;
    sensitive << r.rxmark;
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
    if (rxfifo) {
        delete rxfifo;
    }
    if (txfifo) {
        delete txfifo;
    }
}

template<int log2_fifosz>
void apb_spi<log2_fifosz>::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, i_apbi, i_apbi.name());
        sc_trace(o_vcd, o_apbo, o_apbo.name());
        sc_trace(o_vcd, o_cs, o_cs.name());
        sc_trace(o_vcd, o_sclk, o_sclk.name());
        sc_trace(o_vcd, o_mosi, o_mosi.name());
        sc_trace(o_vcd, i_miso, i_miso.name());
        sc_trace(o_vcd, i_detected, i_detected.name());
        sc_trace(o_vcd, i_protect, i_protect.name());
        sc_trace(o_vcd, r.scaler, pn + ".r.scaler");
        sc_trace(o_vcd, r.scaler_cnt, pn + ".r.scaler_cnt");
        sc_trace(o_vcd, r.wdog, pn + ".r.wdog");
        sc_trace(o_vcd, r.wdog_cnt, pn + ".r.wdog_cnt");
        sc_trace(o_vcd, r.generate_crc, pn + ".r.generate_crc");
        sc_trace(o_vcd, r.rx_ena, pn + ".r.rx_ena");
        sc_trace(o_vcd, r.rx_synced, pn + ".r.rx_synced");
        sc_trace(o_vcd, r.rx_data_block, pn + ".r.rx_data_block");
        sc_trace(o_vcd, r.level, pn + ".r.level");
        sc_trace(o_vcd, r.cs, pn + ".r.cs");
        sc_trace(o_vcd, r.state, pn + ".r.state");
        sc_trace(o_vcd, r.shiftreg, pn + ".r.shiftreg");
        sc_trace(o_vcd, r.ena_byte_cnt, pn + ".r.ena_byte_cnt");
        sc_trace(o_vcd, r.bit_cnt, pn + ".r.bit_cnt");
        sc_trace(o_vcd, r.tx_val, pn + ".r.tx_val");
        sc_trace(o_vcd, r.rx_val, pn + ".r.rx_val");
        sc_trace(o_vcd, r.rx_ready, pn + ".r.rx_ready");
        sc_trace(o_vcd, r.crc7, pn + ".r.crc7");
        sc_trace(o_vcd, r.crc16, pn + ".r.crc16");
        sc_trace(o_vcd, r.spi_resp, pn + ".r.spi_resp");
        sc_trace(o_vcd, r.txmark, pn + ".r.txmark");
        sc_trace(o_vcd, r.rxmark, pn + ".r.rxmark");
        sc_trace(o_vcd, r.resp_valid, pn + ".r.resp_valid");
        sc_trace(o_vcd, r.resp_rdata, pn + ".r.resp_rdata");
        sc_trace(o_vcd, r.resp_err, pn + ".r.resp_err");
    }

    if (pslv0) {
        pslv0->generateVCD(i_vcd, o_vcd);
    }
    if (rxfifo) {
        rxfifo->generateVCD(i_vcd, o_vcd);
    }
    if (txfifo) {
        txfifo->generateVCD(i_vcd, o_vcd);
    }
}

template<int log2_fifosz>
void apb_spi<log2_fifosz>::comb() {
    bool v_posedge;
    bool v_negedge;
    bool v_txfifo_re;
    bool v_txfifo_we;
    sc_uint<8> vb_txfifo_wdata;
    bool v_rxfifo_re;
    bool v_inv7;
    sc_uint<7> vb_crc7;
    bool v_inv16;
    sc_uint<16> vb_crc16;
    sc_uint<32> vb_rdata;
    sc_uint<8> vb_shiftreg_next;

    v = r;
    v_posedge = 0;
    v_negedge = 0;
    v_txfifo_re = 0;
    v_txfifo_we = 0;
    vb_txfifo_wdata = 0;
    v_rxfifo_re = 0;
    v_inv7 = 0;
    vb_crc7 = 0;
    v_inv16 = 0;
    vb_crc16 = 0;
    vb_rdata = 0;
    vb_shiftreg_next = 0;

    // CRC7 = x^7 + x^3 + 1
    v_inv7 = (r.crc7.read()[6] ^ r.shiftreg.read()[7]);
    vb_crc7[6] = r.crc7.read()[5];
    vb_crc7[5] = r.crc7.read()[4];
    vb_crc7[4] = r.crc7.read()[3];
    vb_crc7[3] = (r.crc7.read()[2] ^ v_inv7);
    vb_crc7[2] = r.crc7.read()[1];
    vb_crc7[1] = r.crc7.read()[0];
    vb_crc7[0] = v_inv7;
    // CRC16 = x^16 + x^12 + x^5 + 1
    v_inv16 = (r.crc16.read()[15] ^ i_miso.read());
    vb_crc16[15] = r.crc16.read()[14];
    vb_crc16[14] = r.crc16.read()[13];
    vb_crc16[13] = r.crc16.read()[12];
    vb_crc16[12] = (r.crc16.read()[11] ^ v_inv16);
    vb_crc16[11] = r.crc16.read()[10];
    vb_crc16[10] = r.crc16.read()[9];
    vb_crc16[9] = r.crc16.read()[8];
    vb_crc16[8] = r.crc16.read()[7];
    vb_crc16[7] = r.crc16.read()[6];
    vb_crc16[6] = r.crc16.read()[5];
    vb_crc16[5] = (r.crc16.read()[4] ^ v_inv16);
    vb_crc16[4] = r.crc16.read()[3];
    vb_crc16[3] = r.crc16.read()[2];
    vb_crc16[2] = r.crc16.read()[1];
    vb_crc16[1] = r.crc16.read()[0];
    vb_crc16[0] = v_inv16;

    // system bus clock scaler to baudrate:
    if (r.scaler.read().or_reduce() == 1) {
        if (r.scaler_cnt.read() == (r.scaler.read() - 1)) {
            v.scaler_cnt = 0;
            v.level = (!r.level.read());
            v_posedge = (!r.level.read());
            v_negedge = r.level.read();
        } else {
            v.scaler_cnt = (r.scaler_cnt.read() + 1);
        }
    }

    if (r.rx_ena.read() == 0) {
        vb_shiftreg_next = ((r.shiftreg.read()(6, 0) << 1) | 1);
    } else {
        vb_shiftreg_next = (r.shiftreg.read()(6, 0), i_miso.read());
    }
    if (r.cs.read() == 1) {
        if (((v_negedge == 1) && (r.rx_ena.read() == 0))
                || ((v_posedge == 1) && (r.rx_ena.read() == 1))) {
            v.shiftreg = vb_shiftreg_next;
        }
    }

    if ((v_negedge == 1) && (r.cs.read() == 1)) {
        if (r.bit_cnt.read().or_reduce() == 1) {
            if ((r.rx_ena.read() == 0)
                    || ((r.rx_ena.read() == 1) && (r.rx_synced.read() == 1))) {
                v.bit_cnt = (r.bit_cnt.read() - 1);
            }
        } else {
            v.cs = 0;
        }
    }

    v.rx_ready = 0;
    if (v_posedge == 1) {
        if ((r.cs.read() == 1) && ((r.rx_ena.read() == 0) || (r.rx_synced.read() == 1))) {
            v.crc7 = vb_crc7;
            v.crc16 = vb_crc16;
        }
    }

    // Transmitter's state machine:
    switch (r.state.read()) {
    case idle:
        v.wdog_cnt = r.wdog.read();
        if (r.ena_byte_cnt.read().or_reduce() == 1) {
            v_txfifo_re = (!r.rx_ena.read());
            if ((wb_txfifo_count.read().or_reduce() == 0) || (r.rx_ena.read() == 1)) {
                // FIFO is empty or RX is enabled:
                v.tx_val = ~0ull;
            } else {
                v.tx_val = wb_txfifo_rdata.read();
            }
            v.state = wait_edge;
            v.ena_byte_cnt = (r.ena_byte_cnt.read() - 1);
            v.crc7 = 0;
        } else {
            v.tx_val = ~0ull;
        }
        break;
    case wait_edge:
        if (v_negedge == 1) {
            v.cs = 1;
            v.bit_cnt = 7;
            if (r.rx_ena.read() == 1) {
                v.shiftreg = 0;
                if (r.rx_data_block.read() == 1) {
                    v.state = recv_sync;
                } else {
                    v.state = recv_data;
                }
            } else {
                v.shiftreg = r.tx_val.read();
                v.state = send_data;
            }
        }
        break;
    case send_data:
        if ((r.bit_cnt.read().or_reduce() == 0) && (v_posedge == 1)) {
            if (r.ena_byte_cnt.read().or_reduce() == 1) {
                v_txfifo_re = 1;
                if (wb_txfifo_count.read().or_reduce() == 0) {
                    // FIFO is empty:
                    v.tx_val = ~0ull;
                } else {
                    v.tx_val = wb_txfifo_rdata.read();
                }
                v.state = wait_edge;
                v.ena_byte_cnt = (r.ena_byte_cnt.read() - 1);
            } else if (r.generate_crc.read() == 1) {
                v.tx_val = ((vb_crc7 << 1) | 1);
                v.generate_crc = 0;
                v.state = wait_edge;
            } else {
                v.state = ending;
            }
        }
        break;
    case recv_data:
        if (v_posedge == 1) {
            if (r.rx_synced.read() == 0) {
                v.rx_synced = ((r.cs.read() == 1) && (!i_miso.read()));
                if (r.wdog_cnt.read().or_reduce() == 1) {
                    v.wdog_cnt = (r.wdog_cnt.read() - 1);
                } else if (r.wdog.read().or_reduce() == 0) {
                    // Wait Start bit infinitely
                } else {
                    // Wait Start bit time is out:
                    v.rx_synced = 1;
                }
            }
            // Check RX shift ready
            if (r.bit_cnt.read().or_reduce() == 0) {
                if (r.ena_byte_cnt.read().or_reduce() == 1) {
                    v.state = wait_edge;
                    v.ena_byte_cnt = (r.ena_byte_cnt.read() - 1);
                } else {
                    v.state = ending;
                }
                v.rx_ready = 1;
                v.rx_val = vb_shiftreg_next;
            }
        }
        break;
    case recv_sync:
        if (v_posedge == 1) {
            if ((vb_shiftreg_next == 0xFE)
                    || (r.wdog_cnt.read().or_reduce() == 0)) {
                v.state = ending;
                v.rx_val = vb_shiftreg_next;
                v.rx_ready = 1;
                v.ena_byte_cnt = 0;
                v.bit_cnt = 0;
                v.crc16 = 0;
            } else {
                v.wdog_cnt = (r.wdog_cnt.read() - 1);
            }
        }
        break;
    case ending:
        if (r.cs.read() == 0) {
            v.state = idle;
        }
        break;
    default:
        break;
    }

    // Registers access:
    switch (wb_req_addr.read()(11, 2)) {
    case 0x000:                                             // 0x00: sckdiv
        vb_rdata = r.scaler.read();
        if ((w_req_valid.read() == 1) && (w_req_write.read() == 1)) {
            v.scaler = wb_req_wdata.read()(30, 0);
            v.scaler_cnt = 0;
        }
        break;
    case 0x002:                                             // 0x08: reserved (watchdog)
        vb_rdata(15, 0) = r.wdog.read();
        if ((w_req_valid.read() == 1) && (w_req_write.read() == 1)) {
            v.wdog = wb_req_wdata.read()(15, 0);
        }
        break;
    case 0x011:                                             // 0x44: reserved 4 (txctrl)
        vb_rdata[0] = i_detected.read();                    // [0] sd card inserted
        vb_rdata[1] = i_protect.read();                     // [1] write protect
        vb_rdata[2] = i_miso.read();                        // [2] miso data bit
        vb_rdata(6, 4) = r.state.read();                    // [6:4] state machine
        vb_rdata[7] = r.generate_crc.read();                // [7] Compute and generate CRC as the last Tx byte
        vb_rdata[8] = r.rx_ena.read();                      // [8] Receive data and write into FIFO only if rx_synced
        vb_rdata[9] = r.rx_synced.read();                   // [9] rx_ena=1 and start bit received
        vb_rdata[10] = r.rx_data_block.read();              // [10] rx_data_block=1 receive certain template byte
        vb_rdata(31, 16) = r.ena_byte_cnt.read();           // [31:16] Number of bytes to transmit
        if ((w_req_valid.read() == 1) && (w_req_write.read() == 1)) {
            v.generate_crc = wb_req_wdata.read()[7];
            v.rx_ena = wb_req_wdata.read()[8];
            v.rx_synced = wb_req_wdata.read()[9];
            v.rx_data_block = wb_req_wdata.read()[10];
            v.ena_byte_cnt = wb_req_wdata.read()(31, 16);
        }
        break;
    case 0x012:                                             // 0x48: Tx FIFO Data
        vb_rdata[31] = wb_txfifo_count.read().and_reduce();
        if (w_req_valid.read() == 1) {
            if (w_req_write.read() == 1) {
                v_txfifo_we = 1;
                vb_txfifo_wdata = wb_req_wdata.read()(7, 0);
            }
        }
        break;
    case 0x013:                                             // 0x4C: Rx FIFO Data
        vb_rdata(7, 0) = wb_rxfifo_rdata.read();
        vb_rdata[31] = (!wb_rxfifo_count.read().or_reduce());
        if (w_req_valid.read() == 1) {
            if (w_req_write.read() == 1) {
                // do nothing:
            } else {
                v_rxfifo_re = 1;
            }
        }
        break;
    case 0x014:                                             // 0x50: Tx FIFO Watermark
        vb_rdata((log2_fifosz - 1), 0) = r.txmark.read();
        if (w_req_valid.read() == 1) {
            if (w_req_write.read() == 1) {
                v.txmark = wb_req_wdata.read()((log2_fifosz - 1), 0);
            }
        }
        break;
    case 0x015:                                             // 0x54: Rx FIFO Watermark
        vb_rdata((log2_fifosz - 1), 0) = r.rxmark.read();
        if (w_req_valid.read() == 1) {
            if (w_req_write.read() == 1) {
                v.rxmark = wb_req_wdata.read()((log2_fifosz - 1), 0);
            }
        }
        break;
    case 0x016:                                             // 0x58: CRC16 value (reserved FU740)
        vb_rdata(15, 0) = r.crc16.read();
        if (w_req_valid.read() == 1) {
            if (w_req_write.read() == 1) {
                v.crc16 = wb_req_wdata.read()(15, 0);
            }
        }
        break;
    default:
        break;
    }

    w_rxfifo_we = r.rx_ready.read();
    wb_rxfifo_wdata = r.rx_val.read();
    w_rxfifo_re = v_rxfifo_re;

    w_txfifo_we = v_txfifo_we;
    wb_txfifo_wdata = vb_txfifo_wdata;
    w_txfifo_re = v_txfifo_re;

    v.resp_valid = w_req_valid.read();
    v.resp_rdata = vb_rdata;
    v.resp_err = 0;

    if ((!async_reset_) && (i_nrst.read() == 0)) {
        apb_spi_r_reset(v);
    }

    o_sclk = (r.level.read() & r.cs.read());
    o_mosi = (r.rx_ena.read() || r.shiftreg.read()[7]);
    o_cs = (!r.cs.read());
}

template<int log2_fifosz>
void apb_spi<log2_fifosz>::registers() {
    if ((async_reset_ == 1) && (i_nrst.read() == 0)) {
        apb_spi_r_reset(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

