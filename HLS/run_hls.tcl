# ==============================================================================
# TinyYOLO Convolution Engine - ZCU102 Build Script
# ==============================================================================

# 1. Project Settings
# -------------------
set project_name "tinyyolo_zcu102"
set top_func     "conv_engine"

# ZCU102 Part Number (Zynq UltraScale+ XCZU9EG)
set target_part  "xczu9eg-ffvb1156-2-e" 

set clock_period 7

# 2. Create Project
# -----------------
open_project -reset $project_name

# 3. Add Source Files
# -------------------
add_files conv_engine.cpp
add_files conv_engine.h

# Add Testbench (Essential for verification)
add_files -tb conv_engine_tb.cpp

# 4. Set Top Level
# ----------------
set_top $top_func

# 5. Create Solution
# ------------------
open_solution -reset "solution1"

# Define FPGA part and Clock
set_part $target_part
create_clock -period $clock_period -name default

# 6. Optimization Directives (Optional but recommended for ZCU102)
# --------------------------------------------------------------
# Axi inputs often benefit from max_read_burst_length for high bandwidth
config_interface -m_axi_addr64=1

# 7. Run Synthesis
# ----------------
puts "### Running Synthesis for ZCU102 ###"
csynth_design

# 8. Export IP (Creates the ZIP file)
# -----------------------------------
puts "### Exporting IP to ZIP ###"
# This generates the IP package in: <project_name>/solution1/impl/ip/
export_design -format ip_catalog -description "TinyYOLO_Conv_Engine_ZCU102" -vendor "User" -version "1.0"

puts "### Build Complete ###"
puts "The IP Zip file is located in: $project_name/solution1/impl/ip/"
exit
