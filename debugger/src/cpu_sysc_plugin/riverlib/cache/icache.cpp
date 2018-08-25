/*
 *  Copyright 2018 Sergey Khabarov, sergeykhbr@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include "icache.h"

namespace debugger {

ICache::ICache(sc_module_name name_) : sc_module(name_) {
    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_req_ctrl_valid;
    sensitive << i_req_ctrl_addr;
    sensitive << i_resp_mem_data_valid;
    sensitive << i_resp_mem_data;
    sensitive << i_resp_ctrl_ready;
    sensitive << i_req_mem_ready;
    sensitive << r.iline[0].addr;
    sensitive << r.iline[0].data;
    sensitive << r.iline[1].addr;
    sensitive << r.iline[1].data;
    sensitive << r.iline_addr_req;
    sensitive << r.addr_processing;
    sensitive << r.double_req;
    sensitive << r.delay_valid;
    sensitive << r.delay_data;
    sensitive << r.state;

    SC_METHOD(registers);
    sensitive << i_clk.pos();
};

void ICache::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_nrst, "/top/cache0/i0/i_nrst");
        sc_trace(o_vcd, i_req_ctrl_valid, "/top/cache0/i0/i_req_ctrl_valid");
        sc_trace(o_vcd, i_req_ctrl_addr, "/top/cache0/i0/i_req_ctrl_addr");
        sc_trace(o_vcd, o_req_ctrl_ready, "/top/cache0/i0/o_req_ctrl_ready");
        sc_trace(o_vcd, o_req_mem_valid, "/top/cache0/i0/o_req_mem_valid");
        sc_trace(o_vcd, o_req_mem_addr, "/top/cache0/i0/o_req_mem_addr");
        sc_trace(o_vcd, i_req_mem_ready, "/top/cache0/i0/i_req_mem_ready");
        sc_trace(o_vcd, i_resp_mem_data_valid, "/top/cache0/i0/i_resp_mem_data_valid");
        sc_trace(o_vcd, i_resp_mem_data, "/top/cache0/i0/i_resp_mem_data");
        sc_trace(o_vcd, o_resp_ctrl_valid, "/top/cache0/i0/o_resp_ctrl_valid");
        sc_trace(o_vcd, i_resp_ctrl_ready, "/top/cache0/i0/i_resp_ctrl_ready");
        sc_trace(o_vcd, o_resp_ctrl_addr, "/top/cache0/i0/o_resp_ctrl_addr");
        sc_trace(o_vcd, o_resp_ctrl_data, "/top/cache0/i0/o_resp_ctrl_data");
        sc_trace(o_vcd, r.iline[0].addr, "/top/cache0/i0/r_iline(0)_addr");
        sc_trace(o_vcd, r.iline[0].data, "/top/cache0/i0/r_iline(0)_data");
        sc_trace(o_vcd, r.iline[1].addr, "/top/cache0/i0/r_iline(1)_addr");
        sc_trace(o_vcd, r.iline[1].data, "/top/cache0/i0/r_iline(1)_data");
        sc_trace(o_vcd, r.iline_addr_req, "/top/cache0/i0/r_iline_addr_req");
        sc_trace(o_vcd, r.addr_processing, "/top/cache0/i0/r_addr_processing");
        sc_trace(o_vcd, r.double_req, "/top/cache0/i0/double_req");
        sc_trace(o_vcd, r.delay_valid, "/top/cache0/i0/r_delay_valid");
        sc_trace(o_vcd, r.delay_data, "/top/cache0/i0/r_delay_valid");
        sc_trace(o_vcd, r.state, "/top/cache0/i0/r_state");
        sc_trace(o_vcd, w_need_mem_req, "/top/cache0/i0/w_need_mem_req");
        sc_trace(o_vcd, wb_l[0].hit, "/top/cache0/i0/wb_l(0)_hit");
        sc_trace(o_vcd, wb_l[1].hit, "/top/cache0/i0/wb_l(1)_hit");
        sc_trace(o_vcd, wb_l[0].hit_data, "/top/cache0/i0/wb_l(0)_hit_data");
        sc_trace(o_vcd, wb_l[1].hit_data, "/top/cache0/i0/wb_l(1)_hit_data");
        sc_trace(o_vcd, wb_hit_word, "/top/cache0/i0/wb_hit_word");
        sc_trace(o_vcd, wb_l[0].hit_hold, "/top/cache0/i0/wb_l(0).hit_hold");
        sc_trace(o_vcd, wb_l[1].hit_hold, "/top/cache0/i0/wb_l(1).hit_hold");
        sc_trace(o_vcd, w_reuse_lastline, "/top/cache0/i0/w_reuse_lastline");
        sc_trace(o_vcd, w_wait_response, "/top/cache0/i0/w_wait_response");
    }
}

void ICache::comb() {
    // Instruction cache:
    bool w_o_req_ctrl_ready;
    bool w_o_req_mem_valid;
    sc_uint<BUS_ADDR_WIDTH> wb_o_req_mem_addr;
    bool w_req_ctrl_valid;
    bool w_req_fire;
    bool w_o_resp_valid;
    sc_uint<BUS_ADDR_WIDTH> wb_o_resp_addr;
    sc_uint<32> wb_o_resp_data;
    sc_uint<BUS_ADDR_WIDTH> wb_req_addr[2];
    sc_uint<BUS_ADDR_WIDTH> wb_hold_addr[2];
    
    v = r;

    w_wait_response = 0;
    if (r.state.read() == State_WaitResp && i_resp_mem_data_valid.read() == 0) {
        w_wait_response = 1;
    }
    w_req_ctrl_valid = !w_wait_response 
                    && (i_req_ctrl_valid.read() || r.double_req.read());
    wb_req_addr[0] = i_req_ctrl_addr.read();
    wb_req_addr[1] = i_req_ctrl_addr.read() + 2;

    wb_hold_addr[0] = r.addr_processing.read();
    wb_hold_addr[1] = r.addr_processing.read() + 2;

    for (int i = 0; i < ILINE_TOTAL; i++) {
        wb_l[i].hit = 0;
        wb_l[i].hit_data = 0;
        if (wb_req_addr[i](BUS_ADDR_WIDTH-1, 3) == r.iline[0].addr.read()) {
            wb_l[i].hit[Hit_Line1] = w_req_ctrl_valid;
            wb_l[i].hit_data = r.iline[0].data.read();
        } else if (wb_req_addr[i](BUS_ADDR_WIDTH-1, 3) ==
                   r.iline[1].addr.read()) {
            wb_l[i].hit[Hit_Line2] = w_req_ctrl_valid;
            wb_l[i].hit_data = r.iline[1].data.read();
        } else if (wb_req_addr[i](BUS_ADDR_WIDTH-1, 3) ==
            r.iline_addr_req.read()(BUS_ADDR_WIDTH-1, 3)) {
            wb_l[i].hit[Hit_Response] = i_resp_mem_data_valid.read();
            wb_l[i].hit_data = i_resp_mem_data.read();
        }

        wb_l[i].hit_hold = 0;
        wb_l[i].hold_data = 0;
        if (wb_hold_addr[i](BUS_ADDR_WIDTH-1, 3) == r.iline[0].addr.read()) {
            wb_l[i].hit_hold[Hit_Line1] = 1;
            wb_l[i].hold_data = r.iline[0].data.read();
        } else if (wb_hold_addr[i](BUS_ADDR_WIDTH-1, 3) ==
                   r.iline[1].addr.read()) {
            wb_l[i].hit_hold[Hit_Line2] = 1;
            wb_l[i].hold_data = r.iline[1].data.read();
        } else if (wb_hold_addr[i](BUS_ADDR_WIDTH-1, 3) ==
            r.iline_addr_req.read()(BUS_ADDR_WIDTH-1, 3)) {
            wb_l[i].hold_data = i_resp_mem_data.read();
        }
    }

    wb_hit_word = 0;
    w_need_mem_req = 1;
    if (wb_l[0].hit != 0 && wb_l[1].hit != 0) {
        w_need_mem_req = 0;
    }
    switch (r.addr_processing.read()(2, 1)) {
    case 0:
        wb_hit_word = wb_l[0].hold_data(31, 0);
        break;
    case 1:
        wb_hit_word = wb_l[0].hold_data(47, 16);
        break;
    case 2:
        wb_hit_word = wb_l[0].hold_data(63, 32);
        break;
    default:
        wb_hit_word = (wb_l[1].hold_data(15, 0) << 16)
                     | wb_l[0].hold_data(63, 48);
    }

    if (w_req_ctrl_valid && !w_need_mem_req) {
        v.delay_valid = 1;
        switch (i_req_ctrl_addr.read()(2, 1)) {
        case 0:
            v.delay_data = wb_l[0].hit_data(31, 0);
            break;
        case 1:
            v.delay_data = wb_l[0].hit_data(47, 16);
            break;
        case 2:
            v.delay_data = wb_l[0].hit_data(63, 32);
            break;
        default:
            v.delay_data = (wb_l[1].hit_data(15, 0) << 16)
                            | wb_l[0].hit_data(63, 48);
        }
    } else if (i_resp_ctrl_ready.read()) {
        v.delay_valid = 0;
        v.delay_data = 0;
    }

    w_o_req_mem_valid = w_need_mem_req & w_req_ctrl_valid;
    if (r.double_req.read()) {
        if ((r.addr_processing.read()(BUS_ADDR_WIDTH-1, 3) == 
            r.iline_addr_req.read()(BUS_ADDR_WIDTH-1, 3))
            || (r.addr_processing.read()(BUS_ADDR_WIDTH-1, 3) == 
                wb_hold_addr[0](BUS_ADDR_WIDTH-1, 3))
            ) {
            wb_o_req_mem_addr = wb_hold_addr[1](BUS_ADDR_WIDTH-1, 3) << 3;
        } else {
            wb_o_req_mem_addr = wb_hold_addr[0](BUS_ADDR_WIDTH-1, 3) << 3;
        }
    } else if (wb_l[0].hit == 0) {
        wb_o_req_mem_addr = wb_req_addr[0](BUS_ADDR_WIDTH-1, 3) << 3;
    } else {
        wb_o_req_mem_addr = wb_req_addr[1](BUS_ADDR_WIDTH-1, 3) << 3;
    }

    w_o_req_ctrl_ready = !w_need_mem_req 
                       | (i_req_mem_ready.read() & !w_wait_response);
    w_req_fire = w_req_ctrl_valid && w_o_req_ctrl_ready;

    if ((w_o_req_mem_valid && i_req_mem_ready.read() && !w_wait_response)
        || r.double_req.read()) {
        v.iline_addr_req = wb_o_req_mem_addr;
    }

    switch (r.state.read()) {
    case State_Idle:
        if (w_req_ctrl_valid) {
            if (!w_need_mem_req) {
                v.state = State_WaitAccept;
            } else if (i_req_mem_ready.read()) {
                v.state = State_WaitResp;
            } else {
                v.state = State_WaitGrant;
            }
        }
        break;
    case State_WaitGrant:
        if (i_req_mem_ready.read()) {
            v.state = State_WaitResp;
        } else if (!w_need_mem_req) {
            /** Fetcher can change request address while request wasn't
             *  accepteed. */
            v.state = State_WaitAccept;
        }
        break;
    case State_WaitResp:
        if (i_resp_mem_data_valid.read()) {
            if (!i_resp_ctrl_ready.read()) {
                v.state = State_WaitAccept;
            } else if (!w_req_ctrl_valid) {
                v.state = State_Idle;
            } else {
                // New request
                if (!w_need_mem_req) {
                    v.state = State_WaitAccept;
                } else if (i_req_mem_ready.read()) {
                    v.state = State_WaitResp;
                } else {
                    v.state = State_WaitGrant;
                }
            }
        }
        break;
    case State_WaitAccept:
        if (i_resp_ctrl_ready.read()) {
            if (!w_req_ctrl_valid) {
                v.state = State_Idle;
            } else {
                if (!w_need_mem_req) {
                    v.state = State_WaitAccept;
                } else if (i_req_mem_ready.read()) {
                    v.state = State_WaitResp;
                } else {
                    v.state = State_WaitGrant;
                }
            }
        }
        break;
    default:;
    }

    if (w_req_fire) {
        v.double_req = 0;
        if (i_req_ctrl_addr.read()(2, 1) == 0x3
            && wb_l[0].hit == 0 && wb_l[1].hit == 0
            && r.double_req.read() == 0) {
            v.double_req = 1;
        }
        if (!r.double_req.read()) {
            v.addr_processing = i_req_ctrl_addr;
        }
    }

    w_reuse_lastline = 0;

    if (i_resp_mem_data_valid.read()) {
        /** Condition to avoid removing the last line:
            */
        if (i_resp_ctrl_ready.read()) {
            if ((wb_l[0].hit[Hit_Line2] || wb_l[1].hit[Hit_Line2]) == 1
                && r.iline[1].addr.read() 
                    != i_req_ctrl_addr.read()(BUS_ADDR_WIDTH-1, 3)) {
                w_reuse_lastline = w_need_mem_req;
            }
        } else {
            if ((wb_l[0].hit_hold[Hit_Line2] || wb_l[1].hit_hold[Hit_Line2]) == 1
                && (wb_l[0].hit_hold[Hit_Line1] || wb_l[1].hit_hold[Hit_Line1]) == 0
                && r.iline[1].addr.read()
                    != r.iline_addr_req.read()(BUS_ADDR_WIDTH-1, 3)) {
                w_reuse_lastline = 1;
            }
        }
        if (!w_reuse_lastline) {
            v.iline[1].addr = r.iline[0].addr;
            v.iline[1].data = r.iline[0].data;
        }
        
        v.iline[0].addr = r.iline_addr_req.read()(BUS_ADDR_WIDTH-1, 3);
        v.iline[0].data = i_resp_mem_data;
    }

    if (r.state.read() == State_WaitAccept) {
        w_o_resp_valid = !r.double_req.read();
    } else {
        w_o_resp_valid = i_resp_mem_data_valid && !r.double_req.read();
    }
    if (r.delay_valid.read()) {
        wb_o_resp_data = r.delay_data.read();
    } else {
        wb_o_resp_data = wb_hit_word;
    }
    wb_o_resp_addr = r.addr_processing;


    if (!i_nrst.read()) {
        v.iline[0].addr = ~0;
        v.iline[0].data = 0;
        v.iline[1].addr = ~0;
        v.iline[1].data = 0;
        v.iline_addr_req = 0;
        v.addr_processing = 0;
        v.state = State_Idle;
        v.double_req = 0;
        v.delay_valid = 0;
        v.delay_data = 0;
    }

    o_req_ctrl_ready = w_o_req_ctrl_ready;

    o_req_mem_valid = w_o_req_mem_valid;
    o_req_mem_addr = wb_o_req_mem_addr;
    o_req_mem_write = false;
    o_req_mem_strob = 0;
    o_req_mem_data = 0;

    o_resp_ctrl_valid = w_o_resp_valid;
    o_resp_ctrl_data = wb_o_resp_data;
    o_resp_ctrl_addr = wb_o_resp_addr;
    o_istate = r.state;
}

