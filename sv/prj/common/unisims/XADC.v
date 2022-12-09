///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 1995/2015 Xilinx, Inc.
// All Right Reserved.
///////////////////////////////////////////////////////////////////////////////
//   ____  ____
//  /   /\/   /
// /___/  \  /    Vendor : Xilinx
// \   \   \/     Version : 2015.2
//  \   \         Description : Xilinx Timing Simulation Library Component
//  /   /                 System Monitor 
// /___/   /\     Filename  : XADC.v
// \   \  /  \    Timestamp :
//  \___\/\___\
//
// Revision:
//    12/09/09 - Initial version.
//    05/24/10 - Correctly write result to data_reg for ADC2 (CR561364)
//    08/02/10 - Not generate eoc_out2 when ADC2 not used (CR568374)
//    09/09/10 - Change to bus timing
//    10/15/10 - use 273.15 for temperature calculation (CR579001)
//    10/22/10 - Add BUSY to EOS to 18 DCLK instead of 11 (CR579591)
//               Extend calibration cycle to 4 conversions (CR580176)
//    11/11/10 - Match hardware (CR580660 580663 580598)
//    11/16/10 - Set seq_num=5 for sequence mode 0000 (CR579051)
//    11/29/10 - Using curr_chan_lat for data_reg update due to extend of 
//               eoc (CR581758)
//    12/01/10 - Set analog data to fixed channel for external mux and
//               simultaneous sampling mode. (CR580598)
//    12/09/10 - Remove check for clock divider (CR586253)
//    03/03/11 - Add VCCPINT VCCPAUX VCCDDRO channel and SIM_DEVICE attibute 
//               (CR593005)
//    03/03/11 - disable timing check when RESET=1 (CR594618)
//    03/22/11 - use INIT_53 for ot upper limit (CR602195)
//    03/28/11 - Set data_reg range to 47:0 (CR603247)
//    03/29/11 - Set column_real  range to 42:0 (CR602520)
//    08/26/11 - Change ot_limit_reg to CA3h(125 C) (CR623029) 
//               Allow tmp_dr_sram_out out when address 58h (CR623028)
//    09/13/11 - Add ZYNQ to SIM_DEVICE (CR624910)
//               Add curr_seq2_tmps for simultaneous mode (CR621932)
//    12/13/11 - Added `celldefine and `endcelldefine (CR 524859).
//    04/05/12 - Fixed specify block to handle X's (CR 654692).
//    09/26/12 - Removed DRC for event-driven/continuous sampling mode (CR 680075).
//    10/08/12 - Fixed single pass sequence mode (CR 675227).
//    01/18/13 - Added DRP monitor (CR 695630).
//    07/26/13 - Added invertible pins support (CR 715417).
//    08/29/13 - Updated undefined address location (CR 719013).
//    10/28/13 - Removed DRC for event mode timing (CR 736315).
//    11/20/13 - Updated VBRAM to VCCBRAM (CR 755167).
//    01/09/14 - Updated to take both VBRAM and VCCBRAM (CR 767734).
//    03/20/14 - Fixed event driven mode in single pass sequence (CR 764936).
//    03/21/14 - Balanced all inputs with xor (CR 778933).
//    04/30/14 - Initialized chan_val and chan_valn (CR 782388).
//    06/17/14 - Fixed default mode sequencer (CR 800173).
//    10/01/14 - Updated conditional timing check for IS_INVERTED parameter.
//    10/22/14 - Added #1 to $finish (CR 808642).
//    02/04/15 - Fixed DO output with DCLK division 4 or lower (CR 840852).
//    02/27/15 - Fixed alarm in independent ADC mode (CR 847176).
//    06/04/15 - 865391 - Model feature to allow sequence updates without reset
// End Revision


`timescale 1ps / 1ps

`celldefine

module XADC (
        ALM,
        BUSY,
        CHANNEL,
        DO,
        DRDY,
        EOC,
        EOS,
        JTAGBUSY,
        JTAGLOCKED,
        JTAGMODIFIED,
        MUXADDR,
        OT,
        CONVST,
        CONVSTCLK,
        DADDR,
        DCLK,
        DEN,
        DI,
        DWE,
        RESET,
        VAUXN,
        VAUXP,
        VN,
        VP

);

  output BUSY;
  output DRDY;
  output EOC;
  output EOS;
  output JTAGBUSY;
  output JTAGLOCKED;
  output JTAGMODIFIED;
  output OT;
  output [15:0] DO;
  output [7:0] ALM;
  output [4:0] CHANNEL;
  output [4:0] MUXADDR;
  
  input CONVST;
  input CONVSTCLK;
  input DCLK;
  input DEN;
  input DWE;
  input RESET;
  input VN;
  input VP;
  input [15:0] DI;
  input [15:0] VAUXN;
  input [15:0] VAUXP;
  input [6:0] DADDR;

  parameter  [15:0] INIT_40 = 16'h0;
  parameter  [15:0] INIT_41 = 16'h0;
  parameter  [15:0] INIT_42 = 16'h0800;
  parameter  [15:0] INIT_43 = 16'h0;
  parameter  [15:0] INIT_44 = 16'h0;
  parameter  [15:0] INIT_45 = 16'h0;
  parameter  [15:0] INIT_46 = 16'h0;
  parameter  [15:0] INIT_47 = 16'h0;
  parameter  [15:0] INIT_48 = 16'h0;
  parameter  [15:0] INIT_49 = 16'h0;
  parameter  [15:0] INIT_4A = 16'h0;
  parameter  [15:0] INIT_4B = 16'h0;
  parameter  [15:0] INIT_4C = 16'h0;
  parameter  [15:0] INIT_4D = 16'h0;
  parameter  [15:0] INIT_4E = 16'h0;
  parameter  [15:0] INIT_4F = 16'h0;
  parameter  [15:0] INIT_50 = 16'h0;
  parameter  [15:0] INIT_51 = 16'h0;
  parameter  [15:0] INIT_52 = 16'h0;
  parameter  [15:0] INIT_53 = 16'h0;
  parameter  [15:0] INIT_54 = 16'h0;
  parameter  [15:0] INIT_55 = 16'h0;
  parameter  [15:0] INIT_56 = 16'h0;
  parameter  [15:0] INIT_57 = 16'h0;
  parameter  [15:0] INIT_58 = 16'h0;
  parameter  [15:0] INIT_59 = 16'h0;
  parameter  [15:0] INIT_5A = 16'h0;
  parameter  [15:0] INIT_5B = 16'h0;
  parameter  [15:0] INIT_5C = 16'h0;
  parameter  [15:0] INIT_5D = 16'h0;
  parameter  [15:0] INIT_5E = 16'h0;
  parameter  [15:0] INIT_5F = 16'h0;
  parameter IS_CONVSTCLK_INVERTED = 1'b0;
  parameter IS_DCLK_INVERTED = 1'b0;
  parameter SIM_DEVICE = "7SERIES";
  parameter SIM_MONITOR_FILE = "design.txt";

  `ifdef XIL_TIMING

    parameter LOC = "UNPLACED";

  `endif //

  localparam  S1_ST = 0,
              S6_ST = 1,
              S2_ST = 2,
              S3_ST = 3,
              S5_ST = 5,
              S4_ST = 6;

  time    time_out;
  time    prev_time_out;
  
  integer temperature_index = -1, time_index = -1, vccaux_index = -1;
  integer vccbram_index = -1;
  integer vccint_index = -1, vn_index = -1, vp_index = -1;
  integer vccpint_index = -1;
  integer vccpaux_index = -1;
  integer vccpdro_index = -1;
  integer vauxp_idx0 = -1;  
  integer vauxn_idx0 = -1;
  integer vauxp_idx1 = -1;  
  integer vauxn_idx1 = -1;
  integer vauxp_idx2 = -1;  
  integer vauxn_idx2 = -1;
  integer vauxp_idx3 = -1;  
  integer vauxn_idx3 = -1;
  integer vauxp_idx4 = -1;  
  integer vauxn_idx4 = -1;
  integer vauxp_idx5 = -1;  
  integer vauxn_idx5 = -1;
  integer vauxp_idx6 = -1;  
  integer vauxn_idx6 = -1;
  integer vauxp_idx7 = -1;  
  integer vauxn_idx7 = -1;
  integer vauxp_idx8 = -1;  
  integer vauxn_idx8 = -1;
  integer vauxp_idx9 = -1;  
  integer vauxn_idx9 = -1;
  integer vauxp_idx10 = -1; 
  integer vauxn_idx10 = -1;
  integer vauxp_idx11 = -1; 
  integer vauxn_idx11 = -1;
  integer vauxp_idx12 = -1; 
  integer vauxn_idx12 = -1;
  integer vauxp_idx13 = -1; 
  integer vauxn_idx13 = -1;
  integer vauxp_idx14 = -1; 
  integer vauxn_idx14 = -1;
  integer vauxp_idx15 = -1; 
  integer vauxn_idx15 = -1;
  integer num_arg;
  integer num_val;
  integer clk_count;
  integer seq_count; 
  integer seq_count2; 
  integer seq_count_a;
  integer seq_status_avg;
  integer acq_count;
  integer seq_status_avg2;
  integer conv_pj_count [31:0];
  integer conv_pj_count2 [31:0];
  integer conv_acc [31:0];
  integer conv_acc2 [31:0];
  integer conv_result_int;
  integer conv_result_int2;
  integer conv_time;
  integer conv_count; 
  integer conv_time_cal; 
  integer conv_time_cal_1;
  integer file_line;
  integer char_1;
  integer char_2;
  integer fs;
  integer fd;
  integer h, i, j, k, l, m, n, p;

  // string    
  reg [8*12:1]  label0, label1, label2, label3, label4, label5, label6, label7, label8, label9, label10, label11, label12, label13, label14, label15, label16, label17, label18, label19, label20, label21, label22, label23, label24, label25, label26, label27, label28, label29, label30, label31, label32, label33, label34, label35, label36, label37, label38, label39, label40, label41, label42;
  reg [8*600:1] one_line;
  reg [8*12:1]  label [43:0];
  reg [8*12:1]  tmp_label;
  reg           end_of_file;
  
  real tmp_va0, tmp_va1, column_real00, column_real100, column_real101;
  real column_real0, column_real1, column_real2, column_real3, column_real4, column_real5, column_real6, column_real7, column_real8, column_real9, column_real10, column_real11, column_real12, column_real13, column_real14, column_real15, column_real16, column_real17, column_real18, column_real19, column_real20, column_real21, column_real22, column_real23, column_real24, column_real25, column_real26, column_real27, column_real28, column_real29, column_real30, column_real31, column_real32, column_real33, column_real34, column_real35, column_real36, column_real37, column_real38, column_real39, column_real40, column_real41, column_real42;

  // array of real numbers
  //    real column_real [39:0];
  //    reg [63:0] column_real [39:0];
  reg [63:0] column_real [42:0];
  reg [63:0] chan_val [31:0];
  reg [63:0] chan_val_tmp [31:0];
  reg [63:0] chan_valn [31:0];
  reg [63:0] chan_valn_tmp [31:0];
  reg [63:0] mn_in_diff [31:0];
  reg [63:0] mn_in2_diff [31:0];
  reg [63:0] mn_in_uni [31:0];
  reg [63:0] mn_in2_uni [31:0];
  reg [63:0] mn_comm_in [31:0];
  reg [63:0] mn_comm2_in [31:0];

  real chan_val_p_tmp, chan_val_n_tmp;
  real mn_mux_in, mn_in_tmp, mn_comm_in_tmp, mn_in_comm;
  real mn_mux_in2;
  real tmp_v, tmp_v1;
  real adc_temp_result, adc_intpwr_result;
  real adc_temp_result2, adc_intpwr_result2;
  real adc_ext_result;
  real adc_ext_result2;

  reg simd_f;
  reg seq_reset, seq_reset_dly, seq_reset_flag, seq_reset_flag_dly;
  reg soft_reset = 0;
  reg en_data_flag;
  reg first_cal_chan;
  reg seq_en;
  reg seq_en_dly;
  reg seq_en_drp_updt = 1'b0;
  wire [15:0] status_reg;
  reg [15:0] ot_limit_reg = 16'hCA30;
  reg [15:0] tmp_otv;
  //    reg [15:0]  ot_sf_limit_low_reg = 16'hAE40;
  reg [23:0] conv_acc_vec;
  reg [23:0] conv_acc_vec2;
  reg [15:0] conv_result2;
  reg [15:0] conv_result;
  reg [15:0] conv_result_reg;
  reg [15:0] conv_acc_result;
  reg [15:0] conv_result_reg2; 
  reg [15:0] conv_acc_result2;
  wire [7:0] curr_clkdiv_sel;
  reg [7:0]  alarm_out_reg;
  reg [4:0]  curr_chan;
  reg [4:0]  curr_chan_lat;
  reg [4:0]  curr_chan_tmp;
  reg [4:0]  curr_chan2, curr_chan_lat2;
  reg [2:0]  adc_state;
  reg [2:0]  next_state;
  reg conv_start, conv_end;
  reg eos_en, eos_tmp_en;
  reg drdy_out, drdy_out_tmp1, drdy_out_tmp2, drdy_out_tmp3, drdy_out_tmp4;
  reg ot_out_reg;
  reg [15:0] do_out;
  reg [15:0] do_out_rdtmp;
  reg [15:0] data_reg [47:0];
  reg [15:0] dr_sram [127:64];
  reg sysclk, adcclk_tmp;
  wire adcclk;
  wire adcclk_r;
  wire xadc_en, xadc2_en;
  reg [3:0] curr_seq1_0, curr_seq1_0_lat;
  reg [1:0] tmp_seq1_0 = 2'b00;
  reg curr_e_c, curr_b_u, curr_acq;
  reg ext_mux;
  reg curr_e_c2, curr_b_u2, curr_acq2;
  reg seq_count_en;
  reg [4:0] acq_chan, acq_chan2, acq_chan_m;
  reg [4:0] ext_mux_chan, ext_mux_chan2;
  reg acq_b_u, acq_b_u2;
  reg adc_s1_flag, acq_acqsel;
  wire acq_e_c;
  reg acq_e_c_tmp5, acq_e_c_tmp6;
  reg [1:0] curr_pj_set, curr_pj_set_lat;
  reg [1:0] curr_pj_set2, curr_pj_set_lat2;
  reg eoc_en, eoc_en_delay;
  reg eoc_en2, eoc_en_delay2;
  reg eoc_out_tmp, eos_out_tmp;
  reg eoc_out_tmp2;
  reg eoc_out_tmp1, eos_out_tmp1;
  reg eoc_out_tmp21;
  reg eoc_out, eos_out;
  reg eoc_out2, eoc_out_t;
  reg eoc_last = 1'b0;
  integer eoc_last_count;
  reg busy_r, busy_r_rst;
  reg busy_sync1, busy_sync2;
  wire busy_sync_fall, busy_sync_rise;
  reg [4:0] channel_out; 
  wire [4:0]  muxaddr_o; 
  reg  [4:0]  muxaddr_out;
  reg rst_lock, rst_lock_early, rst_lock_late;
  reg sim_file_flag;
  reg [6:0] daddr_in_lat;
  reg [15:0] init40h_tmp, init41h_tmp, init42h_tmp, init4eh_tmp;
  reg [7:0] alarm_out;
  reg       ot_out;
  reg [15:0] curr_seq,  curr_seq_m;
  reg [15:0] curr_seq2_tmp, curr_seq2_tmps;
  wire [15:0] curr_seq2;
  reg busy_out, busy_rst, busy_conv, busy_out_tmp, busy_seq_rst;
  reg [3:0] seq1_0, seq_bits;
  reg ot_en, alarm_update, drp_update, cal_chan_update;
  reg [6:0] alarm_en;
  reg [4:0] scon_tmp;
  wire [15:0] seq_chan_reg1, seq_chan_reg2, seq_acq_reg1, seq_acq_reg2;
  wire [15:0] seq_pj_reg1, seq_pj_reg2, seq_du_reg1, seq_du_reg2;
  reg [15:0] cfg_reg1_init;
 
  reg [4:0] seq_curr_i, seq_curr_i2, seq_curr_ia;
  integer busy_rst_cnt;
  integer si, seq_num, seq_num2;
  integer first_ch = 0;
  reg skip_updt = 1'b0;
  integer seq_mem [32:0];
  integer seq_mem2 [32:0];

  wire rst_in, adc_convst;
  wire [15:0] cfg_reg0;
  wire [15:0] cfg_reg1;
  wire [15:0] cfg_reg2;
  wire [15:0] di_in;
  wire [6:0] daddr_in;
  wire [15:0] tmp_data_reg_out, tmp_dr_sram_out;
  wire convst_in_tmp;
  reg  convst_in;
  wire rst_in_not_seq;
  wire adcclk_div1;
  wire gsr_in;
  wire convst_raw_in, convstclk_in, dclk_in, den_in, rst_input, dwe_in;
  wire DCLK_dly, DEN_dly, DWE_dly;
  wire [6:0] DADDR_dly;
  wire [15:0] DI_dly;
  wire dclk_inv, convstclk_inv;
  wire convst_raw_inv, den_inv, dwe_inv, rst_input_inv;
  wire [15:0] di_inv;
  wire [6:0] daddr_inv;
  reg attr_err = 1'b0;


  // initialize chan_val and chan_valn
  integer ii, jj;
  
  initial begin
    for (ii = 0; ii < 32; ii = ii + 1) begin
      chan_val[ii] = 64'h0000000000000000;
    end
    for (jj = 0; jj < 32; jj = jj + 1) begin
      chan_valn[jj] = 64'h0000000000000000;
    end
  end
   

   //drp monitor
   reg den_r1 = 1'b0;
   reg den_r2 = 1'b0;
