rem ---------------------------------------------------------------------------

set GNU_TOOLS=E:\fw_tools\gnutools;C:\Projects\auriga\Tools\gnutools
set GCC_DIR=E:\fw_tools\gcc-arm-none-eabi-7-2017-q4-major-win32\bin;C:\Projects\auriga\Tools\gcc-arm-none-eabi-7-2017-q4-major-win32\bin

set PATH=%GCC_DIR%;%GNU_TOOLS%;%PATH%

set TOP_DIR=..\
set OBJ_DIR=%TOP_DIR%makefiles\obj
set ELF_DIR=%TOP_DIR%makefiles\bin

mkdir %OBJ_DIR%
mkdir %ELF_DIR%
make -f make_boot TOP_DIR=%TOP_DIR% OBJ_DIR=%OBJ_DIR% ELF_DIR=%ELF_DIR%

pause
exit
