System-On-Chip template based on Rocket-chip (RISC-V ISA). VHDL implementation.
=====================

This repository provides open source System-on-Chip implementation based on
64-bits CPU "Rocket-chip" distributed under BSD license. SOC source files 
either include general set of peripheries, FPGA CADs projects files, own 
implementation of the Windows/Linux debugger and several examples that help
to run your firmware on almost any FPGA boards. 
Satellite Navigation (GPS/GLONASS/Galileo) modules were stubbed in this 
repository and can be requested on
[gnss-sensor.com](http://www.gnss-sensor.com).


## What is Rocket-chip and [RISC-V ISA](http://www.riscv.org)?

RISC-V (pronounced "risk-five") is a new instruction set architecture (ISA) 
that was originally designed to support computer architecture research and 
education and is now set become a standard open architecture for industry 
implementations under the governance of the RISC-V Foundation. RISC-V was 
originally developed in the Computer Science Division of the EECS Department
at the University of California, Berkeley.

Parameterized generator of the Rocket-chip can be found here:
[https://github.com/ucb-bar](https://github.com/ucb-bar)


## Developing functionality (master -> v3.0)

We think that SOC **v2.0** configuration provides good performance and
stability that allow us to run Satellite Navigation firmware and
compute GPS/GLONASS positions. So, generally we will be focused on
software and tools developing for **v3.0** release:

- Porting open source Real-Time Operation System for Internet of Things 
  Devices provided by [Zephyr Project](https://www.zephyrproject.org/). 
  Early support for the Zephyr Project includes Intel Corporation, 
  NXP Semiconductors N.V., Synopsys, Inc. and UbiquiOS Technology Limited.
- Graphical User Interface (GUI) for the debugger based on QT-libraries
  with significantly increasing of the debugger functionality.
- Debugger integration with Python 2.7 to provide scripting tool for the
  tests automation.
   

## Implemented functionality (v2.0)

To get branch *v2.0* use the following git command:

    $ git clone -b v2.0 https://github.com/sergeykhbr/riscv_vhdl.git

This release add to following features to *v1.0*:

- [**Debug Support Unit**](http://sergeykhbr.github.io/riscv_vhdl/dsu_link.html) 
  (DSU) for the access to all CPU registers (CSRs).
- [**10/100 Ethernet MAC with EDCL**](http://sergeykhbr.github.io/riscv_vhdl/eth_link.html) 
  that allows to debug processor from the
  reset vector redirecting UDP requests directly on system bus.
- GNSS engine and RF-mezzanine card support.
- **Test Mode** (DIP[0]=1) that allows to use SOC with or without
  *RF-mezzanine card*.
- Master/Slave AMBA AXI4 interface refactoring.
- [**Debugger Software (C++)**](http://sergeykhbr.github.io/riscv_vhdl/dbg_link.html)
  for Windows and Linux with built-in simulator and plugins support.
- Portable asynchronous FIFO implementation allowing to connect modules to the 
  System BUS from a separate clock domains (ADC clock domain):
- A lot of system optimizations.


## Implemented functionality (v1.0)

The initial *v1.0* release provides base SOC functionality with minimal
set of peripheries. To get this version use:

    $ git clone -b v1.0 https://github.com/sergeykhbr/riscv_vhdl.git

- Proof-of-concept VHDL SOC based on Verilog generated core *"Rocket-chip"*.
- Peripheries with AMBA AXI4 interfaces: GPIO, LEDs, UART, IRQ controller etc.
- Plug'n-Play support.
- Configuration and constraint files for ML605 (Virtex6) and KC705 (Kintex7) 
  FPGA boards.
- Bit-files for ML605 and KC705 boards.
- Pre-built ROM images with the BootLoader and FW-image. FW-image is copied
  into internal SRAM during boot-stage.
- *"Hello World"* example.


## FPGA output in TEST_MODE

FPGA pre-built images either as built from sources without modifications
provide the following output in TEST_MODE that should be manually switched 
using jumper DIP0 (*i_int_clkrf* signal).

* LEDs sequential switching;
* UART outputs Plug'n'Play configuration message with 1 sec period
  (115200 baud).

```
    Boot . . .OK
    # RISC-V: Rocket-Chip demonstration design
    # HW version: 0x20160316
    # FW id: 20160321
    # Target technology: Virtex6
    # AXI4: slv0: GNSS Sensor Ltd.    Boot ROM
    #    0x00000000...0x00001FFF, size = 8 KB
    # AXI4: slv1: GNSS Sensor Ltd.    FW Image ROM
    #    0x00100000...0x0013FFFF, size = 256 KB
    # AXI4: slv2: GNSS Sensor Ltd.    Internal SRAM
    #    0x10000000...0x1007FFFF, size = 512 KB
    # AXI4: slv3: GNSS Sensor Ltd.    Generic UART
    #    0x80001000...0x80001FFF, size = 4 KB
    # AXI4: slv4: GNSS Sensor Ltd.    Generic GPIO
    #    0x80000000...0x80000FFF, size = 4 KB
    # AXI4: slv5: GNSS Sensor Ltd.    Interrupt Controller
    #    0x80002000...0x80002FFF, size = 4 KB
    # AXI4: slv6: GNSS Sensor Ltd.    GNSS Engine stub
    #    0x80003000...0x80003FFF, size = 4 KB
    # AXI4: slv7: GNSS Sensor Ltd.    RF front-end controller
    #    0x80004000...0x80004FFF, size = 4 KB
    # AXI4: slv8: Empty slot
    # AXI4: slv9: GNSS Sensor Ltd.    Ethernet MAC
    #    0x80040000...0x8007FFFF, size = 256 KB
    # AXI4: slv10: GNSS Sensor Ltd.    Debug Support Unit (DSU)
    #    0x80080000...0x8008FFFF, size = 64 KB
    # AXI4: slv11: GNSS Sensor Ltd.    Plug'n'Play support
    #    0xFFFFF000...0xFFFFFFFF, size = 4 KB
      RF front-end init . . .skipped
    !!!!!!! MAC post init !!!!!!!!!
    gbit . . .disable
    EDCL . . .available
    EDCL . . .enable
    PHYADDR = 07
    TxTable = 0x10005b08
    RxTable = 0x10005f08
    !!!!!! MAC Epoch listener!!!!!!
    control = c4000000
    status  = 00000000
    mdio    = 7949384a
    MDIO is busy
    link is OK
    IP = 192.168.0.51
```

## How to build and run FPGA bitfile on ML605 board (Virtex6)

- Open project file for Xilinx ISE14.7 *prj/ml605/rocket_soc.xise*.
- Edit configuration constants in file **work/comfig_common.vhd** if needed.
  (Skip this step by default).
- Generate bit-file and load it into FPGA.
- You should see a single output message in uart port as follow (use button 
  "*Center*" to reset system and repead message):

```
    Boot . . .OK
    ADC clock not found. Enable DIP int_rf.
```
 
- **Switch "ON" DIP[0]** (i_int_clkrf) to enable *TEST_MODE* that enables
  ADC clock generation as sys_clk/4.
- Now you should see plug'n'play information messages with 1 second period.
- For Xilinx KC705 board use Vivado project  *prj/kc705/rocket_chip.xpr*.


## Simulation with ModelSim

1. Open project file *prj/modelsim/rocket.mpf*.
2. Compile project files.
3. If you get an errors for all files remove and re-create the following 
   libraries in ModelSim library view: 
     * techmap
     * ambalib
     * commonlib
     * rocketlib
     * gnsslib
     * work (was created by default)
4. Use *work/tb/rocket_soc_tb.vhd* to run simulation.
5. Testbench allows to check the following things:
     * LEDs switching
     * UART output 
     * Interrupt controller
     * UDP/EDCL transaction via Ethernet
     * Access to CSR via Ethernet + DSU.
     * and other.


## Debugger

Since revision v2.0 we provide open source platform debugger. The pre-built
binaries can be downloaded [here](http://www.gnss-sensor.com/index.php?LinkID=15).
Instruction of how to connect FPGA board via 
[Ethernet](http://sergeykhbr.github.io/riscv_vhdl/eth_link.html) 
your can find here.
Just after successful connection with FPGA target your can interact
with RISC-V SOC by reading/writing memory, CSR register or load
new elf-file.

```
    riscv# csr MCPUID
    CSR[f00] => 8000000000041101
        Base: RV64IAMS
    riscv# read 0x204 20
    [0000000000000200]:  00 00 02 13 00 00 01 93 00 00 01 13 .. .. .. ..
    [0000000000000210]:  .. .. .. .. .. .. .. .. 00 00 03 13 00 00 02 93
    riscv# exit
```

Full debugger configuration including plugins states is stored in file
**config.json**. You can manually define CSR names and addresses, 
enable/disable platform specific functionality, specify files pathes etc.
Start debugger with command argument *-sim* to connect SOC PC-simulator
instead of FPGA board:

```
     ./../linuxbuild/bin/appdbg64g.exe -sim
     c:\myprj\rocket\debugger\bin\appdbg64g.exe -sim
```

This simulator is analog of *spike* tool that is part of tools
provided RISC-V community. But it's implemented as set of plugins for the
Core library where each plugin is an independent device functional model.
Set of created and connected devices through configuration JSON-file forms
SOC platform that can include any number of different devices, including 
GNSS engine or whatever.
To get more information see 
[debugger's description](http://sergeykhbr.github.io/riscv_vhdl/dbg_link.html).


## Setup GCC toolchain

  You can find step-by-step instruction of how to build your own
toolchain on [riscv.org](http://riscv.org/software-tools/). If you would like
to use pre-build GCC binary files and libraries you can download it here:
 
   [Ubuntu GNU GCC 5.1.0 toolchain RV64IMA (256MB)](http://www.gnss-sensor.com/index.php?LinkID=1013)

  Feature of this GCC build is the configuration *RV64IMA* (without FPU).
Default toolchain configuration generates makefile with hardware FPU that 
makes *libc* library incompatible with the *_-msoft-float_* compiler key.

  Just after you download the toolchain unpack it and set environment variable
as follows:

    $ tar -xzvf gnu-toolchain-rv64ima.tar.gz gnu-toolchain-rv64ima
    $ export PATH=/home/your_path/gnu-toolchain-rv64ima/bin:$PATH

If you would like to generate hex-file used for ROM initialization you probably
need tool *'elf2raw'* and *'libfesvr.so'* library that are not part of the GCC
toolchain. I've put them into *'libexttools'* sub-folder and to use them your
should copy files into *usr/bin* directory or define environment variable:

    $ export LD_LIBRARY_PATH=/home/your_path/gnu-toolchain-rv64ima/libexttools


## Build and run 'Hello World' example.

Build example:

    $ cd /home/your_path/rocket_soc/fw/helloworld/makefiles
    $ make

Run debugger console:

    $ ./git_path/debugger/linuxbuild/bin/riscvdbg

Load elf-file via Ethernet using debugger console:
    
    #riscv loadelf bin/helloworld

You should see something like:

```
    riscv# loadelf e:/helloworld
    [loader0]: Loading '.text' section
    [loader0]: Loading '.eh_frame' section
    [loader0]: Loading '.rodata.str1.8' section
    [loader0]: Loading '.rodata' section
    [loader0]: Loading '.data' section
    [loader0]: Loading '.sdata' section
    [loader0]: Loading '.sbss' section
    [loader0]: Loading '.bss' section
    [loader0]: Loaded: 42912 B
```

Just after image loading finished debugger clears reset CPU signal and starts
execution. This example prints only once UART message *'Hello World - 1'*,
so if you'd like to repeat test reload image using **loadelf** command.

Now you can also generate HEX-file for ROM initialization to do that
see other example with **bootrom** implementation

    $ cd rocket_soc/fw/boot/makefiles
    $ make
    $ cd ../linuxbuild/bin

Opened directory contains the following files:
- _bootimage_       - elf-file (not used by SOC).
- _bootimage.dump_  - disassembled file for the verification.
- *_bootimage.hex_* - HEX-file for the Boot ROM intialization.

You can also check *bootimage.hex* and memory dump for consistence:

    #riscv dump 0 8192 dump.hex hex

I hope your also have run firmware on RISC-V system successfully.

My usual FPGA setup is ML605 board and debugger that is running on Windows 7
from Visual Studio project, so other target configurations (linux + KC705)
could contain errors that are fixing with a small delay. Let me know if see one.

## Doxygen project documentation

[http://sergeykhbr.github.io/riscv_vhdl/](http://sergeykhbr.github.io/riscv_vhdl/)

