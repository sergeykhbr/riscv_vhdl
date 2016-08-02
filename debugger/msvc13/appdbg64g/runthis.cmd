//halt
//loadelf ../../../20160428_zephyr/zephyr/samples/shell/outdir/zephyr.elf
//c 1
//regs
//c
-["wait",500]
-["uart0","ping\r"]
-["wait",500]
-["uart0","ticks\r"]
-["wait",300]
-["uart0","highticks\r"]
-["wait",300]
-["uart0","help\r"]
