
module _T_146_ext(
  input W0_clk,
  input [5:0] W0_addr,
  input W0_en,
  input [87:0] W0_data,
  input [3:0] W0_mask,
  input R0_clk,
  input [5:0] R0_addr,
  input R0_en,
  output [87:0] R0_data
);

  reg [5:0] reg_R0_addr;
  reg [87:0] ram [63:0];
  `ifdef RANDOMIZE
    integer initvar;
    initial begin
      #0.002 begin end
      for (initvar = 0; initvar < 64; initvar = initvar+1)
        ram[initvar] = {3 {$random}};
      reg_R0_addr = {1 {$random}};
    end
  `endif
  integer i;
  always @(posedge R0_clk)
    if (R0_en) reg_R0_addr <= R0_addr;
  always @(posedge W0_clk)
    if (W0_en) begin
      if (W0_mask[0]) ram[W0_addr][21:0] <= W0_data[21:0];
      if (W0_mask[1]) ram[W0_addr][43:22] <= W0_data[43:22];
      if (W0_mask[2]) ram[W0_addr][65:44] <= W0_data[65:44];
      if (W0_mask[3]) ram[W0_addr][87:66] <= W0_data[87:66];
    end
  assign R0_data = ram[reg_R0_addr];

endmodule

module _T_80_ext(
  input W0_clk,
  input [8:0] W0_addr,
  input W0_en,
  input [63:0] W0_data,
  input [0:0] W0_mask,
  input R0_clk,
  input [8:0] R0_addr,
  input R0_en,
  output [63:0] R0_data
);

  reg [8:0] reg_R0_addr;
  reg [63:0] ram [511:0];
  `ifdef RANDOMIZE
    integer initvar;
    initial begin
      #0.002 begin end
      for (initvar = 0; initvar < 512; initvar = initvar+1)
        ram[initvar] = {2 {$random}};
      reg_R0_addr = {1 {$random}};
    end
  `endif
  integer i;
  always @(posedge R0_clk)
    if (R0_en) reg_R0_addr <= R0_addr;
  always @(posedge W0_clk)
    if (W0_en) begin
      if (W0_mask[0]) ram[W0_addr][63:0] <= W0_data[63:0];
    end
  assign R0_data = ram[reg_R0_addr];

endmodule

module tag_array_ext(
  input RW0_clk,
  input [5:0] RW0_addr,
  input RW0_en,
  input RW0_wmode,
  input [3:0] RW0_wmask,
  input [79:0] RW0_wdata,
  output [79:0] RW0_rdata
);

  reg [5:0] reg_RW0_addr;
  reg [79:0] ram [63:0];
  `ifdef RANDOMIZE
    integer initvar;
    initial begin
      #0.002 begin end
      for (initvar = 0; initvar < 64; initvar = initvar+1)
        ram[initvar] = {3 {$random}};
      reg_RW0_addr = {1 {$random}};
    end
  `endif
  integer i;
  always @(posedge RW0_clk)
    if (RW0_en && !RW0_wmode) reg_RW0_addr <= RW0_addr;
  always @(posedge RW0_clk)
    if (RW0_en && RW0_wmode) begin
      if (RW0_wmask[0]) ram[RW0_addr][19:0] <= RW0_wdata[19:0];
      if (RW0_wmask[1]) ram[RW0_addr][39:20] <= RW0_wdata[39:20];
      if (RW0_wmask[2]) ram[RW0_addr][59:40] <= RW0_wdata[59:40];
      if (RW0_wmask[3]) ram[RW0_addr][79:60] <= RW0_wdata[79:60];
    end
  assign RW0_rdata = ram[reg_RW0_addr];

endmodule

