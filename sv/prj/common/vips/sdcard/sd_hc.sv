// 
//  Copyright 2023 Sergey Khabarov, sergeykhbr@gmail.com
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

module sd_hc
#(
    parameter real half_period_clk = 50ns, // 20 MHz = 50ns
    parameter integer block_size = 512
)
(
    input i_csn,
    input i_sck,
    input i_mosi,
    output logic o_miso
);


parameter SD_STATE_UNKNOWN = 2'd0;
parameter SD_STATE_IDLE = 2'd1;
parameter SD_STATE_READY = 2'd2;
parameter SD_STATE_DATA = 2'd3;

parameter CMD_STATE_IDLE = 3'd0;
parameter CMD_STATE_SHIFT = 3'd1;
parameter CMD_STATE_CRC = 3'd2;
parameter CMD_STATE_UPDATE = 3'd3;
parameter CMD_STATE_DUMMY = 3'd4;

typedef struct {
    logic [1:0] sdstate;
    logic [2:0] cmdstate;
    logic [39:0] tx_shift;
    logic [55:0] rx_shift;  // cmd + crc + dummy
    logic miso;
    logic sck;
    logic update;
    logic cmd_ready;
    logic cmd_transmission;
    logic [6:0] cmd_type;
    logic [31:0] cmd_data;
    logic [7:0] cmd_data_crc;
    logic [7:0] cmd_calc_crc;
    logic [6:0] crc7;
    logic [39:0] cmd_resp;
    logic cmd_app_specific;
    logic HCS;
    logic [17:0] data_bitcnt;
} sd_hc_registers;

const sd_hc_registers sd_hc_r_reset = '{
    SD_STATE_UNKNOWN,                   // sdstate
    CMD_STATE_IDLE,                     // cmdstate
    '1,                                 // tx_shift
    '1,                                 // rx_shift
    1'b1,                               // miso
    1'b0,                               // sck
    1'b0,                               // update
    1'b0,                               // cmd_ready
    1'b0,                               // cmd_transmission
    '0,                                 // cmd_type
    '0,                                 // cmd_data
    '0,                                 // cmd_data_crc
    '0,                                 // cmd_calc_crc
    '0,                                 // crc7
    '0,                                 // cmd_resp
    1'b0,                               // cmd_app_specific
    1'b0,                               // HCS
    '0                                  // data_bitcnt
};

logic clk;
int clk_cnt;
sd_hc_registers r, rin;

initial begin
    clk = 0;
    clk_cnt = 0;
end

always #half_period_clk clk=~clk;

