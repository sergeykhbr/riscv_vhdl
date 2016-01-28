------------------------------------------------------------------------------
--! @file
--! @copyright Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author    Sergey Khabarov - sergeykhbr@gmail.com
--! @brief     Top Level GPS-CA Fast Search Engine
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
library techmap;
use techmap.gencomp.all;
library commonlib;
use commonlib.types_common.all;
library gnsslib;
use gnsslib.types_gnss.all;
use gnsslib.types_fse_v2.all;
use gnsslib.types_sync.all;
library rocketlib;
use rocketlib.types_nasti.all;

entity TopFSE is
generic
(
    tech   : integer := 0;
    xindex : integer := 0;
    xaddr  : integer := 0;
    xmask  : integer := 16#FFFFF#;
    sys    : integer := GEN_SYSTEM_GPSCA
);
port (
    i : in fse_in_type;
    o : out fse_out_type
);
end;

architecture rtl of TopFSE is
   signal i_ahb_fse : bridge_in_type;
   signal o_ahb_fse : bridge_out_type;

   signal i_reclk : reclk_in_type;
   signal o_reclk : reclk_out_type;

   signal i_control : fsectrl_in_type;
   signal o_control : fsectrl_out_type;
   
   signal i_recorder : rec_in_type;
   signal o_recorder : rec_out_type;
   
   signal i_carrnco_if : carnco_in_type;
   signal o_carrnco_if : carnco_out_type;
   
   signal i_decim : decim_in_type;
   signal o_decim : decim_out_type;
   
   signal i_requant_if : rqnt_in_type;
   signal o_requant_if : rqnt_out_type;

   signal i_carrnco_dop : carnco_in_type;
   signal o_carrnco_dop : carnco_out_type;
   
   signal i_requant_dop : rqnt_in_type;
   signal o_requant_dop : rqnt_out_type;

   signal o_fifo_I    : std_logic_vector(2*fse_section_num(sys)*1024-1 downto 0);
   signal o_fifo_Q    : std_logic_vector(2*fse_section_num(sys)*1024-1 downto 0);
   signal o_fifo_rdyI : std_logic;
   signal o_fifo_rdyQ : std_logic;
   signal wFifoRdy    : std_ulogic;

   signal o_prngen_prn : std_logic_vector(fse_section_num(sys)*1024-1 downto 0);

   signal o_matrix_I   : std_logic_vector(14 downto 0);
   signal o_matrix_Q   : std_logic_vector(14 downto 0);
   signal o_matrix_rdy : std_logic;

   signal i_accum : accm_in_type;
   signal o_accum : accm_out_type;

