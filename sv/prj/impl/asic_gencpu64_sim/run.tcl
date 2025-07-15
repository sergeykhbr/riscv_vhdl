set severity_pack_assert_off {warning}

run 1

set logfilename uart_0.log.tmp
cat /dev/null > $logfilename
set uart_term_cmd [list xterm -geometry 100x50+0+0 -bg black -fg green -T {UART_0 output} -e tail -f $logfilename &]
eval exec $uart_term_cmd
# run
