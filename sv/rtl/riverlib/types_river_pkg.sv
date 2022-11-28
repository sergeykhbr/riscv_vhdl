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
package types_river_pkg;

import river_cfg_pkg::*;
import types_amba_pkg::*;



// Debug interface
typedef struct {
    logic haltreq;
    logic resumereq;
    logic resethaltreq;
    logic hartreset;
    logic req_valid;
    logic [DPortReq_Total-1:0] dtype;
    logic [RISCV_ARCH-1:0] addr;
    logic [RISCV_ARCH-1:0] wdata;
    logic [2:0] size;
    logic resp_ready;
} dport_in_type;

const dport_in_type dport_in_none = '{
    1'h0,  // haltreq
    1'h0,  // resumereq
    1'h0,  // resethaltreq
    1'h0,  // hartreset
    1'h0,  // req_valid
    '0,  // dtype
    '0,  // addr
    '0,  // wdata
    '0,  // size
    1'h0  // resp_ready
};

typedef struct {
    logic req_ready;                                        // ready to accept request
    logic resp_valid;                                       // rdata is valid
    logic resp_error;                                       // response error
    logic [RISCV_ARCH-1:0] rdata;
} dport_out_type;

const dport_out_type dport_out_none = '{
    1'h1,  // req_ready
    1'h1,  // resp_valid
    1'h0,  // resp_error
    '0  // rdata
};

typedef dport_in_type dport_in_vector[0:CFG_CPU_MAX - 1];
typedef dport_out_type dport_out_vector[0:CFG_CPU_MAX - 1];

typedef logic hart_signal_vector[0:CFG_CPU_MAX - 1];
typedef logic [IRQ_TOTAL-1:0] hart_irq_vector[0:CFG_CPU_MAX - 1];

// L1 AXI interface
typedef struct {
    logic aw_valid;
    axi4_metadata_type aw_bits;
    logic [CFG_CPU_ID_BITS-1:0] aw_id;
    logic [CFG_SYSBUS_USER_BITS-1:0] aw_user;
    logic w_valid;
    logic [L1CACHE_LINE_BITS-1:0] w_data;
    logic w_last;
    logic [L1CACHE_BYTES_PER_LINE-1:0] w_strb;
    logic [CFG_CPU_USER_BITS-1:0] w_user;
    logic b_ready;
    logic ar_valid;
    axi4_metadata_type ar_bits;
    logic [CFG_CPU_ID_BITS-1:0] ar_id;
    logic [CFG_SYSBUS_USER_BITS-1:0] ar_user;
    logic r_ready;
    // ACE signals;
    logic [1:0] ar_domain;                                  // 00=Non-shareable (single master in domain)
    logic [3:0] ar_snoop;                                   // Table C3-7:
    logic [1:0] ar_bar;                                     // read barrier transaction
    logic [1:0] aw_domain;
    logic [2:0] aw_snoop;                                   // Table C3-8
    logic [1:0] aw_bar;                                     // write barrier transaction
    logic ac_ready;
    logic cr_valid;
    logic [4:0] cr_resp;
    logic cd_valid;
    logic [L1CACHE_LINE_BITS-1:0] cd_data;
    logic cd_last;
    logic rack;
    logic wack;
} axi4_l1_out_type;

const axi4_l1_out_type axi4_l1_out_none = '{
    1'h0,  // aw_valid
    META_NONE,  // aw_bits
    1'h0,  // aw_id
    1'h0,  // aw_user
    1'h0,  // w_valid
    '0,  // w_data
    1'h0,  // w_last
    '0,  // w_strb
    1'h0,  // w_user
    1'h0,  // b_ready
    1'h0,  // ar_valid
    META_NONE,  // ar_bits
    1'h0,  // ar_id
    1'h0,  // ar_user
    1'h0,  // r_ready
    '0,  // ar_domain
    '0,  // ar_snoop
    '0,  // ar_bar
    '0,  // aw_domain
    '0,  // aw_snoop
    '0,  // aw_bar
    1'h1,  // ac_ready
    1'h1,  // cr_valid
    '0,  // cr_resp
    1'h0,  // cd_valid
    '0,  // cd_data
    1'h0,  // cd_last
    1'h0,  // rack
    1'h0  // wack
};

