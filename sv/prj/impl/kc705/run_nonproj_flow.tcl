# A Vivado script that demonstrates a very simple RTL-to-bitstream batch flow
#
# NOTE: typical usage would be "vivado -mode tcl -source create_bft_batch.tcl"
#
#

# Iteration counter for phys_opt:
set NLOOPS 5

# Configure basic attributes:
	#set init true
set load_files true
set load_constr true
set strategy basic

# Some Vivado optimization properties:
#set_property CARRY_REMAP 2 [get_cells -hier -filter {ref_name == CARRY4}]

# Parse input command line attributes:
set arg_len_var [llength $argv]
puts $arg_len_var
if {$arg_len_var > 0} {
   for {set i 0} {$i < $arg_len_var} {incr i} {

      # Bind "strategy" attribute:
      if {([lindex $argv $i] == "-strategy") && ([expr {$i + 1}] <= $arg_len_var)} {
         puts [lindex $argv [expr {$i + 1}]]
         set strategy [lindex $argv [expr {$i + 1}]]
         #set top_level debug
      }

      # Bind "opt_param" attribute:
      # Avalible: "lo", "hi"
      if {([lindex $argv $i] == "-opt_param") && ([expr {$i + 1}] <= $arg_len_var)} {
         puts [lindex $argv [expr {$i + 1}]]
         set opt_param [lindex $argv [expr {$i + 1}]]
      }

      # Bind "phys_opt_param" attribute:
      # Avalible: "lo", "hi"
      if {([lindex $argv $i] == "-phys_opt_param") && ([expr {$i + 1}] <= $arg_len_var)} {
         puts [lindex $argv [expr {$i + 1}]]
         set phys_opt_param [lindex $argv [expr {$i + 1}]]
      }

      # Bind "reports" attribute:
      if {([lindex $argv $i] == "-reports") && ([expr {$i + 1}] <= $arg_len_var)} {
         puts [lindex $argv [expr {$i + 1}]]
         set write_reports [lindex $argv [expr {$i + 1}]]
      }

      # Bind "init" attribute:
      if {([lindex $argv $i] == "-init") && ([expr {$i + 1}] <= $arg_len_var)} {
         puts [lindex $argv [expr {$i + 1}]]
         set init [lindex $argv [expr {$i + 1}]]
      }

      # Bind "load_synth" attribute:
      if {[lindex $argv $i] == "-load_synth"} { set load_synth true }

      # Bind "load_opt" attribute:
      if {[lindex $argv $i] == "-load_opt"} { set load_opt true }

      # Bind "load_place" attribute:
      if {[lindex $argv $i] == "-load_place"} { set load_place true }

      # Bind "fast" attribute:
      if {[lindex $argv $i] == "-fast"} { set fast true }

#      # Bind alternative top level attribute:
#      if {[lindex $argv $i] == "-top"} {
#         puts [lindex $argv [expr {$i + 1}]]
#         set strategy [lindex $argv [expr {$i + 1}]]
#      }
   }
}


#
# STEP#0: define output directory area.
#
set outputDir ./output
file mkdir $outputDir

# Main verilog defines:
set VERILOG_DEF ""
lappend VERILOG_DEF TARGET_KC705
lappend VERILOG_DEF FPGA
lappend VERILOG_DEF WF_CPU_CLOCK=40.0
#lappend VERILOG_DEF WF_CPU_CLOCK=100.0


#
# STEP#1: setup design sources and constraints
#
set REPO_ROOT ../../..
set LIST_ROOT ${REPO_ROOT}/prj/common/lists
source -notrace scripts/set_paths.tcl

# Load project files if LOAD_FILES = true:
if {[string is true $load_files]} {
	read_verilog -sv config_target_pkg.sv

	set filelist_name ${LIST_ROOT}/ambalib.f;       source -notrace scripts/get_file_list.tcl; read_verilog -sv [subst $filelist]
	set filelist_name ${LIST_ROOT}/techmap.f;	source -notrace scripts/get_file_list.tcl; read_verilog -sv [subst $filelist]
	set filelist_name ${LIST_ROOT}/techmap_ddr_kc705.f; source -notrace scripts/get_file_list.tcl; read_verilog -sv [subst $filelist]
	set filelist_name ${LIST_ROOT}/riverlib.f;     	source -notrace scripts/get_file_list.tcl; read_verilog -sv [subst $filelist]
	set filelist_name ${LIST_ROOT}/misclib.f;    	source -notrace scripts/get_file_list.tcl; read_verilog -sv [subst $filelist]
	set filelist_name ${LIST_ROOT}/riscv_soc.f;    	source -notrace scripts/get_file_list.tcl; read_verilog -sv [subst $filelist]
	set filelist_name ${PRJ_ROOT}/kc705_top.f;  	source -notrace scripts/get_file_list.tcl; read_verilog -sv [subst $filelist]

	# Apply Verilog Defines:
	set_property verilog_define $VERILOG_DEF [current_fileset]
}

