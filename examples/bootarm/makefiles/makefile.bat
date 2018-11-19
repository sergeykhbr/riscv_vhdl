rem ---------------------------------------------------------------------------

set ARM_GCC=C:\Projects\auriga\Tools\gcc-arm-none-eabi-7-2017-q4-major-win32\bin
set RISCV_GCC=C:\Projects\install\riscv64-gcc-7.2.0\bin
set GNU_TOOLS=C:\Projects\auriga\Tools\gnutools

set PATH=%ARM_GCC%;%RISCV_GCC%;%GNU_TOOLS%;%PATH%

set TOP_DIR=..\
set OBJ_DIR=%TOP_DIR%linuxbuild\obj
set ELF_DIR=%TOP_DIR%linuxbuild\bin

mkdir ..\linuxbuild
mkdir %OBJ_DIR%
mkdir %ELF_DIR%
make -f make_boot TOP_DIR=%TOP_DIR% OBJ_DIR=%OBJ_DIR% ELF_DIR=%ELF_DIR%

pause
exit
