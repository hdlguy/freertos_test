# run at linux command line "xsct setup.tcl"
file delete -force ./workspace

set hw   ../../../fpga/implement/results/top.xsa
set proc "ps7_cortexa9_0"
#set proc "psu_cortexr5_0"
#set proc "psu_cortexa53_0"

setws ./workspace

#platform create -name "streamer" -hw $hw -proc $proc -os standalone
#domain create -name "streamer_domain" -os standalone -proc $proc
#app create    -name test2 -platform streamer -domain streamer_domain -template "Empty Application"
#file link -symbolic ./workspace/test2/src/main.c ../../../src/test2/main.c
#app build test2

platform create -name "rtos_plat"    -hw $hw -proc $proc -os freertos10_xilinx
domain create   -name "rtos_domain"          -proc $proc -os freertos10_xilinx 

app create    -name rtos_test1 -platform rtos_plat -domain rtos_domain -template "Empty Application"
file link -symbolic ./workspace/rtos_test1/src/freertos_hello_world.c ../../../src/freertos_hello_world.c





#app create    -name test3 -platform streamer -domain streamer_domain -template "Hello World"




