# This script sets up a Vivado project with all ip references resolved.
close_project -quiet
file delete -force proj.xpr *.os *.jou *.log proj.srcs proj.cache proj.runs
#
create_project -force proj 
set_property -quiet board_part em.avnet.com:ultrazed_eg_iocc_production:part0:1.0 [current_project]
set_property target_language Verilog [current_project]
set_property default_lib work [current_project]
load_features ipintegrator

read_ip ../stream_gen_fifo/stream_gen_fifo.xci
upgrade_ip -quiet  [get_ips *]
generate_target {all} [get_ips *]

read_verilog -sv ../stream_gen.sv
read_verilog -sv ../stream_gen_tb.sv

add_files -fileset sim_1 -norecurse ./stream_gen_tb_behav.wcfg

close_project

#########################



