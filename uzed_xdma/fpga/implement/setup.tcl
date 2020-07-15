# This script sets up a Vivado project with all ip references resolved.
close_project -quiet
file delete -force proj.xpr *.os *.jou *.log proj.srcs proj.cache proj.runs
#
create_project -force proj
#set_property board_part em.avnet.com:zed:part0:1.4 [current_project]
set_property board_part em.avnet.com:ultrazed_eg_iocc_production:part0:1.0 [current_project]
set_property target_language verilog [current_project]
set_property default_lib work [current_project]
load_features ipintegrator
#tclapp::install ultrafast -quiet

#read_ip ../../source/srl32/srl32.xci
#upgrade_ip -quiet  [get_ips *]
#generate_target {all} [get_ips *]

source ../source/system.tcl
generate_target {synthesis implementation} [get_files ./proj.srcs/sources_1/bd/system/system.bd]
set_property synth_checkpoint_mode None [get_files ./proj.srcs/sources_1/bd/system/system.bd]

read_verilog -sv ../source/stream_gen/stream_gen.sv
read_verilog -sv ../source/top.sv

read_xdc ../source/top.xdc

close_project


