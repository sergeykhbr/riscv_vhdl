//! @brief Hardware Configuration storage with the AMBA AXI4 interface.

module axi4_pnp #(

    parameter async_reset = 1'b0,
    parameter longint xaddr = 0,
    parameter longint xmask = 'hfffff,
    parameter logic [31 : 0] hw_id = 32'h20170101,
    parameter logic [3 : 0] cpu_max = 4'd1,
    parameter logic l2cache_ena = 8'd1,
    parameter logic [7 : 0] plic_irq_max = 8'd127

)(

  input sys_clk,
  input nrst,
  input types_bus0_pkg::bus0_xmst_cfg_vector mstcfg,
  input types_bus0_pkg::bus0_xslv_cfg_vector slvcfg,
  output types_amba_pkg::axi4_slave_config_type cfg,
  input types_amba_pkg::axi4_slave_in_type i,
  output types_amba_pkg::axi4_slave_out_type o,
  output logic o_irq                            // self-test interrupt request on write into read only register
);

  import types_amba_pkg::*;
  import types_bus0_pkg::*;


  const axi4_slave_config_type xconfig = '{
     PNP_CFG_TYPE_SLAVE, //descrtype
     PNP_CFG_SLAVE_DESCR_BYTES, // descrsize
     xaddr, // xaddr
     xmask, //xmask
     VENDOR_GNSSSENSOR, //vid
     GNSSSENSOR_PNP //did
  };

  typedef logic [31 : 0] master_config_map [0 : 2*CFG_BUS0_XMST_TOTAL-1];
  typedef logic [31 : 0] slave_config_map  [0 : 4*CFG_BUS0_XSLV_TOTAL-1];

  typedef struct {
    logic [31 : 0] fw_id;
    logic [63 : 0] idt; //! debug counter
    logic [63 : 0] malloc_addr; //! dynamic allocation addr
    logic [63 : 0] malloc_size; //! dynamic allocation size
    logic [63 : 0] fwdbg1; //! FW marker for the debug porposes
    logic [63 : 0] fwdbg2;
    logic [63 : 0] fwdbg3;
    logic irq;
    global_addr_array_type raddr;
  } registers;

  const registers R_RESET = '{
    '0, '0, '0,
    '0, '0, '0, '0,
    1'b0,
    {'0, '0}
  };

  registers r, rin;

  logic [CFG_SYSBUS_DATA_BITS-1 : 0] wb_dev_rdata;
  global_addr_array_type wb_bus_raddr;
  logic w_bus_re;
  global_addr_array_type wb_bus_waddr;
  logic w_bus_we;
  logic [CFG_SYSBUS_DATA_BYTES-1 : 0] wb_bus_wstrb;
  logic [CFG_SYSBUS_DATA_BITS-1 : 0] wb_bus_wdata;

  axi_slv #(
    .async_reset(async_reset)
  ) axi0 (
    .i_clk(sys_clk),
    .i_nrst(nrst),
    .i_xcfg(xconfig),
    .i_xslvi(i),
    .o_xslvo(o),
    .i_ready(1'b1),
    .i_rdata(wb_dev_rdata),
    .o_re(w_bus_re),
    .o_r32(),
    .o_radr(wb_bus_raddr),
    .o_wadr(wb_bus_waddr),
    .o_we(w_bus_we),
    .o_wstrb(wb_bus_wstrb),
    .o_wdata(wb_bus_wdata)
  );


always_comb
  begin : main_proc
    registers v;
    master_config_map mstmap;
    slave_config_map slvmap;
    logic [9:0] raddr[0:CFG_WORDS_ON_BUS-1];
    logic [9:0] waddr[0:CFG_WORDS_ON_BUS-1];
    logic [CFG_SYSBUS_DATA_BITS-1 : 0] vrdata;
    logic [31 : 0] rtmp;
    logic [31 : 0] wtmp;

    v = r;

    v.irq = 1'b0;
    v.raddr = wb_bus_raddr;

    for (int k = 0; k <= CFG_BUS0_XMST_TOTAL-1; k++) begin
      mstmap[2*k]   = {2'b00, 20'h00000, mstcfg[k].descrtype, mstcfg[k].descrsize};
      mstmap[2*k+1] = {mstcfg[k].vid, mstcfg[k].did};
    end

    for (int k = 0; k <= CFG_BUS0_XSLV_TOTAL-1; k++) begin
      slvmap[4*k] = {8'h00,
                     8'h00,
                     6'b000000, slvcfg[k].descrtype,
                     slvcfg[k].descrsize};
      slvmap[4*k+1] = {slvcfg[k].vid, slvcfg[k].did};
      slvmap[4*k+2] = {slvcfg[k].xmask, 12'h000};
      slvmap[4*k+3] = {slvcfg[k].xaddr, 12'h000};
    end


    vrdata = '0;
    for (int n = 0; n <= CFG_WORDS_ON_BUS-1; n++) begin
       waddr[n] = '0;                     // to avoid latches
       raddr[n] = r.raddr[n][11 : 2];

       rtmp = '0;
       if (raddr[n] == 10'd0) begin
          rtmp = hw_id;
       end else if (raddr[n] == 10'd1) begin
          rtmp = r.fw_id;
       end else if (raddr[n] == 10'd2) begin
          rtmp = {cpu_max[3:0],
                  3'd0,    // reserved
                  l2cache_ena,
                  CFG_BUS0_XMST_TOTAL[7:0],
                  CFG_BUS0_XSLV_TOTAL[7:0],
                  plic_irq_max[7:0]};
       end else if (raddr[n] == 10'd3) begin
          ; // reserved
       end else if (raddr[n] == 10'd4) begin
          rtmp = r.idt[31 : 0];
       end else if (raddr[n] == 10'd5) begin
          rtmp = r.idt[63 : 32];
       end else if (raddr[n] == 10'd6) begin
          rtmp = r.malloc_addr[31 : 0];
       end else if (raddr[n] == 10'd7) begin
          rtmp = r.malloc_addr[63 : 32];
       end else if (raddr[n] == 10'd8) begin
          rtmp = r.malloc_size[31 : 0];
       end else if (raddr[n] == 10'd9) begin
          rtmp = r.malloc_size[63 : 32];
       end else if (raddr[n] == 10'd10) begin
          rtmp = r.fwdbg1[31 : 0];
       end else if (raddr[n] == 10'd11) begin
          rtmp = r.fwdbg1[63 : 32];
       end else if (raddr[n] == 10'd12) begin
          rtmp = r.fwdbg2[31 : 0];
       end else if (raddr[n] == 10'd13) begin
          rtmp = r.fwdbg2[63 : 32];
       end else if (raddr[n] == 10'd14) begin
          rtmp = r.fwdbg3[31 : 0];
       end else if (raddr[n] == 10'd15) begin
          rtmp = r.fwdbg3[63 : 32];
       end else if ((int'(raddr[n]) >= 16) & (int'(raddr[n]) < 16+2*CFG_BUS0_XMST_TOTAL)) begin
          rtmp = mstmap[int'(raddr[n]) - 16];
       end else if ((int'(raddr[n]) >= 16+2*CFG_BUS0_XMST_TOTAL)
                  & (int'(raddr[n]) < 16+2*CFG_BUS0_XMST_TOTAL+4*CFG_BUS0_XSLV_TOTAL)) begin
          rtmp = slvmap[int'(raddr[n]) - 16 - 2*CFG_BUS0_XMST_TOTAL];
       end

       vrdata[32*n +: 32] = rtmp;
    end //loop


    if (w_bus_we == 1'b1) begin
      for (int n = 0; n <= CFG_WORDS_ON_BUS-1; n++) begin
         if (wb_bus_wstrb[CFG_ALIGN_BYTES*n +: CFG_ALIGN_BYTES] != 0) begin
           waddr[n] = wb_bus_waddr[n][11 : 2];
           wtmp  = wb_bus_wdata[32*n +: 32];

           case (waddr[n])
             10'd0: v.irq = 1'b1;
             10'd1: v.fw_id = wtmp;
             10'd4: v.idt[31 : 0] = wtmp;
             10'd5: v.idt[63 : 32] = wtmp;
             10'd6: v.malloc_addr[31 : 0] = wtmp;
             10'd7: v.malloc_addr[63 : 32] = wtmp;
             10'd8: v.malloc_size[31 : 0] = wtmp;
             10'd9: v.malloc_size[63 : 32] = wtmp;
             10'd10: v.fwdbg1[31 : 0] = wtmp;
             10'd11: v.fwdbg1[63 : 32] = wtmp;
             10'd12: v.fwdbg2[31 : 0]  = wtmp;
             10'd13: v.fwdbg2[63 : 32] = wtmp;
             10'd14: v.fwdbg3[31 : 0]  = wtmp;
             10'd15: v.fwdbg3[63 : 32] = wtmp;
             default: ;
           endcase
         end
      end // loop
    end

    if (~async_reset & (nrst == 1'b0)) begin
        v = R_RESET;
    end

    rin = v;
    wb_dev_rdata = vrdata;
    o_irq = r.irq;

  end : main_proc

  assign cfg = xconfig;

  generate

   if(async_reset) begin: gen_async_reset

      always_ff@(posedge sys_clk, negedge nrst)
          if(!nrst)
              r <= R_RESET;
          else
              r <= rin;

   end
   else begin: gen_sync_reset

      always_ff@(posedge sys_clk)
              r <= rin;

   end

  endgenerate

endmodule

