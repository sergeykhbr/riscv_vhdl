include src.mk

TIMESCALE=1ns/10ps
BENCH=system_top
CYCLES=30000

VLOG_PARAMS=-64 -sv +define+functional
INC=+incdir+${RTL_HOME}/work/tb/dpi1/
ELAB_PARAMS=-64 -v93 -timescale ${TIMESCALE} \
            -access +rwc
SIM_OPTS = -64 -input ./ncsim_settings work.${BENCH} +cycles=${CYCLES} \
           -sv_lib ${TOP_ROOT}/../libdpiwrapper/linuxbuild/bin/libdpiwrapper.so


prep:
	@mkdir -p work

clr_logs:
	@rm -f *.log
	@rm -f *.log.tmp
	@rm -f ./*.dis ./*.ecs*
	@rm -f ./*.errmsg* ./*.mix


comp: prep clr_logs
	@ncvlog ${VLOG_PARAMS} ${INC}   	-f	vlog.f

build: comp
	@ncelab ${ELAB_PARAMS} ${BENCH}

gui:clr_logs
	@ncsim ${SIM_OPTS} -gui