void ICache::registers() {
    r = v;
}

#ifdef DBG_ICACHE_TB
void ICache_tb::comb0() {
    v = r;
    v.clk_cnt = r.clk_cnt.read() + 1;

    if (r.clk_cnt.read() < 10) {
        w_nrst = 0;
        v.mem_state = 0;
        v.mem_cnt = 0;
        v.fetch_state = 0;
        v.fetch_cnt = 0;
        return;
    }
    w_nrst = 1;

    w_req_ctrl_valid = 0;
    wb_req_ctrl_addr = 0;
    w_resp_ctrl_ready = 0;

    struct FetchDelayType {
        uint32_t raddr;
        int req_wait;
        int accept_wait;
    };
    static const FetchDelayType RADDR[4] = {{0x100008f4, 0, 0}, {0x100007b0, 0, 0}, {0x100008f0, 2, 0}, {0x100007b4, 0, 0}};
    struct MemDelayType {
        unsigned rdy_wait;
        unsigned valid_wait;
    };
    //static const MemDelayType MEM_DELAY[4] = {{2,3}, {2,3}, {0,0}, {0,0}};
    static const MemDelayType MEM_DELAY[4] = {{0,0}, {0,0}, {5,0}, {5,0}};


    // fetch model:
    w_resp_ctrl_ready = 0;
    if (r.clk_cnt.read() >= 15) {
        switch (r.fetch_state.read()) {
        case 0:
            if (r.fetch_cnt.read() < 4) {
                if (RADDR[r.fetch_cnt.read()].req_wait == 0) {
                    w_req_ctrl_valid = 1;
                    wb_req_ctrl_addr = RADDR[r.fetch_cnt.read()].raddr;
                    if (w_req_ctrl_ready) {
                        v.fetch_state = 3;
                        v.fetch_cnt = r.fetch_cnt.read() + 1;
                        v.fetch_wait_cnt = RADDR[r.fetch_cnt.read()].accept_wait;
                    } else {
                        v.fetch_state = 2;
                    }
                } else {
                    if (RADDR[r.fetch_cnt.read()].req_wait == 1) {
                        v.fetch_state = 2;
                    } else {
                        v.fetch_state = 1;
                    }
                    v.fetch_wait_cnt = RADDR[r.fetch_cnt.read()].req_wait;
                }
            }
            break;
        case 1:
            // wait to request:
            v.fetch_wait_cnt = r.fetch_wait_cnt.read() - 1;
            if (r.fetch_wait_cnt.read() == 1) {
                v.fetch_state = 2;
            }
            break;
        case 2:// wait ready signal
            w_req_ctrl_valid = 1;
            wb_req_ctrl_addr = RADDR[r.fetch_cnt.read()].raddr;
            if (w_req_ctrl_ready) {
                v.fetch_state = 3;
                v.fetch_cnt = r.fetch_cnt.read() + 1;
                v.fetch_wait_cnt = RADDR[r.fetch_cnt.read()].accept_wait;
            }
            break;
        case 3: // wait valid signal:
            if (w_resp_ctrl_valid) {
                w_resp_ctrl_ready = 1;
                if (r.fetch_wait_cnt.read()) {
                    v.fetch_wait_cnt = r.fetch_wait_cnt.read() - 1;
                    w_resp_ctrl_ready = 0;
                } else if (r.fetch_cnt.read() < 4) {
                    if (RADDR[r.fetch_cnt.read()].req_wait == 0) {
                        w_req_ctrl_valid = 1;
                        wb_req_ctrl_addr = RADDR[r.fetch_cnt.read()].raddr;
                        if (w_req_ctrl_ready) {
                            v.fetch_state = 3;
                            v.fetch_cnt = r.fetch_cnt.read() + 1;
                            v.fetch_wait_cnt = RADDR[r.fetch_cnt.read()].accept_wait;
                        } else {
                            v.fetch_state = 2;
                        }
                    } else {
                        if (RADDR[r.fetch_cnt.read()].req_wait == 1) {
                            v.fetch_state = 2;
                        } else {
                            v.fetch_state = 1;
                        }
                        v.fetch_wait_cnt = RADDR[r.fetch_cnt.read()].req_wait;
                    }
                } else {
                    v.fetch_state = 0;
                }
            }
            break;
        default:;
        }
    }

    if (r.clk_cnt.read() == 21) {
        wb_req_ctrl_addr = 0x100008f8;
    } else if (r.clk_cnt.read() == 22) {
        wb_req_ctrl_addr = 0x100007b4;
    }


    // Memory model:
    w_req_mem_ready = 0;
    w_resp_mem_data_valid = 0;

    switch (r.mem_state.read()) {
    case 0: // MemIdle
        if (w_req_mem_valid && r.mem_cnt.read() < 4) {
            if (MEM_DELAY[r.mem_cnt.read()].rdy_wait == 0) {
                if (MEM_DELAY[r.mem_cnt.read()].valid_wait == 0) {
                    v.mem_state = 3;
                    v.mem_raddr = wb_req_mem_addr;
                    w_req_mem_ready = 1;
                    v.mem_cnt = r.mem_cnt.read() + 1;
                } else {
                    v.mem_state = 2;
                    v.mem_wait_cnt = MEM_DELAY[r.mem_cnt.read()].valid_wait;
                }
            } else {
                v.mem_state = 1;
                v.mem_wait_cnt = MEM_DELAY[r.mem_cnt.read()].rdy_wait;
            }
        }
        break;
    case 1: 
        v.mem_wait_cnt = r.mem_wait_cnt.read() - 1;
        if (r.mem_wait_cnt.read() == 1) {
            if (w_req_mem_valid) {
                v.mem_raddr = wb_req_mem_addr;
                w_req_mem_ready = 1;
                v.mem_cnt = r.mem_cnt.read() + 1;
                if (MEM_DELAY[r.mem_cnt.read()].valid_wait == 0) {
                    v.mem_state = 3;
                } else {
                    v.mem_state = 2;
                    v.mem_wait_cnt = MEM_DELAY[r.mem_cnt.read()].valid_wait;
                }
            } else {
                v.mem_state = 0;
            }
        }
        break;
    case 2:
        v.mem_wait_cnt = r.mem_wait_cnt.read() - 1;
        if (r.mem_wait_cnt.read() == 1) {
            v.mem_state = 3;
        }
        break;
    case 3:
        w_resp_mem_data_valid = 1;
        if (r.mem_raddr.read() == 0x100008f0) {
            wb_resp_mem_data = 0xffdff06fea9ff0efull;
        } else if (r.mem_raddr.read() == 0x100007b0) {
            wb_resp_mem_data = 0xfa0a0a1300004a17;
        }

        if (w_req_mem_valid && r.mem_cnt.read() < 4) {
            if (MEM_DELAY[r.mem_cnt.read()].rdy_wait == 0) {
                if (MEM_DELAY[r.mem_cnt.read()].valid_wait == 0) {
                    v.mem_state = 3;
                    v.mem_raddr = wb_req_mem_addr;
                    w_req_mem_ready = 1;
                    v.mem_cnt = r.mem_cnt.read() + 1;
                } else {
                    v.mem_state = 2;
                    v.mem_wait_cnt = MEM_DELAY[r.mem_cnt.read()].valid_wait;
                }
            } else {
                v.mem_state = 1;
                v.mem_wait_cnt = MEM_DELAY[r.mem_cnt.read()].rdy_wait;
            }
        } else {
            v.mem_state = 0;
        }
        break;
    default:;
    }


}
#endif

}  // namespace debugger

