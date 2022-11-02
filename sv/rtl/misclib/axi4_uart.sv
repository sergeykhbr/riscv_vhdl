//!
//! Copyright 2021 Sergey Khabarov, sergeykhbr@gmail.com
//!
//! Licensed under the Apache License, Version 2.0 (the "License");
//! you may not use this file except in compliance with the License.
//! You may obtain a copy of the License at
//!
//!     http://www.apache.org/licenses/LICENSE-2.0
//!
//! Unless required by applicable law or agreed to in writing, software
//! distributed under the License is distributed on an "AS IS" BASIS,
//! WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//! See the License for the specific language governing permissions and
//! limitations under the License.
//!

module axi4_uart
#(
    parameter int async_reset = 0,
    parameter longint xaddr = 0,
    parameter longint xmask = 'hfffff,
    parameter int fifosz = 16
)
(
    input clk,
    input nrst,
    input i_rd,
    output logic o_td,
    input types_amba_pkg::axi4_slave_in_type i_axi,
    output types_amba_pkg::axi4_slave_out_type o_axi,
    output types_amba_pkg::axi4_slave_config_type cfg,
    output logic o_irq
);  

import types_amba_pkg::*;

const axi4_slave_config_type xconfig = '{
    descrtype : PNP_CFG_TYPE_SLAVE,
    descrsize : PNP_CFG_SLAVE_DESCR_BYTES,
    xaddr     : xaddr[CFG_SYSBUS_CFG_ADDR_BITS-1:0],
    xmask     : xmask[CFG_SYSBUS_CFG_ADDR_BITS-1:0],
    vid       : VENDOR_GNSSSENSOR,
    did       : GNSSSENSOR_UART
};

typedef logic [7:0] fifo_mem [0:fifosz-1];

typedef enum logic [2:0] {idle, startbit, data, parity, stopbit} state_type;

typedef struct {
    logic [$clog2(fifosz)-1:0]  raddr;
    logic [$clog2(fifosz)-1:0]  waddr; 
    logic                       we;
    logic [7:0]                 wdata; 
} fifo_in_type;

const fifo_in_type fifo_in_none = '{0, 0, 1'b0, 8'h00};

fifo_in_type    rfifoi;
logic [7:0]     rx_fifo_rdata;
fifo_mem        rx_fifo;

fifo_in_type    tfifoi;
logic [7:0]     tx_fifo_rdata;
fifo_mem        tx_fifo;

typedef struct {
    state_type                          tx_state;
    logic [$clog2(fifosz)-1:0]          tx_wr_cnt;
    logic [$clog2(fifosz)-1:0]          tx_rd_cnt;
    logic [$clog2(fifosz)-1:0]          tx_byte_cnt;
    logic [10:0]                        tx_shift;
    logic [31:0]                        tx_data_cnt;
    logic [31:0]                        tx_scaler_cnt;
    logic                               tx_level;
    logic [$clog2(fifosz)-1:0]          tx_irq_thresh;
    
    state_type                          rx_state;
    logic [$clog2(fifosz)-1:0]          rx_wr_cnt;
    logic [$clog2(fifosz)-1:0]          rx_rd_cnt;
    logic [$clog2(fifosz)-1:0]          rx_byte_cnt;
    logic [7:0]                         rx_shift;
    logic [31:0]                        rx_data_cnt;
    logic [31:0]                        rx_scaler_cnt;
    logic                               rx_level;
    logic [$clog2(fifosz)-1:0]          rx_irq_thresh;
    
    logic [CFG_SYSBUS_DATA_BITS-1:0]    rdata;
    logic [31:0]                        scaler;
    logic                               err_parity;
    logic                               err_stopbit;
    logic                               tx_parity_bit;
    logic                               rx_parity_bit;
    logic                               txena;
    logic                               rxena;
    logic                               tx_ie;
    logic                               rx_ie;
    logic                               tx_ip;
    logic                               rx_ip;
    logic                               tx_nstop;
    logic                               rx_nstop;
    logic                               tx_stop_cnt;
    logic                               rx_stop_cnt;
    logic [31:0]                        fwcpuid;
    logic                               tx_amo_guard;  // AMO operation read-modify-write often hit on full flag border
} registers;
    
