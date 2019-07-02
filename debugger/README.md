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

### Debugging Dual-Core River CPU SoC on FPGA ML605 board

Brief video description:

  - Core[0] runs Zephyr 6.0
  - Core[1] goes into the Idle Cycle (see examples/boot/) just after init.
  - Manually run OS tests and Drhystone on Core[0] while Core[1] is running and halted
  - Change *npc* register on Core[1] to start function *timestamp_output*
  - Run two cores in parallel and check HW statistic graphs.

[![Dual-Core River on FPGA](https://img.youtube.com/vi/h-MVY0eSb9kaA/hqdefault.jpg)](https://youtu.be/MVY0eSb9kaA)

Information:

  - Source code of everything from this video is already in this repository.
  - Used tools: Xilinx ISE Studio, GCC compiler with debug symbols


## Simulation of the ARMv7 processor based system

Brief video description:

  - Showing system features: display, buttons, LEDs, ADC channel
  - Debugging features: symbol browser, code stepping, breakpoints
  - Interaction with emulated ARM processor from GUI
  - Work with the Stack trace and symbols

[![ARM-based simulator](https://img.youtube.com/vi/h-NNvXWnNEU/hqdefault.jpg)](https://youtu.be/h-NNvXWnNEU)

Information:

  - This video shows system that doesn't exist in the real world as the HW board
  - Not all components of the shown simulation platform available in this repository some 
    of them provided only by request
  - Used tools: GCC compiler for ARM


## Siemens C167 processor based platform emulation (16-bits) video

Brief video description:

  - In this video is used *Digital Twin* (precise simulation platform) of the 
    real medical device
  - Simulation provides access to all standard debugging features plus additional
    capability to emulate hardware faults.
  

Information:

  - Custom platform includes many not published device models: motors, LED displays, analog sensors etc.
  - Keil compiler
  - MAP-file with debug information to load the symbols


## Motorola HC08 processor based platform (8-bits) video

Information:

  - Custom platform includes many not published device models: motors, LED displays, analog sensors etc.
  - CasmHCS08 Pro compiler
  - Binary MAP-file with debug information to load the symbols

