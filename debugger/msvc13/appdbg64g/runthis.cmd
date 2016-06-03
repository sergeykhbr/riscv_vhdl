halt
loadelf ../../../20160428_zephyr/zephyr/samples/shell/outdir/zephyr.elf
c 1
regs
c
-["wait",2000]
-["uart0","ping"]
