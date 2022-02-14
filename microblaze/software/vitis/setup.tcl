# run at linux command line 
#       xsct setup.tcl
#       vitis --workspace ./workspace
#
file delete -force ./workspace

set hw ../../implement/results/top.xsa
#set proc "ps7_cortexa9_0"
#set proc "psu_cortexr5_0"
#set proc "psu_cortexa53_0"
set proc "microblaze_0"

setws ./workspace

#platform create -name "standalone_plat"  -hw $hw -proc $proc -os standalone
#bsp setlib -name lwip211 -ver 1.6


platform create -name "freertos_plat"    -hw $hw -proc $proc -os freertos

#app create -name freertos_test -platform freertos_plat -domain freertos_domain -template "Empty Application(C)"
#file link -symbolic ./workspace/freertos_test/src/main.c                ../../../src/freertos_test/main.c
#file delete  -force ./workspace/freertos_test/src/lscript.ld
#file link -symbolic ./workspace/freertos_test/src/lscript.ld            ../../../src/freertos_test/lscript.ld


#app create -name freertos_cli -platform freertos_plat -domain freertos_domain -template "Empty Application(C)"
#file link -symbolic ./workspace/freertos_cli/src/main.c                ../../../src/freertos_cli/main.c
#file link -symbolic ./workspace/freertos_cli/src/FreeRTOS_CLI.h        ../../../src/freertos_cli/FreeRTOS_CLI.h
#file link -symbolic ./workspace/freertos_cli/src/FreeRTOS_CLI.c        ../../../src/freertos_cli/FreeRTOS_CLI.c
#file delete  -force ./workspace/freertos_cli/src/lscript.ld
#file link -symbolic ./workspace/freertos_cli/src/lscript.ld            ../../../src/freertos_cli/lscript.ld


app create -name command_proc -platform freertos_plat -domain freertos_domain -template "Empty Application(C)"
file link -symbolic ./workspace/command_proc/src/main.c                ../../../src/command_proc/main.c
file link -symbolic ./workspace/command_proc/src/fpga.h                ../../../src/fpga.h
file link -symbolic ./workspace/command_proc/src/platform.c            ../../../src/command_proc/platform.c
file link -symbolic ./workspace/command_proc/src/platform.h            ../../../src/command_proc/platform.h
file link -symbolic ./workspace/command_proc/src/platform_config.h     ../../../src/command_proc/platform_config.h


#app create -name lwip_echo -platform standalone_plat -domain standalone_domain -template "Empty Application(C)"
#file link -symbolic ./workspace/lwip_echo/src/main.c                ../../../src/lwip_echo/main.c
#file link -symbolic ./workspace/lwip_echo/src/echo.c                ../../../src/lwip_echo/echo.c
#file link -symbolic ./workspace/lwip_echo/src/i2c_access.c          ../../../src/lwip_echo/i2c_access.c
#file link -symbolic ./workspace/lwip_echo/src/iic_phyreset.c        ../../../src/lwip_echo/iic_phyreset.c
#file link -symbolic ./workspace/lwip_echo/src/platform.c            ../../../src/lwip_echo/platform.c
#file link -symbolic ./workspace/lwip_echo/src/platform.h            ../../../src/lwip_echo/platform.h
#file link -symbolic ./workspace/lwip_echo/src/platform_mb.c         ../../../src/lwip_echo/platform_mb.c
#file link -symbolic ./workspace/lwip_echo/src/platform_config.h     ../../../src/lwip_echo/platform_config.h
#file link -symbolic ./workspace/lwip_echo/src/sfp.c                 ../../../src/lwip_echo/sfp.c
#file link -symbolic ./workspace/lwip_echo/src/si5324.c              ../../../src/lwip_echo/si5324.c
#file delete  -force ./workspace/lwip_echo/src/lscript.ld
#file link -symbolic ./workspace/lwip_echo/src/lscript.ld            ../../../src/lwip_echo/lscript.ld


#app create -name srec_spi_boot -platform standalone_plat -domain standalone_domain -template "Empty Application(C)"
#file link -symbolic ./workspace/srec_spi_boot/src/platform.c        ../../../src/srec_spi_boot/platform.c
#file link -symbolic ./workspace/srec_spi_boot/src/blconfig.h        ../../../src/srec_spi_boot/blconfig.h
#file link -symbolic ./workspace/srec_spi_boot/src/bootloader.c      ../../../src/srec_spi_boot/bootloader.c
#file link -symbolic ./workspace/srec_spi_boot/src/srec.c            ../../../src/srec_spi_boot/srec.c
#file link -symbolic ./workspace/srec_spi_boot/src/srec.h            ../../../src/srec_spi_boot/srec.h
#file link -symbolic ./workspace/srec_spi_boot/src/portab.h          ../../../src/srec_spi_boot/portab.h
#file link -symbolic ./workspace/srec_spi_boot/src/platform_config.h ../../../src/srec_spi_boot/platform_config.h
#file link -symbolic ./workspace/srec_spi_boot/src/errors.h          ../../../src/srec_spi_boot/errors.h
#file delete -force ./workspace/srec_spi_boot/src/lscript.ld
#file link -symbolic ./workspace/srec_spi_boot/src/lscript.ld        ../../../src/srec_spi_boot/lscript.ld


#app create -name reg_test -platform standalone_plat -domain standalone_domain -template "Empty Application(C)"
#file link -symbolic ./workspace/reg_test/src/platform.c         ../../../src/reg_test/platform.c
#file link -symbolic ./workspace/reg_test/src/platform.h         ../../../src/reg_test/platform.h
#file link -symbolic ./workspace/reg_test/src/platform_config.h  ../../../src/reg_test/platform_config.h
#file link -symbolic ./workspace/reg_test/src/test.c             ../../../src/reg_test/test.c
#file delete -force  ./workspace/reg_test/src/lscript.ld
#file link -symbolic ./workspace/reg_test/src/lscript.ld         ../../../src/reg_test/lscript.ld


#app create -name hello1 -platform standalone_plat -domain standalone_domain -template "Empty Application(C)"
#file link -symbolic ./workspace/hello1/src/hello1.c             ../../../src/hello1/hello1.c
#file link -symbolic ./workspace/hello1/src/platform.c           ../../../src/hello1/platform.c
#file link -symbolic ./workspace/hello1/src/platform.h           ../../../src/hello1/platform.h
#file link -symbolic ./workspace/hello1/src/platform_config.h    ../../../src/hello1/platform_config.h
#file delete -force  ./workspace/hello1/src/lscript.ld
#file link -symbolic ./workspace/hello1/src/lscript.ld           ../../../src/hello1/lscript.ld


app build all

