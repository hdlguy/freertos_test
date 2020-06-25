# run at linux command line "xsct setup.tcl"
set sdk_dir ./dma_test.sdk

file delete -force $sdk_dir

set hwproject "hw"
set plat   ../../../fpga/implement/results/top.xsa
set bsp "bsp"
#set proc "ps7_cortexa9_0"
#set proc "psu_cortexr5_0"
set proc "psu_cortexa53_0"
set os "standalone"

setws $sdk_dir

platform create -name "streamer" -hw $plat -proc $proc -os $os

platform generate

domain create -name "streamer_domain" -os $os -proc $proc

app create -name test2 -platform streamer -domain streamer_domain -template "Empty Application"
app create -name test3 -platform streamer -domain streamer_domain -template "Hello World"

file link -symbolic $sdk_dir/test2/src/main.c ../../../src/main.c                                                                                                                                            

#app build -all

