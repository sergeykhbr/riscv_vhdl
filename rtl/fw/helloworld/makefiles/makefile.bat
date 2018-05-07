rem ---------------------------------------------------------------------------

rem Perforce paths
set PATH=E:\fw_tools\SysGCC\risc_v\bin;%PATH%
set PATH=E:\fw_tools\gnutools;E:\fw_tools\gnutools\cyg;%PATH%
set PATH=C:\Projects\install\riscv64-gcc-7.2.0\bin;C:\Projects\auriga\Tools\gnutools;%PATH%
set PATH=..\..\elf2raw64\msvc13\Debug;%PATH%

set TOP_DIR=../../
set OBJ_DIR=%TOP_DIR%helloworld/makefiles/obj
set ELF_DIR=%TOP_DIR%helloworld/makefiles/bin

mkdir obj
mkdir bin
make -f make_example TOP_DIR=%TOP_DIR% OBJ_DIR=%OBJ_DIR% ELF_DIR=%ELF_DIR%

pause
exit
