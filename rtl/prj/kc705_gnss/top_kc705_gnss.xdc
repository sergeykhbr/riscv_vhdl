
create_clock -name i_sclk_p -period 5.000 [get_ports i_sclk_p]
set_property IOSTANDARD LVDS [get_ports i_sclk_p]
set_property IOSTANDARD LVDS [get_ports i_sclk_n]
set_property PACKAGE_PIN AD12 [get_ports i_sclk_p]
set_property PACKAGE_PIN AD11 [get_ports i_sclk_n]

# button "Center"
set_property PACKAGE_PIN G12 [get_ports i_rst]
set_property CLOCK_DEDICATED_ROUTE FALSE [get_nets i_rst]
set_property IOSTANDARD LVCMOS25 [get_ports i_rst]

set_property PACKAGE_PIN AF22 [get_ports i_clk_adc]
set_property IOSTANDARD LVCMOS25 [get_ports i_clk_adc]


#UART interface
set_property PACKAGE_PIN M19 [get_ports i_uart1_rd]
set_property IOSTANDARD LVCMOS25 [get_ports i_uart1_rd]

set_property PACKAGE_PIN K24 [get_ports o_uart1_td]
set_property IOSTANDARD LVCMOS25 [get_ports o_uart1_td]

# UART2 interface (debug port)
# HPC H19
set_property PACKAGE_PIN C24 [get_ports i_uart2_rd]
set_property IOSTANDARD LVCMOS25 [get_ports i_uart2_rd]
# HPC G18
set_property PACKAGE_PIN B27 [get_ports o_uart2_td]
set_property IOSTANDARD LVCMOS25 [get_ports o_uart2_td]

# DIP switch: SW1.1
set_property PACKAGE_PIN Y29 [get_ports {io_gpio[0]}]
set_property IOSTANDARD LVCMOS25 [get_ports {io_gpio[0]}]
set_property PACKAGE_PIN W29 [get_ports {io_gpio[1]}]
set_property IOSTANDARD LVCMOS25 [get_ports {io_gpio[1]}]
set_property PACKAGE_PIN AA28 [get_ports {io_gpio[2]}]
set_property IOSTANDARD LVCMOS25 [get_ports {io_gpio[2]}]
set_property PACKAGE_PIN Y28 [get_ports {io_gpio[3]}]
set_property IOSTANDARD LVCMOS25 [get_ports {io_gpio[3]}]

# User's LEDs:
set_property PACKAGE_PIN AB8 [get_ports {io_gpio[4]}]
set_property IOSTANDARD LVCMOS15 [get_ports {io_gpio[4]}]
set_property PACKAGE_PIN AA8 [get_ports {io_gpio[5]}]
set_property IOSTANDARD LVCMOS15 [get_ports {io_gpio[5]}]
set_property PACKAGE_PIN AC9 [get_ports {io_gpio[6]}]
set_property IOSTANDARD LVCMOS15 [get_ports {io_gpio[6]}]
set_property PACKAGE_PIN AB9 [get_ports {io_gpio[7]}]
set_property IOSTANDARD LVCMOS15 [get_ports {io_gpio[7]}]
set_property PACKAGE_PIN AE26 [get_ports {io_gpio[8]}]
set_property IOSTANDARD LVCMOS25 [get_ports {io_gpio[8]}]
set_property PACKAGE_PIN G19 [get_ports {io_gpio[9]}]
set_property IOSTANDARD LVCMOS25 [get_ports {io_gpio[9]}]
set_property PACKAGE_PIN E18 [get_ports {io_gpio[10]}]
set_property IOSTANDARD LVCMOS25 [get_ports {io_gpio[10]}]
set_property PACKAGE_PIN F16 [get_ports {io_gpio[11]}]
set_property IOSTANDARD LVCMOS25 [get_ports {io_gpio[11]}]


# Ethernet signals
set_property PACKAGE_PIN G8 [get_ports i_gmiiclk_p]
set_property PACKAGE_PIN G7 [get_ports i_gmiiclk_n]

