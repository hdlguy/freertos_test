# run at linux command line:
#  xsct setup.tcl
#  vitis -workspace  ./workspace/
file delete -force ./workspace

set hw   ../../../fpga/implement/results/top.xsa
set proc "ps7_cortexa9_0"
#set proc "psu_cortexr5_0"
#set proc "psu_cortexa53_0"

setws ./workspace

platform create -name "standalone_plat"    -hw $hw -proc $proc -os standalone
#domain create   -name "standalone_domain"          -proc $proc -os standalone 

app create -name xdma_test -platform standalone_plat -domain standalone_domain -template "Empty Application"
file link -symbolic ./workspace/xdma_test/src/xdma_test.c ../../../src/xdma_test.c



