rem ---------------------------------------------------------------------------

set TOP_DIR=../

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
set PATH=%ARM_GCC%;%PATH%
set OBJ_DIR=%TOP_DIR%makefiles/objarm
set ELF_DIR=%TOP_DIR%makefiles/binarm
set MAKE_TARGET=make_arm

mkdir objarm
mkdir binarm

:endwork
make -f %MAKE_TARGET% TOP_DIR=%TOP_DIR% OBJ_DIR=%OBJ_DIR% ELF_DIR=%ELF_DIR%


pause
exit
