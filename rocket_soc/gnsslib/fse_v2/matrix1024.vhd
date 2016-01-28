-----------------------------------------------------------------------------
--! @file
--! @copyright Copyright 2015 GNSS Sensor Ltd. All right reserved.
--! @author    Sergey Khabarov - sergeykhbr@gmail.com
--! @brief     Matrix correlator 4096 samples length
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
library commonlib;
use commonlib.types_common.all;
library gnsslib;
use gnsslib.types_fse_v2.all;

entity Matrix1024 is
port (
    i_nrst  : in std_logic;
    i_clk   : in std_logic;
    i_ena   : in std_logic;
    i_re_im : in std_logic;
    i_prn   : in std_logic_vector(1024-1 downto 0);
    i_I     : in std_logic_vector(2*1024-1 downto 0);
    i_Q     : in std_logic_vector(2*1024-1 downto 0);
    o_sum   : out std_logic_vector(12 downto 0)
);
end;

architecture rtl of Matrix1024 is

  type regtype is record
    Lvl4     : std_logic_vector(64*7-1 downto 0);
    Lvl8     : std_logic_vector(4*11-1 downto 0);
    ena      : std_logic_vector(1 downto 0);
  end record;

  signal r, rin : regtype;
  signal wbLvl1 : std_logic_vector(4*512-1 downto 0);

  component Matrix24 is
  port (
    i_nrst          : in std_logic;
    i_clk           : in std_logic;
    i_ena           : in std_logic;
    i_re_im         : in std_logic;
    i_prn           : in std_logic_vector(1024-1 downto 0);
    i_I             : in std_logic_vector(2*1024-1 downto 0);
    i_Q             : in std_logic_vector(2*1024-1 downto 0);
    o_lvl1          : out std_logic_vector(512*4-1 downto 0)
  );
  end component;

