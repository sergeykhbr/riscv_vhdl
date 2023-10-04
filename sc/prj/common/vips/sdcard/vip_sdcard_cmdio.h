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
#include "vip_sdcard_crc7.h"

namespace debugger {

SC_MODULE(vip_sdcard_cmdio) {
 public:
    sc_in<bool> i_nrst;
    sc_in<bool> i_clk;
    sc_in<bool> i_cs;                                       // dat3 in SPI mode.
    sc_out<bool> o_spi_mode;                                // Detected SPI mode on CMD0
    sc_in<bool> i_cmd;
    sc_out<bool> o_cmd;
    sc_out<bool> o_cmd_dir;
    sc_out<bool> o_cmd_req_valid;
    sc_out<sc_uint<6>> o_cmd_req_cmd;
    sc_out<sc_uint<32>> o_cmd_req_data;
    sc_in<bool> i_cmd_req_ready;
    sc_in<bool> i_cmd_resp_valid;
    sc_in<sc_uint<32>> i_cmd_resp_data32;
    sc_out<bool> o_cmd_resp_ready;
    sc_in<bool> i_cmd_resp_r1b;                             // Same as R1 with zero line (any number of bits) card is busy, non-zero is ready
    sc_in<bool> i_cmd_resp_r2;                              // 2-Bytes status
    sc_in<bool> i_cmd_resp_r3;                              // Read OCR 32 bits
    sc_in<bool> i_cmd_resp_r7;                              // CMD8 interface condition response
    sc_in<bool> i_stat_idle_state;                          // Card in idle state and running initialization process
    sc_in<bool> i_stat_erase_reset;                         // Erase sequence was cleared before executing
    sc_in<bool> i_stat_illegal_cmd;                         // Illegal command was detected
    sc_in<bool> i_stat_err_erase_sequence;                  // An error ini the sequence of erase commands occured
    sc_in<bool> i_stat_err_address;                         // A misaligned adddres that didnot mathc block length
    sc_in<bool> i_stat_err_parameter;                       // The command argument was otuside the allows range
    sc_in<bool> i_stat_locked;                              // Is set when card is locked by the user
    sc_in<bool> i_stat_wp_erase_skip;                       // Erase wp-sector or password error during card lock/unlock
    sc_in<bool> i_stat_err;                                 // A general or an unknown error occured during operation
    sc_in<bool> i_stat_err_cc;                              // Internal card controller error
    sc_in<bool> i_stat_ecc_failed;                          // Card internal ECC eas applied but failed to correct data
    sc_in<bool> i_stat_wp_violation;                        // The command tried to write wp block
    sc_in<bool> i_stat_erase_param;                         // An invalid selection for erase, sectors or groups
    sc_in<bool> i_stat_out_of_range;
    sc_out<bool> o_busy;

    void comb();
    void registers();

    SC_HAS_PROCESS(vip_sdcard_cmdio);

    vip_sdcard_cmdio(sc_module_name name,
                     bool async_reset);
    virtual ~vip_sdcard_cmdio();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;

    // 
    // Receiver CMD state:
    static const uint8_t CMDSTATE_REQ_STARTBIT = 0;
    static const uint8_t CMDSTATE_REQ_TXBIT = 1;
    static const uint8_t CMDSTATE_REQ_CMD = 2;
    static const uint8_t CMDSTATE_REQ_ARG = 3;
    static const uint8_t CMDSTATE_REQ_CRC7 = 4;
    static const uint8_t CMDSTATE_REQ_STOPBIT = 5;
    static const uint8_t CMDSTATE_REQ_VALID = 6;
    static const uint8_t CMDSTATE_WAIT_RESP = 7;
    static const uint8_t CMDSTATE_RESP = 8;
    static const uint8_t CMDSTATE_RESP_CRC7 = 9;
    static const uint8_t CMDSTATE_RESP_STOPBIT = 10;
    static const uint8_t CMDSTATE_INIT = 15;

    struct vip_sdcard_cmdio_registers {
        sc_signal<sc_uint<8>> clkcnt;
        sc_signal<bool> cs;
        sc_signal<bool> spi_mode;
        sc_signal<bool> cmdz;
        sc_signal<bool> cmd_dir;
        sc_signal<sc_uint<48>> cmd_rxshift;
        sc_signal<sc_uint<48>> cmd_txshift;
        sc_signal<sc_uint<4>> cmd_state;
        sc_signal<bool> cmd_req_crc_err;
        sc_signal<sc_uint<6>> bitcnt;
        sc_signal<bool> txbit;
        sc_signal<sc_uint<7>> crc_calc;
        sc_signal<sc_uint<7>> crc_rx;
        sc_signal<bool> cmd_req_valid;
        sc_signal<sc_uint<6>> cmd_req_cmd;
        sc_signal<sc_uint<32>> cmd_req_data;
        sc_signal<bool> cmd_resp_ready;
    } v, r;

    void vip_sdcard_cmdio_r_reset(vip_sdcard_cmdio_registers &iv) {
        iv.clkcnt = 0;
        iv.cs = 0;
        iv.spi_mode = 0;
        iv.cmdz = 1;
        iv.cmd_dir = 1;
        iv.cmd_rxshift = ~0ull;
        iv.cmd_txshift = ~0ull;
        iv.cmd_state = CMDSTATE_INIT;
        iv.cmd_req_crc_err = 0;
        iv.bitcnt = 0;
        iv.txbit = 0;
        iv.crc_calc = 0;
        iv.crc_rx = 0;
        iv.cmd_req_valid = 0;
        iv.cmd_req_cmd = 0;
        iv.cmd_req_data = 0;
        iv.cmd_resp_ready = 0;
    }

    sc_signal<bool> w_cmd_out;
    sc_signal<bool> w_crc7_clear;
    sc_signal<bool> w_crc7_next;
    sc_signal<bool> w_crc7_dat;
    sc_signal<sc_uint<7>> wb_crc7;

    vip_sdcard_crc7 *crccmd0;

};

}  // namespace debugger