module _T_850_ext(
  input RW0_clk,
  input [8:0] RW0_addr,
  input RW0_en,
  input RW0_wmode,
  input [63:0] RW0_wdata,
  output [63:0] RW0_rdata
);

  reg [8:0] reg_RW0_addr;
  reg [63:0] ram [511:0];
  `ifdef RANDOMIZE
    integer initvar;
    initial begin
      #0.002 begin end
      for (initvar = 0; initvar < 512; initvar = initvar+1)
        ram[initvar] = {2 {$random}};
      reg_RW0_addr = {1 {$random}};
    end
  `endif
  integer i;
  always @(posedge RW0_clk)
    if (RW0_en && !RW0_wmode) reg_RW0_addr <= RW0_addr;
  always @(posedge RW0_clk)
    if (RW0_en && RW0_wmode) begin
      ram[RW0_addr][63:0] <= RW0_wdata[63:0];
    end
  assign RW0_rdata = ram[reg_RW0_addr];

endmodule

module mem_ext(
  input W0_clk,
  input [24:0] W0_addr,
  input W0_en,
  input [63:0] W0_data,
  input [7:0] W0_mask,
  input R0_clk,
  input [24:0] R0_addr,
  input R0_en,
  output [63:0] R0_data
);

  reg [24:0] reg_R0_addr;
  reg [63:0] ram [33554431:0];
  `ifdef RANDOMIZE
    integer initvar;
    initial begin
      #0.002 begin end
      for (initvar = 0; initvar < 33554432; initvar = initvar+1)
        ram[initvar] = {2 {$random}};
      reg_R0_addr = {1 {$random}};
    end
  `endif
  integer i;
  always @(posedge R0_clk)
    if (R0_en) reg_R0_addr <= R0_addr;
  always @(posedge W0_clk)
    if (W0_en) begin
      if (W0_mask[0]) ram[W0_addr][7:0] <= W0_data[7:0];
      if (W0_mask[1]) ram[W0_addr][15:8] <= W0_data[15:8];
      if (W0_mask[2]) ram[W0_addr][23:16] <= W0_data[23:16];
      if (W0_mask[3]) ram[W0_addr][31:24] <= W0_data[31:24];
      if (W0_mask[4]) ram[W0_addr][39:32] <= W0_data[39:32];
      if (W0_mask[5]) ram[W0_addr][47:40] <= W0_data[47:40];
      if (W0_mask[6]) ram[W0_addr][55:48] <= W0_data[55:48];
      if (W0_mask[7]) ram[W0_addr][63:56] <= W0_data[63:56];
    end
  assign R0_data = ram[reg_R0_addr];

endmodule

module mem_0_ext(
  input W0_clk,
  input [8:0] W0_addr,
  input W0_en,
  input [63:0] W0_data,
  input [7:0] W0_mask,
  input R0_clk,
  input [8:0] R0_addr,
  input R0_en,
  output [63:0] R0_data
);

  reg [8:0] reg_R0_addr;
  reg [63:0] ram [511:0];
  `ifdef RANDOMIZE
    integer initvar;
    initial begin
      #0.002 begin end
      for (initvar = 0; initvar < 512; initvar = initvar+1)
        ram[initvar] = {2 {$random}};
      reg_R0_addr = {1 {$random}};
    end
  `endif
  integer i;
  always @(posedge R0_clk)
    if (R0_en) reg_R0_addr <= R0_addr;
  always @(posedge W0_clk)
    if (W0_en) begin
      if (W0_mask[0]) ram[W0_addr][7:0] <= W0_data[7:0];
      if (W0_mask[1]) ram[W0_addr][15:8] <= W0_data[15:8];
      if (W0_mask[2]) ram[W0_addr][23:16] <= W0_data[23:16];
      if (W0_mask[3]) ram[W0_addr][31:24] <= W0_data[31:24];
      if (W0_mask[4]) ram[W0_addr][39:32] <= W0_data[39:32];
      if (W0_mask[5]) ram[W0_addr][47:40] <= W0_data[47:40];
      if (W0_mask[6]) ram[W0_addr][55:48] <= W0_data[55:48];
      if (W0_mask[7]) ram[W0_addr][63:56] <= W0_data[63:56];
    end
  assign R0_data = ram[reg_R0_addr];

endmodule
