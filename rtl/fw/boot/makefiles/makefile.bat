rem ---------------------------------------------------------------------------

rem Perforce paths
set PATH=E:\fw_tools\SysGCC\risc_v\bin;%PATH%
set PATH=E:\fw_tools\gnutools;E:\fw_tools\gnutools\cyg;%PATH%

set TOP_DIR=..\
set OBJ_DIR=%TOP_DIR%linuxbuild\obj
set ELF_DIR=%TOP_DIR%linuxbuild\bin

mkdir ..\linuxbuild
mkdir %OBJ_DIR%
mkdir %ELF_DIR%
make -f make_boot TOP_DIR=%TOP_DIR% OBJ_DIR=%OBJ_DIR% ELF_DIR=%ELF_DIR%

pause
exit
