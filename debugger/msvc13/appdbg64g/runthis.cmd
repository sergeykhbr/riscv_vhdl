//halt
//loadelf ../../../20160428_zephyr/zephyr/samples/shell/outdir/zephyr.elf
//c 1
//regs
//c
-["wait",5000]
-["uart0","ping"]
-["wait",5000]
-["uart0","ticks"]
//-["wait",3000]
//-["uart0","highticks"]
//-["wait",3000]
//-["uart0","help"]
