System-On-Chip template based on Rocket-chip (RISC-V ISA). VHDL implementation.
=====================

This repository contains all neccessary files to build ready-to-use 
top-level of the System-on-Chip with the CPU and common set of the peripheries
for some well-known development FPGA boards. 

## What is Rocket-chip and [RISC-V ISA](http://www.riscv.org)?

RISC-V (pronounced "risk-five") is a new instruction set architecture (ISA) 
that was originally designed to support computer architecture research and 
education and is now set become a standard open architecture for industry 
implementations under the governance of the RISC-V Foundation. RISC-V was 
originally developed in the Computer Science Division of the EECS Department
at the University of California, Berkeley.

Parameterized generator of the Rocket-chip can be found here:
[https://github.com/ucb-bar](https://github.com/ucb-bar)
   

## Implemented functionality

+ Pre-configured single "Rocket" core (Verilog) integrated in our VHDL top
  level.
+ System-on-Chip top-level with the common set of peripheries with the 
  AMBA AXI4 interfaces: GPIO, LEDs, UART, IRQ controller etc.
+ Configuration and constraint files for ML605 (Virtex6) and KC705 (Kintex7) 
  FPGA boards.
+ Pre-built ROM images with the BootLoader and FW-image. FW-image is copied
  into internal SRAM during boot-stage.
+ "Hello World" example

## Final result without modification of code

+ LEDs sequential switching;
+ UART outputs Plug'n'Play configuraiton message with the 1 sec period
  (115200 baud).

## How to create and build project using ISE Studio

  Use "rocket_soc.xise" project file to build image for the default target ML605
(Virtex6) FPGA board or "rocket_soc_kc705.xdc" for the KC705 board. 
Do the following steps to change target on any unsupported board yet:
+ Generate System PLL for your FPGA/ASIC with the similar to outputs pins to
  'SysPLL_v6.vhd' and 'SysPLL_k7.vhd'.
+ Add component declaration into 'techmap/pll/syspll.vhd' file and put it
  into the library 'techmap'.
+ Add you own target name into 'techmap/gencomp/gencomp.vhd' file.
+ Create new target configuration file 'config_xx.vhd' in the 'work' directory
  and library and setup there constant defintion:
    - CFG_FABTECH
    - CFG_MEMTECH
    - CFG_PADTECH
    - CFG_JTAGTECH
+ Use your new 'config_xx.vhd' file instead of default config_v6.vhd.
+ Overwrite constraint file 'rocket_soc.ucf' with the proper pins assignments.
+ Change FPGA type of the top-level file in ISE Studio using proper menu.
+ Make and load *.bit file into FPGA.

## Simulation with the ModelSim

1. Create new project.
2. Create libraries with the following names using ModelSim 'libraries' view:
       + techmap
       + commonlib
       + rocketlib
       + work (was created by default)
3. Using project browser create the same folders structure like in the 
   'riscv_vhdl'. On the top level of the project should be folders matching
   to the libraries names and they should include all exist sub-folders.
4. Add existing files into the proper folder using righ-click menu of the
   project browser in ModelSim.
       + Add all files ".._tech.vhd" and ".._inferred.vhd"
       + Don't include files ".._v6.vhd", ".._k7.vhd" etc.
5. Make sure that all files in libraries folder will be compiled into appropriate
   VHDL library. By default all files will be compiled into 'work' library.
6. Use 'config_common.vhd' and 'config_msim.vhd' to configurate target for the
   behaviour simualtion.
7. Use 'work/tb/rocket_soc_tb.vhd' to run simulation.
8. Default firmware does the following things:
     - Switching LEDs;
     - Print information into UART;
     - Check Interupt controller
     - and other.

## Firmware example

  Use 'fw/boot/' example as a "Hello World" example. It implements general
functionality, such as:
+ Initial Rocket-chip boot-up
+ Interrupt handling setup
+ UART output
+ LED switching
+ Target type auto-detection (RTL simulatation or not)
+ Coping image from ROM to SRAM using libc method memcpy()


## Doxygen project documentation

[http://sergeykhbr.github.io/riscv_vhdl/](http://sergeykhbr.github.io/riscv_vhdl/)