if {[string is true $load_constr]} {
	read_xdc ./kc705_top.xdc
}

#
# STEP#2: run synthesis, report utilization and timing estimates, write checkpoint design
#
# synthesis related settings:
lappend SYNTH_ARGS -top kc705_top
lappend SYNTH_ARGS -flatten_hierarchy none
#lappend SYNTH_ARGS -flatten_hierarchy rebuilt
# Gated clock conversion work only in flat design:
#lappend SYNTH_ARGS -gated_clock_conversion on
lappend SYNTH_ARGS -gated_clock_conversion off
lappend SYNTH_ARGS -bufg 32
#lappend SYNTH_ARGS -fanout_limit 10000
lappend SYNTH_ARGS -directive Default
#lappend SYNTH_ARGS -directive AlternateRoutability
#lappend SYNTH_ARGS -directive FewerCarryChains
#lappend SYNTH_ARGS -directive runtimeoptimized
#lappend SYNTH_ARGS -fsm_extraction user_encoding
lappend SYNTH_ARGS -fsm_extraction auto
lappend SYNTH_ARGS -keep_equivalent_registers
#lappend SYNTH_ARGS -resource_sharing auto
lappend SYNTH_ARGS -resource_sharing off
lappend SYNTH_ARGS -control_set_opt_threshold auto
#lappend SYNTH_ARGS -no_lc
#lappend SYNTH_ARGS -shreg_min_size 3
lappend SYNTH_ARGS -shreg_min_size 5
lappend SYNTH_ARGS -max_bram -1
lappend SYNTH_ARGS -max_dsp -1
#lappend SYNTH_ARGS -cascade_dsp auto
#lappend SYNTH_ARGS -retiming
#lappend SYNTH_ARGS -verbose
lappend SYNTH_ARGS -part xc7k325tffg900-2


