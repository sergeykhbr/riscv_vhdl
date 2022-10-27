# DIP switch:
set_property PACKAGE_PIN "F22" [get_ports "io_gpio[0]"]
set_property iostandard "LVCMOS33" [get_ports "io_gpio[0]"]
set_property PIO_DIRECTION "BIDIR" [get_ports "io_gpio[0]"]

set_property PACKAGE_PIN "G22" [get_ports "io_gpio[1]"]
set_property iostandard "LVCMOS33" [get_ports "io_gpio[1]"]
set_property PIO_DIRECTION "BIDIR" [get_ports "io_gpio[1]"]

set_property PACKAGE_PIN "H22" [get_ports "io_gpio[2]"]
set_property iostandard "LVCMOS33" [get_ports "io_gpio[2]"]
set_property PIO_DIRECTION "BIDIR" [get_ports "io_gpio[2]"]

set_property PACKAGE_PIN "F21" [get_ports "io_gpio[3]"]
set_property iostandard "LVCMOS33" [get_ports "io_gpio[3]"]
set_property PIO_DIRECTION "BIDIR" [get_ports "io_gpio[3]"]
# User's LEDs:
set_property PACKAGE_PIN "T22" [get_ports "io_gpio[4]"]
set_property iostandard "LVCMOS33" [get_ports "io_gpio[4]"]
set_property PIO_DIRECTION "BIDIR" [get_ports "io_gpio[4]"]

set_property PACKAGE_PIN "T21" [get_ports "io_gpio[5]"]
set_property iostandard "LVCMOS33" [get_ports "io_gpio[5]"]
set_property PIO_DIRECTION "BIDIR" [get_ports "io_gpio[5]"]

set_property PACKAGE_PIN "U22" [get_ports "io_gpio[6]"]
set_property iostandard "LVCMOS33" [get_ports "io_gpio[6]"]
set_property PIO_DIRECTION "BIDIR" [get_ports "io_gpio[6]"]

set_property PACKAGE_PIN "U21" [get_ports "io_gpio[7]"]
set_property iostandard "LVCMOS33" [get_ports "io_gpio[7]"]
set_property PIO_DIRECTION "BIDIR" [get_ports "io_gpio[7]"]

set_property PACKAGE_PIN "V22" [get_ports "io_gpio[8]"]
set_property iostandard "LVCMOS33" [get_ports "io_gpio[8]"]
set_property PIO_DIRECTION "BIDIR" [get_ports "io_gpio[8]"]

set_property PACKAGE_PIN "W22" [get_ports "io_gpio[9]"]
set_property iostandard "LVCMOS33" [get_ports "io_gpio[9]"]
set_property PIO_DIRECTION "BIDIR" [get_ports "io_gpio[9]"]

set_property PACKAGE_PIN "U19" [get_ports "io_gpio[10]"]
set_property iostandard "LVCMOS33" [get_ports "io_gpio[10]"]
set_property PIO_DIRECTION "BIDIR" [get_ports "io_gpio[10]"]

set_property PACKAGE_PIN "U14" [get_ports "io_gpio[11]"]
set_property iostandard "LVCMOS33" [get_ports "io_gpio[11]"]
set_property PIO_DIRECTION "BIDIR" [get_ports "io_gpio[11]"]


# UART interface
set_property PACKAGE_PIN "K15" [get_ports "i_uart1_rd"]
set_property iostandard "LVCMOS33" [get_ports "i_uart1_rd"]
set_property PIO_DIRECTION "INPUT" [get_ports "i_uart1_rd"]
set_property PACKAGE_PIN "J15" [get_ports "o_uart1_td"]
set_property iostandard "LVCMOS33" [get_ports "o_uart1_td"]
set_property PIO_DIRECTION "OUTPUT" [get_ports "o_uart1_td"]

# UART2 interface (debug port): General Purpose IO
set_property PACKAGE_PIN "H15" [get_ports "i_uart2_rd"]
set_property iostandard "LVCMOS33" [get_ports "i_uart2_rd"]
set_property PIO_DIRECTION "INPUT" [get_ports "i_uart2_rd"]
set_property PACKAGE_PIN "R15" [get_ports "o_uart2_td"]
set_property iostandard "LVCMOS33" [get_ports "o_uart2_td"]
set_property PIO_DIRECTION "OUTPUT" [get_ports "o_uart2_td"]

# JTAG Pmpd JA1
set_property PACKAGE_PIN "Y11" [get_ports "o_jtag_vref"]
set_property iostandard "LVCMOS33" [get_ports "o_jtag_vref"]
set_property PIO_DIRECTION "OUTPUT" [get_ports "o_jtag_vref"]

set_property PACKAGE_PIN "AA11" [get_ports "i_jtag_ntrst"]
set_property iostandard "LVCMOS33" [get_ports "i_jtag_ntrst"]
set_property PIO_DIRECTION "INPUT" [get_ports "i_jtag_ntrst"]

set_property PACKAGE_PIN "Y10" [get_ports "i_jtag_tdi"]
set_property iostandard "LVCMOS33" [get_ports "i_jtag_tdi"]
set_property PIO_DIRECTION "INPUT" [get_ports "i_jtag_tdi"]

set_property PACKAGE_PIN "AA9" [get_ports "i_jtag_tms"]
set_property iostandard "LVCMOS33" [get_ports "i_jtag_tms"]
set_property PIO_DIRECTION "INPUT" [get_ports "i_jtag_tms"]

set_property PACKAGE_PIN "AB11" [get_ports "i_jtag_tck"]
set_property iostandard "LVCMOS33" [get_ports "i_jtag_tck"]
set_property CLOCK_DEDICATED_ROUTE "FALSE" [get_nets "i_jtag_tck"]
set_property PIO_DIRECTION "INPUT" [get_ports "i_jtag_tck"]

set_property PACKAGE_PIN "AB10" [get_ports "o_jtag_tdo"]
set_property iostandard "LVCMOS33" [get_ports "o_jtag_tdo"]
set_property PIO_DIRECTION "OUTPUT" [get_ports "o_jtag_tdo"]
