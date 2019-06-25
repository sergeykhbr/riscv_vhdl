Universal Platform Simulator and Debugger.
=====================

This repository folder contains source code of the Unversal Platform Simulator
with capability to debug real hardware (fpga/asic) using absolutely the
same set of features.

JSON-configuration files in sub-folder *./targets* defines the structure
and capabilities of any platform:

- Processor and ISA architecture
- Set of peripheries
- Remote access and Python support
- Enable/disable GUI implemented as the independent plugin based on Qt-libraries

## Demonstration video of 64-bits RISC-V processor

# 1. FPGA board ML605 with River core inside

# 2. Simulation

Information:

  - Fully opened platform from this repository loaded with Zephy OS firmware.
  - GCC compiler
  - ELF-file with enabled debug information to load the symbols


## ARMv7 processor based platform emulation video

[![ARM-based simulator](https://img.youtube.com/vi/h-NNvXWnNEU/hqdefault.jpg)](https://youtu.be/h-NNvXWnNEU)

Information:

  - Demonstration platform based on ARM processor model from this repository (more details by request)
  - GCC compiler
  - ELF-file with enabled debug information to load the symbols


## Siemens C167 processor based platform emulation (16-bits) video

Information:

  - Custom platform includes many not published device models: motors, LED displays, analog sensors etc.
  - Keil compiler
  - MAP-file with debug information to load the symbols


## Motorola HC08 processor based platform (8-bits) video

Information:

  - Custom platform includes many not published device models: motors, LED displays, analog sensors etc.
  - CasmHCS08 Pro compiler
  - Binary MAP-file with debug information to load the symbols

