# MIPT University
# sergeykhbr@gmail.com
#

include src.mk

TIMESCALE=1ns/10ps
BENCH=kc705_top_tb
CYCLES=30000

INC=+incdir+${RTL_HOME}
INC+=+incdir+${RTL_HOME}/techmap/cdc_axi_sync/kc705
INC+=+incdir+${VIPS_HOME}/ddr3
VLOG_PARAMS_TB=-64 -sv -work work
ELAB_PARAMS=-64 -v93 -namemap_mixgen -timescale ${TIMESCALE} -access +rwc
SIM_OPTS = -64 -input ./ncsim_settings work.${BENCH} +cycles=${CYCLES} \
           +UART_LOG_PATH=uart

GLOBAL_DEFINE+=+define+TARGET_KC705

#================================================================================================


#================================================================================================
# Slurm controls
#================================================================================================

#SRUN=@srun --pty -c 12 --exclude=node-b01,node-b02
SRUN=@srun --pty -c 4

prep:
	@mkdir -p work

clr_logs:
	@echo "==========================================================================================="
	@echo " START CLEAR LOGS: "
	@echo "==========================================================================================="
	@rm -f *.log
	@rm -f *.log.tmp
	@rm -f ./*.dis ./*.ecs*
	@rm -f ./*.errmsg* ./*.mix


comp: prep clr_logs
	@echo "==========================================================================================="
	@echo " START COMPILATION: "
	@echo "==========================================================================================="

	${SRUN} xmvlog ${VLOG_PARAMS_TB} ${INC} ${GLOBAL_DEFINE} ../kc705/config_target_pkg.sv
	${SRUN} xmvlog ${VLOG_PARAMS_TB} ${INC} ${GLOBAL_DEFINE} -f ${LISTS_HOME}/ambalib.f
	${SRUN} xmvlog ${VLOG_PARAMS_TB} ${INC} ${GLOBAL_DEFINE} -f ${LISTS_HOME}/riverlib.f
	${SRUN} xmvlog ${VLOG_PARAMS_TB} ${INC} ${GLOBAL_DEFINE} -f ${LISTS_HOME}/techmap.f
	${SRUN} xmvlog ${VLOG_PARAMS_TB} ${INC} ${GLOBAL_DEFINE} -f ${LISTS_HOME}/techmap_ddr_kc705.f
	${SRUN} xmvlog ${VLOG_PARAMS_TB} ${INC} ${GLOBAL_DEFINE} -f ${LISTS_HOME}/misclib.f
	${SRUN} xmvlog ${VLOG_PARAMS_TB} ${INC} ${GLOBAL_DEFINE} -f ${LISTS_HOME}/riscv_soc.f
	${SRUN} xmvlog ${VLOG_PARAMS_TB} ${INC} ${GLOBAL_DEFINE} -f ../kc705/kc705_top.f
	${SRUN} xmvlog ${VLOG_PARAMS_TB} ${INC} ${GLOBAL_DEFINE} -f kc705_top_sim.f


build: comp
	@echo "==========================================================================================="
	@echo " START BUILD: "
	@echo "==========================================================================================="
	${SRUN} xmelab ${ELAB_PARAMS} ${BENCH}

gui:clr_logs
	@echo "==========================================================================================="
	@echo " START GUI: "
	@echo "==========================================================================================="
	${SRUN} xmsim ${SIM_OPTS} -gui -input run.tcl

gui_full: clean build gui

mce_flow: prep clr_logs
	@srun --pty -c 12 --exclude=node-b01,node-b02 xrun -gui -mce \
	-timescale 1ns/10ps \
	-access -rwc \
    +UART_LOG_PATH=uart -input run.tcl  ${VLOG_PARAMS_TB} ${INC} ${GLOBAL_DEFINE} \
	../kc705/config_target_pkg.sv \
	-f ${LISTS_HOME}/ambalib.f \
	-f ${LISTS_HOME}/riverlib.f \
	-f ${LISTS_HOME}/techmap.f \
	-f ${LISTS_HOME}/techmap_ddr_kc705.f \
	-f ${LISTS_HOME}/misclib.f \
	-f ${LISTS_HOME}/riscv_soc.f \
	-f ../kc705/kc705_top.f \
	-f kc705_top_sim.f \
	-top kc705_top_tb \
	-mce_top kc705_top_tb.tt


clean:
	@echo "==========================================================================================="
	@echo " START CLEAN: "
	@echo "==========================================================================================="
	@rm -f *.log *.key *.diag *.err
	@rm -rf work
	@rm -rf waves* INCA_* .simvision
	@rm -f ./*.tmp
	@rm -f ./*.dis ./*.ecs*
	@rm -f ./*.errmsg* ./*.mix
	@rm -f ./*.X
	@rm -rf ./*.shm
	@rm -rf ./xcelium.d
	@rm -rf ./mcesim.run

