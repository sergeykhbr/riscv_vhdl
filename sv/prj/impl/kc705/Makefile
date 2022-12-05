all: run

prep:
	export LC_ALL=en_US.UTF-8
	export LANG=en_US.UTF-8

run_batch: prep
	@vivado -mode batch -source run_nonproj_flow.tcl

run_dbg: prep
	@vivado -mode batch -source run_dbg_flow.tcl

run_rtl: prep
	# Run RTL in GUI mode:
	@vivado -source run_nonproj_flow.tcl -tclargs -strategy rtl

run_synth: prep
	# Run Synthesis in GUI mode:
	@vivado -source run_nonproj_flow.tcl -tclargs -strategy synth

run: prep
	@vivado -source run_nonproj_flow.tcl

clean:  
	@rm -rf *.log; \
	rm -rf *.jou; \
	rm -rf *.str; \
	rm -rf .Xil; \
	rm -rf .cache \
        rm -rf output