###############################################################################################################################
#
#  BASIC strategy:
#
###############################################################################################################################
if {$strategy == "basic"} {
	puts "Synthesis start at time: [clock format [clock seconds] -format %H:%M:%S]"

	#synth_design -top asic_top -part xc7k325tffg900-2 -flatten rebuilt
	#synth_design -top asic_top {$SYNTH_ARGS} -part xc7k325tffg900-2
	#eval "synth_design -top asic_top $SYNTH_ARGS -part xc7k325tffg900-2"
	#eval "synth_design -top kc705_top $SYNTH_ARGS -part xc7k325tffg900-2"
	eval "synth_design $SYNTH_ARGS"

	puts "Synthesis end at time: [clock format [clock seconds] -format %H:%M:%S]"

	write_checkpoint -force $outputDir/post_synth
	report_timing_summary -file $outputDir/post_synth_timing_summary.rpt
	report_power -file $outputDir/post_synth_power.rpt


	#
	# STEP#3: run placement and logic optimization, report utilization and timing estimates, write checkpoint design
	#
	puts "Opt_desing start at time: [clock format [clock seconds] -format %H:%M:%S]"
	#opt_design
	opt_design -directive ExploreWithRemap
	puts "Opt_design end at time: [clock format [clock seconds] -format %H:%M:%S]"

	#write_checkpoint -force $outputDir/post_opt.dcp

	#power_opt_design

	puts "Place_design start at time: [clock format [clock seconds] -format %H:%M:%S]"
	#place_design
	place_design -directive Explore
	#place_design -directive ExtraNetDelay_high
	puts "Place_design end at time: [clock format [clock seconds] -format %H:%M:%S]"

	write_checkpoint -force $outputDir/post_place
	report_timing_summary -file $outputDir/post_place_timing_summary.rpt

	#phys_opt_design

	set SETUP_SLACK_OPT true
	set HOLD_SLACK_OPT true
	set TNS_PREV 0
	set THS_PREV 0
	set WNS_SRCH_STR "WNS="; set TNS_SRCH_STR "TNS="
	set WHS_SRCH_STR "WHS="; set THS_SRCH_STR "THS="

	# phys_opt_desing: see UG904 p.76
	for {set index 0} {$index < $NLOOPS} {incr index} {
		#phys_opt_design
		#phys_opt_design -directive AggressiveExplore
		#phys_opt_design -directive AggressiveFanoutOpt
		#phys_opt_design -directive AlternateReplication

		#phys_opt_design -directive ExploreWithHoldFix
		#phys_opt_design -directive ExploreWithAggressiveHoldFix

		# Check for SETUP slack:
		if {[string is true $SETUP_SLACK_OPT]} {
			phys_opt_design -directive AggressiveExplore
			#phys_opt_design -directive AggressiveExplore -quiet
			set WNS [exec grep $WNS_SRCH_STR vivado.log | tail -1 | sed -n -e "s/^.*$WNS_SRCH_STR//p" | sed "s/|/ /g" | cut -d\  -f 1]
			set TNS [exec grep $TNS_SRCH_STR vivado.log | tail -1 | sed -n -e "s/^.*$TNS_SRCH_STR//p" | sed "s/|/ /g" | cut -d\  -f 1]
			puts "Time: [clock format [clock seconds] -format %H:%M:%S] Iteration: $index Place: WNS=$WNS | TNS=$TNS "

			if {($TNS == $TNS_PREV && $index > 0) || $WNS >= 0.000} { break }
			#if {($TNS == $TNS_PREV && $index > 0) || $WNS >= 0.000} { set SETUP_SLACK_OPT false }
			set TNS_PREV $TNS
		}

		if {[string is true $SETUP_SLACK_OPT]} {
			phys_opt_design -directive AggressiveFanoutOpt
			set WNS [exec grep $WNS_SRCH_STR vivado.log | tail -1 | sed -n -e "s/^.*$WNS_SRCH_STR//p" | sed "s/|/ /g" | cut -d\  -f 1]
			set TNS [exec grep $TNS_SRCH_STR vivado.log | tail -1 | sed -n -e "s/^.*$TNS_SRCH_STR//p" | sed "s/|/ /g" | cut -d\  -f 1]
			puts "Time: [clock format [clock seconds] -format %H:%M:%S] Iteration: $index Place: WNS=$WNS | TNS=$TNS "

			if {($TNS == $TNS_PREV && $index > 0) || $WNS >= 0.000} { break }
			#if {($TNS == $TNS_PREV && $index > 0) || $WNS >= 0.000} { set SETUP_SLACK_OPT false }
			set TNS_PREV $TNS
		}

		if {[string is true $SETUP_SLACK_OPT]} {
			phys_opt_design -directive AlternateReplication
			set WNS [exec grep $WNS_SRCH_STR vivado.log | tail -1 | sed -n -e "s/^.*$WNS_SRCH_STR//p" | sed "s/|/ /g" | cut -d\  -f 1]
			set TNS [exec grep $TNS_SRCH_STR vivado.log | tail -1 | sed -n -e "s/^.*$TNS_SRCH_STR//p" | sed "s/|/ /g" | cut -d\  -f 1]
			puts "Time: [clock format [clock seconds] -format %H:%M:%S] Iteration: $index Place: WNS=$WNS | TNS=$TNS "

			if {($TNS == $TNS_PREV && $index > 0) || $WNS >= 0.000} { break }
			#if {($TNS == $TNS_PREV && $index > 0) || $WNS >= 0.000} { set SETUP_SLACK_OPT false }
			set TNS_PREV $TNS
		}
	}

	# Save phys_opt settings:
	write_iphys_opt_tcl -place ./tcl/post_place_physopt.tcl

	write_checkpoint -force $outputDir/phys_opt_post_place
	#report_timing_summary -file $outputDir/post_place_timing_summary.rpt

	#
	# STEP#4: run router, report actual utilization and timing, write checkpoint design, run drc, write verilog and xdc out
	#
	puts "Route_design start at time: [clock format [clock seconds] -format %H:%M:%S]"
	#route_design
	route_design -directive Explore
	#route_design -directive Explore -tns_cleanup
	puts "Route_design end at time: [clock format [clock seconds] -format %H:%M:%S]"

	write_checkpoint -force $outputDir/post_route
	report_timing_summary -file $outputDir/post_route_timing_summary.rpt
	report_clock_utilization -file $outputDir/clock_util.rpt
	report_utilization -file $outputDir/post_route_util.rpt
	report_power -file $outputDir/post_route_power.rpt
	report_drc -file $outputDir/post_imp_drc.rpt
	write_verilog -force $outputDir/bft_impl_netlist.v
	write_xdc -no_fixed_only -force $outputDir/kc705_top.xdc

	#
	# STEP#5: generate a bitstream
	#
	write_bitstream -force $outputDir/kc705_top.bit

	exit
}

###############################################################################################################################
#
#  Synthesys Only strategy:
#
###############################################################################################################################
if {$strategy == "synth"} {
	#synth_design -top kc705_top -rtl -name kc705_rtl -include_dirs $INC
	#synth_design -top kc705_top -rtl -name kc705_rtl
	#synth_design -top kc705_top $SYNTH_ARGS
	eval "synth_design $SYNTH_ARGS"
}

###############################################################################################################################
#
#  Start Vivado with RTL Design:
#
###############################################################################################################################
if {$strategy == "rtl"} {
	#synth_design -top kc705_top -rtl -name kc705_rtl -include_dirs $INC
	synth_design -top kc705_top -rtl -name kc705_rtl
	#exit
}


