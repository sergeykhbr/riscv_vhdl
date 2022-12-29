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

`timescale 1ns/10ps

module dmidebug #(
    parameter bit async_reset = 1'b0
)
(
    input logic i_clk,
    input logic i_nrst,                                     // full reset including dmi (usually via reset button)
    // JTAG interface:
    input logic i_trst,                                     // Test reset: must be open-drain, pullup
    input logic i_tck,                                      // Test Clock
    input logic i_tms,                                      // Test Mode State
    input logic i_tdi,                                      // Test Data Input
    output logic o_tdo,                                     // Test Data Output
    // Bus interface (APB):
    input types_amba_pkg::mapinfo_type i_mapinfo,           // interconnect slot information
    output types_amba_pkg::dev_config_type o_cfg,           // Device descriptor
    input types_amba_pkg::apb_in_type i_apbi,               // APB input interface
    output types_amba_pkg::apb_out_type o_apbo,             // APB output interface
    // DMI interface:
    output logic o_ndmreset,                                // system reset: cores + peripheries (except dmi itself)
    input logic [river_cfg_pkg::CFG_CPU_MAX-1:0] i_halted,  // Halted cores
    input logic [river_cfg_pkg::CFG_CPU_MAX-1:0] i_available,// Existing and available cores
    output logic [river_cfg_pkg::CFG_LOG2_CPU_MAX-1:0] o_hartsel,// Selected hart index
    output logic o_haltreq,
    output logic o_resumereq,
    output logic o_resethaltreq,                            // Halt core after reset request.
    output logic o_hartreset,                               // Reset currently selected hart
    output logic o_dport_req_valid,                         // Debug access from DSU is valid
    output logic [river_cfg_pkg::DPortReq_Total-1:0] o_dport_req_type,// Debug access types
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_dport_addr,// Register index
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_dport_wdata,// Write value
    output logic [2:0] o_dport_size,                        // 0=1B;1=2B;2=4B;3=8B;4=128B
    input logic i_dport_req_ready,                          // Response is ready
    output logic o_dport_resp_ready,                        // ready to accepd response
    input logic i_dport_resp_valid,                         // Response is valid
    input logic i_dport_resp_error,                         // Something goes wrong
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_dport_rdata,// Response value or error code
    output logic [(32 * river_cfg_pkg::CFG_PROGBUF_REG_TOTAL)-1:0] o_progbuf
);

import types_amba_pkg::*;
import river_cfg_pkg::*;
import dmidebug_pkg::*;

logic w_tap_dmi_req_valid;
logic w_tap_dmi_req_write;
logic [6:0] wb_tap_dmi_req_addr;
logic [31:0] wb_tap_dmi_req_data;
logic w_tap_dmi_reset;
logic w_tap_dmi_hardreset;
logic w_cdc_dmi_req_valid;
logic w_cdc_dmi_req_ready;
logic w_cdc_dmi_req_write;
logic [6:0] wb_cdc_dmi_req_addr;
logic [31:0] wb_cdc_dmi_req_data;
logic w_cdc_dmi_reset;
logic w_cdc_dmi_hardreset;
logic [31:0] wb_jtag_dmi_resp_data;
logic w_jtag_dmi_busy;
logic w_jtag_dmi_error;
dmidebug_registers r, rin;

jtagtap #(
    .idcode(32'h10e31913),
    .abits(7),
    .irlen(5)
) tap (
    .i_trst(i_trst),
    .i_tck(i_tck),
    .i_tms(i_tms),
    .i_tdi(i_tdi),
    .o_tdo(o_tdo),
    .o_dmi_req_valid(w_tap_dmi_req_valid),
    .o_dmi_req_write(w_tap_dmi_req_write),
    .o_dmi_req_addr(wb_tap_dmi_req_addr),
    .o_dmi_req_data(wb_tap_dmi_req_data),
    .i_dmi_resp_data(wb_jtag_dmi_resp_data),
    .i_dmi_busy(w_jtag_dmi_busy),
    .i_dmi_error(w_jtag_dmi_error),
    .o_dmi_reset(w_tap_dmi_reset),
    .o_dmi_hardreset(w_tap_dmi_hardreset)
);


jtagcdc #(
    .async_reset(async_reset)
) cdc (
    .i_clk(i_clk),
    .i_nrst(i_nrst),
    .i_dmi_req_valid(w_tap_dmi_req_valid),
    .i_dmi_req_write(w_tap_dmi_req_write),
    .i_dmi_req_addr(wb_tap_dmi_req_addr),
    .i_dmi_req_data(wb_tap_dmi_req_data),
    .i_dmi_reset(w_tap_dmi_reset),
    .i_dmi_hardreset(w_tap_dmi_hardreset),
    .i_dmi_req_ready(w_cdc_dmi_req_ready),
    .o_dmi_req_valid(w_cdc_dmi_req_valid),
    .o_dmi_req_write(w_cdc_dmi_req_write),
    .o_dmi_req_addr(wb_cdc_dmi_req_addr),
    .o_dmi_req_data(wb_cdc_dmi_req_data),
    .o_dmi_reset(w_cdc_dmi_reset),
    .o_dmi_hardreset(w_cdc_dmi_hardreset)
);


always_comb
begin: comb_proc
    dmidebug_registers v;
    dev_config_type vcfg;
    apb_out_type vapbo;
    logic [DPortReq_Total-1:0] vb_req_type;
    logic [31:0] vb_resp_data;
    logic [CFG_LOG2_CPU_MAX-1:0] vb_hartselnext;
    logic v_resp_valid;
    int hsel;
    logic v_cmd_busy;
    logic v_cdc_dmi_req_ready;
    logic [63:0] vb_arg1;
    logic [31:0] t_command;
    logic [(32 * CFG_PROGBUF_REG_TOTAL)-1:0] t_progbuf;
    int t_idx;

    vcfg = dev_config_none;
    vapbo = apb_out_none;
    vb_req_type = 0;
    vb_resp_data = 0;
    vb_hartselnext = 0;
    v_resp_valid = 0;
    hsel = 0;
    v_cmd_busy = 0;
    v_cdc_dmi_req_ready = 0;
    vb_arg1 = 0;
    t_command = 0;
    t_progbuf = 0;
    t_idx = 0;

    v = r;

    vcfg.descrsize = PNP_CFG_DEV_DESCR_BYTES;
    vcfg.descrtype = PNP_CFG_TYPE_SLAVE;
    vcfg.addr_start = i_mapinfo.addr_start;
    vcfg.addr_end = i_mapinfo.addr_end;
    vcfg.vid = VENDOR_OPTIMITECH;
    vcfg.did = OPTIMITECH_RIVER_DMI;

    vb_hartselnext = r.wdata[((16 + CFG_LOG2_CPU_MAX) - 1): 16];
    hsel = int'(r.hartsel);
    v_cmd_busy = (|r.cmdstate);
    vb_arg1 = {r.data3, r.data2};
    t_command = r.command;
    t_progbuf = r.progbuf_data;
    t_idx = int'(r.regidx[3: 0]);

    if ((r.haltreq & i_halted[hsel]) == 1'b1) begin
        v.haltreq = 1'b0;
    end
    if ((r.resumereq & (~i_halted[hsel])) == 1'b1) begin
        v.resumereq = 1'b0;
        v.resumeack = 1'b1;
    end

    case (r.dmstate)
    DM_STATE_IDLE: begin
        if (w_cdc_dmi_req_valid == 1'b1) begin
            v_cdc_dmi_req_ready = 1'b1;
            v.bus_jtag = 1'b1;
            v.dmstate = DM_STATE_ACCESS;
            v.regidx = wb_cdc_dmi_req_addr;
            v.wdata = wb_cdc_dmi_req_data;
            v.regwr = w_cdc_dmi_req_write;
            v.regrd = (~w_cdc_dmi_req_write);
        end else if (((i_apbi.pselx == 1'b1) && (i_apbi.pwrite == 1'b0))
                    || ((i_apbi.pselx == 1'b1) && (i_apbi.penable == 1'b1) && (i_apbi.pwrite == 1'b1))) begin
            v.bus_jtag = 1'b0;
            v.dmstate = DM_STATE_ACCESS;
            v.regidx = i_apbi.paddr[6: 0];
            v.wdata = i_apbi.pwdata;
            v.regwr = i_apbi.pwrite;
            v.regrd = (~i_apbi.pwrite);
        end
    end
    DM_STATE_ACCESS: begin
        v_resp_valid = 1'b1;
        v.dmstate = DM_STATE_IDLE;
        if (r.regidx == 7'h04) begin                        // arg0[31:0]
            vb_resp_data = r.data0;
            if (r.regwr == 1'b1) begin
                v.data0 = r.wdata;
            end
            if ((r.autoexecdata[0] == 1'b1) && (r.cmderr == CMDERR_NONE)) begin
                if (v_cmd_busy == 1'b1) begin
                    v.cmderr = CMDERR_BUSY;
                end else begin
                    v.cmdstate = CMD_STATE_INIT;
                end
            end
        end else if (r.regidx == 7'h05) begin               // arg0[63:32]
            vb_resp_data = r.data1;
            if (r.regwr == 1'b1) begin
                v.data1 = r.wdata;
            end
            if ((r.autoexecdata[1] == 1'b1) && (r.cmderr == CMDERR_NONE)) begin
                if (v_cmd_busy == 1'b1) begin
                    v.cmderr = CMDERR_BUSY;
                end else begin
                    v.cmdstate = CMD_STATE_INIT;
                end
            end
        end else if (r.regidx == 7'h06) begin               // arg1[31:0]
            vb_resp_data = r.data2;
            if (r.regwr == 1'b1) begin
                v.data2 = r.wdata;
            end
            if ((r.autoexecdata[2] == 1'b1) && (r.cmderr == CMDERR_NONE)) begin
                if (v_cmd_busy == 1'b1) begin
                    v.cmderr = CMDERR_BUSY;
                end else begin
                    v.cmdstate = CMD_STATE_INIT;
                end
            end
        end else if (r.regidx == 7'h07) begin               // arg1[63:32]
            vb_resp_data = r.data3;
            if (r.regwr == 1'b1) begin
                v.data3 = r.wdata;
            end
            if ((r.autoexecdata[3] == 1'b1) && (r.cmderr == CMDERR_NONE)) begin
                if (v_cmd_busy == 1'b1) begin
                    v.cmderr = CMDERR_BUSY;
                end else begin
                    v.cmdstate = CMD_STATE_INIT;
                end
            end
        end else if (r.regidx == 7'h10) begin               // dmcontrol
            vb_resp_data[29] = r.hartreset;                 // hartreset
            vb_resp_data[28] = 1'b0;                        // ackhavereset
            vb_resp_data[26] = 1'b0;                        // hasel: single selected hart only
            vb_resp_data[((16 + CFG_LOG2_CPU_MAX) - 1): 16] = r.hartsel;// hartsello
            vb_resp_data[1] = r.ndmreset;
            vb_resp_data[0] = r.dmactive;
            if (r.regwr == 1'b1) begin
                if (r.wdata[31] == 1'b1) begin
                    if (i_halted[vb_hartselnext] == 1'b1) begin
                        v.cmderr = CMDERR_WRONGSTATE;
                    end else begin
                        v.haltreq = 1'b1;
                    end
                end else if (r.wdata[30] == 1'b1) begin
                    // resumereq must be ignored if haltreq is set (@see spec)
                    if (i_halted[vb_hartselnext] == 1'b1) begin
                        v.resumereq = 1'b1;
                    end else begin
                        v.cmderr = CMDERR_WRONGSTATE;
                    end
                end
                v.hartreset = r.wdata[29];
                v.hartsel = r.wdata[((16 + CFG_LOG2_CPU_MAX) - 1): 16];
                if (r.wdata[3] == 1'b1) begin               // setresethaltreq
                    v.resethaltreq = 1'b1;
                end else if (r.wdata[2] == 1'b1) begin      // clearresethaltreq
                    v.resethaltreq = 1'b0;
                end
                v.ndmreset = r.wdata[1];                    // 1=Reset whole system including all harts
                v.dmactive = r.wdata[0];
            end
        end else if (r.regidx == 7'h11) begin               // dmstatus
            // Currently selected ONLY. We support only one selected at once 'hasel=0'
            vb_resp_data[22] = 1'b0;                        // impebreak
            vb_resp_data[19] = 1'b0;                        // allhavereset: selected hart reset but not acknowledged
            vb_resp_data[18] = 1'b0;                        // anyhavereset
            vb_resp_data[17] = r.resumeack;                 // allresumeack
            vb_resp_data[16] = r.resumeack;                 // anyresumeack
            vb_resp_data[15] = (~i_available[hsel]);        // allnonexistent
            vb_resp_data[14] = (~i_available[hsel]);        // anynonexistent
            vb_resp_data[13] = (~i_available[hsel]);        // allunavail
            vb_resp_data[12] = (~i_available[hsel]);        // anyunavail
            vb_resp_data[11] = ((~i_halted[hsel]) && i_available[hsel]);// allrunning:
            vb_resp_data[10] = ((~i_halted[hsel]) && i_available[hsel]);// anyrunning:
            vb_resp_data[9] = (i_halted[hsel] && i_available[hsel]);// allhalted:
            vb_resp_data[8] = (i_halted[hsel] && i_available[hsel]);// anyhalted:
            vb_resp_data[7] = 1'b1;                         // authenticated:
            vb_resp_data[5] = 1'b1;                         // hasresethaltreq
            vb_resp_data[3: 0] = 4'h2;                      // version: dbg spec v0.13
        end else if (r.regidx == 7'h12) begin               // hartinfo
            // Not available core should returns 0
            if (i_available[hsel] == 1'b1) begin
                vb_resp_data[23: 20] = CFG_DSCRATCH_REG_TOTAL;// nscratch
                vb_resp_data[16] = 1'b0;                    // dataaccess: 0=CSR shadowed;1=memory shadowed
                vb_resp_data[15: 12] = 4'h0;                // datasize
                vb_resp_data[11: 0] = 12'h000;              // dataaddr
            end
        end else if (r.regidx == 7'h16) begin               // abstractcs
            vb_resp_data[28: 24] = CFG_PROGBUF_REG_TOTAL;
            vb_resp_data[12] = v_cmd_busy;                  // busy
            vb_resp_data[10: 8] = r.cmderr;
            vb_resp_data[3: 0] = CFG_DATA_REG_TOTAL;
            if ((r.regwr == 1'b1) && ((|r.wdata[10: 8]) == 1'b1)) begin
                v.cmderr = CMDERR_NONE;
            end
        end else if (r.regidx == 7'h17) begin               // command
            if (r.regwr == 1'b1) begin
                if (r.cmderr == CMDERR_NONE) begin
                    // If cmderr is non-zero, writes to this register are ignores (@see spec)
                    if (v_cmd_busy == 1'b1) begin
                        v.cmderr = CMDERR_BUSY;
                    end else begin
                        v.command = r.wdata;
                        v.cmdstate = CMD_STATE_INIT;
                    end
                end
            end
        end else if (r.regidx == 7'h18) begin               // abstractauto
            vb_resp_data[(CFG_DATA_REG_TOTAL - 1): 0] = r.autoexecdata;
            vb_resp_data[((16 + CFG_PROGBUF_REG_TOTAL) - 1): 16] = r.autoexecprogbuf;
            if (r.regwr == 1'b1) begin
                v.autoexecdata = r.wdata[(CFG_DATA_REG_TOTAL - 1): 0];
                v.autoexecprogbuf = r.wdata[((16 + CFG_PROGBUF_REG_TOTAL) - 1): 16];
            end
        end else if (r.regidx[6: 4] == 3'h2) begin          // progbuf[n]
            vb_resp_data = r.progbuf_data[(32 * t_idx) +: 32];
            if (r.regwr == 1'b1) begin
                t_progbuf[(32 * t_idx) +: 32] = r.wdata;
                v.progbuf_data = t_progbuf;
            end
            if ((r.autoexecprogbuf[t_idx] == 1'b1)
                    && (r.cmderr == CMDERR_NONE)) begin
                if (v_cmd_busy == 1'b1) begin
                    v.cmderr = CMDERR_BUSY;
                end else begin
                    v.cmdstate = CMD_STATE_INIT;
                end
            end
        end else if (r.regidx == 7'h40) begin               // haltsum0
            vb_resp_data[(CFG_CPU_MAX - 1): 0] = i_halted;
        end
    end
    default: begin
    end
    endcase

    // Abstract Command executor state machine:
    case (r.cmdstate)
    CMD_STATE_IDLE: begin
        v.aamvirtual = 1'b0;
        v.postincrement = 1'b0;
        v.cmd_regaccess = 1'b0;
        v.cmd_quickaccess = 1'b0;
        v.cmd_memaccess = 1'b0;
        v.cmd_progexec = 1'b0;
        v.cmd_read = 1'b0;
        v.cmd_write = 1'b0;
        v.dport_req_valid = 1'b0;
        v.dport_resp_ready = 1'b0;
    end
    CMD_STATE_INIT: begin
        v.postincrement = r.command[CmdPostincrementBit];
        if (r.command[31: 24] == 8'h00) begin
            // Register access command
            if (r.command[CmdTransferBit] == 1'b1) begin    // transfer
                v.cmdstate = CMD_STATE_REQUEST;
                v.cmd_regaccess = 1'b1;
                v.cmd_read = (~r.command[CmdWriteBit]);
                v.cmd_write = r.command[CmdWriteBit];

                v.dport_req_valid = 1'b1;
                v.dport_addr = {48'h000000000000, r.command[15: 0]};
                v.dport_wdata = {r.data1, r.data0};
                v.dport_size = r.command[22: 20];
            end else if (r.command[CmdPostexecBit] == 1'b1) begin// no transfer only buffer execution
                v.cmdstate = CMD_STATE_REQUEST;

                v.dport_req_valid = 1'b1;
                v.cmd_progexec = 1'b1;
            end else begin
                // Empty command: do nothing
                v.cmdstate = CMD_STATE_IDLE;
            end
        end else if (r.command[31: 24] == 8'h01) begin
            // Quick access command
            if (i_halted[hsel] == 1'b1) begin
                v.cmderr = CMDERR_WRONGSTATE;
                v.cmdstate = CMD_STATE_IDLE;
            end else begin
                v.haltreq = 1'b1;
                v.cmd_quickaccess = 1'b1;
                v.cmdstate = CMD_STATE_WAIT_HALTED;
            end
        end else if (r.command[31: 24] == 8'h02) begin
            // Memory access command
            v.cmdstate = CMD_STATE_REQUEST;
            v.cmd_memaccess = 1'b1;
            v.cmd_read = (~r.command[CmdWriteBit]);
            v.cmd_write = r.command[CmdWriteBit];
            v.aamvirtual = r.command[23];

            v.dport_req_valid = 1'b1;
            v.dport_addr = {r.data3, r.data2};
            v.dport_wdata = {r.data1, r.data0};
            v.dport_size = r.command[22: 20];
        end else begin
            // Unsupported command type:
            v.cmdstate = CMD_STATE_IDLE;
        end
    end
    CMD_STATE_REQUEST: begin
        if (i_dport_req_ready == 1'b1) begin
            v.cmdstate = CMD_STATE_RESPONSE;
            v.dport_req_valid = 1'b0;
            v.dport_resp_ready = 1'b1;
        end
    end
    CMD_STATE_RESPONSE: begin
        if (i_dport_resp_valid == 1'b1) begin
            v.dport_resp_ready = 1'b0;
            if (r.cmd_read == 1'b1) begin
                case (r.command[22: 20])                    // aamsize/aarsize
                3'h0: begin
                    v.data0 = {24'h000000, i_dport_rdata[7: 0]};
                    v.data1 = '0;
                end
                3'h1: begin
                    v.data0 = {16'h0000, i_dport_rdata[15: 0]};
                    v.data1 = '0;
                end
                3'h2: begin
                    v.data0 = i_dport_rdata[31: 0];
                    v.data1 = '0;
                end
                3'h3: begin
                    v.data0 = i_dport_rdata[31: 0];
                    v.data1 = i_dport_rdata[63: 32];
                end
                default: begin
                end
                endcase
            end
            if (r.postincrement == 1'b1) begin
                v.postincrement = 1'b0;
                if (r.command[31: 24] == 8'h00) begin
                    // Register access command:
                    t_command[15: 0] = (r.command[15: 0] + 1);
                    v.command = t_command;
                end else if (r.command[31: 24] == 8'h02) begin
                    // Memory access command
                    case (r.command[22: 20])                // aamsize
                    3'h0: begin
                        vb_arg1 = (vb_arg1 + 64'h0000000000000001);
                    end
                    3'h1: begin
                        vb_arg1 = (vb_arg1 + 64'h0000000000000002);
                    end
                    3'h2: begin
                        vb_arg1 = (vb_arg1 + 64'h0000000000000004);
                    end
                    3'h3: begin
                        vb_arg1 = (vb_arg1 + 64'h0000000000000008);
                    end
                    default: begin
                    end
                    endcase
                    v.data3 = vb_arg1[63: 32];
                    v.data2 = vb_arg1[31: 0];
                end
            end
            if (i_dport_resp_error == 1'b1) begin
                v.cmdstate = CMD_STATE_IDLE;
                if (r.cmd_memaccess == 1'b1) begin
                    // @spec The abstract command failed due to a bus error
                    //       (e.g. alignment, access size, or timeout).
                    v.cmderr = CMDERR_BUSERROR;
                end else begin
                    // @spec  An exception occurred while executing the
                    //        command (e.g. while executing the Program Buffer).
                    v.cmderr = CMDERR_EXCEPTION;
                end
            end else if ((r.cmd_regaccess == 1'b1) && (r.command[CmdPostexecBit] == 1'b1)) begin
                v.dport_req_valid = 1'b1;
                v.cmd_progexec = 1'b1;
                v.cmd_regaccess = 1'b0;
                v.cmd_write = 1'b0;
                v.cmd_read = 1'b0;
                v.cmdstate = CMD_STATE_REQUEST;
            end else begin
                v.cmdstate = CMD_STATE_IDLE;
            end
            if (r.cmd_quickaccess == 1'b1) begin
                // fast command continued even if progbuf execution failed (@see spec)
                v.resumereq = 1'b1;
            end
        end
    end
    CMD_STATE_WAIT_HALTED: begin
        if (i_halted[hsel] == 1'b1) begin
            v.cmdstate = CMD_STATE_REQUEST;

            v.dport_req_valid = 1'b1;
            v.cmd_progexec = 1'b1;
        end
    end
    default: begin
    end
    endcase

    if (v_resp_valid == 1'b1) begin
        if (r.bus_jtag == 1'b0) begin
            v.prdata = vb_resp_data;
        end else begin
            v.jtag_resp_data = vb_resp_data;
        end
    end
    v.pready = 1'b0;
    if ((v_resp_valid == 1'b1) && (r.bus_jtag == 1'b0)) begin
        v.pready = 1'b1;
    end else if (i_apbi.penable == 1'b1) begin
        v.pready = 1'b0;
    end

    vapbo.pready = r.pready;
    vapbo.prdata = r.prdata;

    vb_req_type[DPortReq_Write] = r.cmd_write;
    vb_req_type[DPortReq_RegAccess] = r.cmd_regaccess;
    vb_req_type[DPortReq_MemAccess] = r.cmd_memaccess;
    vb_req_type[DPortReq_MemVirtual] = r.aamvirtual;
    vb_req_type[DPortReq_Progexec] = r.cmd_progexec;

    if (~async_reset && i_nrst == 1'b0) begin
        v = dmidebug_r_reset;
    end

    o_ndmreset = r.ndmreset;
    o_hartsel = r.hartsel;
    o_haltreq = r.haltreq;
    o_resumereq = r.resumereq;
    o_resethaltreq = r.resethaltreq;
    o_hartreset = r.hartreset;
    o_dport_req_valid = r.dport_req_valid;
    o_dport_req_type = vb_req_type;
    o_dport_addr = r.dport_addr;
    o_dport_wdata = r.dport_wdata;
    o_dport_size = r.dport_size;
    o_dport_resp_ready = r.dport_resp_ready;
    o_progbuf = r.progbuf_data;

    w_cdc_dmi_req_ready = v_cdc_dmi_req_ready;
    wb_jtag_dmi_resp_data = r.jtag_resp_data;
    w_jtag_dmi_busy = 1'b0;                                 // |r.dmstate
    w_jtag_dmi_error = 1'b0;

    o_cfg = vcfg;
    o_apbo = vapbo;

    rin = v;
end: comb_proc

generate
    if (async_reset) begin: async_rst_gen

        always_ff @(posedge i_clk, negedge i_nrst) begin: rg_proc
            if (i_nrst == 1'b0) begin
                r <= dmidebug_r_reset;
            end else begin
                r <= rin;
            end
        end: rg_proc


    end: async_rst_gen
    else begin: no_rst_gen

        always_ff @(posedge i_clk) begin: rg_proc
            r <= rin;
        end: rg_proc

    end: no_rst_gen
endgenerate

endmodule: dmidebug