always_comb
begin: comb_proc
    sd_hc_registers v;
    logic v_posedge;
    logic v_negedge;
    logic v_idle;

    v_posedge = 1'b0;
    v_negedge = 1'b0;
    v_idle = 1'b0;
    v = r;

    v.sck = i_sck;
    v.update = 1'b0;
    v.cmd_ready = 1'b0;

    if (r.sck == 1'b1 && i_sck == 1'b0) begin
        v_negedge = 1'b1;
    end

    if (r.sck == 1'b0 && i_sck == 1'b1) begin
        v_posedge = 1'b1;
    end

    if (i_csn == 1'b0 && v_posedge == 1'b1) begin
        v.update = 1'b1;
        v.rx_shift = {r.rx_shift[54:0], i_mosi};
        if (r.sdstate == SD_STATE_DATA) begin
            // 4 dummy + prefix + 512 + 2 CRC + 1 dummy
            if (r.data_bitcnt < (8*(7 + 512) - 1)) begin
                v.data_bitcnt = r.data_bitcnt + 1;
            end else begin
                v.data_bitcnt = '0; 
            end
        end
    end else if (v_negedge == 1'b1) begin
        v.miso = r.tx_shift[39];
        v.tx_shift = {r.tx_shift[38:0], 1'b1};
        if (r.sdstate == SD_STATE_DATA && r.data_bitcnt[2:0] == 3'd7) begin
            if (r.data_bitcnt[17:3] == 4) begin
                // Prefix
                v.tx_shift = 40'hFEFFFFFFFF;
            end else if (r.data_bitcnt[17:3] >= 5 && r.data_bitcnt[17:3] < (512 + 5)) begin
                // Data Block
                v.tx_shift = {(r.data_bitcnt[10:3] - 8'd5), 32'hFFFFFFFF};
            end else if (r.data_bitcnt[17:3] == (512 + 5)) begin
                // CRC
                v.tx_shift = {16'h1122, 24'hFFFFFF};
            end
        end
    end;

    case (r.cmdstate)
    CMD_STATE_IDLE: begin
        if (r.update == 1'b1 && r.rx_shift[0] == 1'b0) begin
            v.cmdstate = CMD_STATE_SHIFT;
            v.rx_shift[55:1] = '1;
        end
    end
    CMD_STATE_SHIFT: begin
        if (r.update == 1'b1) begin
            if (r.rx_shift[39] == 1'b0) begin
                v.cmdstate = CMD_STATE_CRC;
                v.cmd_calc_crc = r.crc7;
            end
        end
    end
    CMD_STATE_CRC: begin
        if (r.rx_shift[47] == 1'b0) begin
            v.cmdstate = CMD_STATE_UPDATE;
        end
    end
    CMD_STATE_UPDATE: begin
        v.cmd_ready = 1'b1;
        v.cmd_transmission = r.rx_shift[46];
        v.cmd_type = r.rx_shift[45:40];
        v.cmd_data = {r.rx_shift[15:8], r.rx_shift[23:16], r.rx_shift[31:24], r.rx_shift[39:32]};
        v.cmd_data_crc = r.rx_shift[7:0];
        v.cmdstate = CMD_STATE_DUMMY;
    end
    CMD_STATE_DUMMY: begin
        if (r.rx_shift[55] == 1'b0) begin
            v.cmdstate = CMD_STATE_IDLE;
            v.tx_shift = r.cmd_resp;
        end
    end
    default: begin
    end
    endcase

    if (r.sdstate == SD_STATE_IDLE) begin
        v_idle = 1'b1;
    end 

    if (r.cmd_ready == 1'b1) begin
        v.cmd_resp = '1;
        v.cmd_app_specific = 1'b0;
        if (r.cmd_transmission == 1'b1) begin
            case (r.cmd_type)
            6'd0: begin
                v.sdstate = SD_STATE_IDLE;
                v.cmd_resp = {8'h01, 32'hFFFFFFFF};
            end
            6'd8: begin
                v.cmd_resp = {7'd0, v_idle, 32'hFFFFFFFF};
            end
            6'd18: begin
                if (r.sdstate == SD_STATE_READY) begin
                    v.cmd_resp = {8'd0, 32'hFFFFFFFF};
                    v.sdstate = SD_STATE_DATA;
                end else begin
                    v.cmd_resp = {8'd5, 32'hFFFFFFFF};
                end
            end
            6'd41: begin
                v.cmd_resp = {7'd0, v_idle, 32'hFFFFFFFF};
                if (r.cmd_app_specific == 1'b1) begin
                    v.sdstate = SD_STATE_READY;
                    v.HCS = r.cmd_data[30];
                end
            end
            6'd55: begin
                v.cmd_app_specific = 1'b1;
                v.cmd_resp = {7'd0, v_idle, 32'hFFFFFFFF};
            end
            6'd58: begin
                if (v_idle == 1'b1) begin
                    v.cmd_resp = {7'd0, v_idle, 32'h00FF8000};
                end else begin
                    v.cmd_resp = {8'd0, 1'd1, r.HCS, 30'h00FF8000};  // [30] HCS = 1, High Capacity
                end
            end
            default: begin
                // Unsupported command
                v.cmd_resp = {7'h02, v_idle, 32'hFFFFFFFF};
            end
            endcase
        end
    end

    o_miso = r.miso;

    rin = v;
end: comb_proc


always @(posedge clk) begin
    clk_cnt = clk_cnt + 1;
    if (clk_cnt < 5) begin
        r <= sd_hc_r_reset;
    end else begin
        r <= rin;
    end
end

endmodule
