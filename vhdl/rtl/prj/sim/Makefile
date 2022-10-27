# MIPT University
# sergeykhbr@gmail.com
#

include src.mk

TIMESCALE=1ns/10ps
BENCH=asic_top_tb
CYCLES=30000

INC=+incdir+${RTL_HOME}
VLOG_PARAMS_TB=-64 -sv -work work
VHDL_PARAMS=-64 -v93 -smartorder
ELAB_PARAMS=-64 -v93 -namemap_mixgen -timescale ${TIMESCALE} \
            -access +rwc
SIM_OPTS = -64 -input ./ncsim_settings work.${BENCH} +cycles=${CYCLES}

SRUN=@srun --pty -c 12

prep:
	@mkdir -p work
	@mkdir -p ambalib
	@mkdir -p techmap 
	@mkdir -p ethlib
	@mkdir -p misclib 
	@mkdir -p riverlib 
	@mkdir -p commonlib
	@mkdir -p gnsslib 

clr_logs:
	@rm -f *.log
	@rm -f *.log.tmp
	@rm -f ./*.dis ./*.ecs*
	@rm -f ./*.errmsg* ./*.mix


comp: prep clr_logs
	${SRUN} xmvlog ${VLOG_PARAMS_TB} ${INC}   	-f	vlog.f
	${SRUN} xmvhdl ${VHDL_PARAMS} -work commonlib 	-f 	lists/commonlib.f
	${SRUN} xmvhdl ${VHDL_PARAMS} -work ambalib   	-f 	lists/ambalib.f
	${SRUN} xmvhdl ${VHDL_PARAMS} -work techmap   	-f 	lists/techmap.f
	${SRUN} xmvhdl ${VHDL_PARAMS} -work ethlib 	-f 	lists/ethlib.f
	${SRUN} xmvhdl ${VHDL_PARAMS} -work misclib  	-f 	lists/misclib.f
	${SRUN} xmvhdl ${VHDL_PARAMS} -work riverlib  	-f	lists/riverlib.f
	${SRUN} xmvhdl ${VHDL_PARAMS} -work gnsslib  	-f	lists/gnsslib.f
	${SRUN} xmvhdl ${VHDL_PARAMS} -work work      	-f 	vhdl.f

build: comp
	${SRUN} xmelab ${ELAB_PARAMS} ${BENCH}

gui:clr_logs
	${SRUN} xmsim ${SIM_OPTS} -gui

clean:
	@rm -f *.log *.key *.diag *.err
	@rm -rf gnsslib commonlib ambalib techmap ethlib misclib riverlib work
	@rm -rf waves* INCA_* .simvision
	@rm -f ./*.tmp
	@rm -f ./*.dis ./*.ecs*
	@rm -f ./*.errmsg* ./*.mix
	@rm -f ./*.X
	@rm -rf ./*.shm
