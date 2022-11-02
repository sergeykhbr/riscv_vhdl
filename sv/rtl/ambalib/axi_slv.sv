`timescale 1ns/10ps

module axi_slv
#(
    parameter async_reset = 0
)
(
    input i_clk,
    input i_nrst,
    input types_amba_pkg::axi4_slave_config_type i_xcfg,
    input types_amba_pkg::axi4_slave_in_type i_xslvi,
    output types_amba_pkg::axi4_slave_out_type o_xslvo,
    input i_ready,
    input [types_amba_pkg::CFG_SYSBUS_DATA_BITS-1 : 0] i_rdata,
    output logic o_re,
    output logic o_r32,
    output types_amba_pkg::global_addr_array_type o_radr,
    output types_amba_pkg::global_addr_array_type o_wadr,
    output logic o_we,
    output logic [types_amba_pkg::CFG_SYSBUS_DATA_BYTES-1 : 0] o_wstrb,
    output logic [types_amba_pkg::CFG_SYSBUS_DATA_BITS-1 : 0] o_wdata
);

import types_amba_pkg::*;
import axi_slv_pkg::*;

axi_slave_bank_type rin, r;


always_comb
begin: main_proc
    axi_slave_bank_type v;
    logic [CFG_SYSBUS_ADDR_BITS-1 : 0] traddr;
    logic [CFG_SYSBUS_ADDR_BITS-1 : 0] twaddr;
    logic [CFG_SYSBUS_ADDR_BITS-1 : 0] v_raddr_bus;
    logic [CFG_SYSBUS_ADDR_BITS-1 : 0] v_raddr_bus_nxt;
    logic [CFG_SYSBUS_ADDR_BITS-1 : 0] v_raddr_burst_nxt;
    logic [CFG_SYSBUS_ADDR_BITS-1 : 0] v_waddr_bus;
    logic [CFG_SYSBUS_ADDR_BITS-1 : 0] v_waddr_burst_nxt;
    logic v_re;
    logic v_r32;
    logic [CFG_SYSBUS_ADDR_BITS-1 : 0] v_radr;
    logic [CFG_SYSBUS_DATA_BYTES-1 : 0] v_we;
    logic [CFG_SYSBUS_DATA_BYTES-1 : 0] v_wstrb;
    logic [CFG_SYSBUS_DATA_BITS-1 : 0] v_wdata;
    logic v_aw_ready;
    logic v_w_ready;
    logic v_ar_ready;
    logic v_r_valid;
    logic v_r_last;
    logic [CFG_SYSBUS_DATA_BITS-1 : 0] vb_r_data;

    v = r;

    traddr = {(i_xslvi.ar_bits.addr[CFG_SYSBUS_ADDR_BITS-1 : 12] & (~ i_xcfg.xmask))
                , i_xslvi.ar_bits.addr[11 : 0]};

    twaddr = {(i_xslvi.aw_bits.addr[CFG_SYSBUS_ADDR_BITS-1 : 12] & (~ i_xcfg.xmask))
                , i_xslvi.aw_bits.addr[11 : 0]};

    v_re      = 1'b0;
    v_r32     = 1'b0;
    v_radr[0] = '0;
    v_radr[1] = '0;
    vb_r_data = i_rdata;

    v_raddr_bus = traddr;
    v_raddr_bus_nxt[CFG_SYSBUS_ADDR_BITS-1 : 12] = traddr[CFG_SYSBUS_ADDR_BITS-1 : 12];
    v_raddr_bus_nxt[11:0] = traddr[11:0] + XSizeToBytes(int'(i_xslvi.ar_bits.size));
    v_waddr_bus = twaddr;


    // Next burst read address 
    v_raddr_burst_nxt[CFG_SYSBUS_ADDR_BITS-1 : 12] = r.raddr[CFG_SYSBUS_ADDR_BITS-1 : 12];
    v_raddr_burst_nxt[11:0] = r.raddr[11:0] + r.rsize;
    if (r.rburst == AXI_BURST_WRAP) begin
        v_raddr_burst_nxt[CFG_SYSBUS_ADDR_BITS-1 : 5]
              = r.raddr[CFG_SYSBUS_ADDR_BITS-1 : 5];
    end

    // Next burst write address 
    v_waddr_burst_nxt[CFG_SYSBUS_ADDR_BITS-1 : 12] = r.waddr[CFG_SYSBUS_ADDR_BITS-1 : 12];
    v_waddr_burst_nxt[11:0] = r.waddr[11:0] + r.wsize;
    if (r.wburst == AXI_BURST_WRAP) begin
        v_waddr_burst_nxt[CFG_SYSBUS_ADDR_BITS-1 : 5]
              = r.waddr[CFG_SYSBUS_ADDR_BITS-1 : 5];
    end

    v_we = '0;

    v_ar_ready = 1'b0;
    v_r_valid = 1'b0;
    v_r_last = 1'b0;
    v_aw_ready = 1'b0;
    v_w_ready = 1'b0;

    // Reading state machine:
    case (r.rstate)
    rwait: begin
        v_ar_ready = 1'b1;
        v_radr = v_raddr_bus;
   
        if (i_xslvi.ar_valid == 1'b1) begin
            if (i_xslvi.aw_valid == 1'b0 && r.wstate == wwait) begin
                v_re = 1'b1;
                v.rstate = rtrans;
                v.raddr = v_raddr_bus_nxt;
            end else begin
                v.rstate = rhold;
                v.raddr = v_raddr_bus;
            end

            if (i_xslvi.ar_bits.size == 3'b010) begin
                v_r32 = 1'b1;
            end
            v.rswap  = i_xslvi.ar_bits.addr[2];
            v.rsize  = XSizeToBytes(int'(i_xslvi.ar_bits.size));
            v.rburst = i_xslvi.ar_bits.burst;
            v.rlen   = i_xslvi.ar_bits.len;
            v.rid    = i_xslvi.ar_id;
            v.rresp  = AXI_RESP_OKAY;
            v.ruser  = i_xslvi.ar_user;
            v.rskip = 1'b0;
        end
    end  // rwait

    rhold: begin
        v_radr = r.raddr;
        if (r.rsize == 4) begin
            v_r32 = 1'b1;
        end
        if (i_xslvi.aw_valid == 1'b0 && r.wstate == wwait) begin
            v_re = 1'b1;
            v.rstate = rtrans;
            v.raddr = v_raddr_burst_nxt;
        end
    end   // rhold

    rtrans: begin
        v_r_valid = i_ready;
        v_radr = r.raddr;
        v_re = (|r.rlen);  // request next burst read address even if no ready data
        v_r_last = ~v_re;
        if (v.rsize == 4) begin
            v_r32 = 1'b1;
        end
        if (r.rskip == 1'b1) begin
            vb_r_data = r.skip_rdata;
            if (i_xslvi.r_ready == 1'b1) begin
               v.rskip = 1'b0;
            end
        end else if (r.rswap == 1'b1) begin
            vb_r_data = {i_rdata[63 : 32], i_rdata[63 : 32]};
        end

        if (i_xslvi.r_ready == 1'b1 && i_ready == 1'b1) begin
            v.rswap = r.raddr[2];
            v.raddr = v_raddr_burst_nxt;
            // End of transaction (or process another one):
            if (r.rlen == 0) begin
                v_ar_ready = 1'b1;
                v_radr = v_raddr_bus;
                if (i_xslvi.ar_valid == 1'b1) begin
                    if (i_xslvi.aw_valid == 1'b0 && r.wstate == wwait) begin
                        v_re = 1'b1;
                        v.rstate = rtrans;
                        v.raddr = v_raddr_bus_nxt;
                    end else begin 
                        v.rstate = rhold;
                        v.raddr = v_raddr_bus;
                    end

                    if (i_xslvi.ar_bits.size == 3'b010) begin
                        v_r32 = 1'b1;
                    end
                    v.rswap = i_xslvi.ar_bits.addr[2];
                    v.rsize  = XSizeToBytes(int'(i_xslvi.ar_bits.size));
                    v.rburst = i_xslvi.ar_bits.burst;
                    v.rlen   = i_xslvi.ar_bits.len;
                    v.rid    = i_xslvi.ar_id;
                    v.rresp  = AXI_RESP_OKAY;
                    v.ruser  = i_xslvi.ar_user;
                end else begin
                    v.rstate = rwait;
                end
            end else begin
                v.rlen = r.rlen - 1;
            end
        end else if (i_xslvi.r_ready == 1'b0 && i_ready == 1'b1 && r.rskip == 1'b0) begin
            v.rskip = 1'b1;
            v.skip_rdata = vb_r_data;
        end
    end   // rtrans
    endcase

    // Writing state machine:
    case (r.wstate)
    wwait: begin
        if (r.rlen == 0 || r.rstate == rhold) begin
            v_aw_ready = 1'b1;
        end
        if (i_xslvi.aw_valid == 1'b1 && (r.rlen == 0 || r.rstate == rhold)) begin
            v.wstate = wtrans;
            v.waddr  = v_waddr_bus;
            v.wswap  = i_xslvi.aw_bits.addr[2];
            v.wsize  = XSizeToBytes(int'(i_xslvi.aw_bits.size));
            v.wburst = i_xslvi.aw_bits.burst;
            v.wlen   = i_xslvi.aw_bits.len;
            v.wid    = i_xslvi.aw_id;
            v.wresp  = AXI_RESP_OKAY;
            v.wuser  = i_xslvi.aw_user;
        end
    end  // wwait

    wtrans: begin
        v_we = '1;
        v_w_ready = i_ready;
        if (i_xslvi.w_valid == 1'b1 && i_ready == 1'b1) begin
            v.wswap = r.waddr[2];
            v.waddr = v_waddr_burst_nxt;
            // End of transaction:
            if (r.wlen == 0) begin
                v.b_valid = 1'b1;
                v_aw_ready = 1'b1;
                if (i_xslvi.aw_valid == 1'b0) begin
                    v.wstate = wwait;
                end else begin
                    v.waddr = v_waddr_bus;
                    v.wswap  = i_xslvi.aw_bits.addr[2];
                    v.wsize  = XSizeToBytes(int'(i_xslvi.aw_bits.size));
                    v.wburst = i_xslvi.aw_bits.burst;
                    v.wlen   = i_xslvi.aw_bits.len;
                    v.wid    = i_xslvi.aw_id;
                    v.wresp  = AXI_RESP_OKAY;
                    v.wuser  = i_xslvi.aw_user;
                end
            end else begin
                v.wlen = r.wlen - 1;
            end
        end
    end  // wtrans
    endcase

    if ((i_xslvi.b_ready == 1'b1) && (r.b_valid == 1'b1))
    begin
        if ((r.wstate == wtrans) && (i_xslvi.w_valid == 1'b1) && (r.wlen == 0))
        begin
            v.b_valid = 1'b1;
        end
        else
        begin
            v.b_valid = 1'b0;
        end
    end


    o_re    = v_re;
    o_radr[0]  = {v_radr[CFG_SYSBUS_ADDR_BITS-1:3], 3'b000};
    o_radr[1]  = {v_radr[CFG_SYSBUS_ADDR_BITS-1:3], 3'b100};
    o_r32   = v_r32;
    o_wadr[0]  = {r.waddr[CFG_SYSBUS_ADDR_BITS-1:3], 3'b000};
    o_wadr[1]  = {r.waddr[CFG_SYSBUS_ADDR_BITS-1:3], 3'b100};
    o_we    = i_xslvi.w_valid;
    o_wdata = i_xslvi.w_data;
    o_wstrb = i_xslvi.w_strb;


    if ( !async_reset && (i_nrst == 1'b0))
    begin
        v = AXI_SLAVE_BANK_RESET();
    end

    rin = v;

    o_xslvo.aw_ready = v_aw_ready;
    o_xslvo.w_ready  = v_w_ready;
    o_xslvo.ar_ready = v_ar_ready;
    o_xslvo.r_valid  = v_r_valid;
    o_xslvo.r_last   = v_r_last;
    o_xslvo.r_data   = vb_r_data;
    o_xslvo.r_id     = r.rid;
    o_xslvo.r_resp   = r.rresp;
    o_xslvo.r_user   = r.ruser;

    // Write Handshaking:
    o_xslvo.b_id    = r.wid;
    o_xslvo.b_resp  = r.wresp;
    o_xslvo.b_user  = r.wuser;
    o_xslvo.b_valid = r.b_valid;

end: main_proc

// registers
 generate

    if (async_reset)
        begin: async_rst_gen
            always_ff @(posedge i_clk or negedge i_nrst)
                begin: rg_proc
                    if ( i_nrst == 1'b0 ) begin
                        r <= AXI_SLAVE_BANK_RESET();
                    end else begin
                        r <= rin;
                    end
                end: rg_proc
        end: async_rst_gen
    else
        begin: no_rst_gen
            always_ff @(posedge i_clk)
                begin: rg_proc
                    r <= rin;
                end: rg_proc
        end: no_rst_gen

endgenerate

endmodule: axi_slv
