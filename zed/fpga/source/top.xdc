set_property C_CLK_INPUT_FREQ_HZ 300000000 [get_debug_cores dbg_hub]
set_property C_ENABLE_CLK_DIVIDER false [get_debug_cores dbg_hub]
set_property C_USER_SCAN_CHAIN 1 [get_debug_cores dbg_hub]
connect_debug_port dbg_hub/clk [get_nets clk]

set_property IOSTANDARD LVCMOS33 [get_ports {pmod_tri_o[*]}]
set_property PACKAGE_PIN Y11  [get_ports {pmod_tri_o[0]}];   # "JA1"
set_property PACKAGE_PIN AA11 [get_ports {pmod_tri_o[1]}];   # "JA2"
set_property PACKAGE_PIN Y10  [get_ports {pmod_tri_o[2]}];   # "JA3"
set_property PACKAGE_PIN AA9  [get_ports {pmod_tri_o[3]}];   # "JA4"
set_property PACKAGE_PIN AB11 [get_ports {pmod_tri_o[4]}];   # "JA7"
set_property PACKAGE_PIN AB10 [get_ports {pmod_tri_o[5]}];   # "JA8"
set_property PACKAGE_PIN AB9  [get_ports {pmod_tri_o[6]}];   # "JA9"
set_property PACKAGE_PIN AA8  [get_ports {pmod_tri_o[7]}];   # "JA10"

set_property PACKAGE_PIN W12  [get_ports {pmod_tri_o[8]}];   # "JB1"
set_property PACKAGE_PIN W11  [get_ports {pmod_tri_o[9]}];   # "JB2"
set_property PACKAGE_PIN V10  [get_ports {pmod_tri_o[10]}];  # "JB3"
set_property PACKAGE_PIN W8   [get_ports {pmod_tri_o[11]}];  # "JB4"
set_property PACKAGE_PIN V12  [get_ports {pmod_tri_o[12]}];  # "JB7"
set_property PACKAGE_PIN W10  [get_ports {pmod_tri_o[13]}];  # "JB8"
set_property PACKAGE_PIN V9   [get_ports {pmod_tri_o[14]}];  # "JB9"
set_property PACKAGE_PIN V8   [get_ports {pmod_tri_o[15]}];  # "JB10"