//   reg dwe_r1 = 1'b0;
//   reg dwe_r2 = 1'b0;
   
   reg [1:0] sfsm = 2'b01;
    
   localparam FSM_IDLE = 2'b01;  
   localparam FSM_WAIT = 2'b10;
  

  always @(posedge dclk_in) begin
    // pipeline the DEN and DWE
    den_r1 <= den_in;
//    dwe_r1 <= dwe_in;
    den_r2 <= den_r1;
//    dwe_r2 <= dwe_r1;

    // Check -  if DEN or DWE is more than 1 DCLK
    if ((den_r1 == 1'b1) && (den_r2 == 1'b1))  begin
      $display("DRC Error : DEN is high for more than 1 DCLK on %m instance");
      $finish; 
    end
    
    // DWE can only be 1 for 2 CLK if DEN is 1 for 2 clk which is not allowed.
    // DWE ignored when DEN = 0
//    if ((dwe_r1 == 1'b1) && (dwe_r2 == 1'b1))  begin
//      $display("DRC Error : DWE is high for more than 1 DCLK on %m instance");
//      $finish;
//    end

    //After the 1st DEN pulse, check the DEN and DRDY.
    case (sfsm)
      FSM_IDLE:  
        begin
          if(den_in == 1'b1)
            sfsm <= FSM_WAIT;  
        end
          
      FSM_WAIT:
        begin
          // After the 1st DEN, 4 cases can happen
          // DEN DRDY NEXT STATE
          // 0     0      FSM_WAIT - wait for DRDY
          // 0     1      FSM_IDLE - normal operation
          // 1     0      FSM_WAIT - display error and wait for DRDY
          // 1     1      FSM_WAIT - normal operation. Per UG470, DEN and DRDY can be at the same cycle.
          
          //Add the check for another DPREN pulse
          if(den_in === 1'b1 && drdy_out === 1'b0)  begin
             $display("DRC Error : DEN is enabled before DRDY returns on %m instance");  
             $finish;
          end

          //Add the check for another DWE pulse
          // DWE ignored when DEN = 0
//          if ((dwe_in === 1'b1) && (den_in === 1'b0)) begin
//           $display("DRC Error : DWE is enabled before DRDY returns on %m instance");
//           $finish;
//          end
                    
          if ((drdy_out === 1'b1) && (den_in === 1'b0)) begin
            sfsm <= FSM_IDLE;
          end  
               
          if ((drdy_out === 1'b1)&& (den_in === 1'b1)) begin
            sfsm <= FSM_WAIT;
          end  
        end
        
      default:                  
        begin
          $display("DRC Error : Default state in DRP FSM.");
          $finish;
        end
    endcase

  end // always @ (posedge DCLK)
  //end drp monitor
   
   
  //CR 675227
  integer halt_adc = 0;
  reg int_rst;
  reg int_rst_halt_adc = 0;
  
  always @(posedge rst_input)
  halt_adc <= 0;

  always @(seq1_0) begin
    if (halt_adc == 2 && seq1_0 == 4'b0001) begin
      halt_adc <= 0;
      int_rst_halt_adc <= 1;
      @(posedge dclk_in)
      int_rst_halt_adc <= 0;
    end
  end
    
  tri0 GSR = glbl.GSR;

`ifndef XIL_TIMING

  assign  BUSY = busy_out;
  assign  DRDY = drdy_out;
  assign  EOC = eoc_out;
  assign  EOS = eos_out;
  assign  OT = ot_out;
  assign  DO = do_out;
  assign  CHANNEL = channel_out;
  assign  MUXADDR = muxaddr_out;
  assign  ALM = alarm_out;
  
  assign convst_raw_inv = CONVST;
  assign convstclk_inv = CONVSTCLK;
  assign dclk_inv = DCLK;
  assign den_inv = DEN;
  assign rst_input_inv = RESET;
  assign dwe_inv = DWE;
  assign di_inv = DI; 
  assign daddr_inv = DADDR;

`endif //  `ifndef XIL_TIMING

`ifdef XIL_TIMING

  assign  BUSY = busy_out;
  assign  DRDY = drdy_out;
  assign  EOC = eoc_out;
  assign  EOS = eos_out;
  assign  OT = ot_out;
  assign  DO = do_out;
  assign  CHANNEL = channel_out;
  assign  MUXADDR = muxaddr_out;
  assign  ALM = alarm_out;
  
  assign convst_raw_inv = CONVST;
  assign convstclk_inv = CONVSTCLK;
  assign dclk_inv = DCLK_dly;
  assign den_inv = DEN_dly;
  assign rst_input_inv = RESET;
  assign dwe_inv = DWE_dly;
  assign di_inv = DI_dly; 
  assign daddr_inv = DADDR_dly;

`endif //  `ifdef XIL_TIMING

  assign convst_raw_in = convst_raw_inv ^ 1'b0;
  assign den_in = den_inv ^ 1'b0;
  assign dwe_in = dwe_inv ^ 1'b0;
  assign rst_input = rst_input_inv ^ 1'b0;
  assign di_in = di_inv ^ 16'h0000;
  assign daddr_in = daddr_inv ^ 7'b0000000;
  
  assign convstclk_in = convstclk_inv ^ IS_CONVSTCLK_INVERTED;
  assign dclk_in = dclk_inv ^ IS_DCLK_INVERTED;
  assign gsr_in = GSR;
  assign convst_in_tmp = (convst_raw_in===1 || convstclk_in===1) ? 1: 0;
  assign JTAGLOCKED = 0;
  assign JTAGMODIFIED = 0;
  assign JTAGBUSY = 0;

  
  always @(posedge convst_in_tmp or negedge convst_in_tmp or posedge rst_in)  begin
    if (rst_in == 1 || rst_lock == 1)
      convst_in <= 0;
    else if (convst_in_tmp == 1)
      convst_in <= 1;
    else if (convst_in_tmp == 0)
        convst_in <= 0;
  end


  initial begin

    case (SIM_DEVICE)
      "7SERIES" : simd_f = 0;
      "ZYNQ" : simd_f = 1;
      default : begin
        $display("Attribute Syntax Error : The Attribute SIM_DEVICE on XADC instance %m is set to %s.  Legal values for this attribute are 7SERIES, or ZYNQ.", SIM_DEVICE);
        #1 $finish;
      end
    endcase

    init40h_tmp = INIT_40;
    init41h_tmp = INIT_41;
    init42h_tmp = INIT_42;
    init4eh_tmp = INIT_4E;

    if ((init41h_tmp[15:12]==4'b0011) && (init40h_tmp[8]==1) && (init40h_tmp[4:0] != 5'b00011) && (init40h_tmp[4:0] < 5'b10000))
      $display(" Attribute Syntax warning : The attribute INIT_40 on XADC instance %m is set to %x.  Bit[8] of this attribute must be set to 0. Long acquistion mode is only allowed for external channels", INIT_40);

    if ((init41h_tmp[15:12]!=4'b0011) && (init4eh_tmp[10:0]!=11'b0) && (init4eh_tmp[15:12]!=4'b0))
      $display(" Attribute Syntax warning : The attribute INIT_4E on XADC instance %m is set to %x.  Bit[15:12] and bit[10:0] of this attribute must be set to 0. Long acquistion mode is only allowed for external channels", INIT_4E);

    //if ((init41h_tmp[15:12]==4'b0011) && (init40h_tmp[9]==1) && (init40h_tmp[4:0] != 5'b00011) && (init40h_tmp[4:0] < 5'b10000))
    //  $display(" Attribute Syntax warning : The attribute INIT_40 on XADC instance %m is set to %x.  Bit[9] of this attribute must be set to 0. Event mode timing can only be used with external channels, and only in single channel mode.", INIT_40);

    if ((init41h_tmp[15:12]==4'b0011) && (init40h_tmp[13:12]!=2'b00) && (INIT_48 != 16'h0000) &&  (INIT_49 != 16'h0000))
      $display(" Attribute Syntax warning : INIT_48 and INIT_49 are %x and %x on XADC instance %m. Those attributes must be set to 0000h in single channel mode and averaging enabled.", INIT_48, INIT_49);
 
    if (init42h_tmp[1:0] != 2'b00) 
      $display(" Attribute Syntax Error : The attribute INIT_42 on XADC instance %m is set to %x.  Bit[1:0] of this attribute must be set to 0h.", INIT_42);

    //if (init42h_tmp[15:8] < 8'b00000010) begin 
    //  $display(" Attribute Syntax Error : The attribute INIT_42 on XADC instance %m is set to %x.  Bit[15:8] of this attribute is the ADC Clock divider and must be equal or greater than 2. ", INIT_42);
    //  $finish;
    //end
        
     if (INIT_43 != 16'h0) 
       $display(" Warning : The attribute INIT_43 on XADC instance %m is set to %x.  This must be set to 0000h.", INIT_43);
          
     if (INIT_44 != 16'h0) 
       $display(" Warning : The attribute INIT_44 on XADC instance %m is set to %x. This must be set to  0000h.", INIT_44);

     if (INIT_45 != 16'h0) 
       $display(" Warning : The attribute INIT_45 on XADC instance %m is set to %x.  This must be set to 0000h.", INIT_45);

     if (INIT_46 != 16'h0) 
       $display(" Warning : The attribute INIT_46 on XADC instance %m is set to %x.  This must be set to  0000h.", INIT_46);

     if (INIT_47 != 16'h0) 
       $display(" Warning : The attribute INIT_47 on XADC instance %m is set to %x.  This must be set to 0000h.", INIT_47);

        
     if (!((IS_CONVSTCLK_INVERTED >= 1'b0) && (IS_CONVSTCLK_INVERTED <= 1'b1))) begin
       $display("Attribute Syntax Error : The attribute IS_CONVSTCLK_INVERTED on XADC instance %m is set to %b.  Legal values for this attribute are 1'b0 to 1'b1.", IS_CONVSTCLK_INVERTED);
       attr_err = 1'b1;
     end

     if (!((IS_DCLK_INVERTED >= 1'b0) && (IS_DCLK_INVERTED <= 1'b1))) begin
       $display("Attribute Syntax Error : The attribute IS_DCLK_INVERTED on XADC instance %m is set to %b.  Legal values for this attribute are 1'b0 to 1'b1.", IS_DCLK_INVERTED);
       attr_err = 1'b1;
     end  

     if (attr_err == 1'b1) 
       #1 $finish;

  end

  initial begin
    dr_sram[7'h40] = INIT_40;
    dr_sram[7'h41] = INIT_41;
    dr_sram[7'h42] = INIT_42;
    dr_sram[7'h43] = INIT_43;
    dr_sram[7'h44] = INIT_44;
    dr_sram[7'h45] = INIT_45;
    dr_sram[7'h46] = INIT_46;
    dr_sram[7'h47] = INIT_47;
    dr_sram[7'h48] = INIT_48;
    dr_sram[7'h49] = INIT_49;
    dr_sram[7'h4A] = INIT_4A;
    dr_sram[7'h4B] = INIT_4B;
    dr_sram[7'h4C] = INIT_4C;
    dr_sram[7'h4D] = INIT_4D;
    dr_sram[7'h4E] = INIT_4E;
    dr_sram[7'h4F] = INIT_4F;
    dr_sram[7'h50] = INIT_50;
    dr_sram[7'h51] = INIT_51;
    dr_sram[7'h52] = INIT_52;
    tmp_otv = INIT_53;

    if (tmp_otv [3:0] == 4'b0011) begin
      dr_sram[7'h53] = INIT_53;
      ot_limit_reg  = INIT_53;
    end
    else begin
      dr_sram[7'h53] = 16'hCA30;
      ot_limit_reg  = 16'hCA30;
    end

    dr_sram[7'h54] = INIT_54;
    dr_sram[7'h55] = INIT_55;
    dr_sram[7'h56] = INIT_56;
    dr_sram[7'h57] = INIT_57;
    dr_sram[7'h58] = INIT_58;
    dr_sram[7'h59] = INIT_59;
    dr_sram[7'h5A] = INIT_5A;
    dr_sram[7'h5B] = INIT_5B;
    dr_sram[7'h5C] = INIT_5C;
    dr_sram[7'h5D] = INIT_5D;
    dr_sram[7'h5E] = INIT_5E;
    dr_sram[7'h5F] = INIT_5F;
  end // initial begin

  // read input file
  initial begin
    char_1 = 0;
    char_2 = 0;
    time_out = 0;
    sim_file_flag = 0;
    file_line = -1;
    end_of_file = 0;
    fd = $fopen(SIM_MONITOR_FILE, "r"); 

    if  (fd == 0) begin
      $display(" *** Warning: The analog data file %s for XADC instance %m was not found. Use the SIM_MONITOR_FILE parameter to specify the analog data file name or use the default name: design.txt.\n", SIM_MONITOR_FILE);
      sim_file_flag = 1;
    end
    
    if (sim_file_flag == 0) begin
      while (end_of_file==0) begin
        file_line = file_line + 1;
        char_1 = $fgetc (fd);
        char_2 = $fgetc (fd);
        // if(char_2==`EOFile) 
        if(char_2== -1) 
          end_of_file = 1;
        else begin
          // Ignore Comments
          if ((char_1 == "/" & char_2 == "/") | char_1 == "#" | (char_1 == "-" & char_2 == "-")) begin
            fs = $ungetc (char_2, fd);
            fs = $ungetc (char_1, fd);
            fs = $fgets (one_line, fd);
          end
          // Getting labels
          else if ((char_1 == "T" & char_2 == "I" ) ||
                   (char_1 == "T" & char_2 == "i" )  ||
                   (char_1 == "t" & char_2 == "i" ) || (char_1 == "t" & char_2 == "I" ))  begin
        
            fs = $ungetc (char_2, fd);
            fs = $ungetc (char_1, fd);
            fs = $fgets (one_line, fd);

            num_arg = $sscanf (one_line, "%s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s", label0, label1, label2, label3, label4, label5, label6, label7, label8, label9, label10, label11, label12, label13, label14, label15, label16, label17, label18, label19, label20, label21, label22, label23, label24, label25, label26, label27, label28, label29, label30,label31, label32, label33, label34, label35, label36, label37, label38, label39, label40, label41, label42);
        
            label[0] = label0;
            label[1] = label1;
            label[2] = label2;
            label[3] = label3;
            label[4] = label4;
            label[5] = label5;
            label[6] = label6;
            label[7] = label7;
            label[8] = label8;
            label[9] = label9;
            label[10] = label10;
            label[11] = label11;
            label[12] = label12;
            label[13] = label13;
            label[14] = label14;
            label[15] = label15;
            label[16] = label16;
            label[17] = label17;
            label[18] = label18;
            label[19] = label19;
            label[20] = label20;
            label[21] = label21;
            label[22] = label22;
            label[23] = label23;
            label[24] = label24;
            label[25] = label25;
            label[26] = label26;
            label[27] = label27;
            label[28] = label28;
            label[29] = label29;
            label[30] = label30;
            label[31] = label31;
            label[32] = label32;
            label[33] = label33;
            label[34] = label34;
            label[35] = label35;
            label[36] = label36;
            label[37] = label37;
            label[38] = label38;
            label[39] = label39;
            label[40] = label40;
            label[41] = label41;
            label[42] = label42;
        
            for (m = 0; m < num_arg; m = m +1) begin
              tmp_label = 96'b0;
              tmp_label = to_upcase_label(label[m]);

              case (tmp_label)
                "TEMP"            : temperature_index = m; 
                "TIME"            : time_index = m;
                "VCCAUX"          : vccaux_index = m;
                "VCCINT"          : vccint_index = m;
                "VCCBRAM","VBRAM" : vccbram_index = m;
                "VCCPINT"         : vccpint_index = m;
                "VCCPAUX"         : vccpaux_index = m;
                "VCCDDRO"         : vccpdro_index = m;
                "VN"              : vn_index = m;
                "VAUXN[0]"        : vauxn_idx0 = m;
                "VAUXN[1]"        : vauxn_idx1 = m;
                "VAUXN[2]"        : vauxn_idx2 = m;
                "VAUXN[3]"        : vauxn_idx3 = m;
                "VAUXN[4]"        : vauxn_idx4 = m;
                "VAUXN[5]"        : vauxn_idx5 = m;
                "VAUXN[6]"        : vauxn_idx6 = m;
                "VAUXN[7]"        : vauxn_idx7 = m;
                "VAUXN[8]"        : vauxn_idx8 = m;
                "VAUXN[9]"        : vauxn_idx9 = m;
                "VAUXN[10]"       : vauxn_idx10 = m;
                "VAUXN[11]"       : vauxn_idx11 = m;
                "VAUXN[12]"       : vauxn_idx12 = m;
                "VAUXN[13]"       : vauxn_idx13 = m;
                "VAUXN[14]"       : vauxn_idx14 = m;
                "VAUXN[15]"       : vauxn_idx15 = m;
                "VP"              : vp_index = m;
                "VAUXP[0]"        : vauxp_idx0 = m;
                "VAUXP[1]"        : vauxp_idx1 = m;
                "VAUXP[2]"        : vauxp_idx2 = m;
                "VAUXP[3]"        : vauxp_idx3 = m;
                "VAUXP[4]"        : vauxp_idx4 = m;
                "VAUXP[5]"        : vauxp_idx5 = m;
                "VAUXP[6]"        : vauxp_idx6 = m;
                "VAUXP[7]"        : vauxp_idx7 = m;
                "VAUXP[8]"        : vauxp_idx8 = m;
                "VAUXP[9]"        : vauxp_idx9 = m;
                "VAUXP[10]"       : vauxp_idx10 = m;
                "VAUXP[11]"       : vauxp_idx11 = m;
                "VAUXP[12]"       : vauxp_idx12 = m;
                "VAUXP[13]"       : vauxp_idx13 = m;
                "VAUXP[14]"       : vauxp_idx14 = m;
                "VAUXP[15]"       : vauxp_idx15 = m;
                default           : begin
                                      $display("analog Data File Error : The channel name %s is invalid in the input file for XADC instance %m.", tmp_label);
                                      infile_format;
                                    end
              endcase
            end // for (m = 0; m < num_arg; m = m +1)
          end
          // Getting column values
          else if (char_1 == "0" | char_1 == "1" | char_1 == "2" | char_1 == "3" | char_1 == "4" | char_1 == "5" | char_1 == "6" | char_1 == "7" | char_1 == "8" | char_1 == "9") begin
            
            fs = $ungetc (char_2, fd);
            fs = $ungetc (char_1, fd);
            fs = $fgets (one_line, fd);

            column_real0 = 0.0;
            column_real1 = 0.0;
            column_real2 = 0.0;
            column_real3 = 0.0;
            column_real4 = 0.0;
            column_real5 = 0.0;
            column_real6 = 0.0;
            column_real7 = 0.0;
            column_real8 = 0.0;
            column_real9 = 0.0;
            column_real10 = 0.0;
            column_real11 = 0.0;
            column_real12 = 0.0;
            column_real13 = 0.0;
            column_real14 = 0.0;
            column_real15 = 0.0;
            column_real16 = 0.0;
            column_real17 = 0.0;
            column_real18 = 0.0;
            column_real19 = 0.0;
            column_real20 = 0.0;
            column_real21 = 0.0;
            column_real22 = 0.0;
            column_real23 = 0.0;
            column_real24 = 0.0;
            column_real25 = 0.0;
            column_real26 = 0.0;
            column_real27 = 0.0;
            column_real28 = 0.0;
            column_real29 = 0.0;
            column_real30 = 0.0;
            column_real31 = 0.0;
            column_real32 = 0.0;
            column_real33 = 0.0;
            column_real34 = 0.0;
            column_real35 = 0.0;
            column_real36 = 0.0;
            column_real37 = 0.0;
            column_real38 = 0.0;
            column_real39 = 0.0;
            column_real40 = 0.0;
            column_real41 = 0.0;
            column_real42 = 0.0;
              
            num_val = $sscanf (one_line, "%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f", column_real0, column_real1, column_real2, column_real3, column_real4, column_real5, column_real6, column_real7, column_real8, column_real9, column_real10, column_real11, column_real12, column_real13, column_real14, column_real15, column_real16, column_real17, column_real18, column_real19, column_real20, column_real21, column_real22, column_real23, column_real24, column_real25, column_real26, column_real27, column_real28, column_real29, column_real30, column_real31, column_real32, column_real33, column_real34, column_real35, column_real36, column_real37, column_real38, column_real39, column_real40, column_real41, column_real42);

            column_real[0] = $realtobits(column_real0);
            column_real[1] = $realtobits(column_real1);
            column_real[2] = $realtobits(column_real2);
            column_real[3] = $realtobits(column_real3);
            column_real[4] = $realtobits(column_real4);
            column_real[5] = $realtobits(column_real5);
            column_real[6] = $realtobits(column_real6);
            column_real[7] = $realtobits(column_real7);
            column_real[8] = $realtobits(column_real8);
            column_real[9] = $realtobits(column_real9);
            column_real[10] = $realtobits(column_real10);
            column_real[11] = $realtobits(column_real11);
            column_real[12] = $realtobits(column_real12);
            column_real[13] = $realtobits(column_real13);
            column_real[14] = $realtobits(column_real14);
            column_real[15] = $realtobits(column_real15);
            column_real[16] = $realtobits(column_real16);
            column_real[17] = $realtobits(column_real17);
            column_real[18] = $realtobits(column_real18);
            column_real[19] = $realtobits(column_real19);
            column_real[20] = $realtobits(column_real20);
            column_real[21] = $realtobits(column_real21);
            column_real[22] = $realtobits(column_real22);
            column_real[23] = $realtobits(column_real23);
            column_real[24] = $realtobits(column_real24);
            column_real[25] = $realtobits(column_real25);
            column_real[26] = $realtobits(column_real26);
            column_real[27] = $realtobits(column_real27);
            column_real[28] = $realtobits(column_real28);
            column_real[29] = $realtobits(column_real29);
            column_real[30] = $realtobits(column_real30);
            column_real[31] = $realtobits(column_real31);
            column_real[32] = $realtobits(column_real32);
            column_real[33] = $realtobits(column_real33);
            column_real[34] = $realtobits(column_real34);
            column_real[35] = $realtobits(column_real35);
            column_real[36] = $realtobits(column_real36);
            column_real[37] = $realtobits(column_real37);
            column_real[38] = $realtobits(column_real38);
            column_real[39] = $realtobits(column_real39);
            column_real[40] = $realtobits(column_real40);
            column_real[41] = $realtobits(column_real41);
            column_real[42] = $realtobits(column_real42);
            
            chan_val[0] = column_real[temperature_index];
            chan_val[1] = column_real[vccint_index];
            chan_val[2] = column_real[vccaux_index];
            chan_val[3] = column_real[vp_index];
            chan_val[6] = column_real[vccbram_index];
            chan_val[13] = column_real[vccpint_index];
            chan_val[14] = column_real[vccpaux_index];
            chan_val[15] = column_real[vccpdro_index];
            chan_val[16] = column_real[vauxp_idx0];
            chan_val[17] = column_real[vauxp_idx1];
            chan_val[18] = column_real[vauxp_idx2];
            chan_val[19] = column_real[vauxp_idx3];
            chan_val[20] = column_real[vauxp_idx4];
            chan_val[21] = column_real[vauxp_idx5];
            chan_val[22] = column_real[vauxp_idx6];
            chan_val[23] = column_real[vauxp_idx7];
            chan_val[24] = column_real[vauxp_idx8];
            chan_val[25] = column_real[vauxp_idx9];
            chan_val[26] = column_real[vauxp_idx10];
            chan_val[27] = column_real[vauxp_idx11];
            chan_val[28] = column_real[vauxp_idx12];
            chan_val[29] = column_real[vauxp_idx13];
            chan_val[30] = column_real[vauxp_idx14];
            chan_val[31] = column_real[vauxp_idx15];

            chan_valn[3] = column_real[vn_index];
            chan_valn[16] = column_real[vauxn_idx0];
            chan_valn[17] = column_real[vauxn_idx1];
            chan_valn[18] = column_real[vauxn_idx2];
            chan_valn[19] = column_real[vauxn_idx3];
            chan_valn[20] = column_real[vauxn_idx4];
            chan_valn[21] = column_real[vauxn_idx5];
            chan_valn[22] = column_real[vauxn_idx6];
            chan_valn[23] = column_real[vauxn_idx7];
            chan_valn[24] = column_real[vauxn_idx8];
            chan_valn[25] = column_real[vauxn_idx9];
            chan_valn[26] = column_real[vauxn_idx10];
            chan_valn[27] = column_real[vauxn_idx11];
            chan_valn[28] = column_real[vauxn_idx12];
            chan_valn[29] = column_real[vauxn_idx13];
            chan_valn[30] = column_real[vauxn_idx14];
            chan_valn[31] = column_real[vauxn_idx15];
                 
            // identify columns
            if (time_index != -1) begin
              prev_time_out = time_out;
              time_out = $bitstoreal(column_real[time_index]);
              if (prev_time_out > time_out) begin
                $display("analog Data File Error : Time value %f is invalid in the input file for XADC instance %m. Time value should increase.", time_out);
                infile_format;
              end
            end     
            else begin
              $display("analog Data File Error : No TIME label is found in the analog data file for XADC instance %m.");
              infile_format;
              #1 $finish;
            end
            # ((time_out - prev_time_out) * 1000);
            for (p = 0; p < 32; p = p + 1) begin
              // assign to real before minus - to work around a bug in modelsim
              chan_val_tmp[p]  = chan_val[p];
              chan_valn_tmp[p] = chan_valn[p];
              mn_in_tmp        = $bitstoreal(chan_val[p])  - $bitstoreal(chan_valn[p]);
              mn_in_diff[p]    = $realtobits(mn_in_tmp);
              mn_in_uni[p]     = chan_val[p];
            end
            //# ((time_out - prev_time_out) * 1000);
          end // if (char_1 == "0" | char_1 == "9")
            // Ignore any non-comment, label
          else begin
            fs = $ungetc (char_2, fd);
            fs = $ungetc (char_1, fd);
            fs = $fgets (one_line, fd);    
          end
        end 
      end // while (end_file == 0)
    end // if (sim_file_flag == 0)
  end // initial begin

  task infile_format;
    begin
      $display("\n***** XADC Simulation analog Data File Format *****\n");
      $display("NAME: design.txt or user file name passed with parameter/generic SIM_MONITOR_FILE\n");
      $display("FORMAT: First line is header line. Valid column name are: TIME TEMP VCCINT VCCAUX VCCBRAM VCCPINT VCCPAUX VCCDDRO VP VN VAUXP[0] VAUXN[0] ..... \n");
      $display("TIME must be in first column.\n");
      $display("Time value need to be integer in ns scale.\n");
      $display("analog value need to be real and must contain a decimal point '.' ,  e.g. 0.0, 3.0\n");
      $display("Each line including header line can not have extra space after the last character/digit.\n");
      $display("Each data line must have same number of columns as the header line.\n");
      $display("Comment line start with -- or //\n");
      $display("Example:\n");
      $display("TIME TEMP VCCINT  VP VN VAUXP[0] VAUXN[0]\n");
      $display("000  125.6  1.0  0.7  0.4  0.3  0.6\n");
      $display("200  25.6   0.8  0.5  0.3  0.8  0.2\n");
    end
  endtask  //task infile_format

  function [12*8:1] to_upcase_label;
    input  [12*8:1] in_label;
    reg [8:1] tmp_reg;
    begin
      for (i=0; i< 12; i=i+1) begin
        for (j=1; j<=8; j= j+1)
          tmp_reg[j] = in_label[i*8+j];
          if ((tmp_reg >96) && (tmp_reg<123))
            tmp_reg = tmp_reg -32;
          for (j=1; j<=8; j= j+1)
            to_upcase_label[i*8+j] = tmp_reg[j];
      end
    end
  endfunction
  // end read input file

  // Check if (Vp+Vn)/2 = 0.5 +/- 100 mv,  unipolar only
  always @( posedge busy_r ) begin
    if (acq_b_u == 0 && rst_in == 0 && ((acq_chan == 3) || (acq_chan >= 16 && acq_chan <= 31))) begin  
      chan_val_p_tmp = $bitstoreal(chan_val_tmp[acq_chan]);
      chan_val_n_tmp = $bitstoreal(chan_valn_tmp[acq_chan]);

      if ( chan_val_n_tmp > chan_val_p_tmp)
      $display("Input File Warning: The N input for external channel %x must be smaller than P input when in unipolar mode (P=%0.2f N=%0.2f) for XADC instance %m at %.3f ns.", acq_chan, chan_val_p_tmp, chan_val_n_tmp, $time/1000.0);
      if ( chan_val_n_tmp > 0.5 || chan_val_n_tmp < 0.0)
      $display("Input File Warning: The range of N input for external channel %x should be between 0V to 0.5V when in unipolar mode (N=%0.2f) for XADC instance %m at %.3f ns.", acq_chan, chan_val_n_tmp, $time/1000.0);
    end
    if ((seq1_0[3:2] == 2'b01 || seq1_0[3:2] == 2'b10) && acq_b_u == 0 && rst_in == 0 && ((acq_chan2 == 3) || (acq_chan2 >= 16 && acq_chan2 <= 31))) begin  
      chan_val_p_tmp = $bitstoreal(chan_val_tmp[acq_chan2]);
      chan_val_n_tmp = $bitstoreal(chan_valn_tmp[acq_chan2]);

      if ( chan_val_n_tmp > chan_val_p_tmp)
      $display("Input File Warning: The N input for external channel %x must be smaller than P input when in unipolar mode (P=%0.2f N=%0.2f) for XADC instance %m at %.3f ns.", acq_chan2, chan_val_p_tmp, chan_val_n_tmp, $time/1000.0);
      if ( chan_val_n_tmp > 0.5 || chan_val_n_tmp < 0.0)
      $display("Input File Warning: The range of N input for external channel %x should be between 0V to 0.5V when in unipolar mode (N=%0.2f) for XADC instance %m at %.3f ns.", acq_chan2, chan_val_n_tmp, $time/1000.0);
    end
  end

  reg seq_reset_busy_out = 0;
  wire rst_in_out;

  always @(posedge dclk_in or posedge rst_in_out) begin
    if (rst_in_out) begin
      busy_rst <= 1;
      rst_lock <= 1;
      rst_lock_early <= 1;
      rst_lock_late <= 1;
      busy_rst_cnt <= 0;
    end
    else begin
      if (rst_lock == 1) begin
        if (busy_rst_cnt < 29) begin
          busy_rst_cnt <= busy_rst_cnt + 1;
          if ( busy_rst_cnt == 26)
            rst_lock_early <= 0;
        end
        else begin
          busy_rst <= 0;
          rst_lock = 0;
        end
      end
      if (busy_out == 0)
        rst_lock_late <= 0;
    end
  end
  
  initial begin
   busy_out = 0;
   busy_rst = 0;
   busy_conv = 0;
   busy_seq_rst = 0;
   busy_out_tmp = 0;
  end 

  always @(busy_rst or busy_conv or rst_lock) begin
    if (rst_lock)
      busy_out = busy_rst;
    else
      busy_out = busy_conv;
  end

  always @(posedge dclk_in or posedge rst_in) begin
     if (rst_in) begin
       busy_conv <= 0;
       cal_chan_update <= 0;
     end
     else begin
       if (seq_reset_flag == 1 && curr_clkdiv_sel <= 8'h03) 
         busy_conv <= busy_seq_rst; 
       else if (busy_sync_fall)
         busy_conv <= 0;
         else if (busy_sync_rise)
           busy_conv <= 1;
         if (conv_count == 21 && curr_chan == 5'b01000)
           cal_chan_update  <= 1;
         else
           cal_chan_update  <= 0;
     end
  end

  always @(posedge dclk_in or rst_lock) begin
    if (rst_lock) begin
      busy_sync1 <= 0;
      busy_sync2 <= 0;
    end
    else begin
      busy_sync1 <= busy_r;
      busy_sync2 <= busy_sync1;
    end 
  end

  assign busy_sync_fall = (busy_r == 0 && busy_sync1 == 1) ? 1 : 0;
  assign busy_sync_rise = (busy_sync1 == 1 && busy_sync2 == 0 ) ? 1 : 0;

  always @(negedge busy_out or posedge busy_r) begin
    if (seq_reset_flag == 1 && seq1_0 == 4'b0000 && curr_clkdiv_sel <= 8'h03) begin
      repeat (5) @(posedge dclk_in);
      busy_seq_rst <= 1;
    end
    else if (seq_reset_flag == 1 && seq1_0 != 4'b0000 && curr_clkdiv_sel <= 8'h03) begin
      repeat (7) @(posedge dclk_in);
      busy_seq_rst <= 1;
    end
    else
      busy_seq_rst <= 0;
  end


  always @(posedge busy_out or posedge rst_in_out or negedge rst_lock_early)  begin
    if (rst_in_out)
      muxaddr_out <= 5'b0;
    else if (rst_lock_early == 0 && rst_lock_late == 1 )
      muxaddr_out <= muxaddr_o;
    else begin
      repeat (8) @(posedge adcclk);
      muxaddr_out <= muxaddr_o;
    end 
  end
   
  always @(negedge busy_out or posedge busy_out or posedge rst_in_out or posedge cal_chan_update ) begin
    if (rst_in_out || rst_lock_late)
      channel_out <= 5'd0;
    else if (curr_seq1_0_lat[3:2] == 2'b10 && xadc2_en == 0)
      channel_out <= 5'd9; //To match HW, 9=Invalid channel is shown.
    else if (busy_out ==1 && (cal_chan_update == 1) )
      channel_out <= 5'b01000;
    else if (busy_out == 0) begin
      if (curr_seq1_0_lat[3:2] != 2'b10 && xadc2_en == 0 || xadc2_en == 1) //NOT independent adc mode
        channel_out <= curr_chan;  
      //else //independent mode and ADC B powered down
      //  channel_out <= 5'd9; //To match HW, 9=Invalid channel is shown.
      //  //channel_out <= 5'b0;
      curr_chan_lat <= curr_chan;
      curr_chan_lat2 <= curr_chan2;
      curr_pj_set_lat <= curr_pj_set;
      curr_pj_set_lat2 <= curr_pj_set2;
    end
  end


  // START double latch rst_in
    
  reg rst_in1_tmp5;
  reg rst_in2_tmp5;
  reg rst_in1_tmp6;
  reg rst_in2_tmp6;
  wire rst_input_t;
  wire rst_in2;

  initial begin
    int_rst = 1;
    repeat (2) @(posedge dclk_in);
    int_rst <= 0;
  end

  initial begin
    rst_in1_tmp5 = 0;
    rst_in2_tmp5 = 0;
    rst_in1_tmp6 = 0;
    rst_in2_tmp6 = 0;
  end

  assign #1 rst_input_t = int_rst_halt_adc | rst_input | int_rst | soft_reset;
    
  always@(posedge adcclk or posedge rst_input_t) begin
    if (rst_input_t) begin
      rst_in2_tmp6 <= 1;
      rst_in1_tmp6 <= 1;
    end
    else begin
      rst_in2_tmp6 <= rst_in1_tmp6;
      rst_in1_tmp6 <= rst_input_t;
    end
  end

  assign rst_in2 =  rst_in2_tmp6;
  assign #10 rst_in_not_seq = rst_in2;
  assign  rst_in = rst_in_not_seq | seq_reset_dly;
  assign rst_in_out = rst_in_not_seq | seq_reset_busy_out;

  always @(posedge seq_reset) begin
    repeat (2) @(posedge dclk_in);
    seq_reset_dly <= 1;
    repeat (2) @(posedge dclk_in);
    seq_reset_busy_out <= 1;
    repeat (3) @(posedge dclk_in) begin
      seq_reset_dly <= 0;
      seq_reset_busy_out <= 0;
    end
  end
      
  always @(posedge seq_reset_dly or posedge busy_r) begin
    if (seq_reset_dly)
      seq_reset_flag <= 1;
    else
      seq_reset_flag <= 0; 
  end

  always @(posedge seq_reset_flag or posedge busy_out) begin
    if (seq_reset_flag)
      seq_reset_flag_dly <= 1;
    else
      seq_reset_flag_dly <= 0;
  end

  always @(posedge busy_out ) begin
    if (seq_reset_flag_dly == 1 && acq_chan == 5'b01000 && seq1_0 == 4'b0000)
      first_cal_chan <= 1;
    else
      first_cal_chan <= 0;
  end
         

  initial begin
    conv_time = 18;   //minus 3
    // conv_time_cal_1 = 70;
    // conv_time_cal = 70;
    conv_time_cal_1 = 96;
    conv_time_cal = 96;
    sysclk = 0;
    adcclk_tmp = 0;
    seq_count = 1;
    seq_count_a = 1;
    seq_count2 = 1;
    eos_en = 0;
    eos_tmp_en = 0;
    clk_count = -1;
    acq_acqsel = 0;
    acq_e_c_tmp6 = 0;
    acq_e_c_tmp5 = 0;
    eoc_en2 = 0;
    eoc_en = 0;
    eoc_en_delay = 0;
    eoc_en_delay2 = 0;
    rst_lock = 0;
    rst_lock_early = 0;
    alarm_update = 0;
    drp_update = 0;
    cal_chan_update = 0;
    adc_state = S3_ST;
    scon_tmp = 5'b0;
    busy_r = 0;
    busy_r_rst = 0;
    busy_sync1 = 0;
    busy_sync2 = 0;
    conv_count = 0;
    conv_end = 0;
    seq_status_avg = 0;
    seq_status_avg2 = 0;
    for (i = 0; i <=20; i = i +1) begin
      conv_pj_count2[i] = 0;   
      conv_acc2[j] = 0;
      conv_pj_count[i] = 0;   
      conv_acc[j] = 0;
    end
    adc_s1_flag = 0;
    for (k = 0; k <= 31; k = k + 1) begin
       data_reg[k] = 16'b0;
    end 
    seq_count_en = 0;
    eos_out_tmp = 0;
    eoc_out_tmp = 0;
    eoc_out_tmp2 = 0;
    eos_out_tmp1 = 0;
    eoc_out_tmp1 = 0;
    eoc_out_tmp21 = 0;
    eos_out = 0;
    eoc_out = 0;
    eoc_out_t = 0;
    eoc_out2 = 0;
    curr_pj_set = 2'b0;
    curr_pj_set2 = 2'b0;
    curr_pj_set_lat = 2'b0;
    curr_pj_set_lat2 = 2'b0;
    curr_e_c = 0;
    curr_b_u = 0;
    curr_acq = 0;
    curr_e_c2 = 0;
    curr_b_u2 = 0;
    curr_acq2 = 0;
    curr_seq1_0 = 4'b0;
    curr_seq1_0_lat = 4'b0;
    seq1_0 = 4'b0;
    ext_mux = 0;
    ext_mux_chan = 5'b0;
    ext_mux_chan2 = 5'b0;
    daddr_in_lat = 7'b0;
    data_reg[8] = 16'b0;
    data_reg[9] = 16'b0;
    data_reg[10] = 16'b0;
    data_reg[32] = 16'b0;
    data_reg[33] = 16'b0;
    data_reg[34] = 16'b0;
    data_reg[35] = 16'b0;
    data_reg[36] = 16'b1111111111111111;
    data_reg[37] = 16'b1111111111111111;
    data_reg[38] = 16'b1111111111111111;
    data_reg[39] = 16'b1111111111111111;
    data_reg[40] = 16'b0;
    data_reg[41] = 16'b0;
    data_reg[42] = 16'b0;
    data_reg[43] = 16'b0;
    data_reg[44] = 16'b1111111111111111;
    data_reg[45] = 16'b1111111111111111;
    data_reg[46] = 16'b1111111111111111;
    data_reg[47] = 16'b1111111111111111;
    // data_reg[48] = 16'b0;
    // data_reg[49] = 16'b0;
    // data_reg[50] = 16'b0;
    ot_out_reg = 0;
    ot_out = 0;
    alarm_out_reg = 7'b0;
    alarm_out = 7'b0;
    curr_chan =  5'b0;
    curr_chan_lat =  5'b0;
    curr_chan2 =  5'b0;
    curr_chan_lat2 =  5'b0;
    busy_out = 0;
    busy_out_tmp = 0;
    curr_seq = 16'b0;
    curr_seq_m = 16'b0;
    curr_seq2_tmp = 16'b0;
    curr_seq2_tmps = 16'b0;
    seq_num = 0;
    seq_num2 = 0;
    seq_reset_flag_dly = 0;
    seq_reset_flag = 0;
    seq_reset_dly = 0;
    ot_en = 1;
    alarm_en = 7'b1111111;
    do_out_rdtmp = 16'b0;
    acq_chan = 5'b0;
    acq_chan_m = 5'b0;
    acq_chan2 = 5'b0;
    acq_b_u = 0;
    acq_b_u2 = 0;
    conv_result_int = 0;
    conv_result = 0;
    conv_result_reg = 0;
    conv_result_int2 = 0;
    conv_result2 = 0;
    conv_result_reg2 = 0;
  end


  // state machine
  always @(posedge adcclk or posedge rst_in or sim_file_flag) begin
    //CR 675227
    if (!(halt_adc == 2 && seq1_0 == 4'b0011)) begin
      if (sim_file_flag == 1'b1)
        adc_state <= S1_ST; 
      else if (rst_in == 1'b1 || rst_lock_early == 1)
        adc_state <= S1_ST;
      else if (rst_in == 1'b0)
        adc_state <= next_state;
    end
  end
    
  always @(adc_state or eos_en or conv_start or conv_end or curr_seq1_0_lat) begin
    case (adc_state)
      S1_ST   : next_state = S2_ST;
      S2_ST   : if (conv_start)
                  next_state = S3_ST;
                else
                  next_state = S2_ST;
      S3_ST   : if (conv_end)
                  next_state = S5_ST;
                else
                  next_state = S3_ST;
      S5_ST   : if (curr_seq1_0_lat == 4'b0001) begin
                   //CR 675227         if (eos_en)
                   if (eos_tmp_en)
                     next_state = S6_ST;
                   else
                     next_state = S2_ST;
                end
                else
                  next_state = S2_ST;
      S6_ST   : next_state = S1_ST;
      default : next_state = S1_ST;
    endcase // case(adc_state)
  end
  // end state machine    
    

  // DRPORT - SRAM

  initial begin
    drdy_out = 0;
    drdy_out_tmp1 = 0;
    drdy_out_tmp2 = 0;
    drdy_out_tmp3 = 0;
    drdy_out_tmp4 = 0;
    en_data_flag = 0;
    do_out = 16'b0;
    seq_reset = 0;
    cfg_reg1_init = INIT_41;
    seq_en = 0; 
    seq_en_dly = 0;
    seq_en <= #20 (cfg_reg1_init[15:12] != 4'b0011 ) ? 1 : 0;
    seq_en <= #150 0;
  end

  always @(posedge drdy_out_tmp3 or posedge gsr_in)  begin
    if (gsr_in == 1) 
      drdy_out <= 0;
    else begin
      @(posedge dclk_in)
      drdy_out  <= 1;
      @(posedge dclk_in)
      drdy_out <= 0;
    end
  end

  always @(posedge dclk_in or posedge gsr_in) begin
    if (gsr_in == 1) begin
      daddr_in_lat  <= 7'b0;
      do_out <= 16'b0;
    end
    else  begin
      if (den_in == 1'b1) begin
        if (drdy_out_tmp1 == 1'b0) begin
          drdy_out_tmp1 <= 1'b1;
          en_data_flag = 1;
          daddr_in_lat  <= daddr_in;
        end
        else begin
          if (daddr_in != daddr_in_lat) 
           $display("Warning : input pin DEN on XADC instance %m at time %.3f ns can not continue set to high. Need wait DRDY high and then set DEN high again.", $time/1000.0);  
        end
      end
      else
        drdy_out_tmp1 <= 1'b0;

      drdy_out_tmp2 <= drdy_out_tmp1;
      drdy_out_tmp3 <= drdy_out_tmp2;

      if (drdy_out_tmp1 == 1)
        en_data_flag = 0;

      if (drdy_out_tmp3 == 1) 
        do_out <= do_out_rdtmp;

      if (den_in == 1 && (daddr_in >7'h5F || (daddr_in >= 7'h33 && daddr_in < 7'h3F)))
        $display("Invalid Input Warning : The DADDR %x to XADC instance %m at time %.3f ns is accessing an undefined location. The data in this location is invalid.", daddr_in, $time/1000.0);
  
       // write  all available daddr addresses
       if (dwe_in == 1'b1 && en_data_flag == 1) begin
         dr_sram[daddr_in] <= di_in;       
         if (daddr_in == 7'h03)
           soft_reset <= 1;

         // if the window is open, allow the sequence to update
         if (daddr_in == 7'h48 || daddr_in == 7'h49) 
           if ((eoc_last == 1'b1) && (seq1_0[3:2] == 2'b01)) 
             seq_en_drp_updt <= 1'b1;

           if ( daddr_in == 7'h53) begin
             if (di_in[3:0] == 4'b0011)
               ot_limit_reg[15:4] <= di_in[15:4];
           end

           if ( daddr_in == 7'h42 && (di_in[2:0] !=3'b000)) 
             $display(" Invalid Input Error : The DI bit[2:0] %x at DADDR %x on XADC instance %m at %.3f ns is invalid. These must be set to 000.", di_in[2:0], daddr_in, $time/1000.0);

           if ( daddr_in >= 7'h43 && daddr_in <= 7'h47 && (di_in[15:0] != 16'h0000)) 
             $display(" Invalid Input Error : The DI value %x at DADDR %x of XADC instance %m at %.3f ns is invalid. These must be set to 0000h.", di_in, daddr_in, $time/1000.0);

           if ((daddr_in == 7'h40) && 
               ( di_in[4:0] == 5'b00111 || (di_in[4:0] >= 5'b01001 && di_in[4:0] <= 5'b01100))) 
             $display("Invalid Input Warning : The DI bit4:0] at address DADDR %x to XADC instance %m at %.3f ns is %h, which is invalid analog channel.", daddr_in, $time/1000.0, di_in[4:0]);

             if (daddr_in == 7'h40) begin
               if ((cfg_reg1[15:12]==4'b0011) && (di_in[8]==1) && (di_in[4:0] != 5'b00011) && (di_in[4:0] < 5'b10000))
                 $display(" Invalid Input warning : The DI value is %x at DADDR %x on XADC instance %m at %.3f ns.  Bit[8] of DI must be set to 0. Long acquistion mode is only allowed for external channels", di_in, daddr_in, $time/1000.0);

               //if ((cfg_reg1[15:12]==4'b0011) && (di_in[9]==1) && (di_in[4:0] != 5'b00011) && (di_in[4:0] < 5'b10000))
               //   $display(" Invalid Input warning : The DI value is %x at DADDR %x on XADC instance %m at %.3f ns.  Bit[9] of DI must be set to 0. Event mode timing can only be used with external channels", di_in, daddr_in, $time/1000.0);

               if ((cfg_reg1[15:12]==4'b0011) && (di_in[13:12]!=2'b00) && (seq_chan_reg1 != 16'h0000) &&  (seq_chan_reg2 != 16'h0000))
                 $display(" Invalid Input warning : The  Control Regiter 48h and 49h are %x and %x on XADC instance %m at %.3f ns. Those registers should be set to 0000h in single channel mode and averaging enabled.", seq_chan_reg1, seq_chan_reg2, $time/1000.0);
             end

             if (daddr_in == 7'h41 && en_data_flag == 1) begin
               if ((di_in[15:12]==4'b0011) && (cfg_reg0[8]==1) && (cfg_reg0[4:0] != 5'b00011) && (cfg_reg0[4:0] < 5'b10000))
                 $display(" Invalid Input warning : The  Control Regiter 40h value is %x on XADC instance %m at %.3f ns.  Bit[8] of Control Regiter 40h  must be set to 0. Long acquistion mode is only allowed for external channels", cfg_reg0, $time/1000.0);

               //if ((di_in[15:12]==4'b0011) && (cfg_reg0[9]==1) && (cfg_reg0[4:0] != 5'b00011) && (cfg_reg0[4:0] < 5'b10000))
               //  $display(" Invalid Input warning : The  Control Regiter 40h value is %x on XADC instance %m at %.3f ns.  Bit[9] of Control Regiter 40h  must be set to 0. Event mode timing can only be used with external channels", cfg_reg0, $time/1000.0);

               if ((di_in[15:12]!=4'b0011) && (seq_acq_reg1[10:0]!=11'b0) && (seq_acq_reg1[15:12]!=4'b0))
                 $display(" Invalid Input warning : The Control Regiter 4Eh value is %x on XADC instance %m at %.3f ns.  Bit[15:12] and bit[10:0] of this register must be set to 0. Long acquistion mode is only allowed for external channels", seq_acq_reg1, $time/1000.0);

               if ((di_in[15:12]==4'b0011) && (cfg_reg0[13:12]!=2'b00) && (seq_chan_reg1 != 16'h0000) &&  (seq_chan_reg2 != 16'h0000))
                 $display(" Invalid Input warning : The  Control Regiter 48h and 49h are %x and %x on XADC instance %m at %.3f ns. Those registers should be set to 0000h in single channel mode and averaging enabled.", seq_chan_reg1, seq_chan_reg2, $time/1000.0);

             end
         
             if (daddr_in == 7'h41  && en_data_flag == 1) begin
               if (den_in == 1'b1 && dwe_in == 1'b1) begin
                 if (di_in[15:12] != cfg_reg1[15:12]) begin  // writing with the same seq[3:0] will not restart the sequence, matching with hw
                   seq_reset <= 1'b1; 
                   seq_en_drp_updt <= 1'b0;
                 end
                 else
                   seq_reset <= 1'b0;
                 if (di_in[15:12] != 4'b0011 ) begin
                   seq_en <= 1'b1;
                   seq_en_drp_updt <= 1'b0;
                 end
                 else
                   seq_en <= 1'b0;
               end
               else  begin
                 seq_reset <= 1'b0;
                 seq_en <= 1'b0;
               end
             end
             //else  begin
             //  seq_reset <= 0;
             //  seq_en <= 0;
             //end // if (daddr_in == 7'h41)
      end // dwe ==1         

      if (seq_en == 1) 
        seq_en <= 1'b0;
      if (seq_reset == 1)
        seq_reset <= 1'b0;
      if (soft_reset == 1)
        soft_reset <= 0;
      if (seq_en_drp_updt == 1'b1 && eos_out == 1'b1)
        seq_en <= 1'b1;
      if (seq_en_drp_updt == 1'b1 && seq_en == 1'b1)
        seq_en_drp_updt <= 1'b0;

    end // if (gsr == 1)
  end//always
         

  //  DO bus data out
  assign  tmp_dr_sram_out = ( daddr_in_lat >= 7'h40 && daddr_in_lat <= 7'h5F) ? 
                            dr_sram[daddr_in_lat] : 16'b0;

  assign status_reg = {8'b0,  alarm_out[6:3], ot_out, alarm_out[2:0]};

  assign tmp_data_reg_out = (daddr_in_lat >= 7'h00 && daddr_in_lat <= 7'h2E) ?
                            data_reg[daddr_in_lat] : 16'b0;

  always @( daddr_in_lat or  tmp_data_reg_out or tmp_dr_sram_out or status_reg ) begin
    if ((daddr_in_lat >7'h5F || (daddr_in_lat >= 7'h2F && daddr_in_lat < 7'h3F))) begin
      do_out_rdtmp = 16'bx;
    end
  
    if (daddr_in_lat == 7'h3F) begin
      do_out_rdtmp = status_reg;
    end

    if ((daddr_in_lat >= 7'h00  && daddr_in_lat <= 7'h2E)) 
      do_out_rdtmp = tmp_data_reg_out;
    else if (daddr_in_lat >= 7'h40 && daddr_in_lat <= 7'h5F)
      do_out_rdtmp = tmp_dr_sram_out;
  end
  // end DRP RAM

    
  assign cfg_reg0 = dr_sram[7'h40];
  assign cfg_reg1 = dr_sram[7'h41];
  assign cfg_reg2 = dr_sram[7'h42];
  assign seq_chan_reg1 = dr_sram[7'h48];
  assign seq_chan_reg2 = dr_sram[7'h49];
  assign seq_pj_reg1 = dr_sram[7'h4A];
  assign seq_pj_reg2 = dr_sram[7'h4B];
  assign seq_du_reg1 = dr_sram[7'h4C];
  assign seq_du_reg2 = dr_sram[7'h4D];
  assign seq_acq_reg1 = dr_sram[7'h4E];
  assign seq_acq_reg2 = dr_sram[7'h4F];
 
  always @(cfg_reg1)
    seq1_0 = cfg_reg1[15:12];
  always @(cfg_reg0) begin
    ext_mux = cfg_reg0[11];
    ext_mux_chan = cfg_reg0[4:0];
    ext_mux_chan2 = {2'b11, cfg_reg0[2:0]};
  end

  always @(posedge drp_update or posedge rst_in) begin
    if (rst_in) begin
      repeat (2) @(posedge dclk_in);
      seq_bits = seq1_0;
    end
    else
      seq_bits = curr_seq1_0;
  
    if (seq_bits == 4'b0000) begin
      alarm_en <= 8'b0;
      ot_en <= 1;
    end
    else begin
      ot_en  <= ~cfg_reg1[0];
      alarm_en[2:0] <= ~cfg_reg1[3:1];
      alarm_en[6:3] <= ~cfg_reg1[11:8];
    end
  end
  // end DRPORT - sram    
    
  // Clock divider, generate  and adcclk
  always @(posedge dclk_in)
    sysclk <= ~sysclk;

  always @(posedge dclk_in) begin
    if (curr_clkdiv_sel > 8'b00000010 ) begin
      if (clk_count >= curr_clkdiv_sel - 1) 
        clk_count = 0;
      else 
        clk_count = clk_count + 1;
    
       if (clk_count > (curr_clkdiv_sel/2) - 1)
         adcclk_tmp <= 1;
       else
         adcclk_tmp <= 0;
    end 
    else 
      adcclk_tmp <= ~adcclk_tmp;
  end 

  assign curr_clkdiv_sel = cfg_reg2[15:8];
  //power down condition cfg_reg2[5:4]
  // 00: all up
  // 01: invalid
  // 10: ADC B down
  // 11: XADC down
  assign xadc_en = (cfg_reg2[5]===1 && cfg_reg2[4]===1) ? 0 : 1;
  assign xadc2_en = (cfg_reg2[5]===1 ) ? 0 : 1;
  assign adcclk_div1 = (curr_clkdiv_sel > 8'b00000010) ? 0 : 1;
  assign adcclk_r = (adcclk_div1) ? ~sysclk : adcclk_tmp;
  assign adcclk = (xadc_en) ? adcclk_r : 0;
  // end clock divider    
      
  // latch configuration registers
  wire [15:0] cfg_reg0_seq, cfg_reg0_adc;
  reg [15:0] cfg_reg0_seq_tmp5, cfg_reg0_adc_tmp5;
  reg [15:0] cfg_reg0_seq_tmp6, cfg_reg0_adc_tmp6;
  reg [1:0]  acq_avg, acq_avg2;
                                                
  //                                            not independent adc mode
  assign muxaddr_o = (rst_lock_early) ? 5'b0 : (curr_seq1_0_lat[3:2] != 2'b10 && xadc2_en == 0 || xadc2_en == 1) ?  acq_chan_m : 5'b0;

  always @( seq1_0 or adc_s1_flag or curr_seq_m  or cfg_reg0_adc or rst_in) begin
    if (rst_in == 0) begin
      if (seq1_0[3:2] == 2'b01) begin
        acq_chan_m = curr_seq_m[4:0];
      end
      else if (seq1_0[3:2] == 2'b10) begin  //independent ADC mode.
        acq_chan_m = curr_seq_m[4:0];
      end
      else if (seq1_0[3:2] == 2'b11) begin
        acq_chan_m = curr_seq_m[4:0];
      end
      else if (seq1_0 != 4'b0011 && adc_s1_flag == 0) begin
        acq_chan_m = curr_seq_m[4:0];
      end
      else begin
        acq_chan_m = cfg_reg0_adc[4:0];
      end
    end
  end

  //CR 675227  always @( seq1_0 or adc_s1_flag or curr_seq or curr_seq2 or cfg_reg0_adc or rst_in) begin
  always @(adc_s1_flag or curr_seq or curr_seq2 or cfg_reg0_adc or rst_in) begin      
    if ((seq1_0 == 4'b0001 && adc_s1_flag == 0) || seq1_0 == 4'b0010
       || seq1_0[3:2] == 2'b10 || seq1_0[3:2] == 2'b01 || seq1_0[3:2] == 2'b11) begin 
      acq_acqsel = curr_seq[8];   
    end
    else if (seq1_0 == 4'b0011) begin // Single channel mode, sequencer off
      acq_acqsel = cfg_reg0_adc[8];  // The acquisition time  on ext analog
                                     // inputs, extending by 6 cycles. Reset
                                     // value
    end
    else begin
      acq_acqsel = 0;
    end

    if (rst_in == 0) begin
      if (seq1_0[3:2] == 2'b01) begin //simultaneous sampling mode
        acq_avg   = curr_seq[13:12];
        acq_chan  = curr_seq[4:0];
        acq_b_u   = curr_seq[10];
        acq_avg2  = curr_seq2[13:12];
        acq_chan2 = curr_seq[4:0] + 8;
        acq_b_u2  = curr_seq2[10];
      end
      else if (seq1_0[3:2] == 2'b10) begin //In independent adc mode , 
        //ADC A is doing default mode sequence and 
        //ADC B is doing the user selected sequence.
        //ADC B is shown at CHANNEL output, therefore 
        //ADC 2 is assigned with ADC A settings and 
        //ADC 1 with ADC B.
        acq_avg  = curr_seq[13:12]; //ADC B
        acq_chan = curr_seq[4:0]; 
        acq_b_u  = curr_seq[10];
        acq_avg2   = 2'b01;         // Average 16 samples for default sequence
        acq_chan2  = curr_seq2[4:0];
        acq_b_u2   = 0;             // ADCA is unipolar.
      end
      else if (seq1_0[3:2] == 2'b11) begin
        acq_avg   = 2'b01;
        acq_chan  = curr_seq[4:0];
        acq_b_u   = 0;
      end
      else if (seq1_0 != 4'b0011 && adc_s1_flag == 0) begin
        acq_avg   = curr_seq[13:12];
        acq_chan  = curr_seq[4:0];
        acq_b_u   = curr_seq[10];
      end
      else begin
        acq_avg   = cfg_reg0_adc[13:12];
        acq_chan  = cfg_reg0_adc[4:0];
        acq_b_u   = cfg_reg0_adc[10];

        //CR 675227 
        if (seq1_0 == 4'b0001 && acq_e_c == 1'b0) begin
          //CR 764936   if (seq1_0 == 4'b0001) begin
          halt_adc = halt_adc + 1;
          if (halt_adc == 2)
            dr_sram[7'h41][15:12] = 4'b0011;
        end
      end //else
    end//rst_in
  end//always

  reg single_chan_conv_end;
  reg [3:0] conv_end_reg_read;
  reg busy_reg_read;
  reg first_after_reset_tmp5;
  reg first_after_reset_tmp6;

  always@(posedge adcclk or posedge rst_in)   begin
    if(rst_in) conv_end_reg_read <= 4'b0;
      else       conv_end_reg_read <= {conv_end_reg_read[2:0], single_chan_conv_end | conv_end};
  end
        
  always@(posedge DCLK or posedge rst_in) begin
    if(rst_in) 
      busy_reg_read <= 1;
    else       
      busy_reg_read <= ~conv_end_reg_read[2];
  end

  assign cfg_reg0_adc =  cfg_reg0_adc_tmp6;
  assign cfg_reg0_seq =  cfg_reg0_seq_tmp6;
  assign acq_e_c =  acq_e_c_tmp6;

  always @(negedge busy_out or rst_in) begin
    if(rst_in) begin
      cfg_reg0_seq_tmp6 <= 16'b0;
      cfg_reg0_adc_tmp6 <= 16'b0;
      acq_e_c_tmp6 <= 0;
      first_after_reset_tmp6 <= 1;
    end
    else begin
      repeat(3) @(posedge DCLK);
      if(first_after_reset_tmp6) begin
         first_after_reset_tmp6<=0;
         cfg_reg0_adc_tmp6 <= cfg_reg0;
         cfg_reg0_seq_tmp6 <= cfg_reg0;
      end
      else begin
         cfg_reg0_adc_tmp6 <= cfg_reg0_seq;
         cfg_reg0_seq_tmp6 <= cfg_reg0;
      end
      acq_e_c_tmp6      <= cfg_reg0[9];
    end
  end

  always @(posedge conv_start or  posedge busy_r_rst or posedge rst_in) begin
    if (rst_in ==1)
      busy_r <= 0;
    else if (conv_start && rst_lock == 0)
      busy_r <= 1;
    else if (busy_r_rst)
      busy_r <= 0;
  end

  always @(negedge busy_out ) begin
    if (adc_s1_flag == 1)
      //single pass sequence or single channel mode with sequencer of. 
      //go back to default mode after one sequence is completed
      if (curr_seq1_0 == 4'b0001 || curr_seq1_0 == 4'b0011)  // CR 764936
        curr_seq1_0 <= 4'b0011;
      else
        curr_seq1_0 <= 4'b0000;
    else
      curr_seq1_0 <= seq1_0;
  end

  //   always @(posedge conv_start or posedge rst_in ) 
  always @(posedge conv_start or  rst_in ) begin
    if (rst_in == 1) begin
      mn_mux_in <= 0.0;
      mn_mux_in2 <= 0.0;
      curr_chan <= 5'b0;
      curr_chan2 <= 5'b0;
    end
    else  begin
      if ((acq_chan == 5'b00011) || (acq_chan >= 5'b10000 && acq_chan <= 5'b11111)) begin
        if (ext_mux == 1) begin
          tmp_v = $bitstoreal(mn_in_diff[ext_mux_chan]);
          mn_mux_in <= tmp_v; 
        end
        else begin
          tmp_v = $bitstoreal(mn_in_diff[acq_chan]);
          mn_mux_in <= tmp_v; 
        end
      end
      else
        mn_mux_in <= $bitstoreal(mn_in_uni[acq_chan]);
  
      tmp_seq1_0 = curr_seq1_0[3:2];

      if (tmp_seq1_0 == 2'b01 || tmp_seq1_0 == 2'b10) begin
        if ((acq_chan2 == 5'b00011) || (acq_chan2 >= 5'b10000 && acq_chan2 <= 5'b11111)) begin
          if (ext_mux == 1) begin
            tmp_v1 = $bitstoreal(mn_in_diff[ext_mux_chan2]);
            mn_mux_in2 <= tmp_v1;
          end
          else begin
            tmp_v1 = $bitstoreal(mn_in_diff[acq_chan2]);
            mn_mux_in2 <= tmp_v1;
          end
        end
        else
          mn_mux_in2 <= $bitstoreal(mn_in_uni[acq_chan2]);
      end
      curr_chan <= acq_chan;
      curr_chan2 <= acq_chan2;
      curr_seq1_0_lat <= curr_seq1_0;
      
      if ( acq_chan == 5'b00111  || (acq_chan >= 5'b01010 && acq_chan <= 5'b01100))
        $display("Invalid Input Warning : The analog channel %x to XADC instance %m at %.3f ns is invalid.", acq_chan, $time/1000.0);
    
      if ((seq1_0 == 4'b0001 && adc_s1_flag == 0) || seq1_0 == 4'b0010 || seq1_0 == 4'b0000 || seq1_0[3:2] == 2'b01 || seq1_0[3:2] == 2'b10 || seq1_0[3:2] == 2'b11) begin
        curr_pj_set <= curr_seq[13:12];
        curr_b_u <=  curr_seq[10];
        curr_e_c <= curr_seq[9];
        curr_acq <= curr_seq[8];
        curr_pj_set2 <= curr_seq2[13:12];
        curr_b_u2<= curr_seq2[10];
        curr_e_c2 <= curr_seq2[9];
        curr_acq2 <= curr_seq2[8];
      end
      else  begin
        curr_pj_set <= acq_avg;
        curr_b_u <= acq_b_u;
        curr_e_c <= cfg_reg0[9];
        curr_acq <= cfg_reg0[8];
      end
    end // if (rst_in == 0)
  end //always
  // end latch configuration registers
    

  // sequence control
  always @(seq_en )
    seq_en_dly <= #1 seq_en;

  always @(posedge  seq_en_dly) begin
    if (seq1_0  == 4'b0001 || seq1_0 == 4'b0010) begin //single pass sequence or cont sequence mode
      seq_num = 0;
      for (si=0; si<= 15; si=si+1) begin
        if (seq_chan_reg1[si] ==1)  begin
          seq_num = seq_num + 1;
          seq_mem[seq_num] = si;
        end
      end
      for (si=16; si<= 31; si=si+1) begin
        if (seq_chan_reg2[si-16] ==1) begin
          seq_num = seq_num + 1;
          seq_mem[seq_num] = si;
        end
      end
      if (seq_num < 32) begin
        for (si=seq_num+1; si<=32; si=si+1) begin
          seq_mem[si] = 0;
        end
      end
    end
    // if sequence register is updated at the appropriate time, 
    // all but the first channel in the simultaneous sampling mode sequence 
    // can be changed without a reset, trying to handel some error cases too.
    else if (seq1_0[3:2] == 2'b01) begin //simultaneous sampling mode
      seq_num = 0;
      skip_updt = 1'b0;
      if (seq_en_drp_updt == 1'b1) begin
        first_ch = seq_mem[1];
        if (first_ch <= 16) begin
          if (first_ch == 16 && seq_chan_reg2[0] == 1'b0) begin
            seq_num = 1; // first channel turned off, leave on
          end 
          else if (first_ch < 16 && seq_chan_reg1[first_ch] == 1'b0) begin
            seq_num = 1; // first channel turned off, leave on
          end
          for (si=0; si<= first_ch-1; si=si+1) begin
            if (seq_chan_reg1[si] == 1)  begin // look for a ch enabled before first_ch
              skip_updt = 1'b1; // new first channel before old one, ignore
            end
          end
        end 
        else begin //first_ch >16
          if (seq_chan_reg2[first_ch-16] == 1'b0) begin
            seq_num = 1; // first channel turned off, leave on
          end
          for (si=0; si<= 15; si=si+1) begin
            if (seq_chan_reg1[si] == 1)  begin // look for a ch enabled before first_ch
              skip_updt = 1'b1; // new first channel before old one, ignore
            end
          end
          for (si=16; si<= first_ch-1; si=si+1) begin
            if (seq_chan_reg2[si-16] == 1)  begin // look for a ch enabled before first_ch
              skip_updt = 1'b1; // new first channel before old one, ignore
            end
          end
        end
      end //seq_en_drp_updt == 1'b1
      if (skip_updt == 1'b0) begin
        for (si=0; si<= 15; si=si+1) begin
          if (seq_chan_reg1[si] ==1)  begin
            seq_num = seq_num + 1;
            seq_mem[seq_num] = si;
          end
        end
        for (si=16; si<= 23; si=si+1) begin
          if (seq_chan_reg2[si-16] ==1) begin
            seq_num = seq_num + 1;
            seq_mem[seq_num] = si;
          end
        end
        if (seq_num < 32) begin
          for (si=seq_num+1; si<=32; si=si+1) begin
            seq_mem[si] = 0;
          end
        end
      end //skip_updt==0
    end //simultaneous sampling mode
    else if (seq1_0[3:2] == 2'b10) begin //independent ADC mode
      //reset sequences
      for (si=0; si<= 31; si=si+1) begin 
        seq_mem [si] = 0;
        seq_mem2[si] = 0;
      end
      //ADCB assignment, which goes to ADC 1 in the model.
      seq_num = 0;
      //Check register x48.
      if (seq_chan_reg1[11] == 1) begin //if VP-VN dedicated analog inputs are selected
        seq_num = 1;
        seq_mem[1] = 11; 
      end
      //Check register x49
      for (si=16; si<= 32; si=si+1) begin
        if (seq_chan_reg2[si-16] ==1) begin
          seq_num = seq_num + 1;
          seq_mem[seq_num] = si;
        end
      end
      //set the rest to 0
      if (seq_num < 32) begin
        for (si=seq_num+1; si<=32; si=si+1) begin
          seq_mem[si] = 0;
        end
      end
      if(seq_num==0) 
        $display("XADC ERROR: This is not a valid selection. In independent ADC mode, ADC B has to have at least one channel in the sequence. Check registers 48h and 49h.");
      //default sequence is followed in ADC A, which goes to ADC 2 in the model.
      if (simd_f == 0) begin // Not a ZYNQ device
        seq_num2 = 5;
        seq_mem2[1] = 0;
        seq_mem2[2] = 8;
        seq_mem2[3] = 9;
        seq_mem2[4] = 10;
        seq_mem2[5] = 14;
      end
      else if (simd_f == 1) begin // ZYNQ device
        seq_num2 = 8;
        seq_mem2[1] = 0;
        seq_mem2[2] = 5;
        seq_mem2[3] = 6;
        seq_mem2[4] = 7;
        seq_mem2[5] = 8;
        seq_mem2[6] = 9;
        seq_mem2[7] = 10;
        seq_mem2[8] = 14;
      end
    end
    else if (seq1_0  == 4'b0000 || seq1_0[3:2] == 2'b11) begin //default mode
      if (simd_f == 0) begin // Not a ZYNQ device
        seq_num = 5;
        seq_mem[1] = 0;
        seq_mem[2] = 8;
        seq_mem[3] = 9;
        seq_mem[4] = 10;
        seq_mem[5] = 14;
      end
      else if (simd_f == 1) begin //ZYNQ device
        seq_num = 8;
        seq_mem[1] = 0;
        seq_mem[2] = 5;
        seq_mem[3] = 6;
        seq_mem[4] = 7;
        seq_mem[5] = 8;
        seq_mem[6] = 9;
        seq_mem[7] = 10;
        seq_mem[8] = 14;
      end
    end
  end //always
              
  always @( seq_count  or negedge seq_en_dly) begin
    seq_curr_i = seq_mem[seq_count];
    curr_seq = 16'b0;
    if (seq_curr_i >= 0 && seq_curr_i <= 15) begin
       curr_seq [2:0] =  seq_curr_i[2:0];
       curr_seq [4:3] = 2'b01; //Channel select 8-15
       curr_seq [8] = seq_acq_reg1[seq_curr_i];
       curr_seq [10] = seq_du_reg1[seq_curr_i];

       if (seq1_0 == 4'b0000 ||  seq1_0[3:2] == 2'b11) //default mode
          curr_seq [13:12] = 2'b01;
       else if (seq_pj_reg1[seq_curr_i] == 1) // check if averaging is enabled for h4A
          curr_seq [13:12] = cfg_reg0[13:12]; // set averaging count selection (16, 64, or 256)
       else
          curr_seq [13:12] = 2'b00; //No averaging

       if (seq_curr_i >= 0 && seq_curr_i <=7) 
          curr_seq [4:3] = 2'b01; //Channel select 8-15
       else
          curr_seq [4:3] = 2'b00; ///Channel select 0-7
    end
    else if (seq_curr_i >= 16 && seq_curr_i <= 31) begin
       curr_seq [4:0] = seq_curr_i;
       curr_seq [8] = seq_acq_reg2[seq_curr_i - 16];
       curr_seq [10] = seq_du_reg2[seq_curr_i - 16];
       if (seq_pj_reg2[seq_curr_i - 16] == 1)
          curr_seq [13:12] = cfg_reg0[13:12];
       else
          curr_seq [13:12] = 2'b00;
       if (seq_curr_i < 24) begin // simultaneous sampling mode, 1st set of aux channels
         curr_seq2_tmps[4:0] = seq_curr_i + 8;
         curr_seq2_tmps[8] = seq_acq_reg2[seq_curr_i - 8];
         curr_seq2_tmps[10] = seq_du_reg2[seq_curr_i - 8]; 
         if (seq_pj_reg2[seq_curr_i - 8] == 1)
           curr_seq2_tmps[13:12] = cfg_reg0[13:12];
         else
           curr_seq2_tmps [13:12] = 2'b00;
       end
    end
  end

  // choose curr_seq2_tmps for simultaneous sampling mode.
  assign curr_seq2 = (tmp_seq1_0 == 2'b01) ? curr_seq2_tmps : curr_seq2_tmp;

  always @( seq_count_a  or negedge seq_en_dly) begin
    seq_curr_ia = seq_mem[seq_count_a];
    curr_seq_m = 16'b0;
    if (seq_curr_ia >= 0 && seq_curr_ia <= 15) begin
       curr_seq_m [2:0] =  seq_curr_ia[2:0];
       curr_seq_m [4:3] = 2'b01;
       curr_seq_m [8] = seq_acq_reg1[seq_curr_ia];
       curr_seq_m [10] = seq_du_reg1[seq_curr_ia];

       if (seq1_0 == 4'b0000 ||  seq1_0[3:2] == 2'b11) //default mode
         curr_seq_m [13:12] = 2'b01;
       else if (seq_pj_reg1[seq_curr_ia] == 1)
         //if(seq1_0[3:2] == 2'b10) //independent ADC.
         //  curr_seq_m [13:12] = 2'b01; //16 samples
         //else
         curr_seq_m [13:12] = cfg_reg0[13:12];  //averaging count as set
       else //no averaging
         curr_seq_m [13:12] = 2'b00;

       if (seq_curr_ia >= 0 && seq_curr_ia <=7) 
         curr_seq_m [4:3] = 2'b01;
       else
         curr_seq_m [4:3] = 2'b00;
    end
    else if (seq_curr_ia >= 16 && seq_curr_ia <= 31) begin
      curr_seq_m [4:0] = seq_curr_ia;
      curr_seq_m [8] = seq_acq_reg2[seq_curr_ia - 16];
      curr_seq_m [10] = seq_du_reg2[seq_curr_ia - 16];
      if (seq_pj_reg2[seq_curr_ia - 16] == 1)
         curr_seq_m [13:12] = cfg_reg0[13:12];
      else
         curr_seq_m [13:12] = 2'b00;
    end
  end


  always @( seq_count2  or negedge seq_en_dly) begin
    seq_curr_i2 = seq_mem2[seq_count2];
     curr_seq2_tmp = 16'b0;
    if (seq_curr_i2 >= 0 && seq_curr_i2 <= 15) begin
       curr_seq2_tmp [2:0] =  seq_curr_i2[2:0];
       curr_seq2_tmp [4:3] = 2'b01;
       curr_seq2_tmp [8] = seq_acq_reg1[seq_curr_i2];
       curr_seq2_tmp [9] = 0;
       curr_seq2_tmp [10] = seq_du_reg1[seq_curr_i2];

       if ( seq1_0[3:2] == 2'b10) //independent ADC.
          curr_seq2_tmp [13:12] = 2'b01; // Default sequence: always average 16 samples. 
       else
          curr_seq2_tmp [13:12] = 2'b00; // No averaging for simultaneous sampling mode

       if (seq_curr_i2 >= 0 && seq_curr_i2 <=7)
         curr_seq2_tmp [4:3] = 2'b01;  //select (channel +8)
       else
         curr_seq2_tmp [4:3] = 2'b00;  //select (channel mod8)
    end
    else if (seq_curr_i2 >= 16 && seq_curr_i2 <= 31) begin 
       curr_seq2_tmp [4:0] = seq_curr_i2;
       curr_seq2_tmp [8] = seq_acq_reg2[seq_curr_i2 - 16];
       curr_seq2_tmp [10] = seq_du_reg2[seq_curr_i2 - 16];
       if (seq_pj_reg2[seq_curr_i2 - 16] == 1)
         curr_seq2_tmp [13:12] = cfg_reg0[13:12];
       else
         curr_seq2_tmp [13:12] = 2'b00;
    end
  end
    
  always @(posedge busy_out or posedge rst_in ) begin
    if (rst_in == 1 || rst_lock == 1 ) begin
      seq_count_a <= 1;
    end
    else  begin
      if ( curr_seq1_0_lat == 4'b0011  )
        seq_count_a <= 1;
      else  begin
        if (seq_count_a >= 32 || seq_count_a >= seq_num)
          seq_count_a <= 1;
        else
          seq_count_a <= seq_count_a +1;
      end
    end
  end
        
  always @(posedge adcclk or posedge rst_in)  begin
    if (rst_in == 1 ) begin
      seq_count <= 1;
      seq_count2 <= 1;
      eos_en <= 0;
    end
    else  begin
      if (curr_seq1_0_lat[3:2] == 2'b10) begin // independent adc mode
        if ((seq_count2 >= seq_num2  ) && (adc_state ==  S5_ST)  )
          seq_count2 <= 1;
        else if (seq_count_en == 1) 
          seq_count2 <= seq_count2 + 1;
      end

      if ((seq_count == seq_num  ) && (adc_state == S3_ST && next_state == S5_ST) && (curr_seq1_0_lat != 4'b0011) && rst_lock == 0)
        eos_tmp_en <= 1;
      else
        eos_tmp_en <= 0;

      if (eos_tmp_en == 1 && seq_status_avg == 0 ) // delay by 1 adcclk
        eos_en <= 1;
      else
        eos_en <= 0;

      if (eos_tmp_en == 1 || curr_seq1_0_lat == 4'b0011  )
        seq_count <= 1;
      else if (seq_count_en == 1) begin
        if (seq_count >= 32)
          seq_count <= 1;
        else
          seq_count <= seq_count +1;
      end
    end // else: !if(rst_in == 1)
  end
  // end sequence control
    
  
  // Acquisition
  reg first_acq;
  reg shorten_acq;
  wire busy_out_dly;

  assign #10 busy_out_dly = busy_out;

  always @(adc_state or posedge rst_in or first_acq)   begin
    if(rst_in) 
      shorten_acq = 0;
    else if(busy_out_dly==0 && adc_state==S2_ST && first_acq==1)
      shorten_acq = 1;
    else
      shorten_acq = 0;
  end

  always @(posedge adcclk or posedge rst_in) begin
    //  if (rst_in == 1) begin
    if (rst_in == 1 || rst_lock == 1) begin
      acq_count <= 1;
      first_acq <=1;
    end
    else  begin 
      if (adc_state == S2_ST && rst_lock == 0 && (acq_e_c==0)) begin
        first_acq <= 0;
        if (acq_acqsel == 1) begin
          if (acq_count <= 11)
            acq_count <= acq_count + 1 + shorten_acq;
        end
        else begin
          if (acq_count <= 4)
            acq_count <= acq_count + 1 + shorten_acq;
        end // else: !if(acq_acqsel == 1)
               
        if (next_state == S3_ST)
          if ((acq_acqsel == 1 && acq_count < 10) || (acq_acqsel == 0 && acq_count < 4))
            $display ("Warning: Acquisition time is not long enough for XADC instance %m at time %t.", $time);
      end // if (adc_state == S2_ST)
      else
        acq_count <=  (first_acq) ? 1 : 0;
    end // if (rst_in == 0 && rdt_lock==0)
  end
  
  // continuous mode
  reg  conv_start_cont;
  wire reset_conv_start;
  wire conv_start_sel;

  always @(adc_state or acq_acqsel or acq_count) begin
    if (adc_state == S2_ST) begin
      if (rst_lock == 0) begin
        // CR 800173           if (    ((seq_reset_flag == 0 || (seq_reset_flag == 1 && curr_clkdiv_sel > 8'h03))
        //          && ( (acq_acqsel == 1 && acq_count > 10) || (acq_acqsel == 0 && acq_count > 4)) ) )
        if( (acq_acqsel == 1 && acq_count > 10) || (acq_acqsel == 0 && acq_count > 4))
          conv_start_cont = 1;
        else
          conv_start_cont = 0;
      end
    end // if (adc_state == S2_ST)
    else
      conv_start_cont = 0;
  end
  
  assign conv_start_sel = (acq_e_c) ? convst_in : conv_start_cont;
  assign reset_conv_start = rst_in | (conv_count==2);
   
  always@(posedge conv_start_sel or posedge reset_conv_start) begin
    if(reset_conv_start) 
      conv_start <= 0;
    else                 
      conv_start <= 1;
  end
  // end acquisition    
    
  // Conversion
  always @(adc_state or next_state or curr_chan  or mn_mux_in or curr_b_u) begin
    if ((adc_state == S3_ST && next_state == S5_ST) ||  adc_state == S5_ST) begin
      if (curr_chan == 0) begin    // temperature conversion
        //adc_temp_result = (mn_mux_in + 273.15) * 0.001984226*65536;
        adc_temp_result = (mn_mux_in + 273.15) * 0.001984225*65536; //CR 861679
        if (adc_temp_result >= 65535.0)
          conv_result_int = 65535;
        else if (adc_temp_result < 0.0)
          conv_result_int = 0;
        else begin
          conv_result_int = $rtoi(adc_temp_result);
          if (adc_temp_result - conv_result_int > 0.9999)
            conv_result_int = conv_result_int + 1;
          end
        end
        else if (curr_chan == 1 || curr_chan == 2 || curr_chan ==6 ||
                curr_chan == 13 || curr_chan == 14 || curr_chan ==  15) begin     // internal power conversion
          adc_intpwr_result = mn_mux_in * 65536.0 / 3.0;
          if (adc_intpwr_result >= 65535.0)
            conv_result_int = 65535;
          else if (adc_intpwr_result < 0.0)
            conv_result_int = 0;
          else begin
            conv_result_int = $rtoi(adc_intpwr_result);
            if (adc_intpwr_result - conv_result_int > 0.9999)
              conv_result_int = conv_result_int + 1;
          end
        end
        else if (curr_chan == 3 || (curr_chan >=16 && curr_chan <= 31)) begin
          adc_ext_result =  (mn_mux_in) * 65536.0;
          if (curr_b_u == 1) begin
            if (adc_ext_result > 32767.0)
              conv_result_int = 32767;
            else if (adc_ext_result < -32768.0)
              conv_result_int = -32768;
            else begin
              conv_result_int = $rtoi(adc_ext_result);
              if (adc_ext_result - conv_result_int > 0.9999)
                conv_result_int = conv_result_int + 1;
            end
          end
          else begin
            if (adc_ext_result > 65535.0)
              conv_result_int = 65535;
            else if (adc_ext_result < 0.0)
              conv_result_int = 0;
            else begin
              conv_result_int = $rtoi(adc_ext_result);
              if (adc_ext_result - conv_result_int > 0.9999)
                 conv_result_int = conv_result_int + 1;
            end
          end
        end
        else begin
          conv_result_int = 0;
        end
    end 
    conv_result = conv_result_int;
  end // always @ ( adc_state or curr_chan or mn_mux_in, curr_b_u)

  always @(adc_state or next_state or curr_chan2  or mn_mux_in2 or curr_b_u2) begin
    if ((adc_state == S3_ST && next_state == S5_ST) ||  adc_state == S5_ST) begin
      if (curr_chan2 == 0) begin    // temperature conversion
        adc_temp_result2 = (mn_mux_in2 + 273.15) * 0.001984225*65536; //CR 861679, CR 9633888 
        if (adc_temp_result2 >= 65535.0)
          conv_result_int2 = 65535;
        else if (adc_temp_result2 < 0.0)
          conv_result_int2 = 0;
        else begin
          conv_result_int2 = $rtoi(adc_temp_result2);
          if (adc_temp_result2 - conv_result_int2 > 0.9999)
            conv_result_int2 = conv_result_int2 + 1;
        end
      end
      else if (curr_chan2 == 1 || curr_chan2 == 2 || curr_chan2 == 6
              || curr_chan2 == 13 || curr_chan2 == 14 || curr_chan2 == 15) begin     // internal power conversion
        adc_intpwr_result2 = mn_mux_in2 * 65536.0 / 3.0;
        if (adc_intpwr_result2 >= 65535.0)
          conv_result_int2 = 65535;
        else if (adc_intpwr_result2 < 0.0)
          conv_result_int2 = 0;
        else begin
          conv_result_int2 = $rtoi(adc_intpwr_result2);
          if (adc_intpwr_result2 - conv_result_int2 > 0.9999)
             conv_result_int2 = conv_result_int2 + 1;
        end
      end
      else if (curr_chan2 == 3 || (curr_chan2 >=16 && curr_chan2 <= 31)) begin
        adc_ext_result2 =  (mn_mux_in2) * 65536.0;
        if (curr_b_u2 == 1) begin
          if (adc_ext_result2 > 32767.0)
            conv_result_int2 = 32767;
          else if (adc_ext_result2 < -32768.0)
            conv_result_int2 = -32768;
          else begin
            conv_result_int2 = $rtoi(adc_ext_result2);
            if (adc_ext_result2 - conv_result_int2 > 0.9999)
               conv_result_int2 = conv_result_int2 + 1;
          end
        end
        else begin
          if (adc_ext_result2 > 65535.0)
            conv_result_int2 = 65535;
          else if (adc_ext_result2 < 0.0)
            conv_result_int2 = 0;
          else begin
            conv_result_int2 = $rtoi(adc_ext_result2);
            if (adc_ext_result2 - conv_result_int2 > 0.9999)
              conv_result_int2 = conv_result_int2 + 1;
           end
        end
      end
      else begin
        conv_result_int2 = 0;
      end
    end 
    conv_result2 = conv_result_int2;
  end // always @ ( adc_state or curr_chan or mn_mux_in, curr_b_u)
    
  reg busy_r_rst_done;

  always @(posedge adcclk or  posedge rst_in) begin
    if (rst_in == 1) begin
      conv_count <= 6;
      conv_end <= 0;
      seq_status_avg <= 0;
      busy_r_rst <= 0;        
      busy_r_rst_done <= 0;
      for (i = 0; i <=31; i = i +1) begin
        conv_pj_count[i] <= 0;     // array of integer
        conv_pj_count2[i] <= 0;     // array of integer
      end
      single_chan_conv_end <= 0;
    end
    else begin
      if(adc_state == S2_ST) begin
        if(busy_r_rst_done == 0) 
          busy_r_rst <= 1;
        else                             
          busy_r_rst <= 0;
        busy_r_rst_done <= 1;
    end

    if (adc_state == S2_ST && conv_start == 1) begin
      conv_count <= 0;
      conv_end <= 0;
    end
    else if (adc_state == S3_ST ) begin
      busy_r_rst_done <= 0;
      conv_count = conv_count + 1;
      if ((curr_chan != 5'b01000 ) && (conv_count == conv_time ) ||
            (curr_chan == 5'b01000 ) && (conv_count == conv_time_cal_1 ) && (first_cal_chan==1)
            || (curr_chan == 5'b01000 ) && (conv_count == conv_time_cal) && (first_cal_chan == 0))
        conv_end <= 1;
      else
        conv_end <= 0;
    end
    else begin
      conv_end <= 0;
      conv_count <= 0;
    end
    // jmcgrath - to model the behaviour correctly when a cal chanel is being converted
    // an signal to signify the conversion has ended must be produced - this is for single channel mode
    single_chan_conv_end <= 0;
    if( (conv_count == conv_time) || (conv_count == 44))
      single_chan_conv_end <= 1;
       if (adc_state == S3_ST && next_state == S5_ST && rst_lock == 0) begin
         case (curr_pj_set)
           2'b00 :  begin
                      eoc_en <= 1;
                      conv_pj_count[curr_chan] <= 0;
                    end
           2'b01 :  if (conv_pj_count[curr_chan] == 15) begin
                      eoc_en <= 1;
                      conv_pj_count[curr_chan] <= 0;
                      seq_status_avg <= seq_status_avg - 1;
                    end
                    else begin
                      eoc_en <= 0;
                      if (conv_pj_count[curr_chan] == 0)
                        seq_status_avg <= seq_status_avg + 1;
                      conv_pj_count[curr_chan] <= conv_pj_count[curr_chan] + 1;
                    end
           2'b10 :  if (conv_pj_count[curr_chan] == 63) begin
                      eoc_en <= 1;
                      conv_pj_count[curr_chan] <= 0;
                      seq_status_avg <= seq_status_avg - 1;
                    end
                    else begin
                      eoc_en <= 0;
                      if (conv_pj_count[curr_chan] == 0)
                        seq_status_avg <= seq_status_avg + 1;
                      conv_pj_count[curr_chan] <= conv_pj_count[curr_chan] + 1;
                    end
           2'b11 :  if (conv_pj_count[curr_chan] == 255) begin
                      eoc_en <= 1;
                      conv_pj_count[curr_chan] <= 0;
                      seq_status_avg <= seq_status_avg - 1;
                    end
                    else begin
                      eoc_en <= 0;
                      if (conv_pj_count[curr_chan] == 0)
                        seq_status_avg <= seq_status_avg + 1;
                        conv_pj_count[curr_chan] <= conv_pj_count[curr_chan] + 1;
                    end
           default :eoc_en <= 0;
        endcase // case(curr_pj_set)

        case (curr_pj_set2)
          2'b00 :   begin
                      eoc_en2 <= 1;
                      conv_pj_count2[curr_chan2] <= 0;
                    end
          2'b01 :   if (conv_pj_count2[curr_chan2] == 15) begin
                      eoc_en2 <= 1;
                      conv_pj_count2[curr_chan2] <= 0;
                      seq_status_avg2 <= seq_status_avg2 - 1;
                    end
                    else begin
                      eoc_en2 <= 0;
                      if (conv_pj_count2[curr_chan2] == 0)
                        seq_status_avg2 <= seq_status_avg2 + 1;
                      conv_pj_count2[curr_chan2] <= conv_pj_count2[curr_chan2] + 1;
                    end
          2'b10 :   if (conv_pj_count2[curr_chan2] == 63) begin
                      eoc_en2 <= 1;
                      conv_pj_count2[curr_chan2] <= 0;
                      seq_status_avg2 <= seq_status_avg2 - 1;
                    end
                    else begin
                      eoc_en2 <= 0;
                      if (conv_pj_count2[curr_chan2] == 0)
                        seq_status_avg2 <= seq_status_avg2 + 1;
                      conv_pj_count[curr_chan2] <= conv_pj_count[curr_chan2] + 1;
                    end
          2'b11 : if (conv_pj_count2[curr_chan2] == 255) begin
                    eoc_en2 <= 1;
                    conv_pj_count2[curr_chan2] <= 0;
                    seq_status_avg2 <= seq_status_avg2 - 1;
                  end
                  else begin
                    eoc_en2 <= 0;
                    if (conv_pj_count2[curr_chan2] == 0)
                      seq_status_avg2 <= seq_status_avg2 + 1;
                    conv_pj_count2[curr_chan2] <= conv_pj_count2[curr_chan2] + 1;
                  end
          default:eoc_en2 <= 0;
        endcase // case(curr_pj_set)
      end // if (adc_state == S3_ST && next_state == S5_ST)
      else  begin
        eoc_en <= 0;
        eoc_en2 <= 0;
      end
      if (adc_state == S5_ST) begin
        conv_result_reg <= conv_result;
        conv_result_reg2 <= conv_result2;
      end
    end // if (rst_in == 0)
  end
  // end conversion

    
  // average
  always @(adc_state or conv_acc[curr_chan]) 
    if (adc_state == S5_ST ) 
      // no signed or unsigned differences for bit vector conv_acc_vec
      conv_acc_vec = conv_acc[curr_chan];
    else
      conv_acc_vec = 24'b00000000000000000000;

  always @(adc_state or conv_acc2[curr_chan2])
    if (adc_state == S5_ST  )
      // no signed or unsigned differences for bit vector conv_acc_vec
      conv_acc_vec2 = conv_acc2[curr_chan2];
    else
      conv_acc_vec2 = 24'b00000000000000000000;

  always @(posedge adcclk or posedge rst_in) begin
    if (rst_in == 1) begin
      for (j = 0; j <= 31; j = j + 1) begin
        conv_acc[j]  <= 0;
        conv_acc2[j] <= 0;
      end
      conv_acc_result  <= 16'b0000000000000000;
      conv_acc_result2 <= 16'b0000000000000000;
    end
    else  begin
      if (adc_state == S3_ST && next_state == S5_ST) begin
        if (curr_pj_set != 2'b00 && rst_lock != 1)
          conv_acc[curr_chan] <= conv_acc[curr_chan] + conv_result_int;
        else
          conv_acc[curr_chan] <= 0;
          if (curr_pj_set2 != 2'b00 && rst_lock != 1)
            conv_acc2[curr_chan2] <= conv_acc2[curr_chan2] + conv_result_int2;
          else
            conv_acc2[curr_chan2] <= 0;
      end
      else begin
        if (eoc_en == 1) begin
          case (curr_pj_set)
            2'b00 : conv_acc_result <= 16'b0000000000000000;
            2'b01 : conv_acc_result <= conv_acc_vec[19:4];
            2'b10 : conv_acc_result <= conv_acc_vec[21:6];
            2'b11 : conv_acc_result <= conv_acc_vec[23:8];
          endcase 
          conv_acc[curr_chan] <= 0;
        end 

        if (eoc_en2 == 1 ) begin
          case (curr_pj_set2)
            2'b00 : conv_acc_result2 <= 16'b0000000000000000;
            2'b01 : conv_acc_result2 <= conv_acc_vec2[19:4];
            2'b10 : conv_acc_result2 <= conv_acc_vec2[21:6];
            2'b11 : conv_acc_result2 <= conv_acc_vec2[23:8];
          endcase
          conv_acc2[curr_chan2] <= 0;
        end
      end //states
    end // if (rst_in == 0)
  end
  // end average    
        
  // single sequence
  always @(posedge adcclk or posedge rst_in) begin
    if (rst_in == 1)
      adc_s1_flag <= 0;
    else 
      if (adc_state == S6_ST)
        adc_s1_flag <= 1;
  end
  //  end state    

  always @(posedge adcclk or posedge rst_in) begin
    if (rst_in == 1) begin
      seq_count_en <= 0;
      eos_out_tmp <= 0;
      eoc_out_tmp <= 0;
      eoc_out_tmp2 <= 0;
    end 
    else  begin
      if ((adc_state == S3_ST && next_state == S5_ST) && (curr_seq1_0_lat != 4'b0011) && (rst_lock == 0))
        seq_count_en <= 1;
      else
        seq_count_en <= 0;
        
      if (rst_lock == 0) begin
        eos_out_tmp <= eos_en;
        eoc_en_delay <= eoc_en;
        eoc_out_tmp <= eoc_en_delay;
        if (curr_seq1_0_lat[3:2] != 2'b00) begin //simultaneous sampling mode, independent adc mode,or default mode
          eoc_en_delay2 <= eoc_en2;
          eoc_out_tmp2 <= eoc_en_delay2;
        end
      end 
      else begin
        eos_out_tmp <= 0;
        eoc_en_delay <= 0;
        eoc_out_tmp <= 0;
        eoc_en_delay2 <= 0;
        eoc_out_tmp2 <= 0;
      end
    end
  end
    // assign eoc_out_t = eoc_out | eoc_out2;
  always @(eoc_out or eoc_out2)
    eoc_out_t <= #1 (eoc_out | eoc_out2);

  always @(posedge eoc_out_t or posedge rst_in_not_seq) begin
    if (rst_in_not_seq == 1) begin
      for (k = 32; k <= 39; k = k + 1)
        if (k >= 36)
         data_reg[k] <= 16'b1111111111111111;
        else
         data_reg[k] <= 16'b0000000000000000;
      for (k = 40; k <= 42; k = k + 1)
        data_reg[k] <= 16'b0000000000000000;
      for (k = 44; k <= 46; k = k + 1)
        data_reg[k] <= 16'b1111111111111111;
    end 
    else begin
      if ( rst_lock == 0) begin
        if (eoc_out == 1) begin

          if ((curr_chan_lat >= 0 && curr_chan_lat <= 3)  || (curr_chan_lat == 6) ||
               (curr_chan_lat >= 13 && curr_chan_lat <= 31)) begin
            if (curr_pj_set_lat == 2'b00)
              data_reg[curr_chan_lat] <= conv_result_reg;
            else
              data_reg[curr_chan_lat] <= conv_acc_result;
          end

          if  (curr_chan_lat == 4) // VREFP
            data_reg[curr_chan_lat] <= 16'h6AAB; // 1.25V CR-961722
          if (curr_chan_lat == 5) // VREFN
            data_reg[curr_chan_lat] <= 16'h0000; // 0V
              
          if (curr_chan_lat == 0 || curr_chan_lat == 1 || curr_chan_lat == 2) begin
            if (curr_pj_set_lat == 2'b00) begin
              if (conv_result_reg > data_reg[32 + curr_chan_lat])
                data_reg[32 + curr_chan_lat] <= conv_result_reg;
              if (conv_result_reg < data_reg[36 + curr_chan_lat])
                data_reg[36 + curr_chan_lat] <= conv_result_reg;
            end
            else begin
              if (conv_acc_result > data_reg[32 + curr_chan_lat])
                data_reg[32 + curr_chan_lat] <= conv_acc_result;
              if (conv_acc_result < data_reg[36 + curr_chan_lat])
                data_reg[36 + curr_chan_lat] <= conv_acc_result;
            end   
          end
          if (curr_chan_lat == 6) begin
            if (curr_pj_set_lat == 2'b00) begin
              if (conv_result_reg > data_reg[35])
                data_reg[35] <= conv_result_reg;
              if (conv_result_reg < data_reg[39])
                data_reg[39] <= conv_result_reg;
            end
            else begin
              if (conv_acc_result > data_reg[35])
                data_reg[35] <= conv_acc_result;
              if (conv_acc_result < data_reg[39])
                data_reg[39] <= conv_acc_result;
            end
          end
          if (curr_chan_lat == 5'b01101) begin
            if (curr_pj_set_lat == 2'b00) begin
              if (conv_result_reg > data_reg[40])
                data_reg[40] <= conv_result_reg;
              if (conv_result_reg < data_reg[44])
                data_reg[44] <= conv_result_reg;
            end
            else begin
              if (conv_acc_result > data_reg[40])
                data_reg[40] <= conv_acc_result;
              if (conv_acc_result < data_reg[44])
                data_reg[44] <= conv_acc_result;
            end
          end
          if (curr_chan_lat == 5'b01110) begin
            if (curr_pj_set_lat == 2'b00) begin
              if (conv_result_reg > data_reg[41])
                data_reg[41] <= conv_result_reg;
              if (conv_result_reg < data_reg[45])
                data_reg[45] <= conv_result_reg;
            end
            else begin
              if (conv_acc_result > data_reg[41])
                data_reg[41] <= conv_acc_result;
              if (conv_acc_result < data_reg[45])
                data_reg[45] <= conv_acc_result;
            end
          end
          if (curr_chan_lat == 5'b01111) begin
            if (curr_pj_set_lat == 2'b00) begin
              if (conv_result_reg > data_reg[42])
                data_reg[42] <= conv_result_reg;
              if (conv_result_reg < data_reg[46])
                data_reg[46] <= conv_result_reg;
            end
            else begin
              if (conv_acc_result > data_reg[42])
                data_reg[42] <= conv_acc_result;
              if (conv_acc_result < data_reg[46])
                data_reg[46] <= conv_acc_result;
            end
          end
        end //eoc_out=1
       
        if (eoc_out2 == 1) begin
          if ((curr_chan_lat2 >= 0 && curr_chan_lat2 <= 3)  || (curr_chan_lat2 == 6) ||
             (curr_chan_lat2 >= 13 && curr_chan_lat2 <= 31)) begin
            if (curr_pj_set_lat2 == 2'b00)
              data_reg[curr_chan_lat2] <= conv_result_reg2;
            else
              data_reg[curr_chan_lat2] <= conv_acc_result2;
          end
          if (curr_chan_lat2 == 4)
            data_reg[curr_chan_lat2] <= 16'hD555;
          if (curr_chan_lat2 == 5)
            data_reg[curr_chan_lat2] <= 16'h0000;
                
          if (curr_chan_lat2 == 0 || curr_chan_lat2 == 1 || curr_chan_lat2 == 2) begin
            if (curr_pj_set_lat2 == 2'b00) begin
              if (conv_result_reg2 > data_reg[32 + curr_chan_lat2])
                data_reg[32 + curr_chan_lat2] <= conv_result_reg2;
              if (conv_result_reg2 < data_reg[36 + curr_chan_lat2])
                data_reg[36 + curr_chan_lat2] <= conv_result_reg2;
            end
            else begin
              if (conv_acc_result2 > data_reg[32 + curr_chan_lat2])
                data_reg[32 + curr_chan_lat2] <= conv_acc_result2;
              if (conv_acc_result2 < data_reg[36 + curr_chan_lat2])
                data_reg[36 + curr_chan_lat2] <= conv_acc_result2;
            end   
          end
          if (curr_chan_lat2 == 6) begin
            if (curr_pj_set_lat2 == 2'b00) begin
              if (conv_result_reg2 > data_reg[35])
                data_reg[35] <= conv_result_reg2;
              if (conv_result_reg2 < data_reg[39])
                data_reg[39] <= conv_result_reg2;
            end
            else begin
              if (conv_acc_result2 > data_reg[35])
                data_reg[35] <= conv_acc_result2;
              if (conv_acc_result2 < data_reg[39])
                data_reg[39] <= conv_acc_result2;
            end
          end
          if (curr_chan_lat2 == 5'b01101) begin
            if (curr_pj_set_lat2 == 2'b00) begin
              if (conv_result_reg2 > data_reg[40])
                data_reg[40] <= conv_result_reg2;
              if (conv_result_reg2 < data_reg[44])
                data_reg[44] <= conv_result_reg2;
            end
            else begin
              if (conv_acc_result2 > data_reg[40])
                data_reg[40] <= conv_acc_result2;
              if (conv_acc_result2 < data_reg[44])
                data_reg[44] <= conv_acc_result2;
            end
          end
          if (curr_chan_lat2 == 5'b01110) begin
            if (curr_pj_set_lat2 == 2'b00) begin
              if (conv_result_reg2 > data_reg[41])
                data_reg[41] <= conv_result_reg2;
              if (conv_result_reg2 < data_reg[45])
                data_reg[45] <= conv_result_reg2;
            end
            else begin
              if (conv_acc_result2 > data_reg[41])
                data_reg[41] <= conv_acc_result2;
              if (conv_acc_result2 < data_reg[45])
                data_reg[45] <= conv_acc_result2;
            end
          end
          if (curr_chan_lat2 == 5'b01111) begin
            if (curr_pj_set_lat2 == 2'b00) begin
              if (conv_result_reg2 > data_reg[42])
                data_reg[42] <= conv_result_reg2;
              if (conv_result_reg2 < data_reg[46])
                data_reg[46] <= conv_result_reg2;
            end
            else begin
              if (conv_acc_result2 > data_reg[42])
                data_reg[42] <= conv_acc_result2;
              if (conv_acc_result2 < data_reg[46])
                data_reg[46] <= conv_acc_result2;
            end
          end
        end 
      end 
    end
  end //always

  reg [15:0] data_written;
  always @(negedge busy_r or posedge rst_in_not_seq) begin
    if (rst_in_not_seq)
      data_written <= 16'b0;
    else begin
      if (curr_seq1_0[3:2] != 2'b10) begin
        if (curr_pj_set == 2'b00) 
          data_written <= conv_result_reg;
        else
          data_written <= conv_acc_result;
      end
      else begin
        if (curr_pj_set2 == 2'b00) 
          data_written <= conv_result_reg2;
        else
          data_written <= conv_acc_result2;
      end
    end
  end

  reg [4:0] op_count=15;
  reg       busy_out_sync;
  wire      busy_out_low_edge;
    
  // eos and eoc

  always @( posedge eoc_out_tmp or posedge eoc_out or posedge rst_in) begin
    if (rst_in ==1)
      eoc_out_tmp1 <= 0;
    else if ( eoc_out ==1)
      eoc_out_tmp1 <= 0;
    else if ( eoc_out_tmp == 1) begin
      if (curr_chan != 5'b01000 && ( xadc2_en == 1 || (curr_seq1_0[3:2] != 2'b10 && xadc2_en == 0)))    
        eoc_out_tmp1 <= 1; 
      else
        eoc_out_tmp1 <= 0;
    end
  end

  always @( posedge eoc_out_tmp2 or posedge eoc_out2 or posedge rst_in) begin
    if (rst_in ==1)
      eoc_out_tmp21 <= 0;
    else if ( eoc_out2 ==1)
      eoc_out_tmp21 <= 0;
    else if ( eoc_out_tmp2 == 1) begin
      if (curr_chan2 != 5'b01000 && ( xadc2_en == 1 || (curr_seq1_0[3:2] == 2'b10 && xadc2_en == 0)))
        eoc_out_tmp21 <= 1;
      else
        eoc_out_tmp21 <= 0;
    end
  end

  always @( posedge eos_out_tmp or posedge eos_out or posedge rst_in) begin
    if (rst_in ==1)
      eos_out_tmp1 <= 0;
    else if ( eos_out ==1)
      eos_out_tmp1 <= 0;
    else if ( eos_out_tmp == 1 &&  ( xadc2_en == 1 || (curr_seq1_0[3:2] != 2'b10 && xadc2_en == 0)))    
      eos_out_tmp1 <= 1; 
  end

  assign busy_out_low_edge = (busy_out==0 && busy_out_sync==1) ? 1 : 0;

  // create a 4 clock window after the second to last EOC in which the sequence register can be updated without a reset.
  always @( posedge dclk_in or posedge rst_in) begin
    if (rst_in) begin
      eoc_last <= 1'b0;
      eoc_last_count <= 0;
    end 
    else begin
      if ((op_count == 16) && (eoc_out_tmp1 == 1'b1)) begin // EOC conditions
        if (seq_count == seq_num) begin // second to last EOC, last seq starting
          eoc_last <= 1'b1;
        end 
        else begin
          eoc_last <= 1'b0;
        end
      end 
      else if (eoc_last_count >= 3) begin // window size in clocks - 1 
        eoc_last <= 1'b0;
      end
      if (eoc_last == 1'b1) begin // count clocks when window open
        eoc_last_count <= eoc_last_count + 1;
      end 
      else begin
        eoc_last_count <= 0;
      end
    end
  end
   
  always @( posedge dclk_in or posedge rst_in) begin
    if (rst_in) begin
      op_count <= 15;
      busy_out_sync <= 0;
    end
    drp_update   <= 0;
    alarm_update <= 0;
    eoc_out      <= 0;
    eoc_out2     <= 0;
    eos_out      <= 0;

    if(rst_in==0) begin
      busy_out_sync <= busy_out;
      if(op_count==3)           
        drp_update <= 1;
      if(op_count==5 && ((eoc_out_tmp1==1 && curr_seq1_0[3:2] != 2'b10) || eoc_out_tmp21 == 1))
        alarm_update <=1;
        // if(op_count==9 ) begin
        if(op_count== 16 ) begin
          eoc_out <= eoc_out_tmp1;
          eoc_out2 <= eoc_out_tmp21;
        end
        //if(op_count==9)            
        //  eos_out <= eos_out_tmp1;
        if(op_count==16)
          eos_out <= eos_out_tmp1;
        if (busy_out_low_edge==1 )
          op_count <= 0;
        else if(op_count < 22)
          op_count <= op_count +1;
    end
  end
  // end eos and eoc

  // alarm
   
  always @( posedge alarm_update or posedge rst_in_not_seq ) begin
    if (rst_in_not_seq == 1) begin
       ot_out_reg <= 0;
       alarm_out_reg <= 8'b0;
    end
    else 
      if (rst_lock == 0) begin
        if (curr_seq1_0[3:2] == 2'b10)  // independent ADC
          curr_chan_tmp = curr_chan_lat2;
        else
          curr_chan_tmp = curr_chan_lat;
        if (curr_chan_tmp == 0) begin
          if (data_written >= ot_limit_reg)
            ot_out_reg <= 1;
          else if (data_written < dr_sram[7'h57])  
            ot_out_reg <= 0;
        
          if (data_written > dr_sram[7'h50])  
            alarm_out_reg[0] <= 1;
          else if (data_written < dr_sram[7'h54])
            alarm_out_reg[0] <= 0;
        end
        if (curr_chan_tmp == 1) begin
          if (data_written > dr_sram[7'h51] || data_written < dr_sram[7'h55])
            alarm_out_reg[1] <= 1;
          else
            alarm_out_reg[1] <= 0;
        end
        if (curr_chan_tmp == 2) begin
          if (data_written > dr_sram[7'h52] || data_written < dr_sram[7'h56])
            alarm_out_reg[2] <= 1;
          else
            alarm_out_reg[2] <= 0;
        end
        if (curr_chan_tmp == 6) begin
          if (data_written > dr_sram[7'h58] || data_written < dr_sram[7'h5C])
            alarm_out_reg[3] <= 1;
          else
              alarm_out_reg[3] <= 0;
        end
        if (curr_chan_tmp == 13) begin
          if (data_written > dr_sram[7'h59] || data_written < dr_sram[7'h5D])                
            alarm_out_reg[4] <= 1;
          else
            alarm_out_reg[4] <= 0;
        end
        if (curr_chan_tmp == 14) begin
          if (data_written > dr_sram[7'h5A] || data_written < dr_sram[7'h5E])                
            alarm_out_reg[5] <= 1;
          else
            alarm_out_reg[5] <= 0;
        end
        if (curr_chan_tmp == 15) begin
          if (data_written > dr_sram[7'h5B] || data_written < dr_sram[7'h5F])                
            alarm_out_reg[6] <= 1;
          else
            alarm_out_reg[6] <= 0;
        end
      end // if (rst_lock == 0)
  end //always
   
  always @(ot_out_reg or ot_en or alarm_out_reg or alarm_en) begin
    ot_out = ot_out_reg & ot_en;
    alarm_out[6:0] = alarm_out_reg[6:0] & alarm_en[6:0];
    alarm_out[7] = |alarm_out[6:0];
  end

  // end alarm

  //*** Timing_Checks_Start_here

`ifdef XIL_TIMING

  reg notifier, notifier_do;
   
  always @(notifier) begin
    alarm_out_reg = 7'bx;
    ot_out = 1'bx;
    busy_out = 1'bx;
    eoc_out = 1'bx;
    eos_out = 1'bx;
    curr_chan = 5'bx;
    drdy_out = 1'bx;
    do_out = 16'bx;
  end 
   
  always @(notifier_do) begin
    drdy_out = 1'bx;
    do_out = 16'bx;
  end
   
  wire dclk_en_n;
  wire dclk_en_p;
  assign dclk_en_n =  IS_DCLK_INVERTED;
  assign dclk_en_p = ~IS_DCLK_INVERTED;

  wire rst_en_n;
  wire rst_en_p;
  assign rst_en_n = ~rst_input && dclk_en_n;
  assign rst_en_p = ~rst_input && dclk_en_p;

`endif //  `ifdef XIL_TIMING
   
  specify
    (DCLK => ALM) = (100:100:100, 100:100:100);
    (DCLK => BUSY) = (100:100:100, 100:100:100);
    (DCLK => CHANNEL) = (100:100:100, 100:100:100);
    (DCLK => DO) = (100:100:100, 100:100:100);
    (DCLK => DRDY) = (100:100:100, 100:100:100);
    (DCLK => EOC) = (100:100:100, 100:100:100);
    (DCLK => EOS) = (100:100:100, 100:100:100);
    (DCLK => JTAGBUSY) = (100:100:100, 100:100:100);
    (DCLK => JTAGLOCKED) = (100:100:100, 100:100:100);
    (DCLK => JTAGMODIFIED) = (100:100:100, 100:100:100);
    (DCLK => MUXADDR) = (100:100:100, 100:100:100);
    (DCLK => OT) = (100:100:100, 100:100:100);

`ifdef XIL_TIMING

  $period (posedge CONVST, 0:0:0, notifier);
  $period (posedge CONVSTCLK, 0:0:0, notifier);
  $period (negedge CONVSTCLK, 0:0:0, notifier);
  $period (posedge DCLK, 0:0:0, notifier);
  $setuphold (posedge DCLK, negedge DADDR, 0:0:0, 0:0:0, notifier_do, rst_en_p, rst_en_p, DCLK_dly,DADDR_dly);
  $setuphold (posedge DCLK, negedge DEN, 0:0:0, 0:0:0, notifier_do, rst_en_p, rst_en_p, DCLK_dly,DEN_dly);
  $setuphold (posedge DCLK, negedge DI, 0:0:0, 0:0:0, notifier_do, rst_en_p, rst_en_p, DCLK_dly,DI_dly);
  $setuphold (posedge DCLK, negedge DWE, 0:0:0, 0:0:0, notifier_do, rst_en_p, rst_en_p, DCLK_dly,DWE_dly);
  $setuphold (posedge DCLK, posedge DADDR, 0:0:0, 0:0:0, notifier_do, rst_en_p, rst_en_p, DCLK_dly,DADDR_dly);
  $setuphold (posedge DCLK, posedge DEN, 0:0:0, 0:0:0, notifier_do, rst_en_p, rst_en_p, DCLK_dly,DEN_dly);
  $setuphold (posedge DCLK, posedge DI, 0:0:0, 0:0:0, notifier_do, rst_en_p, rst_en_p, DCLK_dly,DI_dly);
  $setuphold (posedge DCLK, posedge DWE, 0:0:0, 0:0:0, notifier_do, rst_en_p, rst_en_p, DCLK_dly,DWE_dly);
  $setuphold (negedge DCLK, negedge DADDR, 0:0:0, 0:0:0, notifier_do, rst_en_n, rst_en_n, DCLK_dly,DADDR_dly);
  $setuphold (negedge DCLK, negedge DEN, 0:0:0, 0:0:0, notifier_do, rst_en_n, rst_en_n, DCLK_dly,DEN_dly);
  $setuphold (negedge DCLK, negedge DI, 0:0:0, 0:0:0, notifier_do, rst_en_n, rst_en_n, DCLK_dly,DI_dly);
  $setuphold (negedge DCLK, negedge DWE, 0:0:0, 0:0:0, notifier_do, rst_en_n, rst_en_n, DCLK_dly,DWE_dly);
  $setuphold (negedge DCLK, posedge DADDR, 0:0:0, 0:0:0, notifier_do, rst_en_n, rst_en_n, DCLK_dly,DADDR_dly);
  $setuphold (negedge DCLK, posedge DEN, 0:0:0, 0:0:0, notifier_do, rst_en_n, rst_en_n, DCLK_dly,DEN_dly);
  $setuphold (negedge DCLK, posedge DI, 0:0:0, 0:0:0, notifier_do, rst_en_n, rst_en_n, DCLK_dly,DI_dly);
  $setuphold (negedge DCLK, posedge DWE, 0:0:0, 0:0:0, notifier_do, rst_en_n, rst_en_n, DCLK_dly,DWE_dly);

`endif //  `ifdef XIL_TIMING

  specparam PATHPULSE$ = 0;

  endspecify

endmodule 

`endcelldefine
