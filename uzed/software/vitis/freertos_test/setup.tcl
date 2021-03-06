# run at linux command line "xsct setup.tcl"
file delete -force ./workspace

set hw   ../../../fpga/implement/results/top.xsa
#set proc "ps7_cortexa9_0"
#set proc "psu_cortexr5_0"
set proc "psu_cortexa53_0"

setws ./workspace

platform create -name "rtos_plat"    -hw $hw -proc $proc -os freertos10_xilinx
domain create   -name "rtos_domain"          -proc $proc -os freertos10_xilinx 

app create    -name rtos_test1 -platform rtos_plat -domain rtos_domain -template "Empty Application"
file link -symbolic ./workspace/rtos_test1/src/freertos_hello_world.c ../../../src/freertos_hello_world.c



