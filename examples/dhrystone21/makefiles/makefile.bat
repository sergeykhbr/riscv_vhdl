rem ---------------------------------------------------------------------------

set TOP_DIR=../
set GNU_TOOLS=E:\fw_tools\gnutools;C:\Projects\auriga\Tools\gnutools

if "%1"=="arm" goto arm_target

:riscv_target
set PATH=%RISCV_GCC%;%PATH%
set OBJ_DIR=%TOP_DIR%makefiles/obj
set ELF_DIR=%TOP_DIR%makefiles/bin
set MAKE_TARGET=make_riscv

mkdir obj
mkdir bin

goto endwork

:arm_target
set GCC_DIR=E:\fw_tools\gcc-arm-none-eabi-7-2017-q4-major-win32\bin;C:\Projects\auriga\Tools\gcc-arm-none-eabi-7-2017-q4-major-win32\bin
set OBJ_DIR=%TOP_DIR%makefiles/objarm
set ELF_DIR=%TOP_DIR%makefiles/binarm
set MAKE_TARGET=make_arm

mkdir objarm
mkdir binarm

:endwork
set PATH=%GCC_DIR%;%GNU_TOOLS%;%PATH%
make -f %MAKE_TARGET% TOP_DIR=%TOP_DIR% OBJ_DIR=%OBJ_DIR% ELF_DIR=%ELF_DIR%


pause
exit
