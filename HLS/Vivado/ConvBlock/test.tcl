# Include HP1 back for gmem2/3
include_bd_addr_seg [get_bd_addr_segs zynq_ultra_ps_e_0/SAXIGP3/HP1_DDR_LOW] \
    -target_address_space [get_bd_addr_spaces conv_engine_0/Data_m_axi_gmem2]
include_bd_addr_seg [get_bd_addr_segs zynq_ultra_ps_e_0/SAXIGP3/HP1_DDR_LOW] \
    -target_address_space [get_bd_addr_spaces conv_engine_0/Data_m_axi_gmem3]

# Exclude HP0 from gmem2/3 (so they use only HP1)
exclude_bd_addr_seg [get_bd_addr_segs zynq_ultra_ps_e_0/SAXIGP2/HP0_DDR_LOW] \
    -target_address_space [get_bd_addr_spaces conv_engine_0/Data_m_axi_gmem2]
exclude_bd_addr_seg [get_bd_addr_segs zynq_ultra_ps_e_0/SAXIGP2/HP0_DDR_LOW] \
    -target_address_space [get_bd_addr_spaces conv_engine_0/Data_m_axi_gmem3]
