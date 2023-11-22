rem ---------------------------------------------------------------------------

set PATH=%RISCV_GCC%;%PATH%

set TOP_DIR=../../
set OBJ_DIR=%TOP_DIR%cpp_demo/makefiles/obj
set ELF_DIR=%TOP_DIR%cpp_demo/makefiles/bin

mkdir obj
mkdir bin
make -f make_cpp_demo TOP_DIR=%TOP_DIR% OBJ_DIR=%OBJ_DIR% ELF_DIR=%ELF_DIR%

pause
exit
