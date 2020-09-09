# River JTAG support

1. Build RISC-V toolchain from here:
[https://github.com/riscv/riscv-tools](https://github.com/riscv/riscv-tools)

        $ git clone --recursive https://github.com/riscv/riscv-tools.git
        $ cd ~/riscv-tools
        $ export RISCV=/home/myfolder/riscv
        $ CC=gcc-5 CXX=g++-5 ./build.sh

2. Run openocd:

        $ sudo ./openocd -f ./kc705_river_1core.cfg

3. Run gdb and connect to openocd server:

        $ cd riscv/bin/
        $ riscv64-unknown-elf-gdb
        (gdb) target remote localhost:3333

4. Now you should be able to debug target board

TODO:  picture with connected J-Link and kc705 board

TODO:  cable and connector pinouts
