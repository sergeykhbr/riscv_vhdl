1. Build libdpiwrapper library using standalone tools. Makefile on linux
      > cd libdpiwrapped/makefiles  
      > make  

2. Define LD_LIBRARY_PATH to run test application.
      > export LD_LIBRARY_PATH=/home/user/riscv_vhdl/libdpiwrapper/linuxbuild/bin  
      > cd ../linuxbuild/bin  
      > ./test  

3. To run simulation:
      > cd rtl/prj/simdpitest  
      > make build  
      > make gui  