set_property CLOCK_DEDICATED_ROUTE FALSE [get_nets -of [get_pins igbebuf0/xk7.x1/x1/O]]

set_property PACKAGE_PIN J21 [get_ports io_emdio]
set_property IOSTANDARD LVCMOS25 [get_ports io_emdio]
set_property PACKAGE_PIN R23 [get_ports o_emdc]
set_property IOSTANDARD LVCMOS25 [get_ports o_emdc]
set_property PACKAGE_PIN N30 [get_ports i_emdint]
set_property IOSTANDARD LVCMOS25 [get_ports i_emdint]
set_property PACKAGE_PIN L20 [get_ports o_erstn]
set_property IOSTANDARD LVCMOS25 [get_ports o_erstn]
set_property PACKAGE_PIN R30 [get_ports i_erx_crs]
set_property IOSTANDARD LVCMOS25 [get_ports i_erx_crs]
set_property PACKAGE_PIN W19 [get_ports i_erx_col]
set_property IOSTANDARD LVCMOS25 [get_ports i_erx_col]
set_property PACKAGE_PIN U27 [get_ports i_erx_clk]
set_property IOSTANDARD LVCMOS25 [get_ports i_erx_clk]
set_property PACKAGE_PIN V26 [get_ports i_erx_er]
set_property IOSTANDARD LVCMOS25 [get_ports i_erx_er]
set_property PACKAGE_PIN R28 [get_ports i_erx_dv]
set_property IOSTANDARD LVCMOS25 [get_ports i_erx_dv]
set_property PACKAGE_PIN U30 [get_ports {i_erxd[0]}]
set_property IOSTANDARD LVCMOS25 [get_ports {i_erxd[0]}]
set_property PACKAGE_PIN U25 [get_ports {i_erxd[1]}]
set_property IOSTANDARD LVCMOS25 [get_ports {i_erxd[1]}]
set_property PACKAGE_PIN T25 [get_ports {i_erxd[2]}]
set_property IOSTANDARD LVCMOS25 [get_ports {i_erxd[2]}]
set_property PACKAGE_PIN U28 [get_ports {i_erxd[3]}]
set_property IOSTANDARD LVCMOS25 [get_ports {i_erxd[3]}]
set_property PACKAGE_PIN K30 [get_ports o_egtx_clk]
set_property IOSTANDARD LVCMOS25 [get_ports o_egtx_clk]
set_property PACKAGE_PIN M28 [get_ports i_etx_clk]
set_property IOSTANDARD LVCMOS25 [get_ports i_etx_clk]
set_property PACKAGE_PIN N29 [get_ports o_etx_er]
set_property IOSTANDARD LVCMOS25 [get_ports o_etx_er]
set_property PACKAGE_PIN M27 [get_ports o_etx_en]
set_property IOSTANDARD LVCMOS25 [get_ports o_etx_en]
set_property PACKAGE_PIN N27 [get_ports {o_etxd[0]}]
set_property IOSTANDARD LVCMOS25 [get_ports {o_etxd[0]}]
set_property PACKAGE_PIN N25 [get_ports {o_etxd[1]}]
set_property IOSTANDARD LVCMOS25 [get_ports {o_etxd[1]}]
set_property PACKAGE_PIN M29 [get_ports {o_etxd[2]}]
set_property IOSTANDARD LVCMOS25 [get_ports {o_etxd[2]}]
set_property PACKAGE_PIN L28 [get_ports {o_etxd[3]}]
set_property IOSTANDARD LVCMOS25 [get_ports {o_etxd[3]}]


# ADC samples:
set_property PACKAGE_PIN AH30 [get_ports {i_gps_I[1]}]
set_property IOSTANDARD LVCMOS25 [get_ports {i_gps_I[1]}]
set_property PACKAGE_PIN AE28 [get_ports {i_gps_I[0]}]
set_property IOSTANDARD LVCMOS25 [get_ports {i_gps_I[0]}]
set_property PACKAGE_PIN AD26 [get_ports {i_gps_Q[1]}]
set_property IOSTANDARD LVCMOS25 [get_ports {i_gps_Q[1]}]
set_property PACKAGE_PIN AG28 [get_ports {i_gps_Q[0]}]
set_property IOSTANDARD LVCMOS25 [get_ports {i_gps_Q[0]}]

