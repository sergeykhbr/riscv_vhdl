System-On-Chip template based on synthesizable processor compliant with the RISC-V architecture.
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

## What is River CPU?

It's my own implementation of RISC-V ISA used in a several projects including
the multi-sytem Satellite Navigation receiver. It is great for an 
embedded applications with active usage of 64-bits computations (like DSP).  
**River CPU** includes the following tools and features:

1. Source code
    - */debugger/cpu_fnc_plugin*  - Functional RISC-V CPU model.
    - */debugger/cpu_sysc_plugin* - Precise SystemC RIVER CPU model.
    - */rtl/riverlib*      -  synthesisable VHDL model of a 64-bit processor compliant with the RISC-V architecture.
2. Floating Point Unit (FPU)
3. Dual-Core configuration
4. Advanced debugging features
    - Test Access Points (TAPs) via Ethernet, UART and JTAG in one system.
    - System Bus tracer
    - Pipeline statistic (CPI, HW stacktrace) in a real-time on HW level.
    - Plug'n'Play information
5. Integration with GUI from the very beginning.

You can find several **demonstration videos**
[here](https://github.com/sergeykhbr/riscv_vhdl/tree/master/debugger) of working with the 
Dual-Core SoC on FPGA and with the emulated platforms.

My goal is to develop open source fault-tolerant processor with the user-friendly
framework.

## System-on-Chip structure

SoC documentation in [.pdf](docs/riscv_vhdl_trm.pdf) and 
[.html](http://sergeykhbr.github.io/riscv_vhdl/) formats.

![SOC top](docs/doxygen/pics/soc_top_v5.png)

## Performance

Performance analysis is based on very compact
[**Dhrystone v2.1. benchmark**](http://fossies.org/linux/privat/old/dhrystone-2.1.tar.gz/)
application available as the bare-metal test in *$(TOP)/example/dhrystone21*
folder and entirely ported into Zephyr shell (see animated gif below). Benchmark was executed
with enabled (-O0) and disabled (-O2) optimization to define HW and GCC-compiler advantages.
All sources are available and could be run on the simulator or on the 
different FPGA targets.

Target             | Git tag | Dhrystone<br> per sec,<br> -O0 | Dhrystone<br> per sec,<br> -O2 | Information.
-------------------|:-------:|:------------------------------:|:------------------------------:|:------------
RISC-V simulator   | v6.0    | **65652.0** | **76719.0**   | Ubuntu GNU GCC 6.1.0 toolchain RV64IMA custom build
"Rocket" CPU       | v6.0    | -           | **23999.0**   | GCC 6.1.0
"River" CPU        | v6.0    | -           | **35121.0**   | GCC 6.1.0
RISC-V simulator   | latest  | **76824.0** | **176469.0**  | *GCC 7.1.1* with the compressed instructions set
"River" CPU        | latest  | **29440.0** | **89420.0**   | *GCC 7.1.1* with the compressed instructions set
ARM simulator      | latest  | **78451.0** | **162600.0**  | *arm-none-eabi-gcc 7.2.0*, ARM ISA only
Cortex-R5 ARM      | No      | **20561.0** | **42401.0**   | *arm-none-eabi-gcc 7.2.0*, custom FPGA system:<br> Single-Core, MPU enabled, **Caches disabled**
Cortex-R5 ARM      | No      | **54052.0** | **132446.0**  | *arm-none-eabi-gcc 7.2.0*, custom FPGA system:<br> Single-Core, MPU enabled, **Caches enabled**
Cortex-M3 Thumb2   | [arm_vhdl](https://github.com/sergeykhbr/arm_vhdl) | soon       | soon          | *arm-none-eabi-gcc 7.2.0*, custom FPGA system
"LEON3" SPARC V8   | No      | **48229.0** | **119515.0**  | *sparc-elf-gcc 4.4.2*, custom FPGA system

Access to all memory banks and peripheries for all targets (including ARM and Leon3) is made 
in the same clock domain and always is
one clock (without wait-states). So, this benchmark 
result (**Dhrystone per seconds**) shows performance of the CPU with integer 
instructions and degradation of the CPI relative ideal (simulation) case.

CPU         | Clocks-Per-Instruction,<br> CPI | Description.
------------|:-------:|:------------------------------
Cortext-R5  | 1.22    | This is **dual-issue** processor capable to execute a pair of instructions per<br> one clock. It's a very good but quite expensive CPU.
LEON3       | 1.5     | CPI information from [here](https://www.gaisler.com/index.php/products/simulators/tsim).
River       | 1.9     | Free-to-use and highly customizable CPU. This result achieved with enabled 16 KB I-cache.
Cortex-M3   | soon    | RTL is under development.

   **Since the tag 'v7.0' RIVER CPU is the main processor in the system and all issues
     related to Rocket-chip instance will be supported only by request.**

## Repository structure

This repository consists of three sub-projects each in own subfolder:

- **rtl** is the folder with VHDL/Verilog sources of the SOC
  including synthesizable processors *"Rocket"* and *"River"* and peripheries. 
  Source code is portable on almost any FPGA is due to the fact that
  technology dependant modules (like *PLL*, *IO-buffers* 
  etc) instantiated inside of "virtual" components 
  in a similar to Gailser's *[GRLIB](www.gailser.com)* way.  
  Full SOC design without FPU occupies less than 5 % of FPGA resources (Virtex6). 
  *"Rocket-chip"* CPU itself is the modern **64-bits processor 
  with L1-cache, branch-predictor, MMU and virtualization support**.  
  This sub-project also contains:
    * *fw_images*: directory with the ROM images in HEX-format.
    * *prj*: project files for different CADs (Xilinx ISE, ModelSim).
    * *tb*: VHDL testbech of the full system and utilities.
    * *bit_files*: Pre-built FPGA images for ML605 and KC705 boards.
- **examples** folder contains several C-examples that could help start working
  with the RISC-V system:
    * *boot* is the code of the Boot Loader. It is also used for the SRAM 
      initialization with the FW image and it allows to run examples on
      FPGA without using the debugger and external flash memory.
    * *helloworld* the simplest example with UART output.
    * *isrdemo* example with 1 second interrupt from timer and debug output.
    * *zephyr* is ported on RISC-V 64-bits operation system.
      Information about this Real-Time Operation System for Internet of
      Things Devices provided by [Zephyr Project](https://www.zephyrproject.org/).
      Early support for the Zephyr Project includes Intel Corporation,
      NXP Semiconductors N.V., Synopsys, Inc. and UbiquiOS Technology Limited.
- **debugger**. The last piece of the ready-to-use open HW/SW system is
  [Software Debugger (C++)](http://sergeykhbr.github.io/riscv_vhdl/sw_debugger_api_link.html)
  with the full system simulator available as a plug-in.
  Debugger interacts with the target (FPGA or Software Simulator) 
  via [Ethernet](http://sergeykhbr.github.io/riscv_vhdl/eth_link.html)
  using EDCL protocol over UDP. To provide this functionality SOC includes
  [**10/100 Ethernet MAC with EDCL**](http://sergeykhbr.github.io/riscv_vhdl/eth_link.html)
  and [**Debug Support Unit (DSU)**](http://sergeykhbr.github.io/riscv_vhdl/periphery_page_1.html)
  devices on AMBA AXI4 bus.

# Step I: Simple FPGA test.

You can use the pre-built FPGA image (for Xilinx ML605 or KC705 board) and any serial
console application (*putty*, *screen* or other) to run Dhrystone v2.1 benchmark as 
on the animated picture below.

![Zephyr demo](docs/doxygen/pics/zephyr_demo.gif)

1. Unpack and load file image *riscv_soc.bit* from */rtl/bit_files/* into FPGA board.
2. Connect to serial port. I used standard console utility *screen* on Ubuntu.

        $ sudo apt-get install screen
        $ sudo screen /dev/ttyUSB0 115200

3. Use button "*Center*" to reset FPGA system and reprint initial messages (or just press Enter):

To end the session, use *Ctrl-A*, *Shift-K*

# Step II: Build and run Software models with GUI.

At this step we're going to build: functional models of CPU and peripheries,
precise SystemC model of 'River' CPU and RISC-V Debugger with GUI
(MS Visual Studio project for Windows is also available).  
This step **doesn't require any Hardware** and the final result will look as on
the following animated picture:

![Debugger demo](docs/doxygen/pics/debugger_demo.gif)  

There's dependency of two others open source projects:

* **[Qt-libraries](https://www.qt.io/download/)**
* **[SystemC library](http://accellera.org/downloads/standards/systemc)**

1. Download and install Qt-package (checked with version 5.7).
2. Specify environment variable QT_PATH:

        $ export QT_PATH=/home/install_dir/Qt5.7.0/5.7/gcc_64

3. If you would like to run SystemC models download the systemc archive.
4. Unpack and build sources:

        $ tar -xvzf systemc-2.3.1a.tar.gz
        $ cd systemc-2.3.1a
        $ mkdir tmp
        $ cd tmp
        $ ./../configure --prefix=/home/user/systemc-2.3.1a/build
        $ make
        $ make install

5. Specify environment variable SYSTEMC_PATH:

        $ export SYSTEMC_PATH=/home/user/systemc-2.3.1a/build")

   **Note: System Simulator supports blocking and non-blocking accesses to the simulated
   devices. You can request additional information of how to connect your
   SystemC device to this SoC.**  

6. Build project:

        $ cd debugger/makefiles
        $ make

7. In a case of successful build start desired configuration:

        $ cd ../linuxbuild/bin

Start Configuration        | Description
---------------------------|-----------------
$ ./_run_functional_sim.sh | Functional RISC-V Full System Model
$ ./_run_systemc_sim.sh    | Use SystemC Precise Model of RIVER CPU
$ ./_run_fpga_gui.sh       | FPGA board. Default port 'COM3', TAP IP = 192.168.0.51

**Note:** Specify correct serial port in the file *debugger/targets/fpga_gui.json* 
(COM3 -> ttyUSB0) if you run debugger on linux.

**Note:** Instruction of how to connect FPGA board via
Ethernet your can find [here](http://sergeykhbr.github.io/riscv_vhdl/eth_link.html).  
Simulation and Hardware targets use identical EDCL over UDP interface so that 
[Debugger](http://sergeykhbr.github.io/riscv_vhdl/sw_debugger_api_link.html) can work 
with any target using the same set of commands.  

**Debugger doesn't implement any specific interface for the simulation.
Debugger uses only architectural access via TAP (EDCL over UDP) for all targets.**


# Step III: Build FPGA image

Default VHDL configuration enables River CPU with full debug support.

![River top](docs/doxygen/pics/river_top.png)

You can enable usage of "Rocket-chip" CPU instead of "River" disabling the
configuration parameter in */rtl/work/config_common.vhd* 
CFG_COMMON_RIVER_CPU_ENABLE.

1. Open ML605 project file for Xilinx ISE14.7 *prj/ml605/riscv_soc.xise*
   or KC705 project file for Xilinx Vivado *prj/kc705/riscv_soc.xpr*.
2. Edit configuration constants in file **work/config_common.vhd** if needed.
   (Skip this step by default).
3. Use *rtl/work/tb/riscv_soc_tb.vhd"* testbench file to verify
   full system including *CPU*, *UART*, *Timers*, *Ethernet*, *GPIO* etc.
4. Generate bit-file and load it into FPGA.


# Step IV: How to build 64-bits Zephyr v1.6.0 for RISC-V or other custom firmware

As an example we're going to build two programs:

* Zephyr OS kernel with ROM-image generation.
* 'Hello world' example. Then load it into the target using Debugger's command.

## 1. Setup GCC toolchain

  You can find step-by-step instruction of how to build your own
toolchain on [riscv.org](http://riscv.org/software-tools/). If you would like
to use pre-build GCC binary files and libraries you can download it here:

   GCC 7.1 from [SiFive](https://www.sifive.com/products/tools/) for Linux, Windows and macOS  
   GCC 7.1 from [SysProgs](http://gnutoolchains.com/risc-v/) for Windows

I'm on transition stage to a new v7.0 release with implemented Compressed
instructions set (C-extensions). It will allow to use the latest GCC builds without modifications.
Some fatal errors can be found during this time, sorry.

Previous obsolete GCC builds:

* Upto release tag v6.0 was used
   [Ubuntu GNU GCC 6.1.0 toolchain RV64IMA (204MB)](http://www.gnss-sensor.com/index.php?LinkID=1017)  

* Upto release tag v3.1 was used
   [Ubuntu GNU GCC 5.1.0 toolchain RV64IMA (256MB)](http://www.gnss-sensor.com/index.php?LinkID=1013)

If you would like to generate hex-file and use it for ROM initialization you can use
*'elf2hex'* and *'libfesvr.so'* library from the GNU toolchain but I suggest to use my version
of such tool *'elf2raw64'*. I've put this binary into pre-built GCC archive 'gnu_toolchain-rv64/bin'. 
If *elf2raw64* conflicts with installed LIBC version re-build it from *examples/elf2raw64/makefiles*
directory.

## 2. Patch and build Zephyr OS v1.6.0 binary

    $ mkdir zephyr_160
    $ cd zephyr_160
    $ git clone https://github.com/zephyrproject-rtos/zephyr.git
    $ cd zephyr
    $ git checkout tags/v1.6.0
    $ cp ../../riscv_vhdl/zephyr/v1.6.0-riscv64-base.diff .
    $ cp ../../riscv_vhdl/zephyr/v1.6.0-riscv64-exten.diff .
    $ git apply v1.6.0-riscv64-base.diff
    $ git apply v1.6.0-riscv64-exten.diff

The first patch adds base functionality for RISC-V 64-bits architecture.  
The second one extends it by adding Dhrystone 2.1. benchmark and
MS Visual Studio target and maybe something else.

Build elf-file:

    $ export ZEPHYR_BASE=/home/zephyr_160/zephyr
    $ cd zephyr/samples/shell
    $ make ARCH=riscv64 CROSS_COMPILE=/home/your_path/gnu-toolchain-rv64ima/bin/riscv64-unknown-elf- BOARD=riscv_gnss 2>&1 | tee _err.log

Create HEX-image for ROM initialization. I use own analog of the *elf2raw*
utility named as *elf2raw64*. You can find it in GNU tools archive.

    $ elf2raw64 outdir/riscv_gnss/zephyr.elf -h -f 262144 -l 8 -o fwimage.hex

Flags:

    -h        -- specify HEX format of the output file.
    -f 262144 -- specify total ROM size in bytes.
    -l 8      -- specify number of bytes in one line (AXI databus width). Default is 16.

Copy *fwimage.hex* to rtl subdirectory

    $ cp fwimage.hex ../../../rtl/fw_images

## 3. Debug Zephyr kernel with debug symbols.

Use the following debugger's console commands to load symbols information
from elf-file:

    riscv# loadelf zephyr.elf
    riscv# loadelf zephyr.elf nocode

The second command loads debug information without target reprogramming.

## 4. Build and run custom FW like 'Hello World' example.

Build example:

    $ cd /your_git_path/examples/helloworld/makefiles
    $ make

Run Risc-V Debugger application:

    $ ./your_git_path/debugger/linuxbuild/bin/_run_functional_sim.sh

Load elf-file using debugger's console:

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

Just after image loading has been finished debugger clears reset CPU signal.
Start the simulation manually (F5) if the processor was in 'halt' state.  
This example prints only once UART message *'Hello World - 1'*,
so if you'd like to repeat test reload image using **loadelf** command.

Now we can also generate HEX-file for ROM initialization to do that
see other example with **bootrom** implementation

    $ cd examples/boot/makefiles
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

## 5. Example of debug session with RF front-end and GNSS IPs on ML605 board.

![GUI gnss](docs/doxygen/pics/dbg_gnss.png)


## Versions History

### Implemented functionality (v6.0)

- GNSS IPs successfully integrated into RISC-V based SoC.
- Add Test Access (TAP) over Serial port.
- Add GUI integration with Open Street Maps and position tracking.
- Add performance analisys tool into GUI.

### Implemented functionality (v5.1)

- "RIVER" critical bugs fixed:Not decoded  SRAI instrucion, missed exception generation.
- Zephyr v1.6.0 ported with *unikernel* instead of the obsolete *nanokernel*.

### Implemented functionality (v5.0)

- New CPU implemented ("RIVER").
- "Rocket-chip" CPU updated to date 18 Jan 2017. TileLink interface was totally redesigned.
- SystemC support was added with the precise CPU model and VCD-stimulus generator.
- Debugger functionality is now oriented only on RIVER implementation
  and includes a lot of new features: breakpoints, disassembler,
  CPI meter and others.
- AXI bus controller significantly improved

### Implemented functionality (v4.0)

- Support new revision of User-Level ISA Spec. 2.1 and Privileged spec. 1.9.
- FW will be binary incompatible with the previous Rocket-chip CPU (changed CSR's 
indexes, instruction ERET removed, new set of instructions xRET was added etc).
- GCC versions (5.x) becomes obsolete.
- FPU enabled by default and pre-built GCC 6.x with --hard-float provided.
- HostIO bus removed.
- HW Debug capability significantly affetcted by new DebugUnit, but Simulation
significantly improved.
- Updated bootloader and FW will become available soon.

### Implemented functionality (v3.1)

To get branch *v3.1* use the following git command:

    $ git clone -b v3.1 https://github.com/sergeykhbr/riscv_vhdl.git

This is the last revision of the RISC-V SOC based on ISA version 1.9.
All afterwards updates will be **binary incompatible** with this tag.
Tag v3.1 adds:

- New Zephyr Kernel with the shell autocompletion.
- Significantly updated GUI of the debugger.

**Use tag v3.1 and GCC 5.1.0 instead of latest revision while release v4.0
won't ready. GCC 6.1.0 and 5.1.0 are binary incompatible either as SoC itself!**


### Implemented functionality (v3.0)

To get branch *v3.0* use the following git command:

    $ git clone -b v3.0 https://github.com/sergeykhbr/riscv_vhdl.git

- Ported open source Real-Time Operation System for Internet of Things
  Devices provided by [Zephyr Project](https://www.zephyrproject.org/).
- Benchmark *Dhrystone v2.1* run on FPGA and Simulator with published results.
- Testmode removed. *'gnsslib'* fully disabled.
- Graphical User Interface (GUI) for the debugger based on QT-libraries
  with significantly increasing of the debugger functionality.

### Implemented functionality (v2.0)

To get branch *v2.0* use the following git command:

    $ git clone -b v2.0 https://github.com/sergeykhbr/riscv_vhdl.git

This release add to following features to *v1.0*:

- *Debug Support Unit* (DSU) for the access to all CPU registers (CSRs).
- *10/100 Ethernet MAC with EDCL* that allows to debug processor from the
  reset vector redirecting UDP requests directly on system bus.
- GNSS engine and RF-mezzanine card support.
- **Test Mode** (DIP[0]=1) that allows to use SOC with or without
  *RF-mezzanine card*.
- Master/Slave AMBA AXI4 interface refactoring.
- *Debugger Software (C++)* for Windows and Linux with built-in simulator 
  and plugins support.
- Portable asynchronous FIFO implementation allowing to connect modules to the
  System BUS from a separate clock domains (ADC clock domain):
- A lot of system optimizations.


### Implemented functionality (v1.0)

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


## Doxygen project documentation

[http://sergeykhbr.github.io/riscv_vhdl/](http://sergeykhbr.github.io/riscv_vhdl/)
