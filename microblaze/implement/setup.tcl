# This script sets up a Vivado project with all ip references resolved.
file delete -force proj.xpr *.os *.jou *.log proj.srcs proj.cache proj.runs
#
create_project -force proj 
set_property board_part digilentinc.com:arty-a7-35:part0:1.0 [current_project]
#set_property part xc7a100tcsg324-1 [current_project]
#set_property board_part digilentinc.com:arty-a7-35:part0:1.0 [current_project]
#set_property board_part numato.com:neso:part0:1.0 [current_project]
#create_project -part xc7a50t-ftg256-1 -force proj 
set_property target_language Verilog [current_project]
set_property default_lib work [current_project]
load_features ipintegrator
tclapp::install ultrafast -quiet


#read_ip ../source/eth_ila/eth_ila.xci
upgrade_ip -quiet  [get_ips *]
generate_target {all} [get_ips *]


# Recreate the Block Diagram of the processor system.
source ../source/system.tcl
generate_target {synthesis implementation} [get_files ./proj.srcs/sources_1/bd/system/system.bd]
set_property synth_checkpoint_mode None    [get_files ./proj.srcs/sources_1/bd/system/system.bd]

read_verilog -sv ../source/axi_regfile/axi_regfile_v1_0_S00_AXI.sv

read_verilog -sv ../source/top.sv

read_xdc ../source/top.xdc

close_project



