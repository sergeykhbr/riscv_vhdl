rem ---------------------------------------------------------------------------

rem Perforce paths
set PATH=%RISCV_GCC%;%PATH%

set TOP_DIR=../../
set OBJ_DIR=%TOP_DIR%fasttask/makefiles/obj
set ELF_DIR=%TOP_DIR%fasttask/makefiles/bin

mkdir obj
mkdir bin
make -f make_gnssfw TOP_DIR=%TOP_DIR% OBJ_DIR=%OBJ_DIR% ELF_DIR=%ELF_DIR%

pause
exit
