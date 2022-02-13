# this tcl script inserts the Vitis elf file into the fpga bit file and then generates an mcs flash programming file.
#set appname "srec_spi_boot"
set appname "enet_cmd_proc"

exec updatemem -force -meminfo ./results/top.mmi -data ../software/vitis/workspace/$appname/Debug/$appname.elf -bit ./results/top.bit -proc system_i/microblaze_0 -out ./results/elftop.bit

write_cfgmem -force -format MCS -size 16 -interface SPIx4 -loadbit "up 0x0 ./results/elftop.bit" -verbose ./results/elftop.mcs



