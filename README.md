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
  AMBA AXI4 interfaces: GPIO, LEDs, UART etc.
+ Configuration and constraint files for ML605 (Virtex6) and KC705 (Kintex7) 
  FPGA boards.
+ Pre-built ROM images with the BootLoader and FW-image that is copying into
  internal SRAM during boot-stage.

## Final result without modification of code

+ LEDs sequential switching;
+ UART outputs "Hello" message.

## How to create and build project using ISE Studio

Use "rocket_soc.xise" project file to build image for the default target ML605
(Virtex6) FPGA board. Do the following steps to change target on KC705 
(Kintex7) board:
+ Remove from the 'techmap' library file SysPLL_v6.vhd and change it on 
  SysPLL_k7.vhd.
+ Remove from the 'work' library file config_v6.vhd and change it
  on config_k7.vhd
+ Rename and overwrite file rocket_soc_k7.ucf to rocket_soc.ucf
+ Change FPGA type of the top-level file in ISE Studio using proper menu.
+ That's all. 

## Simulation with the ModelSim


