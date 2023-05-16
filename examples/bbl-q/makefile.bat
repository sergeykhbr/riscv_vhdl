rem ---------------------------------------------------------------------------

rem Perforce paths
set PATH=%RISCV_GCC%;%PATH%

rem elf2rawx bbl-q-noprintf -h -f 0x800000 -l 8 -o bbl-q-noprintf.hex
rem riscv64-unknown-elf-objcopy -O binary bbl-q-noprintf bbl-q-noprintf.bin

elf2rawx bbl-q -h -f 0x800000 -l 8 -o bbl-q.hex
riscv64-unknown-elf-objcopy -O binary bbl-q bbl-q.bin

pause
exit
