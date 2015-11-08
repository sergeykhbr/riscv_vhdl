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
[https://github.com/ucb-ba/](https://github.com/ucb-bar)
   

## Implemented functionality
+ Rocket-chip core pre-configured for the usage in this VHDL top level.
+ System-on-Chip top-level with the common set of peripheries (GPIO, LEDs
  and UART) and AMBA AXI4 interface.
+ Configuration and constraint files for ML605 (Virtex6) and KL705 (Kintex7) 
  FPGA boards.
+ Pre-built ROM images with the BootLoader and FW-image that is copying into
  internal SRAM during boot-stage.

## Result without modification of code
+ LEDs switching;
+ UART output "Hello" message.

