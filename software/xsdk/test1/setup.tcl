set sdk_dir ./dma_test.sdk

file delete -force $sdk_dir/.metadata
file delete -force $sdk_dir/hw
file delete -force $sdk_dir/bsp

set hwproject "hw"
set hwspec ../../implement/results/top.hdf
set bsp "bsp"
#set proc "ps7_cortexa9_0"
set proc "psu_cortexr5_0"
set os "standalone"
set application "sw"

# Create workspace and import the project into
setws $sdk_dir

createhw -name $hwproject -hwspec $hwspec

createbsp -name $bsp -hwproject $hwproject -proc $proc -os $os

# Update the microprocessor software spec (MSS) and regenerate the BSP
updatemss -mss $sdk_dir/$bsp/system.mss
regenbsp -bsp $bsp

# Create new application project as Empty Application 
createapp -name $application -app {Empty Application} -proc $proc -hwproject $hwproject -bsp $bsp -os $os

# add the libm math library to the linker script.
configapp -app $application libraries m

# Clean and build all projects
projects -clean
projects -build

