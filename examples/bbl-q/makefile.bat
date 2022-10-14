rem ---------------------------------------------------------------------------

rem Perforce paths
set PATH=%RISCV_GCC%;%PATH%

elf2rawx bbl-q-noprintf -h -f 0x800000 -l 8 -o bbl-q-noprintf.hex

pause
exit
