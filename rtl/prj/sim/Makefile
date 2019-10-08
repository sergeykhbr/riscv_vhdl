include src.mk

TIMESCALE=1ns/10ps
BENCH=asic_top_tb

VHDL_PARAMS=-64 -v93 -smartorder
ELAB_PARAMS=-64 -v93 -namemap_mixgen -timescale ${TIMESCALE}

prep:
	@mkdir -p work
	@mkdir -p ambalib
	@mkdir -p techmap 
	@mkdir -p ethlib
	@mkdir -p misclib 
	@mkdir -p riverlib 
	@mkdir -p commonlib

clr_logs:
	@rm -f *.log
	@rm -f *.log.tmp
	@rm -f ./*.dis ./*.ecs*
	@rm -f ./*.errmsg* ./*.mix


comp: prep clr_logs
	@ncvhdl ${VHDL_PARAMS} -work commonlib 	-f 	lists/commonlib
	@ncvhdl ${VHDL_PARAMS} -work ambalib   	-f 	lists/ambalib
	@ncvhdl ${VHDL_PARAMS} -work techmap   	-f 	lists/techmap
	@ncvhdl ${VHDL_PARAMS} -work ethlib 	-f 	lists/ethlib
	@ncvhdl ${VHDL_PARAMS} -work misclib  	-f 	lists/misclib
	@ncvhdl ${VHDL_PARAMS} -work riverlib  	-f	lists/riverlib
	@ncvhdl ${VHDL_PARAMS} -work work      	-f 	vhdl.f

build: comp
	@ncelab ${ELAB_PARAMS} ${BENCH}