const registers R_RESET = '{
        idle, // tx_state
        '0, '0, // tx_wr_cnt, tx_rd_cnt
        '0, // tx_byte_cnt
        '1, // tx_shift
        0, // tx_data_cnt
        0, // tx_scaler_cnt
        1'b0, // tx_level
        '0, // tx_irq_thresh
        idle, // rx_state
        '0, '0, // rx_wr_cnt , rx_rd_cnt
        '0, // rx_byte_cnt
        '0, // rx_shift
        0, // rx_data_cnt
        0, // rx_scaler_cnt
        1'b1, // rx_level
        '0, // rx_irq_thresh
        '0, // rdata
        0, // scaler
        1'b0, // err_parity
        1'b0, // err_stopbit
        1'b0, // tx_parity_bit
        1'b0, // rx_parity_bit
        1'b0, // txena
        1'b0, // rxena
        1'b0, // tx_ie
        1'b0, // rx_ie
        1'b0, // tx_ip
        1'b0, // rx_ip
        1'b0, // tx_nstop
        1'b0, // rx_nstop
        1'b0, // tx_stop_cnt
        1'b0, // rx_stop_cnt
        '0,   // fwcpuid
        1'b0  // tx_amo_guard
};

registers                           r, rin;
global_addr_array_type              wb_bus_raddr;
logic                               w_bus_re;
global_addr_array_type              wb_bus_waddr;
logic                               w_bus_we;
logic [CFG_SYSBUS_DATA_BYTES-1:0]   wb_bus_wstrb;
logic [CFG_SYSBUS_DATA_BITS-1:0]    wb_bus_wdata;

axi_slv #(.async_reset(async_reset))axi0 (
    .i_clk(clk),
    .i_nrst(nrst),
    .i_xcfg(xconfig), 
    .i_xslvi(i_axi),
    .o_xslvo(o_axi),
    .i_ready(1'b1),
    .i_rdata(r.rdata),
    .o_re(w_bus_re),
    .o_r32(),
    .o_radr(wb_bus_raddr),
    .o_wadr(wb_bus_waddr),
    .o_we(w_bus_we),
    .o_wstrb(wb_bus_wstrb),
    .o_wdata(wb_bus_wdata)
);

