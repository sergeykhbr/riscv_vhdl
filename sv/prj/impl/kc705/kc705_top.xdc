
create_clock -name i_sclk_p -period 5.000 [get_ports i_sclk_p]
set_property PACKAGE_PIN AD12 [get_ports i_sclk_p]
set_property IOSTANDARD LVDS [get_ports i_sclk_p]
set_property PACKAGE_PIN AD11 [get_ports i_sclk_n]
set_property IOSTANDARD LVDS [get_ports i_sclk_n]

# button "Center"
set_property PACKAGE_PIN G12 [get_ports i_rst]
set_property CLOCK_DEDICATED_ROUTE FALSE [get_nets i_rst]
set_property IOSTANDARD LVCMOS25 [get_ports i_rst]

# DIP switch: SW1.1
set_property PACKAGE_PIN Y29 [get_ports {io_gpio[0]}]
set_property IOSTANDARD LVCMOS25 [get_ports {io_gpio[0]}]
set_property PACKAGE_PIN W29 [get_ports {io_gpio[1]}]
set_property IOSTANDARD LVCMOS25 [get_ports {io_gpio[1]}]
set_property PACKAGE_PIN AA28 [get_ports {io_gpio[2]}]
set_property IOSTANDARD LVCMOS25 [get_ports {io_gpio[2]}]
set_property PACKAGE_PIN Y28 [get_ports {io_gpio[3]}]
set_property IOSTANDARD LVCMOS25 [get_ports {io_gpio[3]}]
#/ User's LEDs:
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

# JTAG
set_property PACKAGE_PIN AA25 [get_ports i_jtag_tck]
set_property IOSTANDARD LVCMOS25 [get_ports i_jtag_tck]
set_property CLOCK_DEDICATED_ROUTE FALSE [get_nets i_jtag_tck]
create_clock -period 1000.000 -name i_swjtag_clktck -waveform {0.000 500.000} [get_ports i_jtag_tck]

set_property PACKAGE_PIN J23 [get_ports i_jtag_trst]
set_property IOSTANDARD LVCMOS25 [get_ports i_jtag_trst]

set_property PACKAGE_PIN AA27 [get_ports i_jtag_tms]
set_property IOSTANDARD LVCMOS25 [get_ports i_jtag_tms]

set_property PACKAGE_PIN AB25 [get_ports i_jtag_tdi]
set_property IOSTANDARD LVCMOS25 [get_ports i_jtag_tdi]

set_property PACKAGE_PIN AB28 [get_ports o_jtag_tdo]
set_property IOSTANDARD LVCMOS25 [get_ports o_jtag_tdo]

set_property IOSTANDARD LVCMOS25 [get_ports o_jtag_vref]
set_property PACKAGE_PIN W21 [get_ports o_jtag_vref]


#UART interface
set_property PACKAGE_PIN M19 [get_ports i_uart1_rd]
set_property IOSTANDARD LVCMOS25 [get_ports i_uart1_rd]
set_property PACKAGE_PIN K24 [get_ports o_uart1_td]
set_property IOSTANDARD LVCMOS25 [get_ports o_uart1_td]