begin


    -- Bridge:
    i_ahb_fse.nrst       <= i.nrst;
    i_ahb_fse.clk_bus    <= i.clk_bus;
    i_ahb_fse.clk_fse    <= i.clk_fse;
    i_ahb_fse.axi        <= i.axi;
    i_ahb_fse.rdata      <= o_control.rdata;
    i_ahb_fse.rdata_rdy  <= o_control.rdata_rdy;

    clAxiFseBridge : AxiFseBridge generic map
    (
      xindex => xindex,
      xaddr => xaddr,
      xmask => xmask,
      did => GNSSSENSOR_FSE_V2 + sys
    ) port map
    (
      i_ahb_fse,
      o_ahb_fse
    );

    o.axi <= o_ahb_fse.axi;
    o.cfg <= o_ahb_fse.cfg;


    -- Reclock from ADC clock -> FSE clock:
    i_reclk.nrst     <= i.nrst;
    i_reclk.clk_fse  <= i.clk_fse;
    i_reclk.clk_adc  <= i.clk_adc;
    i_reclk.ms_pulse <= i.ms_pulse;
    i_reclk.pps      <= i.pps;
    i_reclk.I        <= i.I;
    i_reclk.Q        <= i.Q;

    clReclk : Reclk port map
    (
      i_reclk,
      o_reclk
    );
    
    -- Controller:
    i_control.nrst         <= i.nrst;
    i_control.clk          <= i.clk_fse;
    i_control.wr_ena       <= o_ahb_fse.wr_ena;
    i_control.rd_ena       <= o_ahb_fse.rd_ena;
    i_control.addr         <= o_ahb_fse.addr;
    i_control.wdata        <= o_ahb_fse.data;
    i_control.ms_pulse     <= o_reclk.ms_pulse;
    i_control.pps          <= o_reclk.pps;
    i_control.adc_valid    <= o_reclk.adc_valid;
    i_control.rec_rdy      <= o_recorder.rec_end;
    i_control.latch_max    <= o_accum.max;
    i_control.latch_ind    <= o_accum.maxind;
    i_control.latch_noise  <= o_accum.noise;
    i_control.latch_dopler <= o_accum.dopler;
    i_control.latch_rdy    <= o_accum.rdy;

    clFseControl : FseControl 
    generic map (sys)
    port map
    (
      i_control,
      o_control
    );

    -- IF frequency mixer and quantizer
    i_carrnco_if.nrst   <= i.nrst;
    i_carrnco_if.clk    <= i.clk_fse;
    i_carrnco_if.I      <= adc2amp(o_control.adc_fmt, o_reclk.I);
    i_carrnco_if.Q      <= adc2amp(o_control.adc_fmt, o_reclk.Q);
    i_carrnco_if.ena    <= o_reclk.adc_valid and o_control.rec_ena;
    i_carrnco_if.thresh <= o_control.carrnco_th;
    i_carrnco_if.nco    <= o_control.carr_nco_if;

    clCarrNcoIF : CarrierNco port map
    (
      i_carrnco_if,
      o_carrnco_if
    );

    i_decim.nrst        <= i.nrst;
    i_decim.clk         <= i.clk_fse;
    i_decim.ena         <= o_carrnco_if.rdy;
    i_decim.I           <= o_carrnco_if.I;
    i_decim.Q           <= o_carrnco_if.Q;
    i_decim.code_nco_th <= o_control.codenco_th;
    i_decim.code_nco    <= o_control.code_nco;
    
    clDecimator : Decimator port map
    (
      i_decim,
      o_decim
    );

    i_requant_if.nrst <= i.nrst;
    i_requant_if.clk  <= i.clk_fse;
    i_requant_if.ena  <= o_decim.rdy;
    i_requant_if.I    <= o_decim.I;
    i_requant_if.Q    <= o_decim.Q;

    clRequantIF : Requant port map 
    (
      i_requant_if,
      o_requant_if
    );


    -- Record/Playback samples module
    i_recorder.nrst      <= i.nrst;
    i_recorder.clk       <= i.clk_fse;
    i_recorder.I         <= o_requant_if.I;
    i_recorder.Q         <= o_requant_if.Q;
    i_recorder.adc_valid <= o_requant_if.rdy;
    i_recorder.rec_ena   <= o_control.rec_ena;
    i_recorder.play_ena  <= o_control.play_ena;
    i_recorder.test_mode <= i.test_mode;

    clRecorder : Recorder 
    generic map
    (
      generic_tech => tech,
      generic_sys => sys
    ) port map 
    (
      i_recorder,
      o_recorder
    );

    -- Dopler frequency mixer and quantizer
    i_carrnco_dop.nrst   <= i.nrst;
    i_carrnco_dop.clk    <= i.clk_fse;
    i_carrnco_dop.I      <= adc2amp('0', o_recorder.I);
    i_carrnco_dop.Q      <= adc2amp('0', o_recorder.Q);
    i_carrnco_dop.ena    <= o_recorder.rdy;
    i_carrnco_dop.thresh <= conv_std_logic_vector((1024000*fse_section_num(sys))/64, 28) & "0000";
    i_carrnco_dop.nco    <= o_control.dopler_nco;

    clCarrNcoDop : CarrierNco port map
    (
      i_carrnco_dop,
      o_carrnco_dop
    );

    i_requant_dop.nrst <= i.nrst;
    i_requant_dop.clk  <= i.clk_fse;
    i_requant_dop.ena  <= o_carrnco_dop.rdy;
    i_requant_dop.I(6 downto 0)  <= o_carrnco_dop.I;
    i_requant_dop.I(15 downto 7) <= (others => o_carrnco_dop.I(6));
    i_requant_dop.Q(6 downto 0)  <= o_carrnco_dop.Q;
    i_requant_dop.Q(15 downto 7) <= (others => o_carrnco_dop.Q(6));

    clRequantDop : Requant port map 
    (
      i_requant_dop,
      o_requant_dop
    );


    -- 2+2 bits move to FIFO:
    clFifo : FifoSection 
    generic map (sys)
    port map
    (
      i_nrst => i.nrst,
      i_clk  => i.clk_fse,
      i_ena  => o_requant_dop.rdy,
      i_I    => o_requant_dop.I,
      i_Q    => o_requant_dop.Q,
      o_I    => o_fifo_I,
      o_Q    => o_fifo_Q,
      o_rdyI => o_fifo_rdyI,
      o_rdyQ => o_fifo_rdyQ
    );

    -- PRN generator:
    clPrnGen : PrnGeneratorFse 
    generic map (
      tech => tech,
      sys => sys
    )
    port map
    (
      i_nrst    => i.nrst,
      i_clk     => i.clk_fse,
      i_ena     => o_control.new_prn,
      i_init_g2 => o_control.prn,
      o_prn     => o_prngen_prn
    );

    -- Pyramidal accumulator:
    wFifoRdy   <= o_fifo_rdyI or o_fifo_rdyQ;

    clMatrix : MatrixCorrelator 
    generic map (sys)
    port map
    (
      i_nrst  => i.nrst,
      i_clk   => i.clk_fse,
      i_ena   => wFifoRdy,
      i_re_im => o_fifo_rdyQ,
      i_prn   => o_prngen_prn,
      i_I     => o_fifo_I,
      i_Q     => o_fifo_Q,
      o_I     => o_matrix_I,
      o_Q     => o_matrix_Q,
      o_rdy   => o_matrix_rdy
    );

    -- 
    i_accum.nrst       <= i.nrst;
    i_accum.clk        <= i.clk_fse;
    i_accum.coh_ena    <= o_control.coh_ena;
    i_accum.dopler     <= o_control.dopler_nco;
    i_accum.ms_total   <= o_control.ms_total;
    i_accum.new_cycle  <= o_control.new_cycle;
    i_accum.rst_max    <= o_control.rst_max;
    i_accum.I          <= o_matrix_I(14) & o_matrix_I;
    i_accum.Q          <= o_matrix_Q(14) & o_matrix_Q;
    i_accum.ena        <= o_matrix_rdy;

    clAccum : Accumulator 
    generic map
    (
      generic_tech => tech,
      generic_sys => sys
    ) port map
    (
      i_accum,
      o_accum
    );


end; 


