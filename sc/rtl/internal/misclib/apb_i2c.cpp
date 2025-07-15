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

#include "apb_i2c.h"
#include "api_core.h"

namespace debugger {

apb_i2c::apb_i2c(sc_module_name name,
                 bool async_reset)
    : sc_module(name),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_mapinfo("i_mapinfo"),
    o_cfg("o_cfg"),
    i_apbi("i_apbi"),
    o_apbo("o_apbo"),
    o_scl("o_scl"),
    o_sda("o_sda"),
    o_sda_dir("o_sda_dir"),
    i_sda("i_sda"),
    o_irq("o_irq"),
    o_nreset("o_nreset") {

    async_reset_ = async_reset;
    pslv0 = 0;

    pslv0 = new apb_slv("pslv0",
                         async_reset,
                         VENDOR_OPTIMITECH,
                         OPTIMITECH_I2C);
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
    sensitive << i_sda;
    sensitive << w_req_valid;
    sensitive << wb_req_addr;
    sensitive << w_req_write;
    sensitive << wb_req_wdata;
    sensitive << r.scaler;
    sensitive << r.scaler_cnt;
    sensitive << r.setup_time;
    sensitive << r.level;
    sensitive << r.addr;
    sensitive << r.R_nW;
    sensitive << r.payload;
    sensitive << r.state;
    sensitive << r.start;
    sensitive << r.sda_dir;
    sensitive << r.shiftreg;
    sensitive << r.rxbyte;
    sensitive << r.bit_cnt;
    sensitive << r.byte_cnt;
    sensitive << r.ack;
    sensitive << r.err_ack_header;
    sensitive << r.err_ack_data;
    sensitive << r.irq;
    sensitive << r.ie;
    sensitive << r.nreset;
    sensitive << r.resp_valid;
    sensitive << r.resp_rdata;
    sensitive << r.resp_err;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
}

apb_i2c::~apb_i2c() {
    if (pslv0) {
        delete pslv0;
    }
}

void apb_i2c::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, i_apbi, i_apbi.name());
        sc_trace(o_vcd, o_apbo, o_apbo.name());
        sc_trace(o_vcd, o_scl, o_scl.name());
        sc_trace(o_vcd, o_sda, o_sda.name());
        sc_trace(o_vcd, o_sda_dir, o_sda_dir.name());
        sc_trace(o_vcd, i_sda, i_sda.name());
        sc_trace(o_vcd, o_irq, o_irq.name());
        sc_trace(o_vcd, o_nreset, o_nreset.name());
        sc_trace(o_vcd, r.scaler, pn + ".r.scaler");
        sc_trace(o_vcd, r.scaler_cnt, pn + ".r.scaler_cnt");
        sc_trace(o_vcd, r.setup_time, pn + ".r.setup_time");
        sc_trace(o_vcd, r.level, pn + ".r.level");
        sc_trace(o_vcd, r.addr, pn + ".r.addr");
        sc_trace(o_vcd, r.R_nW, pn + ".r.R_nW");
        sc_trace(o_vcd, r.payload, pn + ".r.payload");
        sc_trace(o_vcd, r.state, pn + ".r.state");
        sc_trace(o_vcd, r.start, pn + ".r.start");
        sc_trace(o_vcd, r.sda_dir, pn + ".r.sda_dir");
        sc_trace(o_vcd, r.shiftreg, pn + ".r.shiftreg");
        sc_trace(o_vcd, r.rxbyte, pn + ".r.rxbyte");
        sc_trace(o_vcd, r.bit_cnt, pn + ".r.bit_cnt");
        sc_trace(o_vcd, r.byte_cnt, pn + ".r.byte_cnt");
        sc_trace(o_vcd, r.ack, pn + ".r.ack");
        sc_trace(o_vcd, r.err_ack_header, pn + ".r.err_ack_header");
        sc_trace(o_vcd, r.err_ack_data, pn + ".r.err_ack_data");
        sc_trace(o_vcd, r.irq, pn + ".r.irq");
        sc_trace(o_vcd, r.ie, pn + ".r.ie");
        sc_trace(o_vcd, r.nreset, pn + ".r.nreset");
        sc_trace(o_vcd, r.resp_valid, pn + ".r.resp_valid");
        sc_trace(o_vcd, r.resp_rdata, pn + ".r.resp_rdata");
        sc_trace(o_vcd, r.resp_err, pn + ".r.resp_err");
    }

    if (pslv0) {
        pslv0->generateVCD(i_vcd, o_vcd);
    }
}

