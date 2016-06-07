halt
//loadelf ../../../20160428_zephyr/zephyr/samples/shell/outdir/zephyr.elf
c 1
regs
c
-["wait",2000]
-["uart0","ping"]
-["wait",2000]
-["uart0","ticks"]
-["wait",2000]
-["uart0","highticks"]
-["wait",2000]
-["uart0","help"]
