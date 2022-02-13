
set_property IOSTANDARD LVCMOS33    [get_ports eth_refclk]
set_property SLEW SLOW              [get_ports eth_refclk]
set_property PACKAGE_PIN G18        [get_ports eth_refclk]

#set_property CFGBVS VCCO [current_design]
#set_property CONFIG_VOLTAGE 2.5 [current_design]
#set_property BITSTREAM.CONFIG.CONFIGRATE 50 [current_design]
set_property BITSTREAM.Config.SPI_BUSWIDTH 4 [current_design]
set_property BITSTREAM.GENERAL.COMPRESS TRUE [current_design]

set_property IOSTANDARD LVCMOS33    [get_ports led[*]]
set_property PACKAGE_PIN T10        [get_ports led[3]]
set_property PACKAGE_PIN  T9        [get_ports led[2]]
set_property PACKAGE_PIN  J5        [get_ports led[1]]
set_property PACKAGE_PIN  H5        [get_ports led[0]]


set_property IOSTANDARD LVCMOS33    [get_ports qspi_flash_*]
set_property PACKAGE_PIN K17        [get_ports qspi_flash_io0_io]
set_property PACKAGE_PIN K18        [get_ports qspi_flash_io1_io]
set_property PACKAGE_PIN L14        [get_ports qspi_flash_io2_io]
set_property PACKAGE_PIN M14        [get_ports qspi_flash_io3_io]
set_property PACKAGE_PIN L16        [get_ports qspi_flash_sck_io]
set_property PACKAGE_PIN L13        [get_ports qspi_flash_ss_io]