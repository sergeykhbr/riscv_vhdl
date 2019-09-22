1. Setup env. variable $(DPI_GCC)
   DPI_GCC=D:\soft\questasim64_10.4c\gcc-4.5.0-mingw64\bin;D:\Development\Tools\gnutools 

2. Build libdpiwrapper library using standalone tools (dll/so)
   MinGW for a speicifc version for windows only. MSVC cannot be used due the binary
   incompatibility. Makefile on linux
      > cd libdpiwrapped/makefiles
      > makefile.bat (on windows + mingw)
      > make (on linux)

3. Define LD_LIBRARY_PATH to run test application.
      > export LD_LIBRARY_PATH=/home/user/riscv_vhdl/libdpiwrapper/linuxbuild/bin
      > cd ../linuxbuild/bin
      > ./test
 
4. Edit file "questasim64_10.4c/modelsim.ini" variable:
     DpiCppPath = $MODEL_TECH/../gcc-4.5.0-mingw64/bin/gcc

5. Run simulation with linked library (Questa command)
     vsim -novopt -c -sv_lib ../../../libdpiwrapper/linuxbuild/bin/libdpiwrapper system_top

6. Start -all Simulation

7. See 'dpilib.log' in the project directory for the additional debug information
     rtl/prj/questadpi1/dpilib.log (default on windows and linux)
