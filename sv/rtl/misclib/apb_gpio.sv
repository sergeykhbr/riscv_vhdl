// TODO: interrutps
module apb_gpio #(
  parameter bit async_reset = 0,
  parameter integer width = 12
)
(
  input clk,
  input nrst,
  input types_amba_pkg::mapinfo_type i_mapinfo,
  output types_amba_pkg::dev_config_type cfg,
  input types_amba_pkg::apb_in_type i,
  output types_amba_pkg::apb_out_type o,
  input logic [width-1:0] i_gpio,
  output logic [width-1 : 0] o_gpio,
  output logic [width-1 : 0] o_gpio_dir,
  output logic [width-1 : 0] o_irq
);

  import types_amba_pkg::*;


  typedef struct {
    logic [width-1 : 0] direction;
    logic [width-1 : 0] iuser;
    logic [width-1 : 0] ouser;
    logic [width-1 : 0] ie;
    logic [31 : 0] reg32_3;
    logic resp_valid;
    logic [31:0] resp_rdata;
    logic resp_err;
  } registers;

  const registers R_RESET = '{
      '1,  // direction
      '0,  // iuser
      '0,  // ouser
      '0,  // ie
      '0,  // reg32_3
      1'b0,// resp_valid
      '0,  // resp_data
      1'b0 // resp_err
  };

  registers r, rin;

logic w_req_valid;
logic [31:0] wb_req_addr;
logic w_req_write;
logic [31:0] wb_req_wdata;




apb_slv #(
   .async_reset(async_reset),
   .vid(VENDOR_OPTIMITECH),
   .did(OPTIMITECH_GPIO)
) axi0 (
    .i_clk(clk),
    .i_nrst(nrst),
    .i_mapinfo(i_mapinfo),
    .o_cfg(cfg),
    .i_apbi(i),
    .o_apbo(o),
    .o_req_valid(w_req_valid),
    .o_req_addr(wb_req_addr),
    .o_req_write(w_req_write),
    .o_req_wdata(wb_req_wdata),
    .i_resp_valid(r.resp_valid),
    .i_resp_rdata(r.resp_rdata),
    .i_resp_err(r.resp_err)
);

always_comb
  begin : main_proc
    registers v;
    logic [31 : 0] vrdata;

    v = r;
    vrdata = '0;

    case (wb_req_addr[11 : 3])
    0: begin  // direction
        vrdata[width-1:0] = r.direction;
        if (w_req_valid == 1'b1 && w_req_write == 1'b1) begin
            v.direction = wb_req_wdata[width-1:0];
        end
    end
    1: begin  // iuser
        vrdata[width-1:0] = r.iuser;  // [RO]
    end
    2: begin  // ouser
        vrdata[width-1:0] = r.ouser;
        if (w_req_valid == 1'b1 && w_req_write == 1'b1) begin
            v.ouser = wb_req_wdata[width-1:0];
        end
    end
    3: begin // reg32_3
        vrdata = r.reg32_3;
        if (w_req_valid == 1'b1 && w_req_write == 1'b1) begin
            v.reg32_3 = wb_req_wdata;
        end
    end
    default: ;
    endcase

    v.resp_valid = w_req_valid;
    v.resp_rdata = vrdata;
    v.resp_err = '0;
    v.iuser[width-1 : 0] = i_gpio;

    if (~async_reset & (nrst == 1'b0)) begin
        v = R_RESET;
    end

    rin = v;

  end : main_proc

  assign o_gpio = r.ouser;
  assign o_gpio_dir = r.direction;
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