typedef struct {
    logic aw_ready;
    logic w_ready;
    logic b_valid;
    logic [1:0] b_resp;
    logic [CFG_CPU_ID_BITS-1:0] b_id;
    logic [CFG_SYSBUS_USER_BITS-1:0] b_user;
    logic ar_ready;
    logic r_valid;
    logic [3:0] r_resp;
    logic [L1CACHE_LINE_BITS-1:0] r_data;
    logic r_last;
    logic [CFG_CPU_ID_BITS-1:0] r_id;
    logic [CFG_SYSBUS_USER_BITS-1:0] r_user;
    logic ac_valid;
    logic [CFG_CPU_ADDR_BITS-1:0] ac_addr;
    logic [3:0] ac_snoop;                                   // Table C3-19
    logic [2:0] ac_prot;
    logic cr_ready;
    logic cd_ready;
} axi4_l1_in_type;

const axi4_l1_in_type axi4_l1_in_none = '{
    1'h0,  // aw_ready
    1'h0,  // w_ready
    1'h0,  // b_valid
    '0,  // b_resp
    1'h0,  // b_id
    1'h0,  // b_user
    1'h0,  // ar_ready
    1'h0,  // r_valid
    '0,  // r_resp
    '0,  // r_data
    1'h0,  // r_last
    1'h0,  // r_id
    1'h0,  // r_user
    1'h0,  // ac_valid
    '0,  // ac_addr
    '0,  // ac_snoop
    '0,  // ac_prot
    1'h1,  // cr_ready
    1'h1  // cd_ready
};

typedef axi4_l1_in_type axi4_l1_in_vector[0:CFG_SLOT_L1_TOTAL - 1];
typedef axi4_l1_out_type axi4_l1_out_vector[0:CFG_SLOT_L1_TOTAL - 1];

typedef struct {
    logic aw_valid;
    axi4_metadata_type aw_bits;
    logic [CFG_CPU_ID_BITS-1:0] aw_id;
    logic [CFG_SYSBUS_USER_BITS-1:0] aw_user;
    logic w_valid;
    logic [L2CACHE_LINE_BITS-1:0] w_data;
    logic w_last;
    logic [L2CACHE_BYTES_PER_LINE-1:0] w_strb;
    logic [CFG_CPU_USER_BITS-1:0] w_user;
    logic b_ready;
    logic ar_valid;
    axi4_metadata_type ar_bits;
    logic [CFG_CPU_ID_BITS-1:0] ar_id;
    logic [CFG_SYSBUS_USER_BITS-1:0] ar_user;
    logic r_ready;
} axi4_l2_out_type;

const axi4_l2_out_type axi4_l2_out_none = '{
    1'h0,  // aw_valid
    META_NONE,  // aw_bits
    1'h0,  // aw_id
    1'h0,  // aw_user
    1'h0,  // w_valid
    '0,  // w_data
    1'h0,  // w_last
    '0,  // w_strb
    1'h0,  // w_user
    1'h0,  // b_ready
    1'h0,  // ar_valid
    META_NONE,  // ar_bits
    1'h0,  // ar_id
    1'h0,  // ar_user
    1'h0  // r_ready
};

typedef struct {
    logic aw_ready;
    logic w_ready;
    logic b_valid;
    logic [1:0] b_resp;
    logic [CFG_CPU_ID_BITS-1:0] b_id;                       // create ID for L2?
    logic [CFG_SYSBUS_USER_BITS-1:0] b_user;
    logic ar_ready;
    logic r_valid;
    logic [1:0] r_resp;
    logic [L2CACHE_LINE_BITS-1:0] r_data;
    logic r_last;
    logic [CFG_CPU_ID_BITS-1:0] r_id;
    logic [CFG_SYSBUS_USER_BITS-1:0] r_user;
} axi4_l2_in_type;

const axi4_l2_in_type axi4_l2_in_none = '{
    1'h0,  // aw_ready
    1'h0,  // w_ready
    1'h0,  // b_valid
    '0,  // b_resp
    1'h0,  // b_id
    1'h0,  // b_user
    1'h0,  // ar_ready
    1'h0,  // r_valid
    '0,  // r_resp
    '0,  // r_data
    1'h0,  // r_last
    1'h0,  // r_id
    1'h0  // r_user
};


endpackage: types_river_pkg