begin


  l1 : Matrix24 port map
  (
    i_nrst,
    i_clk,
    i_ena,
    i_re_im,
    i_prn,
    i_I,
    i_Q,
    wbLvl1
  );



  comb : process (r, i_nrst, i_ena, wbLvl1)
  variable v : regtype;
  variable wbLvl2 : std_logic_vector(5*256-1 downto 0);
  variable wbLvl3 : std_logic_vector(6*128-1 downto 0);
  variable wbLvl5 : std_logic_vector(8*32-1 downto 0);
  variable wbLvl6 : std_logic_vector(9*16-1 downto 0);
  variable wbLvl7 : std_logic_vector(10*8-1 downto 0);
  variable wbLvl9 : std_logic_vector(12*2-1 downto 0);
  
  begin
    
    --v := r;
    v.ena := r.ena(0) & i_ena;


    wbLvl2(4 downto 0) := (wbLvl1(3) & wbLvl1(3 downto 0)) + (wbLvl1(7) & wbLvl1(7 downto 4));
    wbLvl2(9 downto 5) := (wbLvl1(11) & wbLvl1(11 downto 8)) + (wbLvl1(15) & wbLvl1(15 downto 12));
    wbLvl2(14 downto 10) := (wbLvl1(19) & wbLvl1(19 downto 16)) + (wbLvl1(23) & wbLvl1(23 downto 20));
    wbLvl2(19 downto 15) := (wbLvl1(27) & wbLvl1(27 downto 24)) + (wbLvl1(31) & wbLvl1(31 downto 28));
    wbLvl2(24 downto 20) := (wbLvl1(35) & wbLvl1(35 downto 32)) + (wbLvl1(39) & wbLvl1(39 downto 36));
    wbLvl2(29 downto 25) := (wbLvl1(43) & wbLvl1(43 downto 40)) + (wbLvl1(47) & wbLvl1(47 downto 44));
    wbLvl2(34 downto 30) := (wbLvl1(51) & wbLvl1(51 downto 48)) + (wbLvl1(55) & wbLvl1(55 downto 52));
    wbLvl2(39 downto 35) := (wbLvl1(59) & wbLvl1(59 downto 56)) + (wbLvl1(63) & wbLvl1(63 downto 60));
    wbLvl2(44 downto 40) := (wbLvl1(67) & wbLvl1(67 downto 64)) + (wbLvl1(71) & wbLvl1(71 downto 68));
    wbLvl2(49 downto 45) := (wbLvl1(75) & wbLvl1(75 downto 72)) + (wbLvl1(79) & wbLvl1(79 downto 76));
    wbLvl2(54 downto 50) := (wbLvl1(83) & wbLvl1(83 downto 80)) + (wbLvl1(87) & wbLvl1(87 downto 84));
    wbLvl2(59 downto 55) := (wbLvl1(91) & wbLvl1(91 downto 88)) + (wbLvl1(95) & wbLvl1(95 downto 92));
    wbLvl2(64 downto 60) := (wbLvl1(99) & wbLvl1(99 downto 96)) + (wbLvl1(103) & wbLvl1(103 downto 100));
    wbLvl2(69 downto 65) := (wbLvl1(107) & wbLvl1(107 downto 104)) + (wbLvl1(111) & wbLvl1(111 downto 108));
    wbLvl2(74 downto 70) := (wbLvl1(115) & wbLvl1(115 downto 112)) + (wbLvl1(119) & wbLvl1(119 downto 116));
    wbLvl2(79 downto 75) := (wbLvl1(123) & wbLvl1(123 downto 120)) + (wbLvl1(127) & wbLvl1(127 downto 124));
    wbLvl2(84 downto 80) := (wbLvl1(131) & wbLvl1(131 downto 128)) + (wbLvl1(135) & wbLvl1(135 downto 132));
    wbLvl2(89 downto 85) := (wbLvl1(139) & wbLvl1(139 downto 136)) + (wbLvl1(143) & wbLvl1(143 downto 140));
    wbLvl2(94 downto 90) := (wbLvl1(147) & wbLvl1(147 downto 144)) + (wbLvl1(151) & wbLvl1(151 downto 148));
    wbLvl2(99 downto 95) := (wbLvl1(155) & wbLvl1(155 downto 152)) + (wbLvl1(159) & wbLvl1(159 downto 156));
    wbLvl2(104 downto 100) := (wbLvl1(163) & wbLvl1(163 downto 160)) + (wbLvl1(167) & wbLvl1(167 downto 164));
    wbLvl2(109 downto 105) := (wbLvl1(171) & wbLvl1(171 downto 168)) + (wbLvl1(175) & wbLvl1(175 downto 172));
    wbLvl2(114 downto 110) := (wbLvl1(179) & wbLvl1(179 downto 176)) + (wbLvl1(183) & wbLvl1(183 downto 180));
    wbLvl2(119 downto 115) := (wbLvl1(187) & wbLvl1(187 downto 184)) + (wbLvl1(191) & wbLvl1(191 downto 188));
    wbLvl2(124 downto 120) := (wbLvl1(195) & wbLvl1(195 downto 192)) + (wbLvl1(199) & wbLvl1(199 downto 196));
    wbLvl2(129 downto 125) := (wbLvl1(203) & wbLvl1(203 downto 200)) + (wbLvl1(207) & wbLvl1(207 downto 204));
    wbLvl2(134 downto 130) := (wbLvl1(211) & wbLvl1(211 downto 208)) + (wbLvl1(215) & wbLvl1(215 downto 212));
    wbLvl2(139 downto 135) := (wbLvl1(219) & wbLvl1(219 downto 216)) + (wbLvl1(223) & wbLvl1(223 downto 220));
    wbLvl2(144 downto 140) := (wbLvl1(227) & wbLvl1(227 downto 224)) + (wbLvl1(231) & wbLvl1(231 downto 228));
    wbLvl2(149 downto 145) := (wbLvl1(235) & wbLvl1(235 downto 232)) + (wbLvl1(239) & wbLvl1(239 downto 236));
    wbLvl2(154 downto 150) := (wbLvl1(243) & wbLvl1(243 downto 240)) + (wbLvl1(247) & wbLvl1(247 downto 244));
    wbLvl2(159 downto 155) := (wbLvl1(251) & wbLvl1(251 downto 248)) + (wbLvl1(255) & wbLvl1(255 downto 252));
    wbLvl2(164 downto 160) := (wbLvl1(259) & wbLvl1(259 downto 256)) + (wbLvl1(263) & wbLvl1(263 downto 260));
    wbLvl2(169 downto 165) := (wbLvl1(267) & wbLvl1(267 downto 264)) + (wbLvl1(271) & wbLvl1(271 downto 268));
    wbLvl2(174 downto 170) := (wbLvl1(275) & wbLvl1(275 downto 272)) + (wbLvl1(279) & wbLvl1(279 downto 276));
    wbLvl2(179 downto 175) := (wbLvl1(283) & wbLvl1(283 downto 280)) + (wbLvl1(287) & wbLvl1(287 downto 284));
    wbLvl2(184 downto 180) := (wbLvl1(291) & wbLvl1(291 downto 288)) + (wbLvl1(295) & wbLvl1(295 downto 292));
    wbLvl2(189 downto 185) := (wbLvl1(299) & wbLvl1(299 downto 296)) + (wbLvl1(303) & wbLvl1(303 downto 300));
    wbLvl2(194 downto 190) := (wbLvl1(307) & wbLvl1(307 downto 304)) + (wbLvl1(311) & wbLvl1(311 downto 308));
    wbLvl2(199 downto 195) := (wbLvl1(315) & wbLvl1(315 downto 312)) + (wbLvl1(319) & wbLvl1(319 downto 316));
    wbLvl2(204 downto 200) := (wbLvl1(323) & wbLvl1(323 downto 320)) + (wbLvl1(327) & wbLvl1(327 downto 324));
    wbLvl2(209 downto 205) := (wbLvl1(331) & wbLvl1(331 downto 328)) + (wbLvl1(335) & wbLvl1(335 downto 332));
    wbLvl2(214 downto 210) := (wbLvl1(339) & wbLvl1(339 downto 336)) + (wbLvl1(343) & wbLvl1(343 downto 340));
    wbLvl2(219 downto 215) := (wbLvl1(347) & wbLvl1(347 downto 344)) + (wbLvl1(351) & wbLvl1(351 downto 348));
    wbLvl2(224 downto 220) := (wbLvl1(355) & wbLvl1(355 downto 352)) + (wbLvl1(359) & wbLvl1(359 downto 356));
    wbLvl2(229 downto 225) := (wbLvl1(363) & wbLvl1(363 downto 360)) + (wbLvl1(367) & wbLvl1(367 downto 364));
    wbLvl2(234 downto 230) := (wbLvl1(371) & wbLvl1(371 downto 368)) + (wbLvl1(375) & wbLvl1(375 downto 372));
    wbLvl2(239 downto 235) := (wbLvl1(379) & wbLvl1(379 downto 376)) + (wbLvl1(383) & wbLvl1(383 downto 380));
    wbLvl2(244 downto 240) := (wbLvl1(387) & wbLvl1(387 downto 384)) + (wbLvl1(391) & wbLvl1(391 downto 388));
    wbLvl2(249 downto 245) := (wbLvl1(395) & wbLvl1(395 downto 392)) + (wbLvl1(399) & wbLvl1(399 downto 396));
    wbLvl2(254 downto 250) := (wbLvl1(403) & wbLvl1(403 downto 400)) + (wbLvl1(407) & wbLvl1(407 downto 404));
    wbLvl2(259 downto 255) := (wbLvl1(411) & wbLvl1(411 downto 408)) + (wbLvl1(415) & wbLvl1(415 downto 412));
    wbLvl2(264 downto 260) := (wbLvl1(419) & wbLvl1(419 downto 416)) + (wbLvl1(423) & wbLvl1(423 downto 420));
    wbLvl2(269 downto 265) := (wbLvl1(427) & wbLvl1(427 downto 424)) + (wbLvl1(431) & wbLvl1(431 downto 428));
    wbLvl2(274 downto 270) := (wbLvl1(435) & wbLvl1(435 downto 432)) + (wbLvl1(439) & wbLvl1(439 downto 436));
    wbLvl2(279 downto 275) := (wbLvl1(443) & wbLvl1(443 downto 440)) + (wbLvl1(447) & wbLvl1(447 downto 444));
    wbLvl2(284 downto 280) := (wbLvl1(451) & wbLvl1(451 downto 448)) + (wbLvl1(455) & wbLvl1(455 downto 452));
    wbLvl2(289 downto 285) := (wbLvl1(459) & wbLvl1(459 downto 456)) + (wbLvl1(463) & wbLvl1(463 downto 460));
    wbLvl2(294 downto 290) := (wbLvl1(467) & wbLvl1(467 downto 464)) + (wbLvl1(471) & wbLvl1(471 downto 468));
    wbLvl2(299 downto 295) := (wbLvl1(475) & wbLvl1(475 downto 472)) + (wbLvl1(479) & wbLvl1(479 downto 476));
    wbLvl2(304 downto 300) := (wbLvl1(483) & wbLvl1(483 downto 480)) + (wbLvl1(487) & wbLvl1(487 downto 484));
    wbLvl2(309 downto 305) := (wbLvl1(491) & wbLvl1(491 downto 488)) + (wbLvl1(495) & wbLvl1(495 downto 492));
    wbLvl2(314 downto 310) := (wbLvl1(499) & wbLvl1(499 downto 496)) + (wbLvl1(503) & wbLvl1(503 downto 500));
    wbLvl2(319 downto 315) := (wbLvl1(507) & wbLvl1(507 downto 504)) + (wbLvl1(511) & wbLvl1(511 downto 508));
    wbLvl2(324 downto 320) := (wbLvl1(515) & wbLvl1(515 downto 512)) + (wbLvl1(519) & wbLvl1(519 downto 516));
    wbLvl2(329 downto 325) := (wbLvl1(523) & wbLvl1(523 downto 520)) + (wbLvl1(527) & wbLvl1(527 downto 524));
    wbLvl2(334 downto 330) := (wbLvl1(531) & wbLvl1(531 downto 528)) + (wbLvl1(535) & wbLvl1(535 downto 532));
    wbLvl2(339 downto 335) := (wbLvl1(539) & wbLvl1(539 downto 536)) + (wbLvl1(543) & wbLvl1(543 downto 540));
    wbLvl2(344 downto 340) := (wbLvl1(547) & wbLvl1(547 downto 544)) + (wbLvl1(551) & wbLvl1(551 downto 548));
    wbLvl2(349 downto 345) := (wbLvl1(555) & wbLvl1(555 downto 552)) + (wbLvl1(559) & wbLvl1(559 downto 556));
    wbLvl2(354 downto 350) := (wbLvl1(563) & wbLvl1(563 downto 560)) + (wbLvl1(567) & wbLvl1(567 downto 564));
    wbLvl2(359 downto 355) := (wbLvl1(571) & wbLvl1(571 downto 568)) + (wbLvl1(575) & wbLvl1(575 downto 572));
    wbLvl2(364 downto 360) := (wbLvl1(579) & wbLvl1(579 downto 576)) + (wbLvl1(583) & wbLvl1(583 downto 580));
    wbLvl2(369 downto 365) := (wbLvl1(587) & wbLvl1(587 downto 584)) + (wbLvl1(591) & wbLvl1(591 downto 588));
    wbLvl2(374 downto 370) := (wbLvl1(595) & wbLvl1(595 downto 592)) + (wbLvl1(599) & wbLvl1(599 downto 596));
    wbLvl2(379 downto 375) := (wbLvl1(603) & wbLvl1(603 downto 600)) + (wbLvl1(607) & wbLvl1(607 downto 604));
    wbLvl2(384 downto 380) := (wbLvl1(611) & wbLvl1(611 downto 608)) + (wbLvl1(615) & wbLvl1(615 downto 612));
    wbLvl2(389 downto 385) := (wbLvl1(619) & wbLvl1(619 downto 616)) + (wbLvl1(623) & wbLvl1(623 downto 620));
    wbLvl2(394 downto 390) := (wbLvl1(627) & wbLvl1(627 downto 624)) + (wbLvl1(631) & wbLvl1(631 downto 628));
    wbLvl2(399 downto 395) := (wbLvl1(635) & wbLvl1(635 downto 632)) + (wbLvl1(639) & wbLvl1(639 downto 636));
    wbLvl2(404 downto 400) := (wbLvl1(643) & wbLvl1(643 downto 640)) + (wbLvl1(647) & wbLvl1(647 downto 644));
    wbLvl2(409 downto 405) := (wbLvl1(651) & wbLvl1(651 downto 648)) + (wbLvl1(655) & wbLvl1(655 downto 652));
    wbLvl2(414 downto 410) := (wbLvl1(659) & wbLvl1(659 downto 656)) + (wbLvl1(663) & wbLvl1(663 downto 660));
    wbLvl2(419 downto 415) := (wbLvl1(667) & wbLvl1(667 downto 664)) + (wbLvl1(671) & wbLvl1(671 downto 668));
    wbLvl2(424 downto 420) := (wbLvl1(675) & wbLvl1(675 downto 672)) + (wbLvl1(679) & wbLvl1(679 downto 676));
    wbLvl2(429 downto 425) := (wbLvl1(683) & wbLvl1(683 downto 680)) + (wbLvl1(687) & wbLvl1(687 downto 684));
    wbLvl2(434 downto 430) := (wbLvl1(691) & wbLvl1(691 downto 688)) + (wbLvl1(695) & wbLvl1(695 downto 692));
    wbLvl2(439 downto 435) := (wbLvl1(699) & wbLvl1(699 downto 696)) + (wbLvl1(703) & wbLvl1(703 downto 700));
    wbLvl2(444 downto 440) := (wbLvl1(707) & wbLvl1(707 downto 704)) + (wbLvl1(711) & wbLvl1(711 downto 708));
    wbLvl2(449 downto 445) := (wbLvl1(715) & wbLvl1(715 downto 712)) + (wbLvl1(719) & wbLvl1(719 downto 716));
    wbLvl2(454 downto 450) := (wbLvl1(723) & wbLvl1(723 downto 720)) + (wbLvl1(727) & wbLvl1(727 downto 724));
    wbLvl2(459 downto 455) := (wbLvl1(731) & wbLvl1(731 downto 728)) + (wbLvl1(735) & wbLvl1(735 downto 732));
    wbLvl2(464 downto 460) := (wbLvl1(739) & wbLvl1(739 downto 736)) + (wbLvl1(743) & wbLvl1(743 downto 740));
    wbLvl2(469 downto 465) := (wbLvl1(747) & wbLvl1(747 downto 744)) + (wbLvl1(751) & wbLvl1(751 downto 748));
    wbLvl2(474 downto 470) := (wbLvl1(755) & wbLvl1(755 downto 752)) + (wbLvl1(759) & wbLvl1(759 downto 756));
    wbLvl2(479 downto 475) := (wbLvl1(763) & wbLvl1(763 downto 760)) + (wbLvl1(767) & wbLvl1(767 downto 764));
    wbLvl2(484 downto 480) := (wbLvl1(771) & wbLvl1(771 downto 768)) + (wbLvl1(775) & wbLvl1(775 downto 772));
    wbLvl2(489 downto 485) := (wbLvl1(779) & wbLvl1(779 downto 776)) + (wbLvl1(783) & wbLvl1(783 downto 780));
    wbLvl2(494 downto 490) := (wbLvl1(787) & wbLvl1(787 downto 784)) + (wbLvl1(791) & wbLvl1(791 downto 788));
    wbLvl2(499 downto 495) := (wbLvl1(795) & wbLvl1(795 downto 792)) + (wbLvl1(799) & wbLvl1(799 downto 796));
    wbLvl2(504 downto 500) := (wbLvl1(803) & wbLvl1(803 downto 800)) + (wbLvl1(807) & wbLvl1(807 downto 804));
    wbLvl2(509 downto 505) := (wbLvl1(811) & wbLvl1(811 downto 808)) + (wbLvl1(815) & wbLvl1(815 downto 812));
    wbLvl2(514 downto 510) := (wbLvl1(819) & wbLvl1(819 downto 816)) + (wbLvl1(823) & wbLvl1(823 downto 820));
    wbLvl2(519 downto 515) := (wbLvl1(827) & wbLvl1(827 downto 824)) + (wbLvl1(831) & wbLvl1(831 downto 828));
    wbLvl2(524 downto 520) := (wbLvl1(835) & wbLvl1(835 downto 832)) + (wbLvl1(839) & wbLvl1(839 downto 836));
    wbLvl2(529 downto 525) := (wbLvl1(843) & wbLvl1(843 downto 840)) + (wbLvl1(847) & wbLvl1(847 downto 844));
    wbLvl2(534 downto 530) := (wbLvl1(851) & wbLvl1(851 downto 848)) + (wbLvl1(855) & wbLvl1(855 downto 852));
    wbLvl2(539 downto 535) := (wbLvl1(859) & wbLvl1(859 downto 856)) + (wbLvl1(863) & wbLvl1(863 downto 860));
    wbLvl2(544 downto 540) := (wbLvl1(867) & wbLvl1(867 downto 864)) + (wbLvl1(871) & wbLvl1(871 downto 868));
    wbLvl2(549 downto 545) := (wbLvl1(875) & wbLvl1(875 downto 872)) + (wbLvl1(879) & wbLvl1(879 downto 876));
    wbLvl2(554 downto 550) := (wbLvl1(883) & wbLvl1(883 downto 880)) + (wbLvl1(887) & wbLvl1(887 downto 884));
    wbLvl2(559 downto 555) := (wbLvl1(891) & wbLvl1(891 downto 888)) + (wbLvl1(895) & wbLvl1(895 downto 892));
    wbLvl2(564 downto 560) := (wbLvl1(899) & wbLvl1(899 downto 896)) + (wbLvl1(903) & wbLvl1(903 downto 900));
    wbLvl2(569 downto 565) := (wbLvl1(907) & wbLvl1(907 downto 904)) + (wbLvl1(911) & wbLvl1(911 downto 908));
    wbLvl2(574 downto 570) := (wbLvl1(915) & wbLvl1(915 downto 912)) + (wbLvl1(919) & wbLvl1(919 downto 916));
    wbLvl2(579 downto 575) := (wbLvl1(923) & wbLvl1(923 downto 920)) + (wbLvl1(927) & wbLvl1(927 downto 924));
    wbLvl2(584 downto 580) := (wbLvl1(931) & wbLvl1(931 downto 928)) + (wbLvl1(935) & wbLvl1(935 downto 932));
    wbLvl2(589 downto 585) := (wbLvl1(939) & wbLvl1(939 downto 936)) + (wbLvl1(943) & wbLvl1(943 downto 940));
    wbLvl2(594 downto 590) := (wbLvl1(947) & wbLvl1(947 downto 944)) + (wbLvl1(951) & wbLvl1(951 downto 948));
    wbLvl2(599 downto 595) := (wbLvl1(955) & wbLvl1(955 downto 952)) + (wbLvl1(959) & wbLvl1(959 downto 956));
    wbLvl2(604 downto 600) := (wbLvl1(963) & wbLvl1(963 downto 960)) + (wbLvl1(967) & wbLvl1(967 downto 964));
    wbLvl2(609 downto 605) := (wbLvl1(971) & wbLvl1(971 downto 968)) + (wbLvl1(975) & wbLvl1(975 downto 972));
    wbLvl2(614 downto 610) := (wbLvl1(979) & wbLvl1(979 downto 976)) + (wbLvl1(983) & wbLvl1(983 downto 980));
    wbLvl2(619 downto 615) := (wbLvl1(987) & wbLvl1(987 downto 984)) + (wbLvl1(991) & wbLvl1(991 downto 988));
    wbLvl2(624 downto 620) := (wbLvl1(995) & wbLvl1(995 downto 992)) + (wbLvl1(999) & wbLvl1(999 downto 996));
    wbLvl2(629 downto 625) := (wbLvl1(1003) & wbLvl1(1003 downto 1000)) + (wbLvl1(1007) & wbLvl1(1007 downto 1004));
    wbLvl2(634 downto 630) := (wbLvl1(1011) & wbLvl1(1011 downto 1008)) + (wbLvl1(1015) & wbLvl1(1015 downto 1012));
    wbLvl2(639 downto 635) := (wbLvl1(1019) & wbLvl1(1019 downto 1016)) + (wbLvl1(1023) & wbLvl1(1023 downto 1020));
    wbLvl2(644 downto 640) := (wbLvl1(1027) & wbLvl1(1027 downto 1024)) + (wbLvl1(1031) & wbLvl1(1031 downto 1028));
    wbLvl2(649 downto 645) := (wbLvl1(1035) & wbLvl1(1035 downto 1032)) + (wbLvl1(1039) & wbLvl1(1039 downto 1036));
    wbLvl2(654 downto 650) := (wbLvl1(1043) & wbLvl1(1043 downto 1040)) + (wbLvl1(1047) & wbLvl1(1047 downto 1044));
    wbLvl2(659 downto 655) := (wbLvl1(1051) & wbLvl1(1051 downto 1048)) + (wbLvl1(1055) & wbLvl1(1055 downto 1052));
    wbLvl2(664 downto 660) := (wbLvl1(1059) & wbLvl1(1059 downto 1056)) + (wbLvl1(1063) & wbLvl1(1063 downto 1060));
    wbLvl2(669 downto 665) := (wbLvl1(1067) & wbLvl1(1067 downto 1064)) + (wbLvl1(1071) & wbLvl1(1071 downto 1068));
    wbLvl2(674 downto 670) := (wbLvl1(1075) & wbLvl1(1075 downto 1072)) + (wbLvl1(1079) & wbLvl1(1079 downto 1076));
    wbLvl2(679 downto 675) := (wbLvl1(1083) & wbLvl1(1083 downto 1080)) + (wbLvl1(1087) & wbLvl1(1087 downto 1084));
    wbLvl2(684 downto 680) := (wbLvl1(1091) & wbLvl1(1091 downto 1088)) + (wbLvl1(1095) & wbLvl1(1095 downto 1092));
    wbLvl2(689 downto 685) := (wbLvl1(1099) & wbLvl1(1099 downto 1096)) + (wbLvl1(1103) & wbLvl1(1103 downto 1100));
    wbLvl2(694 downto 690) := (wbLvl1(1107) & wbLvl1(1107 downto 1104)) + (wbLvl1(1111) & wbLvl1(1111 downto 1108));
    wbLvl2(699 downto 695) := (wbLvl1(1115) & wbLvl1(1115 downto 1112)) + (wbLvl1(1119) & wbLvl1(1119 downto 1116));
    wbLvl2(704 downto 700) := (wbLvl1(1123) & wbLvl1(1123 downto 1120)) + (wbLvl1(1127) & wbLvl1(1127 downto 1124));
    wbLvl2(709 downto 705) := (wbLvl1(1131) & wbLvl1(1131 downto 1128)) + (wbLvl1(1135) & wbLvl1(1135 downto 1132));
    wbLvl2(714 downto 710) := (wbLvl1(1139) & wbLvl1(1139 downto 1136)) + (wbLvl1(1143) & wbLvl1(1143 downto 1140));
    wbLvl2(719 downto 715) := (wbLvl1(1147) & wbLvl1(1147 downto 1144)) + (wbLvl1(1151) & wbLvl1(1151 downto 1148));
    wbLvl2(724 downto 720) := (wbLvl1(1155) & wbLvl1(1155 downto 1152)) + (wbLvl1(1159) & wbLvl1(1159 downto 1156));
    wbLvl2(729 downto 725) := (wbLvl1(1163) & wbLvl1(1163 downto 1160)) + (wbLvl1(1167) & wbLvl1(1167 downto 1164));
    wbLvl2(734 downto 730) := (wbLvl1(1171) & wbLvl1(1171 downto 1168)) + (wbLvl1(1175) & wbLvl1(1175 downto 1172));
    wbLvl2(739 downto 735) := (wbLvl1(1179) & wbLvl1(1179 downto 1176)) + (wbLvl1(1183) & wbLvl1(1183 downto 1180));
    wbLvl2(744 downto 740) := (wbLvl1(1187) & wbLvl1(1187 downto 1184)) + (wbLvl1(1191) & wbLvl1(1191 downto 1188));
    wbLvl2(749 downto 745) := (wbLvl1(1195) & wbLvl1(1195 downto 1192)) + (wbLvl1(1199) & wbLvl1(1199 downto 1196));
    wbLvl2(754 downto 750) := (wbLvl1(1203) & wbLvl1(1203 downto 1200)) + (wbLvl1(1207) & wbLvl1(1207 downto 1204));
    wbLvl2(759 downto 755) := (wbLvl1(1211) & wbLvl1(1211 downto 1208)) + (wbLvl1(1215) & wbLvl1(1215 downto 1212));
    wbLvl2(764 downto 760) := (wbLvl1(1219) & wbLvl1(1219 downto 1216)) + (wbLvl1(1223) & wbLvl1(1223 downto 1220));
    wbLvl2(769 downto 765) := (wbLvl1(1227) & wbLvl1(1227 downto 1224)) + (wbLvl1(1231) & wbLvl1(1231 downto 1228));
    wbLvl2(774 downto 770) := (wbLvl1(1235) & wbLvl1(1235 downto 1232)) + (wbLvl1(1239) & wbLvl1(1239 downto 1236));
    wbLvl2(779 downto 775) := (wbLvl1(1243) & wbLvl1(1243 downto 1240)) + (wbLvl1(1247) & wbLvl1(1247 downto 1244));
    wbLvl2(784 downto 780) := (wbLvl1(1251) & wbLvl1(1251 downto 1248)) + (wbLvl1(1255) & wbLvl1(1255 downto 1252));
    wbLvl2(789 downto 785) := (wbLvl1(1259) & wbLvl1(1259 downto 1256)) + (wbLvl1(1263) & wbLvl1(1263 downto 1260));
    wbLvl2(794 downto 790) := (wbLvl1(1267) & wbLvl1(1267 downto 1264)) + (wbLvl1(1271) & wbLvl1(1271 downto 1268));
    wbLvl2(799 downto 795) := (wbLvl1(1275) & wbLvl1(1275 downto 1272)) + (wbLvl1(1279) & wbLvl1(1279 downto 1276));
    wbLvl2(804 downto 800) := (wbLvl1(1283) & wbLvl1(1283 downto 1280)) + (wbLvl1(1287) & wbLvl1(1287 downto 1284));
    wbLvl2(809 downto 805) := (wbLvl1(1291) & wbLvl1(1291 downto 1288)) + (wbLvl1(1295) & wbLvl1(1295 downto 1292));
    wbLvl2(814 downto 810) := (wbLvl1(1299) & wbLvl1(1299 downto 1296)) + (wbLvl1(1303) & wbLvl1(1303 downto 1300));
    wbLvl2(819 downto 815) := (wbLvl1(1307) & wbLvl1(1307 downto 1304)) + (wbLvl1(1311) & wbLvl1(1311 downto 1308));
    wbLvl2(824 downto 820) := (wbLvl1(1315) & wbLvl1(1315 downto 1312)) + (wbLvl1(1319) & wbLvl1(1319 downto 1316));
    wbLvl2(829 downto 825) := (wbLvl1(1323) & wbLvl1(1323 downto 1320)) + (wbLvl1(1327) & wbLvl1(1327 downto 1324));
    wbLvl2(834 downto 830) := (wbLvl1(1331) & wbLvl1(1331 downto 1328)) + (wbLvl1(1335) & wbLvl1(1335 downto 1332));
    wbLvl2(839 downto 835) := (wbLvl1(1339) & wbLvl1(1339 downto 1336)) + (wbLvl1(1343) & wbLvl1(1343 downto 1340));
    wbLvl2(844 downto 840) := (wbLvl1(1347) & wbLvl1(1347 downto 1344)) + (wbLvl1(1351) & wbLvl1(1351 downto 1348));
    wbLvl2(849 downto 845) := (wbLvl1(1355) & wbLvl1(1355 downto 1352)) + (wbLvl1(1359) & wbLvl1(1359 downto 1356));
    wbLvl2(854 downto 850) := (wbLvl1(1363) & wbLvl1(1363 downto 1360)) + (wbLvl1(1367) & wbLvl1(1367 downto 1364));
    wbLvl2(859 downto 855) := (wbLvl1(1371) & wbLvl1(1371 downto 1368)) + (wbLvl1(1375) & wbLvl1(1375 downto 1372));
    wbLvl2(864 downto 860) := (wbLvl1(1379) & wbLvl1(1379 downto 1376)) + (wbLvl1(1383) & wbLvl1(1383 downto 1380));
    wbLvl2(869 downto 865) := (wbLvl1(1387) & wbLvl1(1387 downto 1384)) + (wbLvl1(1391) & wbLvl1(1391 downto 1388));
    wbLvl2(874 downto 870) := (wbLvl1(1395) & wbLvl1(1395 downto 1392)) + (wbLvl1(1399) & wbLvl1(1399 downto 1396));
    wbLvl2(879 downto 875) := (wbLvl1(1403) & wbLvl1(1403 downto 1400)) + (wbLvl1(1407) & wbLvl1(1407 downto 1404));
    wbLvl2(884 downto 880) := (wbLvl1(1411) & wbLvl1(1411 downto 1408)) + (wbLvl1(1415) & wbLvl1(1415 downto 1412));
    wbLvl2(889 downto 885) := (wbLvl1(1419) & wbLvl1(1419 downto 1416)) + (wbLvl1(1423) & wbLvl1(1423 downto 1420));
    wbLvl2(894 downto 890) := (wbLvl1(1427) & wbLvl1(1427 downto 1424)) + (wbLvl1(1431) & wbLvl1(1431 downto 1428));
    wbLvl2(899 downto 895) := (wbLvl1(1435) & wbLvl1(1435 downto 1432)) + (wbLvl1(1439) & wbLvl1(1439 downto 1436));
    wbLvl2(904 downto 900) := (wbLvl1(1443) & wbLvl1(1443 downto 1440)) + (wbLvl1(1447) & wbLvl1(1447 downto 1444));
    wbLvl2(909 downto 905) := (wbLvl1(1451) & wbLvl1(1451 downto 1448)) + (wbLvl1(1455) & wbLvl1(1455 downto 1452));
    wbLvl2(914 downto 910) := (wbLvl1(1459) & wbLvl1(1459 downto 1456)) + (wbLvl1(1463) & wbLvl1(1463 downto 1460));
    wbLvl2(919 downto 915) := (wbLvl1(1467) & wbLvl1(1467 downto 1464)) + (wbLvl1(1471) & wbLvl1(1471 downto 1468));
    wbLvl2(924 downto 920) := (wbLvl1(1475) & wbLvl1(1475 downto 1472)) + (wbLvl1(1479) & wbLvl1(1479 downto 1476));
    wbLvl2(929 downto 925) := (wbLvl1(1483) & wbLvl1(1483 downto 1480)) + (wbLvl1(1487) & wbLvl1(1487 downto 1484));
    wbLvl2(934 downto 930) := (wbLvl1(1491) & wbLvl1(1491 downto 1488)) + (wbLvl1(1495) & wbLvl1(1495 downto 1492));
    wbLvl2(939 downto 935) := (wbLvl1(1499) & wbLvl1(1499 downto 1496)) + (wbLvl1(1503) & wbLvl1(1503 downto 1500));
    wbLvl2(944 downto 940) := (wbLvl1(1507) & wbLvl1(1507 downto 1504)) + (wbLvl1(1511) & wbLvl1(1511 downto 1508));
    wbLvl2(949 downto 945) := (wbLvl1(1515) & wbLvl1(1515 downto 1512)) + (wbLvl1(1519) & wbLvl1(1519 downto 1516));
    wbLvl2(954 downto 950) := (wbLvl1(1523) & wbLvl1(1523 downto 1520)) + (wbLvl1(1527) & wbLvl1(1527 downto 1524));
    wbLvl2(959 downto 955) := (wbLvl1(1531) & wbLvl1(1531 downto 1528)) + (wbLvl1(1535) & wbLvl1(1535 downto 1532));
    wbLvl2(964 downto 960) := (wbLvl1(1539) & wbLvl1(1539 downto 1536)) + (wbLvl1(1543) & wbLvl1(1543 downto 1540));
    wbLvl2(969 downto 965) := (wbLvl1(1547) & wbLvl1(1547 downto 1544)) + (wbLvl1(1551) & wbLvl1(1551 downto 1548));
    wbLvl2(974 downto 970) := (wbLvl1(1555) & wbLvl1(1555 downto 1552)) + (wbLvl1(1559) & wbLvl1(1559 downto 1556));
    wbLvl2(979 downto 975) := (wbLvl1(1563) & wbLvl1(1563 downto 1560)) + (wbLvl1(1567) & wbLvl1(1567 downto 1564));
    wbLvl2(984 downto 980) := (wbLvl1(1571) & wbLvl1(1571 downto 1568)) + (wbLvl1(1575) & wbLvl1(1575 downto 1572));
    wbLvl2(989 downto 985) := (wbLvl1(1579) & wbLvl1(1579 downto 1576)) + (wbLvl1(1583) & wbLvl1(1583 downto 1580));
    wbLvl2(994 downto 990) := (wbLvl1(1587) & wbLvl1(1587 downto 1584)) + (wbLvl1(1591) & wbLvl1(1591 downto 1588));
    wbLvl2(999 downto 995) := (wbLvl1(1595) & wbLvl1(1595 downto 1592)) + (wbLvl1(1599) & wbLvl1(1599 downto 1596));
    wbLvl2(1004 downto 1000) := (wbLvl1(1603) & wbLvl1(1603 downto 1600)) + (wbLvl1(1607) & wbLvl1(1607 downto 1604));
    wbLvl2(1009 downto 1005) := (wbLvl1(1611) & wbLvl1(1611 downto 1608)) + (wbLvl1(1615) & wbLvl1(1615 downto 1612));
    wbLvl2(1014 downto 1010) := (wbLvl1(1619) & wbLvl1(1619 downto 1616)) + (wbLvl1(1623) & wbLvl1(1623 downto 1620));
    wbLvl2(1019 downto 1015) := (wbLvl1(1627) & wbLvl1(1627 downto 1624)) + (wbLvl1(1631) & wbLvl1(1631 downto 1628));
    wbLvl2(1024 downto 1020) := (wbLvl1(1635) & wbLvl1(1635 downto 1632)) + (wbLvl1(1639) & wbLvl1(1639 downto 1636));
    wbLvl2(1029 downto 1025) := (wbLvl1(1643) & wbLvl1(1643 downto 1640)) + (wbLvl1(1647) & wbLvl1(1647 downto 1644));
    wbLvl2(1034 downto 1030) := (wbLvl1(1651) & wbLvl1(1651 downto 1648)) + (wbLvl1(1655) & wbLvl1(1655 downto 1652));
    wbLvl2(1039 downto 1035) := (wbLvl1(1659) & wbLvl1(1659 downto 1656)) + (wbLvl1(1663) & wbLvl1(1663 downto 1660));
    wbLvl2(1044 downto 1040) := (wbLvl1(1667) & wbLvl1(1667 downto 1664)) + (wbLvl1(1671) & wbLvl1(1671 downto 1668));
    wbLvl2(1049 downto 1045) := (wbLvl1(1675) & wbLvl1(1675 downto 1672)) + (wbLvl1(1679) & wbLvl1(1679 downto 1676));
    wbLvl2(1054 downto 1050) := (wbLvl1(1683) & wbLvl1(1683 downto 1680)) + (wbLvl1(1687) & wbLvl1(1687 downto 1684));
    wbLvl2(1059 downto 1055) := (wbLvl1(1691) & wbLvl1(1691 downto 1688)) + (wbLvl1(1695) & wbLvl1(1695 downto 1692));
    wbLvl2(1064 downto 1060) := (wbLvl1(1699) & wbLvl1(1699 downto 1696)) + (wbLvl1(1703) & wbLvl1(1703 downto 1700));
    wbLvl2(1069 downto 1065) := (wbLvl1(1707) & wbLvl1(1707 downto 1704)) + (wbLvl1(1711) & wbLvl1(1711 downto 1708));
    wbLvl2(1074 downto 1070) := (wbLvl1(1715) & wbLvl1(1715 downto 1712)) + (wbLvl1(1719) & wbLvl1(1719 downto 1716));
    wbLvl2(1079 downto 1075) := (wbLvl1(1723) & wbLvl1(1723 downto 1720)) + (wbLvl1(1727) & wbLvl1(1727 downto 1724));
    wbLvl2(1084 downto 1080) := (wbLvl1(1731) & wbLvl1(1731 downto 1728)) + (wbLvl1(1735) & wbLvl1(1735 downto 1732));
    wbLvl2(1089 downto 1085) := (wbLvl1(1739) & wbLvl1(1739 downto 1736)) + (wbLvl1(1743) & wbLvl1(1743 downto 1740));
    wbLvl2(1094 downto 1090) := (wbLvl1(1747) & wbLvl1(1747 downto 1744)) + (wbLvl1(1751) & wbLvl1(1751 downto 1748));
    wbLvl2(1099 downto 1095) := (wbLvl1(1755) & wbLvl1(1755 downto 1752)) + (wbLvl1(1759) & wbLvl1(1759 downto 1756));
    wbLvl2(1104 downto 1100) := (wbLvl1(1763) & wbLvl1(1763 downto 1760)) + (wbLvl1(1767) & wbLvl1(1767 downto 1764));
    wbLvl2(1109 downto 1105) := (wbLvl1(1771) & wbLvl1(1771 downto 1768)) + (wbLvl1(1775) & wbLvl1(1775 downto 1772));
    wbLvl2(1114 downto 1110) := (wbLvl1(1779) & wbLvl1(1779 downto 1776)) + (wbLvl1(1783) & wbLvl1(1783 downto 1780));
    wbLvl2(1119 downto 1115) := (wbLvl1(1787) & wbLvl1(1787 downto 1784)) + (wbLvl1(1791) & wbLvl1(1791 downto 1788));
    wbLvl2(1124 downto 1120) := (wbLvl1(1795) & wbLvl1(1795 downto 1792)) + (wbLvl1(1799) & wbLvl1(1799 downto 1796));
    wbLvl2(1129 downto 1125) := (wbLvl1(1803) & wbLvl1(1803 downto 1800)) + (wbLvl1(1807) & wbLvl1(1807 downto 1804));
    wbLvl2(1134 downto 1130) := (wbLvl1(1811) & wbLvl1(1811 downto 1808)) + (wbLvl1(1815) & wbLvl1(1815 downto 1812));
    wbLvl2(1139 downto 1135) := (wbLvl1(1819) & wbLvl1(1819 downto 1816)) + (wbLvl1(1823) & wbLvl1(1823 downto 1820));
    wbLvl2(1144 downto 1140) := (wbLvl1(1827) & wbLvl1(1827 downto 1824)) + (wbLvl1(1831) & wbLvl1(1831 downto 1828));
    wbLvl2(1149 downto 1145) := (wbLvl1(1835) & wbLvl1(1835 downto 1832)) + (wbLvl1(1839) & wbLvl1(1839 downto 1836));
    wbLvl2(1154 downto 1150) := (wbLvl1(1843) & wbLvl1(1843 downto 1840)) + (wbLvl1(1847) & wbLvl1(1847 downto 1844));
    wbLvl2(1159 downto 1155) := (wbLvl1(1851) & wbLvl1(1851 downto 1848)) + (wbLvl1(1855) & wbLvl1(1855 downto 1852));
    wbLvl2(1164 downto 1160) := (wbLvl1(1859) & wbLvl1(1859 downto 1856)) + (wbLvl1(1863) & wbLvl1(1863 downto 1860));
    wbLvl2(1169 downto 1165) := (wbLvl1(1867) & wbLvl1(1867 downto 1864)) + (wbLvl1(1871) & wbLvl1(1871 downto 1868));
    wbLvl2(1174 downto 1170) := (wbLvl1(1875) & wbLvl1(1875 downto 1872)) + (wbLvl1(1879) & wbLvl1(1879 downto 1876));
    wbLvl2(1179 downto 1175) := (wbLvl1(1883) & wbLvl1(1883 downto 1880)) + (wbLvl1(1887) & wbLvl1(1887 downto 1884));
    wbLvl2(1184 downto 1180) := (wbLvl1(1891) & wbLvl1(1891 downto 1888)) + (wbLvl1(1895) & wbLvl1(1895 downto 1892));
    wbLvl2(1189 downto 1185) := (wbLvl1(1899) & wbLvl1(1899 downto 1896)) + (wbLvl1(1903) & wbLvl1(1903 downto 1900));
    wbLvl2(1194 downto 1190) := (wbLvl1(1907) & wbLvl1(1907 downto 1904)) + (wbLvl1(1911) & wbLvl1(1911 downto 1908));
    wbLvl2(1199 downto 1195) := (wbLvl1(1915) & wbLvl1(1915 downto 1912)) + (wbLvl1(1919) & wbLvl1(1919 downto 1916));
    wbLvl2(1204 downto 1200) := (wbLvl1(1923) & wbLvl1(1923 downto 1920)) + (wbLvl1(1927) & wbLvl1(1927 downto 1924));
    wbLvl2(1209 downto 1205) := (wbLvl1(1931) & wbLvl1(1931 downto 1928)) + (wbLvl1(1935) & wbLvl1(1935 downto 1932));
    wbLvl2(1214 downto 1210) := (wbLvl1(1939) & wbLvl1(1939 downto 1936)) + (wbLvl1(1943) & wbLvl1(1943 downto 1940));
    wbLvl2(1219 downto 1215) := (wbLvl1(1947) & wbLvl1(1947 downto 1944)) + (wbLvl1(1951) & wbLvl1(1951 downto 1948));
    wbLvl2(1224 downto 1220) := (wbLvl1(1955) & wbLvl1(1955 downto 1952)) + (wbLvl1(1959) & wbLvl1(1959 downto 1956));
    wbLvl2(1229 downto 1225) := (wbLvl1(1963) & wbLvl1(1963 downto 1960)) + (wbLvl1(1967) & wbLvl1(1967 downto 1964));
    wbLvl2(1234 downto 1230) := (wbLvl1(1971) & wbLvl1(1971 downto 1968)) + (wbLvl1(1975) & wbLvl1(1975 downto 1972));
    wbLvl2(1239 downto 1235) := (wbLvl1(1979) & wbLvl1(1979 downto 1976)) + (wbLvl1(1983) & wbLvl1(1983 downto 1980));
    wbLvl2(1244 downto 1240) := (wbLvl1(1987) & wbLvl1(1987 downto 1984)) + (wbLvl1(1991) & wbLvl1(1991 downto 1988));
    wbLvl2(1249 downto 1245) := (wbLvl1(1995) & wbLvl1(1995 downto 1992)) + (wbLvl1(1999) & wbLvl1(1999 downto 1996));
    wbLvl2(1254 downto 1250) := (wbLvl1(2003) & wbLvl1(2003 downto 2000)) + (wbLvl1(2007) & wbLvl1(2007 downto 2004));
    wbLvl2(1259 downto 1255) := (wbLvl1(2011) & wbLvl1(2011 downto 2008)) + (wbLvl1(2015) & wbLvl1(2015 downto 2012));
    wbLvl2(1264 downto 1260) := (wbLvl1(2019) & wbLvl1(2019 downto 2016)) + (wbLvl1(2023) & wbLvl1(2023 downto 2020));
    wbLvl2(1269 downto 1265) := (wbLvl1(2027) & wbLvl1(2027 downto 2024)) + (wbLvl1(2031) & wbLvl1(2031 downto 2028));
    wbLvl2(1274 downto 1270) := (wbLvl1(2035) & wbLvl1(2035 downto 2032)) + (wbLvl1(2039) & wbLvl1(2039 downto 2036));
    wbLvl2(1279 downto 1275) := (wbLvl1(2043) & wbLvl1(2043 downto 2040)) + (wbLvl1(2047) & wbLvl1(2047 downto 2044));


    wbLvl3(5 downto 0) := (wbLvl2(4) & wbLvl2(4 downto 0)) + (wbLvl2(9) & wbLvl2(9 downto 5));
    wbLvl3(11 downto 6) := (wbLvl2(14) & wbLvl2(14 downto 10)) + (wbLvl2(19) & wbLvl2(19 downto 15));
    wbLvl3(17 downto 12) := (wbLvl2(24) & wbLvl2(24 downto 20)) + (wbLvl2(29) & wbLvl2(29 downto 25));
    wbLvl3(23 downto 18) := (wbLvl2(34) & wbLvl2(34 downto 30)) + (wbLvl2(39) & wbLvl2(39 downto 35));
    wbLvl3(29 downto 24) := (wbLvl2(44) & wbLvl2(44 downto 40)) + (wbLvl2(49) & wbLvl2(49 downto 45));
    wbLvl3(35 downto 30) := (wbLvl2(54) & wbLvl2(54 downto 50)) + (wbLvl2(59) & wbLvl2(59 downto 55));
    wbLvl3(41 downto 36) := (wbLvl2(64) & wbLvl2(64 downto 60)) + (wbLvl2(69) & wbLvl2(69 downto 65));
    wbLvl3(47 downto 42) := (wbLvl2(74) & wbLvl2(74 downto 70)) + (wbLvl2(79) & wbLvl2(79 downto 75));
    wbLvl3(53 downto 48) := (wbLvl2(84) & wbLvl2(84 downto 80)) + (wbLvl2(89) & wbLvl2(89 downto 85));
    wbLvl3(59 downto 54) := (wbLvl2(94) & wbLvl2(94 downto 90)) + (wbLvl2(99) & wbLvl2(99 downto 95));
    wbLvl3(65 downto 60) := (wbLvl2(104) & wbLvl2(104 downto 100)) + (wbLvl2(109) & wbLvl2(109 downto 105));
    wbLvl3(71 downto 66) := (wbLvl2(114) & wbLvl2(114 downto 110)) + (wbLvl2(119) & wbLvl2(119 downto 115));
    wbLvl3(77 downto 72) := (wbLvl2(124) & wbLvl2(124 downto 120)) + (wbLvl2(129) & wbLvl2(129 downto 125));
    wbLvl3(83 downto 78) := (wbLvl2(134) & wbLvl2(134 downto 130)) + (wbLvl2(139) & wbLvl2(139 downto 135));
    wbLvl3(89 downto 84) := (wbLvl2(144) & wbLvl2(144 downto 140)) + (wbLvl2(149) & wbLvl2(149 downto 145));
    wbLvl3(95 downto 90) := (wbLvl2(154) & wbLvl2(154 downto 150)) + (wbLvl2(159) & wbLvl2(159 downto 155));
    wbLvl3(101 downto 96) := (wbLvl2(164) & wbLvl2(164 downto 160)) + (wbLvl2(169) & wbLvl2(169 downto 165));
    wbLvl3(107 downto 102) := (wbLvl2(174) & wbLvl2(174 downto 170)) + (wbLvl2(179) & wbLvl2(179 downto 175));
    wbLvl3(113 downto 108) := (wbLvl2(184) & wbLvl2(184 downto 180)) + (wbLvl2(189) & wbLvl2(189 downto 185));
    wbLvl3(119 downto 114) := (wbLvl2(194) & wbLvl2(194 downto 190)) + (wbLvl2(199) & wbLvl2(199 downto 195));
    wbLvl3(125 downto 120) := (wbLvl2(204) & wbLvl2(204 downto 200)) + (wbLvl2(209) & wbLvl2(209 downto 205));
    wbLvl3(131 downto 126) := (wbLvl2(214) & wbLvl2(214 downto 210)) + (wbLvl2(219) & wbLvl2(219 downto 215));
    wbLvl3(137 downto 132) := (wbLvl2(224) & wbLvl2(224 downto 220)) + (wbLvl2(229) & wbLvl2(229 downto 225));
    wbLvl3(143 downto 138) := (wbLvl2(234) & wbLvl2(234 downto 230)) + (wbLvl2(239) & wbLvl2(239 downto 235));
    wbLvl3(149 downto 144) := (wbLvl2(244) & wbLvl2(244 downto 240)) + (wbLvl2(249) & wbLvl2(249 downto 245));
    wbLvl3(155 downto 150) := (wbLvl2(254) & wbLvl2(254 downto 250)) + (wbLvl2(259) & wbLvl2(259 downto 255));
    wbLvl3(161 downto 156) := (wbLvl2(264) & wbLvl2(264 downto 260)) + (wbLvl2(269) & wbLvl2(269 downto 265));
    wbLvl3(167 downto 162) := (wbLvl2(274) & wbLvl2(274 downto 270)) + (wbLvl2(279) & wbLvl2(279 downto 275));
    wbLvl3(173 downto 168) := (wbLvl2(284) & wbLvl2(284 downto 280)) + (wbLvl2(289) & wbLvl2(289 downto 285));
    wbLvl3(179 downto 174) := (wbLvl2(294) & wbLvl2(294 downto 290)) + (wbLvl2(299) & wbLvl2(299 downto 295));
    wbLvl3(185 downto 180) := (wbLvl2(304) & wbLvl2(304 downto 300)) + (wbLvl2(309) & wbLvl2(309 downto 305));
    wbLvl3(191 downto 186) := (wbLvl2(314) & wbLvl2(314 downto 310)) + (wbLvl2(319) & wbLvl2(319 downto 315));
    wbLvl3(197 downto 192) := (wbLvl2(324) & wbLvl2(324 downto 320)) + (wbLvl2(329) & wbLvl2(329 downto 325));
    wbLvl3(203 downto 198) := (wbLvl2(334) & wbLvl2(334 downto 330)) + (wbLvl2(339) & wbLvl2(339 downto 335));
    wbLvl3(209 downto 204) := (wbLvl2(344) & wbLvl2(344 downto 340)) + (wbLvl2(349) & wbLvl2(349 downto 345));
    wbLvl3(215 downto 210) := (wbLvl2(354) & wbLvl2(354 downto 350)) + (wbLvl2(359) & wbLvl2(359 downto 355));
    wbLvl3(221 downto 216) := (wbLvl2(364) & wbLvl2(364 downto 360)) + (wbLvl2(369) & wbLvl2(369 downto 365));
    wbLvl3(227 downto 222) := (wbLvl2(374) & wbLvl2(374 downto 370)) + (wbLvl2(379) & wbLvl2(379 downto 375));
    wbLvl3(233 downto 228) := (wbLvl2(384) & wbLvl2(384 downto 380)) + (wbLvl2(389) & wbLvl2(389 downto 385));
    wbLvl3(239 downto 234) := (wbLvl2(394) & wbLvl2(394 downto 390)) + (wbLvl2(399) & wbLvl2(399 downto 395));
    wbLvl3(245 downto 240) := (wbLvl2(404) & wbLvl2(404 downto 400)) + (wbLvl2(409) & wbLvl2(409 downto 405));
    wbLvl3(251 downto 246) := (wbLvl2(414) & wbLvl2(414 downto 410)) + (wbLvl2(419) & wbLvl2(419 downto 415));
    wbLvl3(257 downto 252) := (wbLvl2(424) & wbLvl2(424 downto 420)) + (wbLvl2(429) & wbLvl2(429 downto 425));
    wbLvl3(263 downto 258) := (wbLvl2(434) & wbLvl2(434 downto 430)) + (wbLvl2(439) & wbLvl2(439 downto 435));
    wbLvl3(269 downto 264) := (wbLvl2(444) & wbLvl2(444 downto 440)) + (wbLvl2(449) & wbLvl2(449 downto 445));
    wbLvl3(275 downto 270) := (wbLvl2(454) & wbLvl2(454 downto 450)) + (wbLvl2(459) & wbLvl2(459 downto 455));
    wbLvl3(281 downto 276) := (wbLvl2(464) & wbLvl2(464 downto 460)) + (wbLvl2(469) & wbLvl2(469 downto 465));
    wbLvl3(287 downto 282) := (wbLvl2(474) & wbLvl2(474 downto 470)) + (wbLvl2(479) & wbLvl2(479 downto 475));
    wbLvl3(293 downto 288) := (wbLvl2(484) & wbLvl2(484 downto 480)) + (wbLvl2(489) & wbLvl2(489 downto 485));
    wbLvl3(299 downto 294) := (wbLvl2(494) & wbLvl2(494 downto 490)) + (wbLvl2(499) & wbLvl2(499 downto 495));
    wbLvl3(305 downto 300) := (wbLvl2(504) & wbLvl2(504 downto 500)) + (wbLvl2(509) & wbLvl2(509 downto 505));
    wbLvl3(311 downto 306) := (wbLvl2(514) & wbLvl2(514 downto 510)) + (wbLvl2(519) & wbLvl2(519 downto 515));
    wbLvl3(317 downto 312) := (wbLvl2(524) & wbLvl2(524 downto 520)) + (wbLvl2(529) & wbLvl2(529 downto 525));
    wbLvl3(323 downto 318) := (wbLvl2(534) & wbLvl2(534 downto 530)) + (wbLvl2(539) & wbLvl2(539 downto 535));
    wbLvl3(329 downto 324) := (wbLvl2(544) & wbLvl2(544 downto 540)) + (wbLvl2(549) & wbLvl2(549 downto 545));
    wbLvl3(335 downto 330) := (wbLvl2(554) & wbLvl2(554 downto 550)) + (wbLvl2(559) & wbLvl2(559 downto 555));
    wbLvl3(341 downto 336) := (wbLvl2(564) & wbLvl2(564 downto 560)) + (wbLvl2(569) & wbLvl2(569 downto 565));
    wbLvl3(347 downto 342) := (wbLvl2(574) & wbLvl2(574 downto 570)) + (wbLvl2(579) & wbLvl2(579 downto 575));
    wbLvl3(353 downto 348) := (wbLvl2(584) & wbLvl2(584 downto 580)) + (wbLvl2(589) & wbLvl2(589 downto 585));
    wbLvl3(359 downto 354) := (wbLvl2(594) & wbLvl2(594 downto 590)) + (wbLvl2(599) & wbLvl2(599 downto 595));
    wbLvl3(365 downto 360) := (wbLvl2(604) & wbLvl2(604 downto 600)) + (wbLvl2(609) & wbLvl2(609 downto 605));
    wbLvl3(371 downto 366) := (wbLvl2(614) & wbLvl2(614 downto 610)) + (wbLvl2(619) & wbLvl2(619 downto 615));
    wbLvl3(377 downto 372) := (wbLvl2(624) & wbLvl2(624 downto 620)) + (wbLvl2(629) & wbLvl2(629 downto 625));
    wbLvl3(383 downto 378) := (wbLvl2(634) & wbLvl2(634 downto 630)) + (wbLvl2(639) & wbLvl2(639 downto 635));
    wbLvl3(389 downto 384) := (wbLvl2(644) & wbLvl2(644 downto 640)) + (wbLvl2(649) & wbLvl2(649 downto 645));
    wbLvl3(395 downto 390) := (wbLvl2(654) & wbLvl2(654 downto 650)) + (wbLvl2(659) & wbLvl2(659 downto 655));
    wbLvl3(401 downto 396) := (wbLvl2(664) & wbLvl2(664 downto 660)) + (wbLvl2(669) & wbLvl2(669 downto 665));
    wbLvl3(407 downto 402) := (wbLvl2(674) & wbLvl2(674 downto 670)) + (wbLvl2(679) & wbLvl2(679 downto 675));
    wbLvl3(413 downto 408) := (wbLvl2(684) & wbLvl2(684 downto 680)) + (wbLvl2(689) & wbLvl2(689 downto 685));
    wbLvl3(419 downto 414) := (wbLvl2(694) & wbLvl2(694 downto 690)) + (wbLvl2(699) & wbLvl2(699 downto 695));
    wbLvl3(425 downto 420) := (wbLvl2(704) & wbLvl2(704 downto 700)) + (wbLvl2(709) & wbLvl2(709 downto 705));
    wbLvl3(431 downto 426) := (wbLvl2(714) & wbLvl2(714 downto 710)) + (wbLvl2(719) & wbLvl2(719 downto 715));
    wbLvl3(437 downto 432) := (wbLvl2(724) & wbLvl2(724 downto 720)) + (wbLvl2(729) & wbLvl2(729 downto 725));
    wbLvl3(443 downto 438) := (wbLvl2(734) & wbLvl2(734 downto 730)) + (wbLvl2(739) & wbLvl2(739 downto 735));
    wbLvl3(449 downto 444) := (wbLvl2(744) & wbLvl2(744 downto 740)) + (wbLvl2(749) & wbLvl2(749 downto 745));
    wbLvl3(455 downto 450) := (wbLvl2(754) & wbLvl2(754 downto 750)) + (wbLvl2(759) & wbLvl2(759 downto 755));
    wbLvl3(461 downto 456) := (wbLvl2(764) & wbLvl2(764 downto 760)) + (wbLvl2(769) & wbLvl2(769 downto 765));
    wbLvl3(467 downto 462) := (wbLvl2(774) & wbLvl2(774 downto 770)) + (wbLvl2(779) & wbLvl2(779 downto 775));
    wbLvl3(473 downto 468) := (wbLvl2(784) & wbLvl2(784 downto 780)) + (wbLvl2(789) & wbLvl2(789 downto 785));
    wbLvl3(479 downto 474) := (wbLvl2(794) & wbLvl2(794 downto 790)) + (wbLvl2(799) & wbLvl2(799 downto 795));
    wbLvl3(485 downto 480) := (wbLvl2(804) & wbLvl2(804 downto 800)) + (wbLvl2(809) & wbLvl2(809 downto 805));
    wbLvl3(491 downto 486) := (wbLvl2(814) & wbLvl2(814 downto 810)) + (wbLvl2(819) & wbLvl2(819 downto 815));
    wbLvl3(497 downto 492) := (wbLvl2(824) & wbLvl2(824 downto 820)) + (wbLvl2(829) & wbLvl2(829 downto 825));
    wbLvl3(503 downto 498) := (wbLvl2(834) & wbLvl2(834 downto 830)) + (wbLvl2(839) & wbLvl2(839 downto 835));
    wbLvl3(509 downto 504) := (wbLvl2(844) & wbLvl2(844 downto 840)) + (wbLvl2(849) & wbLvl2(849 downto 845));
    wbLvl3(515 downto 510) := (wbLvl2(854) & wbLvl2(854 downto 850)) + (wbLvl2(859) & wbLvl2(859 downto 855));
    wbLvl3(521 downto 516) := (wbLvl2(864) & wbLvl2(864 downto 860)) + (wbLvl2(869) & wbLvl2(869 downto 865));
    wbLvl3(527 downto 522) := (wbLvl2(874) & wbLvl2(874 downto 870)) + (wbLvl2(879) & wbLvl2(879 downto 875));
    wbLvl3(533 downto 528) := (wbLvl2(884) & wbLvl2(884 downto 880)) + (wbLvl2(889) & wbLvl2(889 downto 885));
    wbLvl3(539 downto 534) := (wbLvl2(894) & wbLvl2(894 downto 890)) + (wbLvl2(899) & wbLvl2(899 downto 895));
    wbLvl3(545 downto 540) := (wbLvl2(904) & wbLvl2(904 downto 900)) + (wbLvl2(909) & wbLvl2(909 downto 905));
    wbLvl3(551 downto 546) := (wbLvl2(914) & wbLvl2(914 downto 910)) + (wbLvl2(919) & wbLvl2(919 downto 915));
    wbLvl3(557 downto 552) := (wbLvl2(924) & wbLvl2(924 downto 920)) + (wbLvl2(929) & wbLvl2(929 downto 925));
    wbLvl3(563 downto 558) := (wbLvl2(934) & wbLvl2(934 downto 930)) + (wbLvl2(939) & wbLvl2(939 downto 935));
    wbLvl3(569 downto 564) := (wbLvl2(944) & wbLvl2(944 downto 940)) + (wbLvl2(949) & wbLvl2(949 downto 945));
    wbLvl3(575 downto 570) := (wbLvl2(954) & wbLvl2(954 downto 950)) + (wbLvl2(959) & wbLvl2(959 downto 955));
    wbLvl3(581 downto 576) := (wbLvl2(964) & wbLvl2(964 downto 960)) + (wbLvl2(969) & wbLvl2(969 downto 965));
    wbLvl3(587 downto 582) := (wbLvl2(974) & wbLvl2(974 downto 970)) + (wbLvl2(979) & wbLvl2(979 downto 975));
    wbLvl3(593 downto 588) := (wbLvl2(984) & wbLvl2(984 downto 980)) + (wbLvl2(989) & wbLvl2(989 downto 985));
    wbLvl3(599 downto 594) := (wbLvl2(994) & wbLvl2(994 downto 990)) + (wbLvl2(999) & wbLvl2(999 downto 995));
    wbLvl3(605 downto 600) := (wbLvl2(1004) & wbLvl2(1004 downto 1000)) + (wbLvl2(1009) & wbLvl2(1009 downto 1005));
    wbLvl3(611 downto 606) := (wbLvl2(1014) & wbLvl2(1014 downto 1010)) + (wbLvl2(1019) & wbLvl2(1019 downto 1015));
    wbLvl3(617 downto 612) := (wbLvl2(1024) & wbLvl2(1024 downto 1020)) + (wbLvl2(1029) & wbLvl2(1029 downto 1025));
    wbLvl3(623 downto 618) := (wbLvl2(1034) & wbLvl2(1034 downto 1030)) + (wbLvl2(1039) & wbLvl2(1039 downto 1035));
    wbLvl3(629 downto 624) := (wbLvl2(1044) & wbLvl2(1044 downto 1040)) + (wbLvl2(1049) & wbLvl2(1049 downto 1045));
    wbLvl3(635 downto 630) := (wbLvl2(1054) & wbLvl2(1054 downto 1050)) + (wbLvl2(1059) & wbLvl2(1059 downto 1055));
    wbLvl3(641 downto 636) := (wbLvl2(1064) & wbLvl2(1064 downto 1060)) + (wbLvl2(1069) & wbLvl2(1069 downto 1065));
    wbLvl3(647 downto 642) := (wbLvl2(1074) & wbLvl2(1074 downto 1070)) + (wbLvl2(1079) & wbLvl2(1079 downto 1075));
    wbLvl3(653 downto 648) := (wbLvl2(1084) & wbLvl2(1084 downto 1080)) + (wbLvl2(1089) & wbLvl2(1089 downto 1085));
    wbLvl3(659 downto 654) := (wbLvl2(1094) & wbLvl2(1094 downto 1090)) + (wbLvl2(1099) & wbLvl2(1099 downto 1095));
    wbLvl3(665 downto 660) := (wbLvl2(1104) & wbLvl2(1104 downto 1100)) + (wbLvl2(1109) & wbLvl2(1109 downto 1105));
    wbLvl3(671 downto 666) := (wbLvl2(1114) & wbLvl2(1114 downto 1110)) + (wbLvl2(1119) & wbLvl2(1119 downto 1115));
    wbLvl3(677 downto 672) := (wbLvl2(1124) & wbLvl2(1124 downto 1120)) + (wbLvl2(1129) & wbLvl2(1129 downto 1125));
    wbLvl3(683 downto 678) := (wbLvl2(1134) & wbLvl2(1134 downto 1130)) + (wbLvl2(1139) & wbLvl2(1139 downto 1135));
    wbLvl3(689 downto 684) := (wbLvl2(1144) & wbLvl2(1144 downto 1140)) + (wbLvl2(1149) & wbLvl2(1149 downto 1145));
    wbLvl3(695 downto 690) := (wbLvl2(1154) & wbLvl2(1154 downto 1150)) + (wbLvl2(1159) & wbLvl2(1159 downto 1155));
    wbLvl3(701 downto 696) := (wbLvl2(1164) & wbLvl2(1164 downto 1160)) + (wbLvl2(1169) & wbLvl2(1169 downto 1165));
    wbLvl3(707 downto 702) := (wbLvl2(1174) & wbLvl2(1174 downto 1170)) + (wbLvl2(1179) & wbLvl2(1179 downto 1175));
    wbLvl3(713 downto 708) := (wbLvl2(1184) & wbLvl2(1184 downto 1180)) + (wbLvl2(1189) & wbLvl2(1189 downto 1185));
    wbLvl3(719 downto 714) := (wbLvl2(1194) & wbLvl2(1194 downto 1190)) + (wbLvl2(1199) & wbLvl2(1199 downto 1195));
    wbLvl3(725 downto 720) := (wbLvl2(1204) & wbLvl2(1204 downto 1200)) + (wbLvl2(1209) & wbLvl2(1209 downto 1205));
    wbLvl3(731 downto 726) := (wbLvl2(1214) & wbLvl2(1214 downto 1210)) + (wbLvl2(1219) & wbLvl2(1219 downto 1215));
    wbLvl3(737 downto 732) := (wbLvl2(1224) & wbLvl2(1224 downto 1220)) + (wbLvl2(1229) & wbLvl2(1229 downto 1225));
    wbLvl3(743 downto 738) := (wbLvl2(1234) & wbLvl2(1234 downto 1230)) + (wbLvl2(1239) & wbLvl2(1239 downto 1235));
    wbLvl3(749 downto 744) := (wbLvl2(1244) & wbLvl2(1244 downto 1240)) + (wbLvl2(1249) & wbLvl2(1249 downto 1245));
    wbLvl3(755 downto 750) := (wbLvl2(1254) & wbLvl2(1254 downto 1250)) + (wbLvl2(1259) & wbLvl2(1259 downto 1255));
    wbLvl3(761 downto 756) := (wbLvl2(1264) & wbLvl2(1264 downto 1260)) + (wbLvl2(1269) & wbLvl2(1269 downto 1265));
    wbLvl3(767 downto 762) := (wbLvl2(1274) & wbLvl2(1274 downto 1270)) + (wbLvl2(1279) & wbLvl2(1279 downto 1275));


    if r.ena(0) = '1' then
      v.Lvl4(6 downto 0) := (wbLvl3(5) & wbLvl3(5 downto 0)) + (wbLvl3(11) & wbLvl3(11 downto 6));
      v.Lvl4(13 downto 7) := (wbLvl3(17) & wbLvl3(17 downto 12)) + (wbLvl3(23) & wbLvl3(23 downto 18));
      v.Lvl4(20 downto 14) := (wbLvl3(29) & wbLvl3(29 downto 24)) + (wbLvl3(35) & wbLvl3(35 downto 30));
      v.Lvl4(27 downto 21) := (wbLvl3(41) & wbLvl3(41 downto 36)) + (wbLvl3(47) & wbLvl3(47 downto 42));
      v.Lvl4(34 downto 28) := (wbLvl3(53) & wbLvl3(53 downto 48)) + (wbLvl3(59) & wbLvl3(59 downto 54));
      v.Lvl4(41 downto 35) := (wbLvl3(65) & wbLvl3(65 downto 60)) + (wbLvl3(71) & wbLvl3(71 downto 66));
      v.Lvl4(48 downto 42) := (wbLvl3(77) & wbLvl3(77 downto 72)) + (wbLvl3(83) & wbLvl3(83 downto 78));
      v.Lvl4(55 downto 49) := (wbLvl3(89) & wbLvl3(89 downto 84)) + (wbLvl3(95) & wbLvl3(95 downto 90));
      v.Lvl4(62 downto 56) := (wbLvl3(101) & wbLvl3(101 downto 96)) + (wbLvl3(107) & wbLvl3(107 downto 102));
      v.Lvl4(69 downto 63) := (wbLvl3(113) & wbLvl3(113 downto 108)) + (wbLvl3(119) & wbLvl3(119 downto 114));
      v.Lvl4(76 downto 70) := (wbLvl3(125) & wbLvl3(125 downto 120)) + (wbLvl3(131) & wbLvl3(131 downto 126));
      v.Lvl4(83 downto 77) := (wbLvl3(137) & wbLvl3(137 downto 132)) + (wbLvl3(143) & wbLvl3(143 downto 138));
      v.Lvl4(90 downto 84) := (wbLvl3(149) & wbLvl3(149 downto 144)) + (wbLvl3(155) & wbLvl3(155 downto 150));
      v.Lvl4(97 downto 91) := (wbLvl3(161) & wbLvl3(161 downto 156)) + (wbLvl3(167) & wbLvl3(167 downto 162));
      v.Lvl4(104 downto 98) := (wbLvl3(173) & wbLvl3(173 downto 168)) + (wbLvl3(179) & wbLvl3(179 downto 174));
      v.Lvl4(111 downto 105) := (wbLvl3(185) & wbLvl3(185 downto 180)) + (wbLvl3(191) & wbLvl3(191 downto 186));
      v.Lvl4(118 downto 112) := (wbLvl3(197) & wbLvl3(197 downto 192)) + (wbLvl3(203) & wbLvl3(203 downto 198));
      v.Lvl4(125 downto 119) := (wbLvl3(209) & wbLvl3(209 downto 204)) + (wbLvl3(215) & wbLvl3(215 downto 210));
      v.Lvl4(132 downto 126) := (wbLvl3(221) & wbLvl3(221 downto 216)) + (wbLvl3(227) & wbLvl3(227 downto 222));
      v.Lvl4(139 downto 133) := (wbLvl3(233) & wbLvl3(233 downto 228)) + (wbLvl3(239) & wbLvl3(239 downto 234));
      v.Lvl4(146 downto 140) := (wbLvl3(245) & wbLvl3(245 downto 240)) + (wbLvl3(251) & wbLvl3(251 downto 246));
      v.Lvl4(153 downto 147) := (wbLvl3(257) & wbLvl3(257 downto 252)) + (wbLvl3(263) & wbLvl3(263 downto 258));
      v.Lvl4(160 downto 154) := (wbLvl3(269) & wbLvl3(269 downto 264)) + (wbLvl3(275) & wbLvl3(275 downto 270));
      v.Lvl4(167 downto 161) := (wbLvl3(281) & wbLvl3(281 downto 276)) + (wbLvl3(287) & wbLvl3(287 downto 282));
      v.Lvl4(174 downto 168) := (wbLvl3(293) & wbLvl3(293 downto 288)) + (wbLvl3(299) & wbLvl3(299 downto 294));
      v.Lvl4(181 downto 175) := (wbLvl3(305) & wbLvl3(305 downto 300)) + (wbLvl3(311) & wbLvl3(311 downto 306));
      v.Lvl4(188 downto 182) := (wbLvl3(317) & wbLvl3(317 downto 312)) + (wbLvl3(323) & wbLvl3(323 downto 318));
      v.Lvl4(195 downto 189) := (wbLvl3(329) & wbLvl3(329 downto 324)) + (wbLvl3(335) & wbLvl3(335 downto 330));
      v.Lvl4(202 downto 196) := (wbLvl3(341) & wbLvl3(341 downto 336)) + (wbLvl3(347) & wbLvl3(347 downto 342));
      v.Lvl4(209 downto 203) := (wbLvl3(353) & wbLvl3(353 downto 348)) + (wbLvl3(359) & wbLvl3(359 downto 354));
      v.Lvl4(216 downto 210) := (wbLvl3(365) & wbLvl3(365 downto 360)) + (wbLvl3(371) & wbLvl3(371 downto 366));
      v.Lvl4(223 downto 217) := (wbLvl3(377) & wbLvl3(377 downto 372)) + (wbLvl3(383) & wbLvl3(383 downto 378));
      v.Lvl4(230 downto 224) := (wbLvl3(389) & wbLvl3(389 downto 384)) + (wbLvl3(395) & wbLvl3(395 downto 390));
      v.Lvl4(237 downto 231) := (wbLvl3(401) & wbLvl3(401 downto 396)) + (wbLvl3(407) & wbLvl3(407 downto 402));
      v.Lvl4(244 downto 238) := (wbLvl3(413) & wbLvl3(413 downto 408)) + (wbLvl3(419) & wbLvl3(419 downto 414));
      v.Lvl4(251 downto 245) := (wbLvl3(425) & wbLvl3(425 downto 420)) + (wbLvl3(431) & wbLvl3(431 downto 426));
      v.Lvl4(258 downto 252) := (wbLvl3(437) & wbLvl3(437 downto 432)) + (wbLvl3(443) & wbLvl3(443 downto 438));
      v.Lvl4(265 downto 259) := (wbLvl3(449) & wbLvl3(449 downto 444)) + (wbLvl3(455) & wbLvl3(455 downto 450));
      v.Lvl4(272 downto 266) := (wbLvl3(461) & wbLvl3(461 downto 456)) + (wbLvl3(467) & wbLvl3(467 downto 462));
      v.Lvl4(279 downto 273) := (wbLvl3(473) & wbLvl3(473 downto 468)) + (wbLvl3(479) & wbLvl3(479 downto 474));
      v.Lvl4(286 downto 280) := (wbLvl3(485) & wbLvl3(485 downto 480)) + (wbLvl3(491) & wbLvl3(491 downto 486));
      v.Lvl4(293 downto 287) := (wbLvl3(497) & wbLvl3(497 downto 492)) + (wbLvl3(503) & wbLvl3(503 downto 498));
      v.Lvl4(300 downto 294) := (wbLvl3(509) & wbLvl3(509 downto 504)) + (wbLvl3(515) & wbLvl3(515 downto 510));
      v.Lvl4(307 downto 301) := (wbLvl3(521) & wbLvl3(521 downto 516)) + (wbLvl3(527) & wbLvl3(527 downto 522));
      v.Lvl4(314 downto 308) := (wbLvl3(533) & wbLvl3(533 downto 528)) + (wbLvl3(539) & wbLvl3(539 downto 534));
      v.Lvl4(321 downto 315) := (wbLvl3(545) & wbLvl3(545 downto 540)) + (wbLvl3(551) & wbLvl3(551 downto 546));
      v.Lvl4(328 downto 322) := (wbLvl3(557) & wbLvl3(557 downto 552)) + (wbLvl3(563) & wbLvl3(563 downto 558));
      v.Lvl4(335 downto 329) := (wbLvl3(569) & wbLvl3(569 downto 564)) + (wbLvl3(575) & wbLvl3(575 downto 570));
      v.Lvl4(342 downto 336) := (wbLvl3(581) & wbLvl3(581 downto 576)) + (wbLvl3(587) & wbLvl3(587 downto 582));
      v.Lvl4(349 downto 343) := (wbLvl3(593) & wbLvl3(593 downto 588)) + (wbLvl3(599) & wbLvl3(599 downto 594));
      v.Lvl4(356 downto 350) := (wbLvl3(605) & wbLvl3(605 downto 600)) + (wbLvl3(611) & wbLvl3(611 downto 606));
      v.Lvl4(363 downto 357) := (wbLvl3(617) & wbLvl3(617 downto 612)) + (wbLvl3(623) & wbLvl3(623 downto 618));
      v.Lvl4(370 downto 364) := (wbLvl3(629) & wbLvl3(629 downto 624)) + (wbLvl3(635) & wbLvl3(635 downto 630));
      v.Lvl4(377 downto 371) := (wbLvl3(641) & wbLvl3(641 downto 636)) + (wbLvl3(647) & wbLvl3(647 downto 642));
      v.Lvl4(384 downto 378) := (wbLvl3(653) & wbLvl3(653 downto 648)) + (wbLvl3(659) & wbLvl3(659 downto 654));
      v.Lvl4(391 downto 385) := (wbLvl3(665) & wbLvl3(665 downto 660)) + (wbLvl3(671) & wbLvl3(671 downto 666));
      v.Lvl4(398 downto 392) := (wbLvl3(677) & wbLvl3(677 downto 672)) + (wbLvl3(683) & wbLvl3(683 downto 678));
      v.Lvl4(405 downto 399) := (wbLvl3(689) & wbLvl3(689 downto 684)) + (wbLvl3(695) & wbLvl3(695 downto 690));
      v.Lvl4(412 downto 406) := (wbLvl3(701) & wbLvl3(701 downto 696)) + (wbLvl3(707) & wbLvl3(707 downto 702));
      v.Lvl4(419 downto 413) := (wbLvl3(713) & wbLvl3(713 downto 708)) + (wbLvl3(719) & wbLvl3(719 downto 714));
      v.Lvl4(426 downto 420) := (wbLvl3(725) & wbLvl3(725 downto 720)) + (wbLvl3(731) & wbLvl3(731 downto 726));
      v.Lvl4(433 downto 427) := (wbLvl3(737) & wbLvl3(737 downto 732)) + (wbLvl3(743) & wbLvl3(743 downto 738));
      v.Lvl4(440 downto 434) := (wbLvl3(749) & wbLvl3(749 downto 744)) + (wbLvl3(755) & wbLvl3(755 downto 750));
      v.Lvl4(447 downto 441) := (wbLvl3(761) & wbLvl3(761 downto 756)) + (wbLvl3(767) & wbLvl3(767 downto 762));
    end if;


    wbLvl5(7 downto 0) := (r.Lvl4(6) & r.Lvl4(6 downto 0)) + (r.Lvl4(13) & r.Lvl4(13 downto 7));
    wbLvl5(15 downto 8) := (r.Lvl4(20) & r.Lvl4(20 downto 14)) + (r.Lvl4(27) & r.Lvl4(27 downto 21));
    wbLvl5(23 downto 16) := (r.Lvl4(34) & r.Lvl4(34 downto 28)) + (r.Lvl4(41) & r.Lvl4(41 downto 35));
    wbLvl5(31 downto 24) := (r.Lvl4(48) & r.Lvl4(48 downto 42)) + (r.Lvl4(55) & r.Lvl4(55 downto 49));
    wbLvl5(39 downto 32) := (r.Lvl4(62) & r.Lvl4(62 downto 56)) + (r.Lvl4(69) & r.Lvl4(69 downto 63));
    wbLvl5(47 downto 40) := (r.Lvl4(76) & r.Lvl4(76 downto 70)) + (r.Lvl4(83) & r.Lvl4(83 downto 77));
    wbLvl5(55 downto 48) := (r.Lvl4(90) & r.Lvl4(90 downto 84)) + (r.Lvl4(97) & r.Lvl4(97 downto 91));
    wbLvl5(63 downto 56) := (r.Lvl4(104) & r.Lvl4(104 downto 98)) + (r.Lvl4(111) & r.Lvl4(111 downto 105));
    wbLvl5(71 downto 64) := (r.Lvl4(118) & r.Lvl4(118 downto 112)) + (r.Lvl4(125) & r.Lvl4(125 downto 119));
    wbLvl5(79 downto 72) := (r.Lvl4(132) & r.Lvl4(132 downto 126)) + (r.Lvl4(139) & r.Lvl4(139 downto 133));
    wbLvl5(87 downto 80) := (r.Lvl4(146) & r.Lvl4(146 downto 140)) + (r.Lvl4(153) & r.Lvl4(153 downto 147));
    wbLvl5(95 downto 88) := (r.Lvl4(160) & r.Lvl4(160 downto 154)) + (r.Lvl4(167) & r.Lvl4(167 downto 161));
    wbLvl5(103 downto 96) := (r.Lvl4(174) & r.Lvl4(174 downto 168)) + (r.Lvl4(181) & r.Lvl4(181 downto 175));
    wbLvl5(111 downto 104) := (r.Lvl4(188) & r.Lvl4(188 downto 182)) + (r.Lvl4(195) & r.Lvl4(195 downto 189));
    wbLvl5(119 downto 112) := (r.Lvl4(202) & r.Lvl4(202 downto 196)) + (r.Lvl4(209) & r.Lvl4(209 downto 203));
    wbLvl5(127 downto 120) := (r.Lvl4(216) & r.Lvl4(216 downto 210)) + (r.Lvl4(223) & r.Lvl4(223 downto 217));
    wbLvl5(135 downto 128) := (r.Lvl4(230) & r.Lvl4(230 downto 224)) + (r.Lvl4(237) & r.Lvl4(237 downto 231));
    wbLvl5(143 downto 136) := (r.Lvl4(244) & r.Lvl4(244 downto 238)) + (r.Lvl4(251) & r.Lvl4(251 downto 245));
    wbLvl5(151 downto 144) := (r.Lvl4(258) & r.Lvl4(258 downto 252)) + (r.Lvl4(265) & r.Lvl4(265 downto 259));
    wbLvl5(159 downto 152) := (r.Lvl4(272) & r.Lvl4(272 downto 266)) + (r.Lvl4(279) & r.Lvl4(279 downto 273));
    wbLvl5(167 downto 160) := (r.Lvl4(286) & r.Lvl4(286 downto 280)) + (r.Lvl4(293) & r.Lvl4(293 downto 287));
    wbLvl5(175 downto 168) := (r.Lvl4(300) & r.Lvl4(300 downto 294)) + (r.Lvl4(307) & r.Lvl4(307 downto 301));
    wbLvl5(183 downto 176) := (r.Lvl4(314) & r.Lvl4(314 downto 308)) + (r.Lvl4(321) & r.Lvl4(321 downto 315));
    wbLvl5(191 downto 184) := (r.Lvl4(328) & r.Lvl4(328 downto 322)) + (r.Lvl4(335) & r.Lvl4(335 downto 329));
    wbLvl5(199 downto 192) := (r.Lvl4(342) & r.Lvl4(342 downto 336)) + (r.Lvl4(349) & r.Lvl4(349 downto 343));
    wbLvl5(207 downto 200) := (r.Lvl4(356) & r.Lvl4(356 downto 350)) + (r.Lvl4(363) & r.Lvl4(363 downto 357));
    wbLvl5(215 downto 208) := (r.Lvl4(370) & r.Lvl4(370 downto 364)) + (r.Lvl4(377) & r.Lvl4(377 downto 371));
    wbLvl5(223 downto 216) := (r.Lvl4(384) & r.Lvl4(384 downto 378)) + (r.Lvl4(391) & r.Lvl4(391 downto 385));
    wbLvl5(231 downto 224) := (r.Lvl4(398) & r.Lvl4(398 downto 392)) + (r.Lvl4(405) & r.Lvl4(405 downto 399));
    wbLvl5(239 downto 232) := (r.Lvl4(412) & r.Lvl4(412 downto 406)) + (r.Lvl4(419) & r.Lvl4(419 downto 413));
    wbLvl5(247 downto 240) := (r.Lvl4(426) & r.Lvl4(426 downto 420)) + (r.Lvl4(433) & r.Lvl4(433 downto 427));
    wbLvl5(255 downto 248) := (r.Lvl4(440) & r.Lvl4(440 downto 434)) + (r.Lvl4(447) & r.Lvl4(447 downto 441));


    wbLvl6(8 downto 0) := (wbLvl5(7) & wbLvl5(7 downto 0)) + (wbLvl5(15) & wbLvl5(15 downto 8));
    wbLvl6(17 downto 9) := (wbLvl5(23) & wbLvl5(23 downto 16)) + (wbLvl5(31) & wbLvl5(31 downto 24));
    wbLvl6(26 downto 18) := (wbLvl5(39) & wbLvl5(39 downto 32)) + (wbLvl5(47) & wbLvl5(47 downto 40));
    wbLvl6(35 downto 27) := (wbLvl5(55) & wbLvl5(55 downto 48)) + (wbLvl5(63) & wbLvl5(63 downto 56));
    wbLvl6(44 downto 36) := (wbLvl5(71) & wbLvl5(71 downto 64)) + (wbLvl5(79) & wbLvl5(79 downto 72));
    wbLvl6(53 downto 45) := (wbLvl5(87) & wbLvl5(87 downto 80)) + (wbLvl5(95) & wbLvl5(95 downto 88));
    wbLvl6(62 downto 54) := (wbLvl5(103) & wbLvl5(103 downto 96)) + (wbLvl5(111) & wbLvl5(111 downto 104));
    wbLvl6(71 downto 63) := (wbLvl5(119) & wbLvl5(119 downto 112)) + (wbLvl5(127) & wbLvl5(127 downto 120));
    wbLvl6(80 downto 72) := (wbLvl5(135) & wbLvl5(135 downto 128)) + (wbLvl5(143) & wbLvl5(143 downto 136));
    wbLvl6(89 downto 81) := (wbLvl5(151) & wbLvl5(151 downto 144)) + (wbLvl5(159) & wbLvl5(159 downto 152));
    wbLvl6(98 downto 90) := (wbLvl5(167) & wbLvl5(167 downto 160)) + (wbLvl5(175) & wbLvl5(175 downto 168));
    wbLvl6(107 downto 99) := (wbLvl5(183) & wbLvl5(183 downto 176)) + (wbLvl5(191) & wbLvl5(191 downto 184));
    wbLvl6(116 downto 108) := (wbLvl5(199) & wbLvl5(199 downto 192)) + (wbLvl5(207) & wbLvl5(207 downto 200));
    wbLvl6(125 downto 117) := (wbLvl5(215) & wbLvl5(215 downto 208)) + (wbLvl5(223) & wbLvl5(223 downto 216));
    wbLvl6(134 downto 126) := (wbLvl5(231) & wbLvl5(231 downto 224)) + (wbLvl5(239) & wbLvl5(239 downto 232));
    wbLvl6(143 downto 135) := (wbLvl5(247) & wbLvl5(247 downto 240)) + (wbLvl5(255) & wbLvl5(255 downto 248));


    wbLvl7(9 downto 0) := (wbLvl6(8) & wbLvl6(8 downto 0)) + (wbLvl6(17) & wbLvl6(17 downto 9));
    wbLvl7(19 downto 10) := (wbLvl6(26) & wbLvl6(26 downto 18)) + (wbLvl6(35) & wbLvl6(35 downto 27));
    wbLvl7(29 downto 20) := (wbLvl6(44) & wbLvl6(44 downto 36)) + (wbLvl6(53) & wbLvl6(53 downto 45));
    wbLvl7(39 downto 30) := (wbLvl6(62) & wbLvl6(62 downto 54)) + (wbLvl6(71) & wbLvl6(71 downto 63));
    wbLvl7(49 downto 40) := (wbLvl6(80) & wbLvl6(80 downto 72)) + (wbLvl6(89) & wbLvl6(89 downto 81));
    wbLvl7(59 downto 50) := (wbLvl6(98) & wbLvl6(98 downto 90)) + (wbLvl6(107) & wbLvl6(107 downto 99));
    wbLvl7(69 downto 60) := (wbLvl6(116) & wbLvl6(116 downto 108)) + (wbLvl6(125) & wbLvl6(125 downto 117));
    wbLvl7(79 downto 70) := (wbLvl6(134) & wbLvl6(134 downto 126)) + (wbLvl6(143) & wbLvl6(143 downto 135));


    if r.ena(1) = '1' then
      v.Lvl8(10 downto 0) := (wbLvl7(9) & wbLvl7(9 downto 0)) + (wbLvl7(19) & wbLvl7(19 downto 10));
      v.Lvl8(21 downto 11) := (wbLvl7(29) & wbLvl7(29 downto 20)) + (wbLvl7(39) & wbLvl7(39 downto 30));
      v.Lvl8(32 downto 22) := (wbLvl7(49) & wbLvl7(49 downto 40)) + (wbLvl7(59) & wbLvl7(59 downto 50));
      v.Lvl8(43 downto 33) := (wbLvl7(69) & wbLvl7(69 downto 60)) + (wbLvl7(79) & wbLvl7(79 downto 70));
    end if;


    wbLvl9(11 downto 0) := (r.Lvl8(10) & r.Lvl8(10 downto 0)) + (r.Lvl8(21) & r.Lvl8(21 downto 11));
    wbLvl9(23 downto 12) := (r.Lvl8(32) & r.Lvl8(32 downto 22)) + (r.Lvl8(43) & r.Lvl8(43 downto 33));


    o_sum <= (wbLvl9(11) & wbLvl9(11 downto 0)) + (wbLvl9(23) & wbLvl9(23 downto 12));
    
    
    if i_nrst = '0' then
      v.Lvl4 := (others => '0');
      v.Lvl8 := (others => '0');
      v.ena := (others => '0');
    end if;

    rin <= v;
  end process;
  

  regs : process(i_clk)
  begin 
    if rising_edge(i_clk) then 
      r <= rin;
    end if;
  end process;

end; 

