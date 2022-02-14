updatemem -force -meminfo ../../implement/results/top.mmi  -bit ../../implement/results/top.bit -data ./workspace/freertos_test/Debug/freertos_test.elf -proc system_i/microblaze_0 -out ./download.bit 

bootgen -arch fpga -image ./bootimage.bif -w -o ./BOOT.bin -interface spi 

program_flash -f ./BOOT.bin -offset 0 -flash_type mt25ql128-spi-x1_x2_x4 -cable type xilinx_tcf url TCP:127.0.0.1:3121 