always_comb begin: comblogic   

    registers       v;
    logic [31:0]    tmp;
    
    fifo_in_type    v_rfifoi;
    fifo_in_type    v_tfifoi;
    
    logic           posedge_flag;
    logic           negedge_flag;
    logic           tx_fifo_empty;
    logic           tx_fifo_full;
    logic           rx_fifo_empty;
    logic           rx_fifo_full;
    logic [7:0]     t_tx, t_rx;
    logic           par;

    par     = 1'b0;
    t_tx    = 8'h00;
    t_rx    = 8'h00;
    
    v       = r;

    v_rfifoi = fifo_in_none;
    v_rfifoi.raddr = r.rx_rd_cnt;
    v_rfifoi.waddr = r.rx_wr_cnt;
    v_rfifoi.wdata = r.rx_shift;

    v_tfifoi = fifo_in_none;
    v_tfifoi.raddr = r.tx_rd_cnt;
    v_tfifoi.waddr = r.tx_wr_cnt;

    // Check FIFOs counters with thresholds:
    if (r.tx_byte_cnt < r.tx_irq_thresh) begin
        v.tx_ip = r.tx_ie;
    end

    if (r.rx_byte_cnt > r.rx_irq_thresh) begin
        v.rx_ip = r.rx_ie;
    end

   
    // system bus clock scaler to baudrate:
    posedge_flag = 1'b0;
    negedge_flag = 1'b0;
    if ((|r.scaler) == 1'b1) begin
        if(r.tx_scaler_cnt == (r.scaler-1)) begin
            v.tx_scaler_cnt = 0;
            v.tx_level = ~r.tx_level;
            posedge_flag = ~r.tx_level;
        end else begin
            v.tx_scaler_cnt = r.tx_scaler_cnt + 1;
        end

        if (r.rx_state == idle && i_rd == 1'b1) begin
            v.rx_scaler_cnt = 0;
            v.rx_level = 1'b1;
        end else if(r.rx_scaler_cnt == (r.scaler-1)) begin
            v.rx_scaler_cnt = 0;
            v.rx_level = ~r.rx_level;
            negedge_flag = r.rx_level;
        end else begin
            v.rx_scaler_cnt = r.rx_scaler_cnt + 1;
        end
    end

    // Transmitter's FIFO:
    tx_fifo_full = 1'b0;
    if (($clog2(fifosz))'(r.tx_wr_cnt + 1) == r.tx_rd_cnt) begin
        tx_fifo_full = 1'b1;
    end

    tx_fifo_empty = 1'b0;
    if (r.tx_rd_cnt == r.tx_wr_cnt) begin
        tx_fifo_empty = 1'b1;
        v.tx_byte_cnt = '0;
    end

    // Receiver's FIFO:
    rx_fifo_full = 1'b0;
    if (($clog2(fifosz))'(r.rx_wr_cnt + 1) == r.rx_rd_cnt) begin
        rx_fifo_full = 1'b1;
    end
 
    rx_fifo_empty = 1'b0;
    if(r.rx_rd_cnt == r.rx_wr_cnt) begin
        rx_fifo_empty = 1'b1;
        v.rx_byte_cnt = '0;
    end

    // Transmitter's state machine:
    if (posedge_flag == 1'b1) begin
        case (r.tx_state)
        idle: begin
            if (tx_fifo_empty == 1'b0 && r.txena == 1'b1) begin
                // stopbit=1,parity=xor,data[7:0],startbit=0
                t_tx = tx_fifo_rdata; //r.tx_fifo(conv_integer(r.tx_rd_cnt));
                if (r.tx_parity_bit == 1'b1) begin
                    par = t_tx[7] ^ t_tx[6] ^ t_tx[5] ^ t_tx[4]
                        ^ t_tx[3] ^ t_tx[2] ^ t_tx[1] ^ t_tx[0];
                    v.tx_shift = {1'b1, par, t_tx, 1'b0};
                end else begin
                    v.tx_shift = {2'b11, t_tx, 1'b0};
                end
                    
                v.tx_state = startbit;
                v.tx_rd_cnt = r.tx_rd_cnt + 1;
                v.tx_byte_cnt = r.tx_byte_cnt - 1;
                v.tx_data_cnt = 0;
            end else begin
                v.tx_shift = '1;
            end
        end
        startbit: begin
            v.tx_state = data;
        end
        data: begin
            if (r.tx_data_cnt == 8) begin
                if (r.tx_parity_bit == 1'b1) begin
                    v.tx_state = parity;
                end else begin
                    v.tx_state = stopbit;
                    v.tx_stop_cnt = r.tx_nstop;
                end
            end
        end
        parity: begin
            v.tx_state = stopbit;
        end
        stopbit: begin
            if (r.tx_stop_cnt == 1'b0) begin
                v.tx_state = idle;
            end else begin
                v.tx_stop_cnt = 1'b0;
            end
        end
        endcase
        
        if (r.tx_state != idle) begin
            v.tx_data_cnt = r.tx_data_cnt + 1;
            v.tx_shift = {1'b1, r.tx_shift[10:1]};
        end
    end //posedge

    //! Receiver's state machine:
    if (negedge_flag == 1'b1)
        case(r.rx_state)
        idle: begin
            if (i_rd == 1'b0 && r.rxena == 1'b1) begin
                v.rx_state = data;
                v.rx_shift = '0;
                v.rx_data_cnt = 0;
            end
        end
        data: begin
            v.rx_shift = {i_rd, r.rx_shift[7:1]};
            if (r.rx_data_cnt == 7) begin
                if (r.rx_parity_bit == 1'b1) begin
                    v.rx_state = parity;
                end else begin
                    v.rx_state = stopbit;
                    v.rx_stop_cnt = r.rx_nstop;
                end
            end else begin
                v.rx_data_cnt = r.rx_data_cnt + 1;
            end
        end
        parity: begin
            t_rx = r.rx_shift;
            par = t_rx[7] ^ t_rx[6] ^ t_rx[5] ^ t_rx[4]
                ^ t_rx[3] ^ t_rx[2] ^ t_rx[1] ^ t_rx[0];
            if (par == i_rd) begin
                v.err_parity = 1'b0;
            end else begin
                v.err_parity = 1'b1;
            end

            v.rx_state = stopbit;
        end
        stopbit: begin
            if (i_rd == 1'b0) begin
                v.err_stopbit = 1'b1;
            end else begin
                v.err_stopbit = 1'b0;
            end

            if (r.rx_stop_cnt == 1'b0) begin
                if (rx_fifo_full == 1'b0) begin
                    v_rfifoi.we = 1'b1;
                    //v.rx_fifo(conv_integer(r.rx_wr_cnt)) = r.rx_shift;
                    v.rx_wr_cnt = r.rx_wr_cnt + 1;
                    v.rx_byte_cnt = r.rx_byte_cnt + 1;
                end
                v.rx_state = idle;
            end else begin
                v.rx_stop_cnt = 1'b0;
            end
        end
        endcase


    for (int n=0; n<=CFG_WORDS_ON_BUS-1; n++) begin
       tmp = '0;
       case(wb_bus_raddr[n][11:2])
       0: begin // 0x00: txdata
           tmp[31] = tx_fifo_full | r.tx_amo_guard;
           if (tx_fifo_full == 1'b1) begin
               v.tx_amo_guard = 1'b1;
           end
       end
       1: begin // 0x04: rxdata
           tmp[31] = rx_fifo_empty;
           tmp[7:0] = rx_fifo_rdata; 
           if (rx_fifo_empty == 1'b0 && w_bus_re == 1'b1) begin
               v.rx_rd_cnt = r.rx_rd_cnt + 1;
               v.rx_byte_cnt = r.rx_byte_cnt - 1;
           end
       end
       2: begin // 0x08: txctrl
           tmp[0] = r.txena;    // [0] txena
           tmp[1] = r.tx_nstop; // [1] Number of stop bits
           tmp[2] = r.tx_parity_bit;
           tmp[18:16] = r.tx_irq_thresh[2:0];
       end
       3: begin // 0x0C: rxctrl
           tmp[0] = r.rxena;    // [0] txena
           tmp[1] = r.rx_nstop; // [1] Number of stop bits
           tmp[2] = r.rx_parity_bit;
           tmp[18:16] = r.rx_irq_thresh[2:0];
       end
       4: begin // 0x10: ie
           tmp[0] = r.tx_ie;
           tmp[1] = r.rx_ie;
       end
       5: begin // 0x14: ip
           tmp[0] = r.tx_ip;
           tmp[1] = r.rx_ip;
       end
       6: begin // 0x18: scaler
           tmp = r.scaler;
       end
       7: begin // 0x1C: fwcpuid
           tmp = r.fwcpuid;
       end
       endcase
       //v.rdata(8*CFG_ALIGN_BYTES*(n+1)-1:8*CFG_ALIGN_BYTES*n) = tmp;
       v.rdata[8*CFG_ALIGN_BYTES*n +: 8*CFG_ALIGN_BYTES] = tmp;
    end

    if (w_bus_we == 1'b1) begin
        for (int n=0; n<=CFG_WORDS_ON_BUS-1; n++) begin
            if (wb_bus_wstrb[CFG_ALIGN_BYTES*n +: CFG_ALIGN_BYTES] != 0) begin
                tmp = wb_bus_wdata[8*CFG_ALIGN_BYTES*n +: 8*CFG_ALIGN_BYTES];
                case(wb_bus_waddr[n][11:2])
                0: begin // 0x00: txdata
                    if (tx_fifo_full == 1'b0 && r.tx_amo_guard == 1'b0) begin
                        v_tfifoi.we = 1'b1;
                        v_tfifoi.wdata = tmp[7:0];
                        v.tx_wr_cnt = r.tx_wr_cnt + 1;
                        v.tx_byte_cnt = r.tx_byte_cnt + 1;
                    end
                    v.tx_amo_guard = 1'b0;
                end
                2: begin // 0x08: txctrl
                    v.txena = tmp[0];    // [0] txena
                    v.tx_nstop = tmp[1]; // [1] Number of stop bits
                    v.tx_parity_bit = tmp[2];
                    v.tx_irq_thresh[2:0] = tmp[18:16];
                end
                3: begin // 0x0C: rxctrl
                    v.rxena = tmp[0];    // [0] txena
                    v.rx_nstop = tmp[1]; // [1] Number of stop bits
                    v.rx_parity_bit = tmp[2];
                    v.rx_irq_thresh[2:0] = tmp[18:16];
                end
                4: begin // 0x10: ie
                    v.tx_ie = tmp[0];
                    v.rx_ie = tmp[1];
                end
                5: begin // 0x14: ip
                    v.tx_ip = tmp[0];
                    v.rx_ip = tmp[1];
                end
                6: begin // 0x18: scaler
                    v.scaler        = tmp[30:0];
                    v.rx_scaler_cnt = 0;
                    v.tx_scaler_cnt = 0;
                end
                7: begin
                    if (r.fwcpuid == 32'h00000000 || tmp == 32'h00000000) begin
                        v.fwcpuid = tmp;
                    end
                end
                endcase
            end
        end
    end

    if (!async_reset && nrst == 1'b0) begin
        v = R_RESET;
    end

    rin = v;

    rfifoi = v_rfifoi;
    tfifoi = v_tfifoi;

    o_td = r.tx_shift[0];
    o_irq = r.tx_ip | r.rx_ip;
end // comblogic  

assign cfg = xconfig;

// fifo pseudo memory:
always_ff@(posedge clk) begin: tfifo0
  if(tfifoi.we == 1'b1)
      tx_fifo[tfifoi.waddr] <= tfifoi.wdata;
end

assign tx_fifo_rdata = tx_fifo[tfifoi.raddr];

always_ff@(posedge clk) begin: rfifo0
  if(rfifoi.we == 1'b1)
      rx_fifo[rfifoi.waddr] <= rfifoi.wdata;
end

assign rx_fifo_rdata = rx_fifo[rfifoi.raddr];

// registers:
generate 

  if(async_reset) begin: gen_async_reset
    
      always_ff@(posedge clk, negedge nrst)
          if(!nrst) 
              r <= R_RESET;
          else
              r <= rin;
          
  end
  else begin: gen_sync_reset

      always_ff@(posedge clk)
              r <= rin;  

  end

endgenerate

endmodule