set_property PACKAGE_PIN AA30 [get_ports {i_glo_I[1]}]
set_property IOSTANDARD LVCMOS25 [get_ports {i_glo_I[1]}]
set_property PACKAGE_PIN Y30 [get_ports {i_glo_I[0]}]
set_property IOSTANDARD LVCMOS25 [get_ports {i_glo_I[0]}]
set_property PACKAGE_PIN AC29 [get_ports {i_glo_Q[1]}]
set_property IOSTANDARD LVCMOS25 [get_ports {i_glo_Q[1]}]
set_property PACKAGE_PIN AC30 [get_ports {i_glo_Q[0]}]
set_property IOSTANDARD LVCMOS25 [get_ports {i_glo_Q[0]}]

set_property PACKAGE_PIN AE30 [get_ports i_gps_ld]
set_property IOSTANDARD LVCMOS25 [get_ports i_gps_ld]
set_property PACKAGE_PIN AF30 [get_ports i_glo_ld]
set_property IOSTANDARD LVCMOS25 [get_ports i_glo_ld]

set_property PACKAGE_PIN H30 [get_ports o_pps]
set_property IOSTANDARD LVCMOS25 [get_ports o_pps]

# RF control:
set_property PACKAGE_PIN AE29 [get_ports o_max_sclk]
set_property IOSTANDARD LVCMOS25 [get_ports o_max_sclk]
set_property PACKAGE_PIN AD29 [get_ports o_max_sdata]
set_property IOSTANDARD LVCMOS25 [get_ports o_max_sdata]
set_property PACKAGE_PIN AB29 [get_ports {o_max_ncs[1]}]
set_property IOSTANDARD LVCMOS25 [get_ports {o_max_ncs[1]}]
set_property PACKAGE_PIN AF28 [get_ports {o_max_ncs[0]}]
set_property IOSTANDARD LVCMOS25 [get_ports {o_max_ncs[0]}]
set_property PACKAGE_PIN AH25 [get_ports i_antext_stat]
set_property IOSTANDARD LVCMOS25 [get_ports i_antext_stat]
set_property PACKAGE_PIN AJ23 [get_ports i_antext_detect]
set_property IOSTANDARD LVCMOS25 [get_ports i_antext_detect]
set_property PACKAGE_PIN AG25 [get_ports o_antext_ena]
set_property IOSTANDARD LVCMOS25 [get_ports o_antext_ena]
set_property PACKAGE_PIN AC26 [get_ports o_antint_contr]
set_property IOSTANDARD LVCMOS25 [get_ports o_antint_contr]


# JTAG
set_property PACKAGE_PIN AD21 [get_ports i_jtag_tck]
set_property IOSTANDARD LVCMOS25 [get_ports i_jtag_tck]
set_property CLOCK_DEDICATED_ROUTE FALSE [get_nets i_jtag_tck]
create_clock -period 1000.000 -name i_swjtag_clktck -waveform {0.000 500.000} [get_ports i_jtag_tck]

set_property PACKAGE_PIN AK20 [get_ports i_jtag_ntrst]
set_property IOSTANDARD LVCMOS25 [get_ports i_jtag_ntrst]

set_property PACKAGE_PIN AE21 [get_ports i_jtag_tms]
set_property IOSTANDARD LVCMOS25 [get_ports i_jtag_tms]

set_property PACKAGE_PIN AJ24 [get_ports i_jtag_tdi]
set_property IOSTANDARD LVCMOS25 [get_ports i_jtag_tdi]

set_property PACKAGE_PIN AK25 [get_ports o_jtag_tdo]
set_property IOSTANDARD LVCMOS25 [get_ports o_jtag_tdo]

set_property IOSTANDARD LVCMOS25 [get_ports o_jtag_vref]
set_property PACKAGE_PIN AK21 [get_ports o_jtag_vref]
