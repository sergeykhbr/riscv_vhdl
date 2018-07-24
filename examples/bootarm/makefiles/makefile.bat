rem ---------------------------------------------------------------------------

set GCC_DIR=E:\fw_tools\gcc-arm-none-eabi-7-2017-q4-major-win32\bin;C:\Projects\auriga\Tools\gcc-arm-none-eabi-7-2017-q4-major-win32\bin
set GNU_TOOLS=E:\fw_tools\gnutools;C:\Projects\auriga\Tools\gnutools

set PATH=%GCC_DIR%;%GNU_TOOLS%;%PATH%

set TOP_DIR=..\
set OBJ_DIR=%TOP_DIR%linuxbuild\obj
set ELF_DIR=%TOP_DIR%linuxbuild\bin

mkdir ..\linuxbuild
mkdir %OBJ_DIR%
mkdir %ELF_DIR%
make -f make_boot TOP_DIR=%TOP_DIR% OBJ_DIR=%OBJ_DIR% ELF_DIR=%ELF_DIR%

pause
exit
