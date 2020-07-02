# run at linux command line "xsct setup.tcl"
file delete -force ./workspace

set hw   ../../../fpga/implement/results/top.xsa
set proc "ps7_cortexa9_0"
#set proc "psu_cortexr5_0"
#set proc "psu_cortexa53_0"

setws ./workspace

platform create -name "standalone_plat"    -hw $hw -proc $proc -os standalone
#domain create   -name "standalone_domain"          -proc $proc -os standalone 

app create -name xintc_example -platform standalone_plat -domain standalone_domain -template "Empty Application"
file link -symbolic ./workspace/xintc_example/src/xintc_example.c ../../../src/xintc_example.c



