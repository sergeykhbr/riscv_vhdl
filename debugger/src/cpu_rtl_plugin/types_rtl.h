/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      RTL types declaration.
 */

#ifndef __DEBUGGER_SOCSIM_TYPES_RTL_H__
#define __DEBUGGER_SOCSIM_TYPES_RTL_H__

#include <inttypes.h>
#include "types_amba.h"

namespace debugger {

#define MSK32(m,l)  (((0xFFFFFFFF>>(31-(m)))>>(l))<<(l))
#define BITS32(val,m,l)  (((val)&(0xFFFFFFFF>>(31-(m))))>>(l))
#define BIT32(val,m)  (((val)>>(m))&0x1)

#define MSK64(m,l)  (((0xFFFFFFFFFFFFFFFF>>(63-(m)))>>(l))<<(l))
#define BITS64(val,m,l)  (((val)&(0xFFFFFFFFFFFFFFFF>>(63-(m))))>>(l))
#define BIT64(val,m)  (((val)>>(m))&0x1)
#define SET64(word,offset,bit) \
     (word = ((word & ~(0x1LL<<offset)) | ((bit) << offset)))

static const uint32_t ACQUIRE_GET_SINGLE_DATA_BEAT = 0x0;
static const uint32_t ACQUIRE_GET_BLOCK_DATA       = 0x1; 
static const uint32_t ACQUIRE_PUT_SINGLE_DATA_BEAT = 0x2;
static const uint32_t ACQUIRE_PUT_BLOCK_DATA       = 0x3;
static const uint32_t ACQUIRE_PUT_ATOMIC_DATA      = 0x4;
static const uint32_t ACQUIRE_PREFETCH_BLOCK       = 0x5;
  
static const uint32_t GRANT_ACK_RELEASE          = 0x0;
static const uint32_t GRANT_ACK_PREFETCH         = 0x1;
static const uint32_t GRANT_ACK_NON_PREFETCH_PUT = 0x3;
static const uint32_t GRANT_SINGLE_BEAT_GET      = 0x4;
static const uint32_t GRANT_BLOCK_GET            = 0x5;

static const uint32_t CACHED_ACQUIRE_SHARED      = 0x0;
static const uint32_t CACHED_ACQUIRE_EXCLUSIVE   = 0x1;

static const uint32_t CACHED_GRANT_SHARED        = 0x0;
static const uint32_t CACHED_GRANT_EXCLUSIVE     = 0x1;  
static const uint32_t CACHED_GRANT_EXCLUSIVE_ACK = 0x2;  

static const uint64_t MEMOP_XSIZE_TOTAL = 8;
static const uint32_t opSizeToXSize[MEMOP_XSIZE_TOTAL] = {
    0,      // MT_B => "000",  // 0=1 byte (AXI specification)
    1,      // MT_H => "001",  // 1=2 bytes (AXI specification)
    2,      // MT_W => "010",  // 2=4 bytes (AXI specification)
    3,      // MT_D => "011",  // 3=8 bytes (AXI specification)
    0,      // MT_BU => "000",
    1,      // MT_HU => "001",
    2,      // MT_WU => "010", --! unimplemented in scala
    4       // MT_Q => log2[CFG_NASTI_DATA_BYTES]
};

static const uint32_t XSizeToBytes[8] = {
    1,      // 0: 8 bits
    2,      // 1: 16 bits
    4,      // 2: 32 bits
    8,      // 3: 64 bits
    16,     // 4: 128 bits
    32,     // 5: 256 bits
    64,     // 6: 512 bits
    128     // 7: 1024 bits
};

struct tile_cached_in_type {
    uint8_t acquire_ready;
    uint8_t grant_valid;
    uint32_t grant_bits_addr_beat;
    uint32_t grant_bits_client_xact_id;
    uint32_t grant_bits_manager_xact_id;
    uint8_t grant_bits_is_builtin_type;
    uint32_t grant_bits_g_type;
    uint32_t grant_bits_data[CFG_NASTI_DATA_BITS/32];
    uint8_t probe_valid;
    uint32_t probe_bits_addr_block;
    uint32_t probe_bits_p_type;
    uint8_t release_ready;
};

struct tile_cached_out_type {
    uint8_t acquire_valid;
    uint32_t acquire_bits_addr_block;
    uint32_t acquire_bits_client_xact_id;
    uint32_t acquire_bits_addr_beat;
    uint8_t acquire_bits_is_builtin_type;
    uint32_t acquire_bits_a_type;
    uint32_t acquire_bits_union;
    uint32_t acquire_bits_data[CFG_NASTI_DATA_BITS/32];
    uint8_t grant_ready;
    uint8_t probe_ready;
    uint8_t release_valid;
    uint32_t release_bits_addr_beat;
    uint32_t release_bits_addr_block;
    uint32_t release_bits_client_xact_id;
    uint32_t release_bits_r_type;
    uint8_t release_bits_voluntary;
    uint32_t release_bits_data[CFG_NASTI_DATA_BITS/32];
};

struct host_in_type {
    uint64_t reset;
    uint64_t id;
    uint64_t csr_req_valid;
    uint64_t csr_req_bits_rw;
    uint64_t csr_req_bits_addr;
    uint64_t csr_req_bits_data;
    uint64_t csr_resp_ready;
};

struct host_out_type {
    uint64_t csr_req_ready;
    uint64_t csr_resp_valid;
    uint64_t csr_resp_bits;
    uint64_t debug_stats_csr;
};

}  // namespace debugger

#endif  // __DEBUGGER_SOCSIM_TYPES_RTL_H__
