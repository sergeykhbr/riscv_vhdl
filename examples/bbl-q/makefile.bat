rem ---------------------------------------------------------------------------

rem Perforce paths
set PATH=%RISCV_GCC%;%PATH%

elf2rawx bbl-q -h -f 0x800000 -l 8 -o bbl-q.hex

pause
exit