void apb_i2c::comb() {
    bool v_change_data;
    bool v_latch_data;
    sc_uint<32> vb_rdata;

    v = r;
    v_change_data = 0;
    v_latch_data = 0;
    vb_rdata = 0;

    // system bus clock scaler to baudrate:
    if (r.scaler.read().or_reduce() == 1) {
        if (r.state.read().or_reduce() == 0) {
            v.scaler_cnt = 0;
            v.level = 1;
        } else if (r.scaler_cnt.read() == (r.scaler.read() - 1)) {
            v.scaler_cnt = 0;
            v.level = (!r.level.read());
        } else {
            // The data on the SDA line must remain stable during the
            // HIGH period of the clock pulse.
            v.scaler_cnt = (r.scaler_cnt.read() + 1);
            if (r.scaler_cnt.read() == r.setup_time.read()) {
                v_change_data = (!r.level.read());
                v_latch_data = r.level.read();
            }
        }
    }

    if (v_change_data == 1) {
        v.shiftreg = ((r.shiftreg.read()(17, 0) << 1) | 1);
    }
    if (v_latch_data == 1) {
        v.rxbyte = (r.rxbyte.read()(6, 0), i_sda.read());
    }

    // Transmitter's state machine:
    switch (r.state.read()) {
    case STATE_IDLE:
        v.start = 0;
        v.shiftreg = ~0ull;
        v.sda_dir = PIN_DIR_OUTPUT;
        if (r.start.read() == 1) {
            // Start condition SDA goes LOW while SCL is HIGH
            v.shiftreg = (0,
                    r.addr.read(),
                    r.R_nW.read(),
                    0,
                    r.payload.read()(7, 0),
                    1);
            v.payload = (0, r.payload.read()(31, 8));
            v.state = STATE_START;
        }
        break;

    case STATE_START:
        if (v_change_data == 1) {
            v.bit_cnt = 7;
            v.state = STATE_HEADER;
        }
        break;

    case STATE_HEADER:
        if (v_change_data == 1) {
            if (r.bit_cnt.read().or_reduce() == 0) {
                v.sda_dir = PIN_DIR_INPUT;
                v.state = STATE_ACK_HEADER;
            } else {
                v.bit_cnt = (r.bit_cnt.read() - 1);
            }
        }
        break;

    case STATE_ACK_HEADER:
        if (v_latch_data == 1) {
            v.ack = i_sda.read();
        }
        if (v_change_data == 1) {
            v.sda_dir = PIN_DIR_OUTPUT;
            if (r.ack.read() == 0) {
                v.bit_cnt = 7;
                if (r.R_nW.read() == 1) {
                    v.state = STATE_RX_DATA;
                    v.sda_dir = PIN_DIR_INPUT;
                } else {
                    v.state = STATE_TX_DATA;
                }
            } else {
                v.err_ack_header = 1;
                v.state = STATE_STOP;
            }
        }
        break;

    case STATE_RX_DATA:
        if (v_change_data == 1) {
            if (r.bit_cnt.read().or_reduce() == 0) {
                v.sda_dir = PIN_DIR_OUTPUT;
                v.byte_cnt = (r.byte_cnt.read() - 1);
                v.payload = (r.payload.read()(23, 0), r.rxbyte.read());
                // A master receiver must signal an end of data to the
                // transmitter by not generating ACK on the last byte
                if (r.byte_cnt.read()(3, 1).or_reduce() == 1) {
                    v.shiftreg = 0;
                } else {
                    v.shiftreg = ~0ull;
                }
                v.state = STATE_ACK_DATA;
            } else {
                v.bit_cnt = (r.bit_cnt.read() - 1);
            }
        }
        break;

    case STATE_ACK_DATA:
        if (v_change_data == 1) {
            if (r.byte_cnt.read().or_reduce() == 0) {
                v.state = STATE_STOP;
            } else {
                v.bit_cnt = 7;
                v.sda_dir = PIN_DIR_INPUT;
                v.state = STATE_RX_DATA;
            }
        }
        break;

    case STATE_TX_DATA:
        if (v_change_data == 1) {
            if (r.bit_cnt.read().or_reduce() == 0) {
                v.shiftreg = 0;                             // set LOW to generate STOP ocndition if last byte
                v.sda_dir = PIN_DIR_INPUT;
                v.state = STATE_WAIT_ACK_DATA;
                if (r.byte_cnt.read().or_reduce() == 1) {
                    v.byte_cnt = (r.byte_cnt.read() - 1);
                }
            } else {
                v.bit_cnt = (r.bit_cnt.read() - 1);
            }
        }
        break;

    case STATE_WAIT_ACK_DATA:
        if (v_latch_data == 1) {
            v.ack = i_sda.read();
        }
        if (v_change_data == 1) {
            v.sda_dir = PIN_DIR_OUTPUT;
            if ((r.ack.read() == 1) || (r.byte_cnt.read().or_reduce() == 0)) {
                v.err_ack_data = r.ack.read();
                v.state = STATE_STOP;
            } else {
                v.bit_cnt = 7;
                v.shiftreg = ((r.payload.read()(7, 0) << 11) | 0x7FF);
                v.payload = (0, r.payload.read()(31, 8));
                v.state = STATE_TX_DATA;
            }
        }
        break;

    case STATE_STOP:
        if (v_latch_data == 1) {
            v.shiftreg = ~0ull;
            v.state = STATE_IDLE;
            v.irq = r.ie.read();
        }
        break;

    default:
        v.state = STATE_IDLE;
        break;
    }

    // Registers access:
    switch (wb_req_addr.read()(11, 2)) {
    case 0x000:                                             // 0x00: scldiv
        vb_rdata = r.scaler.read();
        if ((w_req_valid.read() == 1) && (w_req_write.read() == 1)) {
            v.scaler = wb_req_wdata.read()(15, 0);
            v.setup_time = wb_req_wdata.read()(31, 16);
            v.scaler_cnt = 0;
        }
        break;
    case 0x001:                                             // 0x04: control and status
        vb_rdata(7, 0) = r.state.read();                    // [7:0] state machine
        vb_rdata[8] = i_sda.read();                         // [8] input SDA data bit
        vb_rdata[9] = r.err_ack_header.read();
        vb_rdata[10] = r.err_ack_data.read();
        vb_rdata[12] = r.ie.read();                         // [12] Interrupt enable bit: 1=enabled
        vb_rdata[13] = r.irq.read();                        // [13] Interrupt pending bit. Clear on read.
        vb_rdata[16] = r.nreset.read();                     // [16] 0=unchanged; 1=set HIGH nreset
        vb_rdata[17] = r.nreset.read();                     // [17] 0=unchanged; 1=set LOW nreset
        if (w_req_valid.read() == 1) {
            v.irq = 0;                                      // Reset irq on read
            if (w_req_write.read() == 1) {
                v.err_ack_header = 0;
                v.err_ack_data = 0;
                v.ie = wb_req_wdata.read()[12];
                v.irq = wb_req_wdata.read()[13];
                if (wb_req_wdata.read()[16] == 1) {
                    v.nreset = 1;
                } else if (wb_req_wdata.read()[17] == 1) {
                    v.nreset = 0;
                }
            }
        }
        break;
    case 0x002:                                             // 0x8: Addr
        vb_rdata[31] = r.R_nW.read();
        vb_rdata(19, 16) = r.byte_cnt.read();
        vb_rdata(6, 0) = r.addr.read();
        if (w_req_valid.read() == 1) {
            if (w_req_write.read() == 1) {
                v.R_nW = wb_req_wdata.read()[31];
                v.byte_cnt = wb_req_wdata.read()(19, 16);
                v.addr = wb_req_wdata.read()(6, 0);
                v.start = 1;
            }
        }
        break;
    case 0x003:                                             // 0xC: Payload
        vb_rdata = r.payload.read();
        if (w_req_valid.read() == 1) {
            if (w_req_write.read() == 1) {
                v.payload = wb_req_wdata.read();
            }
        }
        break;
    default:
        break;
    }

    v.resp_valid = w_req_valid.read();
    v.resp_rdata = vb_rdata;
    v.resp_err = 0;

    if ((!async_reset_) && (i_nrst.read() == 0)) {
        apb_i2c_r_reset(v);
    }

    o_scl = r.level.read();
    o_sda = r.shiftreg.read()[18];
    o_sda_dir = r.sda_dir.read();
    o_nreset = r.nreset.read();
}

void apb_i2c::registers() {
    if ((async_reset_ == 1) && (i_nrst.read() == 0)) {
        apb_i2c_r_reset(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

