module axi4_gpio #(
  parameter bit async_reset = 0,
  parameter longint xaddr = 0,
  parameter longint xmask = 'hfffff,
  parameter integer width = 12
)
(
  input clk,
  input nrst,
  output types_amba_pkg::axi4_slave_config_type cfg,
  input types_amba_pkg::axi4_slave_in_type i,
  output types_amba_pkg::axi4_slave_out_type o,
  input logic [width-1:0] i_gpio,
  output logic [width-1 : 0] o_gpio,
  output logic [width-1 : 0] o_gpio_dir,
  output logic [width-1 : 0] o_irq
);

  import types_amba_pkg::*;

  const axi4_slave_config_type xconfig = '{
     PNP_CFG_TYPE_SLAVE, //descrtype
     PNP_CFG_SLAVE_DESCR_BYTES, //descrsize
     xaddr, //xaddr
     xmask, //xmask
     VENDOR_GNSSSENSOR, //vid
     OPTIMITECH_GPIO //did
  };

  typedef struct {
    logic [31 : 0] direction;
    logic [31 : 0] iuser;
    logic [31 : 0] ouser;
    logic [31 : 0] reg32_3;
    global_addr_array_type raddr;
  } registers;

  const registers R_RESET = '{
      '1, '0,
      '0, '0,
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
    .i_clk(clk),
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
    logic [CFG_SYSBUS_DATA_BITS-1 : 0] vrdata;
    logic [31 : 0] tmp;

    v = r;

    v.raddr = wb_bus_raddr;

    for (int n = 0; n <= CFG_WORDS_ON_BUS-1; n++) begin
      tmp = '0;

      case (r.raddr[n][11 : 2])
        0: tmp = r.direction;
        1: tmp = r.iuser;
        2: tmp = r.ouser;
        3: tmp = r.reg32_3;
        default: ;
      endcase
      vrdata[8*CFG_ALIGN_BYTES*n +: 8*CFG_ALIGN_BYTES] = tmp;
    end


    if (w_bus_we == 1'b1) begin

       for (int n = 0; n <= CFG_WORDS_ON_BUS-1; n++) begin
         tmp = wb_bus_wdata[32*n +: 32];

         if (wb_bus_wstrb[CFG_ALIGN_BYTES*n +: CFG_ALIGN_BYTES] != 0) begin
           case (wb_bus_waddr[n][11 : 2])
             0: v.direction = tmp;
             //1: v.iuser = tmp;  // [RO]
             2: v.ouser = tmp;
             3: v.reg32_3 = tmp;
             default: ;
           endcase
         end
       end
    end

    v.iuser[width-1 : 0] = i_gpio;

    if (~async_reset & (nrst == 1'b0)) begin
        v = R_RESET;
    end

    rin = v;
    wb_dev_rdata = vrdata;

  end : main_proc

  assign cfg = xconfig;

  assign o_gpio = r.ouser[width-1 : 0];
  assign o_gpio_dir = r.direction[width-1 : 0];
  assign o_irq = '0;

  // registers
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

