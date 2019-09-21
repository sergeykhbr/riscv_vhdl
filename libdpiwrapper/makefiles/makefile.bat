rem ---------------------------------------------------------------------------

rem Perforce paths
set PATH=%DPI_GCC%;%PATH%

set TOP_DIR=..\
set OBJ_DIR=%TOP_DIR%linuxbuild\obj
set ELF_DIR=%TOP_DIR%linuxbuild\bin

if "%1"=="clean" goto clean_all

mkdir ..\linuxbuild
mkdir %OBJ_DIR%
mkdir %ELF_DIR%
make -f make_libdpiwrapper TOP_DIR=%TOP_DIR% OBJ_DIR=%OBJ_DIR% ELF_DIR=%ELF_DIR%
goto end

:clean_all
rmdir /S /Q ..\linuxbuild

:end
pause
exit
