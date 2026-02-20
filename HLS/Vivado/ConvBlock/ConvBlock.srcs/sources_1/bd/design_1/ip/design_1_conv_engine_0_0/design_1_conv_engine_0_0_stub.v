// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2024 Advanced Micro Devices, Inc. All Rights Reserved.
// --------------------------------------------------------------------------------
// Tool Version: Vivado v.2024.2 (lin64) Build 5239630 Fri Nov 08 22:34:34 MST 2024
// Date        : Sat Feb  7 12:27:35 2026
// Host        : RMP-RESEARCH4 running 64-bit Ubuntu 22.04.5 LTS
// Command     : write_verilog -force -mode synth_stub
//               /home/researchadmin/ConvBlock/ConvBlock.gen/sources_1/bd/design_1/ip/design_1_conv_engine_0_0/design_1_conv_engine_0_0_stub.v
// Design      : design_1_conv_engine_0_0
// Purpose     : Stub declaration of top-level module interface
// Device      : xczu9eg-ffvb1156-2-e
// --------------------------------------------------------------------------------

// This empty module with port declaration file causes synthesis tools to infer a black box for IP.
// The synthesis directives are for Synopsys Synplify support to prevent IO buffer insertion.
// Please paste the declaration into a Verilog source file or add the file as an additional source.
(* CHECK_LICENSE_TYPE = "design_1_conv_engine_0_0,conv_engine,{}" *) (* DowngradeIPIdentifiedWarnings = "yes" *) (* IP_DEFINITION_SOURCE = "HLS" *) 
(* X_CORE_INFO = "conv_engine,Vivado 2024.2" *) (* hls_module = "yes" *) 
module design_1_conv_engine_0_0(s_axi_control_ARADDR, 
  s_axi_control_ARREADY, s_axi_control_ARVALID, s_axi_control_AWADDR, 
  s_axi_control_AWREADY, s_axi_control_AWVALID, s_axi_control_BREADY, 
  s_axi_control_BRESP, s_axi_control_BVALID, s_axi_control_RDATA, s_axi_control_RREADY, 
  s_axi_control_RRESP, s_axi_control_RVALID, s_axi_control_WDATA, s_axi_control_WREADY, 
  s_axi_control_WSTRB, s_axi_control_WVALID, ap_clk, ap_rst_n, interrupt, m_axi_gmem0_ARADDR, 
  m_axi_gmem0_ARBURST, m_axi_gmem0_ARCACHE, m_axi_gmem0_ARID, m_axi_gmem0_ARLEN, 
  m_axi_gmem0_ARLOCK, m_axi_gmem0_ARPROT, m_axi_gmem0_ARQOS, m_axi_gmem0_ARREADY, 
  m_axi_gmem0_ARREGION, m_axi_gmem0_ARSIZE, m_axi_gmem0_ARVALID, m_axi_gmem0_AWADDR, 
  m_axi_gmem0_AWBURST, m_axi_gmem0_AWCACHE, m_axi_gmem0_AWID, m_axi_gmem0_AWLEN, 
  m_axi_gmem0_AWLOCK, m_axi_gmem0_AWPROT, m_axi_gmem0_AWQOS, m_axi_gmem0_AWREADY, 
  m_axi_gmem0_AWREGION, m_axi_gmem0_AWSIZE, m_axi_gmem0_AWVALID, m_axi_gmem0_BID, 
  m_axi_gmem0_BREADY, m_axi_gmem0_BRESP, m_axi_gmem0_BVALID, m_axi_gmem0_RDATA, 
  m_axi_gmem0_RID, m_axi_gmem0_RLAST, m_axi_gmem0_RREADY, m_axi_gmem0_RRESP, 
  m_axi_gmem0_RVALID, m_axi_gmem0_WDATA, m_axi_gmem0_WID, m_axi_gmem0_WLAST, 
  m_axi_gmem0_WREADY, m_axi_gmem0_WSTRB, m_axi_gmem0_WVALID, m_axi_gmem1_ARADDR, 
  m_axi_gmem1_ARBURST, m_axi_gmem1_ARCACHE, m_axi_gmem1_ARID, m_axi_gmem1_ARLEN, 
  m_axi_gmem1_ARLOCK, m_axi_gmem1_ARPROT, m_axi_gmem1_ARQOS, m_axi_gmem1_ARREADY, 
  m_axi_gmem1_ARREGION, m_axi_gmem1_ARSIZE, m_axi_gmem1_ARVALID, m_axi_gmem1_AWADDR, 
  m_axi_gmem1_AWBURST, m_axi_gmem1_AWCACHE, m_axi_gmem1_AWID, m_axi_gmem1_AWLEN, 
  m_axi_gmem1_AWLOCK, m_axi_gmem1_AWPROT, m_axi_gmem1_AWQOS, m_axi_gmem1_AWREADY, 
  m_axi_gmem1_AWREGION, m_axi_gmem1_AWSIZE, m_axi_gmem1_AWVALID, m_axi_gmem1_BID, 
  m_axi_gmem1_BREADY, m_axi_gmem1_BRESP, m_axi_gmem1_BVALID, m_axi_gmem1_RDATA, 
  m_axi_gmem1_RID, m_axi_gmem1_RLAST, m_axi_gmem1_RREADY, m_axi_gmem1_RRESP, 
  m_axi_gmem1_RVALID, m_axi_gmem1_WDATA, m_axi_gmem1_WID, m_axi_gmem1_WLAST, 
  m_axi_gmem1_WREADY, m_axi_gmem1_WSTRB, m_axi_gmem1_WVALID, m_axi_gmem2_ARADDR, 
  m_axi_gmem2_ARBURST, m_axi_gmem2_ARCACHE, m_axi_gmem2_ARID, m_axi_gmem2_ARLEN, 
  m_axi_gmem2_ARLOCK, m_axi_gmem2_ARPROT, m_axi_gmem2_ARQOS, m_axi_gmem2_ARREADY, 
  m_axi_gmem2_ARREGION, m_axi_gmem2_ARSIZE, m_axi_gmem2_ARVALID, m_axi_gmem2_AWADDR, 
  m_axi_gmem2_AWBURST, m_axi_gmem2_AWCACHE, m_axi_gmem2_AWID, m_axi_gmem2_AWLEN, 
  m_axi_gmem2_AWLOCK, m_axi_gmem2_AWPROT, m_axi_gmem2_AWQOS, m_axi_gmem2_AWREADY, 
  m_axi_gmem2_AWREGION, m_axi_gmem2_AWSIZE, m_axi_gmem2_AWVALID, m_axi_gmem2_BID, 
  m_axi_gmem2_BREADY, m_axi_gmem2_BRESP, m_axi_gmem2_BVALID, m_axi_gmem2_RDATA, 
  m_axi_gmem2_RID, m_axi_gmem2_RLAST, m_axi_gmem2_RREADY, m_axi_gmem2_RRESP, 
  m_axi_gmem2_RVALID, m_axi_gmem2_WDATA, m_axi_gmem2_WID, m_axi_gmem2_WLAST, 
  m_axi_gmem2_WREADY, m_axi_gmem2_WSTRB, m_axi_gmem2_WVALID, m_axi_gmem3_ARADDR, 
  m_axi_gmem3_ARBURST, m_axi_gmem3_ARCACHE, m_axi_gmem3_ARID, m_axi_gmem3_ARLEN, 
  m_axi_gmem3_ARLOCK, m_axi_gmem3_ARPROT, m_axi_gmem3_ARQOS, m_axi_gmem3_ARREADY, 
  m_axi_gmem3_ARREGION, m_axi_gmem3_ARSIZE, m_axi_gmem3_ARVALID, m_axi_gmem3_AWADDR, 
  m_axi_gmem3_AWBURST, m_axi_gmem3_AWCACHE, m_axi_gmem3_AWID, m_axi_gmem3_AWLEN, 
  m_axi_gmem3_AWLOCK, m_axi_gmem3_AWPROT, m_axi_gmem3_AWQOS, m_axi_gmem3_AWREADY, 
  m_axi_gmem3_AWREGION, m_axi_gmem3_AWSIZE, m_axi_gmem3_AWVALID, m_axi_gmem3_BID, 
  m_axi_gmem3_BREADY, m_axi_gmem3_BRESP, m_axi_gmem3_BVALID, m_axi_gmem3_RDATA, 
  m_axi_gmem3_RID, m_axi_gmem3_RLAST, m_axi_gmem3_RREADY, m_axi_gmem3_RRESP, 
  m_axi_gmem3_RVALID, m_axi_gmem3_WDATA, m_axi_gmem3_WID, m_axi_gmem3_WLAST, 
  m_axi_gmem3_WREADY, m_axi_gmem3_WSTRB, m_axi_gmem3_WVALID)
/* synthesis syn_black_box black_box_pad_pin="s_axi_control_ARADDR[7:0],s_axi_control_ARREADY,s_axi_control_ARVALID,s_axi_control_AWADDR[7:0],s_axi_control_AWREADY,s_axi_control_AWVALID,s_axi_control_BREADY,s_axi_control_BRESP[1:0],s_axi_control_BVALID,s_axi_control_RDATA[31:0],s_axi_control_RREADY,s_axi_control_RRESP[1:0],s_axi_control_RVALID,s_axi_control_WDATA[31:0],s_axi_control_WREADY,s_axi_control_WSTRB[3:0],s_axi_control_WVALID,ap_rst_n,interrupt,m_axi_gmem0_ARADDR[63:0],m_axi_gmem0_ARBURST[1:0],m_axi_gmem0_ARCACHE[3:0],m_axi_gmem0_ARID[0:0],m_axi_gmem0_ARLEN[7:0],m_axi_gmem0_ARLOCK[1:0],m_axi_gmem0_ARPROT[2:0],m_axi_gmem0_ARQOS[3:0],m_axi_gmem0_ARREADY,m_axi_gmem0_ARREGION[3:0],m_axi_gmem0_ARSIZE[2:0],m_axi_gmem0_ARVALID,m_axi_gmem0_AWADDR[63:0],m_axi_gmem0_AWBURST[1:0],m_axi_gmem0_AWCACHE[3:0],m_axi_gmem0_AWID[0:0],m_axi_gmem0_AWLEN[7:0],m_axi_gmem0_AWLOCK[1:0],m_axi_gmem0_AWPROT[2:0],m_axi_gmem0_AWQOS[3:0],m_axi_gmem0_AWREADY,m_axi_gmem0_AWREGION[3:0],m_axi_gmem0_AWSIZE[2:0],m_axi_gmem0_AWVALID,m_axi_gmem0_BID[0:0],m_axi_gmem0_BREADY,m_axi_gmem0_BRESP[1:0],m_axi_gmem0_BVALID,m_axi_gmem0_RDATA[255:0],m_axi_gmem0_RID[0:0],m_axi_gmem0_RLAST,m_axi_gmem0_RREADY,m_axi_gmem0_RRESP[1:0],m_axi_gmem0_RVALID,m_axi_gmem0_WDATA[255:0],m_axi_gmem0_WID[0:0],m_axi_gmem0_WLAST,m_axi_gmem0_WREADY,m_axi_gmem0_WSTRB[31:0],m_axi_gmem0_WVALID,m_axi_gmem1_ARADDR[63:0],m_axi_gmem1_ARBURST[1:0],m_axi_gmem1_ARCACHE[3:0],m_axi_gmem1_ARID[0:0],m_axi_gmem1_ARLEN[7:0],m_axi_gmem1_ARLOCK[1:0],m_axi_gmem1_ARPROT[2:0],m_axi_gmem1_ARQOS[3:0],m_axi_gmem1_ARREADY,m_axi_gmem1_ARREGION[3:0],m_axi_gmem1_ARSIZE[2:0],m_axi_gmem1_ARVALID,m_axi_gmem1_AWADDR[63:0],m_axi_gmem1_AWBURST[1:0],m_axi_gmem1_AWCACHE[3:0],m_axi_gmem1_AWID[0:0],m_axi_gmem1_AWLEN[7:0],m_axi_gmem1_AWLOCK[1:0],m_axi_gmem1_AWPROT[2:0],m_axi_gmem1_AWQOS[3:0],m_axi_gmem1_AWREADY,m_axi_gmem1_AWREGION[3:0],m_axi_gmem1_AWSIZE[2:0],m_axi_gmem1_AWVALID,m_axi_gmem1_BID[0:0],m_axi_gmem1_BREADY,m_axi_gmem1_BRESP[1:0],m_axi_gmem1_BVALID,m_axi_gmem1_RDATA[255:0],m_axi_gmem1_RID[0:0],m_axi_gmem1_RLAST,m_axi_gmem1_RREADY,m_axi_gmem1_RRESP[1:0],m_axi_gmem1_RVALID,m_axi_gmem1_WDATA[255:0],m_axi_gmem1_WID[0:0],m_axi_gmem1_WLAST,m_axi_gmem1_WREADY,m_axi_gmem1_WSTRB[31:0],m_axi_gmem1_WVALID,m_axi_gmem2_ARADDR[63:0],m_axi_gmem2_ARBURST[1:0],m_axi_gmem2_ARCACHE[3:0],m_axi_gmem2_ARID[0:0],m_axi_gmem2_ARLEN[7:0],m_axi_gmem2_ARLOCK[1:0],m_axi_gmem2_ARPROT[2:0],m_axi_gmem2_ARQOS[3:0],m_axi_gmem2_ARREADY,m_axi_gmem2_ARREGION[3:0],m_axi_gmem2_ARSIZE[2:0],m_axi_gmem2_ARVALID,m_axi_gmem2_AWADDR[63:0],m_axi_gmem2_AWBURST[1:0],m_axi_gmem2_AWCACHE[3:0],m_axi_gmem2_AWID[0:0],m_axi_gmem2_AWLEN[7:0],m_axi_gmem2_AWLOCK[1:0],m_axi_gmem2_AWPROT[2:0],m_axi_gmem2_AWQOS[3:0],m_axi_gmem2_AWREADY,m_axi_gmem2_AWREGION[3:0],m_axi_gmem2_AWSIZE[2:0],m_axi_gmem2_AWVALID,m_axi_gmem2_BID[0:0],m_axi_gmem2_BREADY,m_axi_gmem2_BRESP[1:0],m_axi_gmem2_BVALID,m_axi_gmem2_RDATA[255:0],m_axi_gmem2_RID[0:0],m_axi_gmem2_RLAST,m_axi_gmem2_RREADY,m_axi_gmem2_RRESP[1:0],m_axi_gmem2_RVALID,m_axi_gmem2_WDATA[255:0],m_axi_gmem2_WID[0:0],m_axi_gmem2_WLAST,m_axi_gmem2_WREADY,m_axi_gmem2_WSTRB[31:0],m_axi_gmem2_WVALID,m_axi_gmem3_ARADDR[63:0],m_axi_gmem3_ARBURST[1:0],m_axi_gmem3_ARCACHE[3:0],m_axi_gmem3_ARID[0:0],m_axi_gmem3_ARLEN[7:0],m_axi_gmem3_ARLOCK[1:0],m_axi_gmem3_ARPROT[2:0],m_axi_gmem3_ARQOS[3:0],m_axi_gmem3_ARREADY,m_axi_gmem3_ARREGION[3:0],m_axi_gmem3_ARSIZE[2:0],m_axi_gmem3_ARVALID,m_axi_gmem3_AWADDR[63:0],m_axi_gmem3_AWBURST[1:0],m_axi_gmem3_AWCACHE[3:0],m_axi_gmem3_AWID[0:0],m_axi_gmem3_AWLEN[7:0],m_axi_gmem3_AWLOCK[1:0],m_axi_gmem3_AWPROT[2:0],m_axi_gmem3_AWQOS[3:0],m_axi_gmem3_AWREADY,m_axi_gmem3_AWREGION[3:0],m_axi_gmem3_AWSIZE[2:0],m_axi_gmem3_AWVALID,m_axi_gmem3_BID[0:0],m_axi_gmem3_BREADY,m_axi_gmem3_BRESP[1:0],m_axi_gmem3_BVALID,m_axi_gmem3_RDATA[31:0],m_axi_gmem3_RID[0:0],m_axi_gmem3_RLAST,m_axi_gmem3_RREADY,m_axi_gmem3_RRESP[1:0],m_axi_gmem3_RVALID,m_axi_gmem3_WDATA[31:0],m_axi_gmem3_WID[0:0],m_axi_gmem3_WLAST,m_axi_gmem3_WREADY,m_axi_gmem3_WSTRB[3:0],m_axi_gmem3_WVALID" */
/* synthesis syn_force_seq_prim="ap_clk" */;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 s_axi_control ARADDR" *) (* X_INTERFACE_MODE = "slave" *) (* X_INTERFACE_PARAMETER = "XIL_INTERFACENAME s_axi_control, ADDR_WIDTH 8, DATA_WIDTH 32, PROTOCOL AXI4LITE, READ_WRITE_MODE READ_WRITE, FREQ_HZ 99990005, ID_WIDTH 0, AWUSER_WIDTH 0, ARUSER_WIDTH 0, WUSER_WIDTH 0, RUSER_WIDTH 0, BUSER_WIDTH 0, HAS_BURST 0, HAS_LOCK 0, HAS_PROT 0, HAS_CACHE 0, HAS_QOS 0, HAS_REGION 0, HAS_WSTRB 1, HAS_BRESP 1, HAS_RRESP 1, SUPPORTS_NARROW_BURST 0, NUM_READ_OUTSTANDING 1, NUM_WRITE_OUTSTANDING 1, MAX_BURST_LENGTH 1, PHASE 0.0, CLK_DOMAIN design_1_zynq_ultra_ps_e_0_0_pl_clk0, NUM_READ_THREADS 1, NUM_WRITE_THREADS 1, RUSER_BITS_PER_BYTE 0, WUSER_BITS_PER_BYTE 0, INSERT_VIP 0" *) input [7:0]s_axi_control_ARADDR;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 s_axi_control ARREADY" *) output s_axi_control_ARREADY;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 s_axi_control ARVALID" *) input s_axi_control_ARVALID;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 s_axi_control AWADDR" *) input [7:0]s_axi_control_AWADDR;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 s_axi_control AWREADY" *) output s_axi_control_AWREADY;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 s_axi_control AWVALID" *) input s_axi_control_AWVALID;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 s_axi_control BREADY" *) input s_axi_control_BREADY;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 s_axi_control BRESP" *) output [1:0]s_axi_control_BRESP;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 s_axi_control BVALID" *) output s_axi_control_BVALID;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 s_axi_control RDATA" *) output [31:0]s_axi_control_RDATA;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 s_axi_control RREADY" *) input s_axi_control_RREADY;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 s_axi_control RRESP" *) output [1:0]s_axi_control_RRESP;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 s_axi_control RVALID" *) output s_axi_control_RVALID;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 s_axi_control WDATA" *) input [31:0]s_axi_control_WDATA;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 s_axi_control WREADY" *) output s_axi_control_WREADY;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 s_axi_control WSTRB" *) input [3:0]s_axi_control_WSTRB;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 s_axi_control WVALID" *) input s_axi_control_WVALID;
  (* X_INTERFACE_INFO = "xilinx.com:signal:clock:1.0 ap_clk CLK" *) (* X_INTERFACE_MODE = "slave" *) (* X_INTERFACE_PARAMETER = "XIL_INTERFACENAME ap_clk, ASSOCIATED_BUSIF s_axi_control:m_axi_gmem0:m_axi_gmem1:m_axi_gmem2:m_axi_gmem3, ASSOCIATED_RESET ap_rst_n, FREQ_HZ 99990005, FREQ_TOLERANCE_HZ 0, PHASE 0.0, CLK_DOMAIN design_1_zynq_ultra_ps_e_0_0_pl_clk0, INSERT_VIP 0" *) input ap_clk /* synthesis syn_isclock = 1 */;
  (* X_INTERFACE_INFO = "xilinx.com:signal:reset:1.0 ap_rst_n RST" *) (* X_INTERFACE_MODE = "slave" *) (* X_INTERFACE_PARAMETER = "XIL_INTERFACENAME ap_rst_n, POLARITY ACTIVE_LOW, INSERT_VIP 0" *) input ap_rst_n;
  (* X_INTERFACE_INFO = "xilinx.com:signal:interrupt:1.0 interrupt INTERRUPT" *) (* X_INTERFACE_MODE = "master" *) (* X_INTERFACE_PARAMETER = "XIL_INTERFACENAME interrupt, SENSITIVITY LEVEL_HIGH, PortWidth 1" *) output interrupt;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem0 ARADDR" *) (* X_INTERFACE_MODE = "master" *) (* X_INTERFACE_PARAMETER = "XIL_INTERFACENAME m_axi_gmem0, NUM_READ_OUTSTANDING 16, NUM_WRITE_OUTSTANDING 1, MAX_READ_BURST_LENGTH 64, MAX_WRITE_BURST_LENGTH 16, MAX_BURST_LENGTH 256, PROTOCOL AXI4, READ_WRITE_MODE READ_ONLY, HAS_BURST 0, SUPPORTS_NARROW_BURST 0, ADDR_WIDTH 64, DATA_WIDTH 256, FREQ_HZ 99990005, ID_WIDTH 1, AWUSER_WIDTH 0, ARUSER_WIDTH 0, WUSER_WIDTH 0, RUSER_WIDTH 0, BUSER_WIDTH 0, HAS_LOCK 1, HAS_PROT 1, HAS_CACHE 1, HAS_QOS 1, HAS_REGION 1, HAS_WSTRB 1, HAS_BRESP 1, HAS_RRESP 1, PHASE 0.0, CLK_DOMAIN design_1_zynq_ultra_ps_e_0_0_pl_clk0, NUM_READ_THREADS 1, NUM_WRITE_THREADS 1, RUSER_BITS_PER_BYTE 0, WUSER_BITS_PER_BYTE 0, INSERT_VIP 0" *) output [63:0]m_axi_gmem0_ARADDR;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem0 ARBURST" *) output [1:0]m_axi_gmem0_ARBURST;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem0 ARCACHE" *) output [3:0]m_axi_gmem0_ARCACHE;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem0 ARID" *) output [0:0]m_axi_gmem0_ARID;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem0 ARLEN" *) output [7:0]m_axi_gmem0_ARLEN;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem0 ARLOCK" *) output [1:0]m_axi_gmem0_ARLOCK;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem0 ARPROT" *) output [2:0]m_axi_gmem0_ARPROT;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem0 ARQOS" *) output [3:0]m_axi_gmem0_ARQOS;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem0 ARREADY" *) input m_axi_gmem0_ARREADY;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem0 ARREGION" *) output [3:0]m_axi_gmem0_ARREGION;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem0 ARSIZE" *) output [2:0]m_axi_gmem0_ARSIZE;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem0 ARVALID" *) output m_axi_gmem0_ARVALID;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem0 AWADDR" *) output [63:0]m_axi_gmem0_AWADDR;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem0 AWBURST" *) output [1:0]m_axi_gmem0_AWBURST;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem0 AWCACHE" *) output [3:0]m_axi_gmem0_AWCACHE;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem0 AWID" *) output [0:0]m_axi_gmem0_AWID;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem0 AWLEN" *) output [7:0]m_axi_gmem0_AWLEN;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem0 AWLOCK" *) output [1:0]m_axi_gmem0_AWLOCK;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem0 AWPROT" *) output [2:0]m_axi_gmem0_AWPROT;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem0 AWQOS" *) output [3:0]m_axi_gmem0_AWQOS;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem0 AWREADY" *) input m_axi_gmem0_AWREADY;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem0 AWREGION" *) output [3:0]m_axi_gmem0_AWREGION;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem0 AWSIZE" *) output [2:0]m_axi_gmem0_AWSIZE;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem0 AWVALID" *) output m_axi_gmem0_AWVALID;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem0 BID" *) input [0:0]m_axi_gmem0_BID;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem0 BREADY" *) output m_axi_gmem0_BREADY;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem0 BRESP" *) input [1:0]m_axi_gmem0_BRESP;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem0 BVALID" *) input m_axi_gmem0_BVALID;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem0 RDATA" *) input [255:0]m_axi_gmem0_RDATA;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem0 RID" *) input [0:0]m_axi_gmem0_RID;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem0 RLAST" *) input m_axi_gmem0_RLAST;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem0 RREADY" *) output m_axi_gmem0_RREADY;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem0 RRESP" *) input [1:0]m_axi_gmem0_RRESP;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem0 RVALID" *) input m_axi_gmem0_RVALID;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem0 WDATA" *) output [255:0]m_axi_gmem0_WDATA;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem0 WID" *) output [0:0]m_axi_gmem0_WID;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem0 WLAST" *) output m_axi_gmem0_WLAST;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem0 WREADY" *) input m_axi_gmem0_WREADY;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem0 WSTRB" *) output [31:0]m_axi_gmem0_WSTRB;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem0 WVALID" *) output m_axi_gmem0_WVALID;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem1 ARADDR" *) (* X_INTERFACE_MODE = "master" *) (* X_INTERFACE_PARAMETER = "XIL_INTERFACENAME m_axi_gmem1, NUM_READ_OUTSTANDING 1, NUM_WRITE_OUTSTANDING 16, MAX_READ_BURST_LENGTH 16, MAX_WRITE_BURST_LENGTH 64, MAX_BURST_LENGTH 256, PROTOCOL AXI4, READ_WRITE_MODE WRITE_ONLY, HAS_BURST 0, SUPPORTS_NARROW_BURST 0, ADDR_WIDTH 64, DATA_WIDTH 256, FREQ_HZ 99990005, ID_WIDTH 1, AWUSER_WIDTH 0, ARUSER_WIDTH 0, WUSER_WIDTH 0, RUSER_WIDTH 0, BUSER_WIDTH 0, HAS_LOCK 1, HAS_PROT 1, HAS_CACHE 1, HAS_QOS 1, HAS_REGION 1, HAS_WSTRB 1, HAS_BRESP 1, HAS_RRESP 1, PHASE 0.0, CLK_DOMAIN design_1_zynq_ultra_ps_e_0_0_pl_clk0, NUM_READ_THREADS 1, NUM_WRITE_THREADS 1, RUSER_BITS_PER_BYTE 0, WUSER_BITS_PER_BYTE 0, INSERT_VIP 0" *) output [63:0]m_axi_gmem1_ARADDR;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem1 ARBURST" *) output [1:0]m_axi_gmem1_ARBURST;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem1 ARCACHE" *) output [3:0]m_axi_gmem1_ARCACHE;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem1 ARID" *) output [0:0]m_axi_gmem1_ARID;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem1 ARLEN" *) output [7:0]m_axi_gmem1_ARLEN;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem1 ARLOCK" *) output [1:0]m_axi_gmem1_ARLOCK;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem1 ARPROT" *) output [2:0]m_axi_gmem1_ARPROT;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem1 ARQOS" *) output [3:0]m_axi_gmem1_ARQOS;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem1 ARREADY" *) input m_axi_gmem1_ARREADY;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem1 ARREGION" *) output [3:0]m_axi_gmem1_ARREGION;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem1 ARSIZE" *) output [2:0]m_axi_gmem1_ARSIZE;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem1 ARVALID" *) output m_axi_gmem1_ARVALID;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem1 AWADDR" *) output [63:0]m_axi_gmem1_AWADDR;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem1 AWBURST" *) output [1:0]m_axi_gmem1_AWBURST;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem1 AWCACHE" *) output [3:0]m_axi_gmem1_AWCACHE;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem1 AWID" *) output [0:0]m_axi_gmem1_AWID;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem1 AWLEN" *) output [7:0]m_axi_gmem1_AWLEN;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem1 AWLOCK" *) output [1:0]m_axi_gmem1_AWLOCK;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem1 AWPROT" *) output [2:0]m_axi_gmem1_AWPROT;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem1 AWQOS" *) output [3:0]m_axi_gmem1_AWQOS;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem1 AWREADY" *) input m_axi_gmem1_AWREADY;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem1 AWREGION" *) output [3:0]m_axi_gmem1_AWREGION;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem1 AWSIZE" *) output [2:0]m_axi_gmem1_AWSIZE;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem1 AWVALID" *) output m_axi_gmem1_AWVALID;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem1 BID" *) input [0:0]m_axi_gmem1_BID;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem1 BREADY" *) output m_axi_gmem1_BREADY;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem1 BRESP" *) input [1:0]m_axi_gmem1_BRESP;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem1 BVALID" *) input m_axi_gmem1_BVALID;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem1 RDATA" *) input [255:0]m_axi_gmem1_RDATA;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem1 RID" *) input [0:0]m_axi_gmem1_RID;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem1 RLAST" *) input m_axi_gmem1_RLAST;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem1 RREADY" *) output m_axi_gmem1_RREADY;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem1 RRESP" *) input [1:0]m_axi_gmem1_RRESP;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem1 RVALID" *) input m_axi_gmem1_RVALID;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem1 WDATA" *) output [255:0]m_axi_gmem1_WDATA;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem1 WID" *) output [0:0]m_axi_gmem1_WID;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem1 WLAST" *) output m_axi_gmem1_WLAST;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem1 WREADY" *) input m_axi_gmem1_WREADY;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem1 WSTRB" *) output [31:0]m_axi_gmem1_WSTRB;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem1 WVALID" *) output m_axi_gmem1_WVALID;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem2 ARADDR" *) (* X_INTERFACE_MODE = "master" *) (* X_INTERFACE_PARAMETER = "XIL_INTERFACENAME m_axi_gmem2, NUM_READ_OUTSTANDING 16, NUM_WRITE_OUTSTANDING 1, MAX_READ_BURST_LENGTH 64, MAX_WRITE_BURST_LENGTH 16, MAX_BURST_LENGTH 256, PROTOCOL AXI4, READ_WRITE_MODE READ_ONLY, HAS_BURST 0, SUPPORTS_NARROW_BURST 0, ADDR_WIDTH 64, DATA_WIDTH 256, FREQ_HZ 99990005, ID_WIDTH 1, AWUSER_WIDTH 0, ARUSER_WIDTH 0, WUSER_WIDTH 0, RUSER_WIDTH 0, BUSER_WIDTH 0, HAS_LOCK 1, HAS_PROT 1, HAS_CACHE 1, HAS_QOS 1, HAS_REGION 1, HAS_WSTRB 1, HAS_BRESP 1, HAS_RRESP 1, PHASE 0.0, CLK_DOMAIN design_1_zynq_ultra_ps_e_0_0_pl_clk0, NUM_READ_THREADS 1, NUM_WRITE_THREADS 1, RUSER_BITS_PER_BYTE 0, WUSER_BITS_PER_BYTE 0, INSERT_VIP 0" *) output [63:0]m_axi_gmem2_ARADDR;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem2 ARBURST" *) output [1:0]m_axi_gmem2_ARBURST;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem2 ARCACHE" *) output [3:0]m_axi_gmem2_ARCACHE;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem2 ARID" *) output [0:0]m_axi_gmem2_ARID;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem2 ARLEN" *) output [7:0]m_axi_gmem2_ARLEN;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem2 ARLOCK" *) output [1:0]m_axi_gmem2_ARLOCK;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem2 ARPROT" *) output [2:0]m_axi_gmem2_ARPROT;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem2 ARQOS" *) output [3:0]m_axi_gmem2_ARQOS;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem2 ARREADY" *) input m_axi_gmem2_ARREADY;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem2 ARREGION" *) output [3:0]m_axi_gmem2_ARREGION;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem2 ARSIZE" *) output [2:0]m_axi_gmem2_ARSIZE;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem2 ARVALID" *) output m_axi_gmem2_ARVALID;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem2 AWADDR" *) output [63:0]m_axi_gmem2_AWADDR;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem2 AWBURST" *) output [1:0]m_axi_gmem2_AWBURST;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem2 AWCACHE" *) output [3:0]m_axi_gmem2_AWCACHE;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem2 AWID" *) output [0:0]m_axi_gmem2_AWID;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem2 AWLEN" *) output [7:0]m_axi_gmem2_AWLEN;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem2 AWLOCK" *) output [1:0]m_axi_gmem2_AWLOCK;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem2 AWPROT" *) output [2:0]m_axi_gmem2_AWPROT;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem2 AWQOS" *) output [3:0]m_axi_gmem2_AWQOS;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem2 AWREADY" *) input m_axi_gmem2_AWREADY;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem2 AWREGION" *) output [3:0]m_axi_gmem2_AWREGION;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem2 AWSIZE" *) output [2:0]m_axi_gmem2_AWSIZE;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem2 AWVALID" *) output m_axi_gmem2_AWVALID;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem2 BID" *) input [0:0]m_axi_gmem2_BID;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem2 BREADY" *) output m_axi_gmem2_BREADY;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem2 BRESP" *) input [1:0]m_axi_gmem2_BRESP;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem2 BVALID" *) input m_axi_gmem2_BVALID;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem2 RDATA" *) input [255:0]m_axi_gmem2_RDATA;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem2 RID" *) input [0:0]m_axi_gmem2_RID;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem2 RLAST" *) input m_axi_gmem2_RLAST;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem2 RREADY" *) output m_axi_gmem2_RREADY;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem2 RRESP" *) input [1:0]m_axi_gmem2_RRESP;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem2 RVALID" *) input m_axi_gmem2_RVALID;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem2 WDATA" *) output [255:0]m_axi_gmem2_WDATA;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem2 WID" *) output [0:0]m_axi_gmem2_WID;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem2 WLAST" *) output m_axi_gmem2_WLAST;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem2 WREADY" *) input m_axi_gmem2_WREADY;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem2 WSTRB" *) output [31:0]m_axi_gmem2_WSTRB;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem2 WVALID" *) output m_axi_gmem2_WVALID;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem3 ARADDR" *) (* X_INTERFACE_MODE = "master" *) (* X_INTERFACE_PARAMETER = "XIL_INTERFACENAME m_axi_gmem3, NUM_READ_OUTSTANDING 8, NUM_WRITE_OUTSTANDING 16, MAX_READ_BURST_LENGTH 32, MAX_WRITE_BURST_LENGTH 16, MAX_BURST_LENGTH 256, PROTOCOL AXI4, READ_WRITE_MODE READ_ONLY, HAS_BURST 0, SUPPORTS_NARROW_BURST 0, ADDR_WIDTH 64, DATA_WIDTH 32, FREQ_HZ 99990005, ID_WIDTH 1, AWUSER_WIDTH 0, ARUSER_WIDTH 0, WUSER_WIDTH 0, RUSER_WIDTH 0, BUSER_WIDTH 0, HAS_LOCK 1, HAS_PROT 1, HAS_CACHE 1, HAS_QOS 1, HAS_REGION 1, HAS_WSTRB 1, HAS_BRESP 1, HAS_RRESP 1, PHASE 0.0, CLK_DOMAIN design_1_zynq_ultra_ps_e_0_0_pl_clk0, NUM_READ_THREADS 1, NUM_WRITE_THREADS 1, RUSER_BITS_PER_BYTE 0, WUSER_BITS_PER_BYTE 0, INSERT_VIP 0" *) output [63:0]m_axi_gmem3_ARADDR;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem3 ARBURST" *) output [1:0]m_axi_gmem3_ARBURST;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem3 ARCACHE" *) output [3:0]m_axi_gmem3_ARCACHE;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem3 ARID" *) output [0:0]m_axi_gmem3_ARID;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem3 ARLEN" *) output [7:0]m_axi_gmem3_ARLEN;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem3 ARLOCK" *) output [1:0]m_axi_gmem3_ARLOCK;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem3 ARPROT" *) output [2:0]m_axi_gmem3_ARPROT;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem3 ARQOS" *) output [3:0]m_axi_gmem3_ARQOS;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem3 ARREADY" *) input m_axi_gmem3_ARREADY;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem3 ARREGION" *) output [3:0]m_axi_gmem3_ARREGION;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem3 ARSIZE" *) output [2:0]m_axi_gmem3_ARSIZE;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem3 ARVALID" *) output m_axi_gmem3_ARVALID;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem3 AWADDR" *) output [63:0]m_axi_gmem3_AWADDR;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem3 AWBURST" *) output [1:0]m_axi_gmem3_AWBURST;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem3 AWCACHE" *) output [3:0]m_axi_gmem3_AWCACHE;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem3 AWID" *) output [0:0]m_axi_gmem3_AWID;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem3 AWLEN" *) output [7:0]m_axi_gmem3_AWLEN;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem3 AWLOCK" *) output [1:0]m_axi_gmem3_AWLOCK;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem3 AWPROT" *) output [2:0]m_axi_gmem3_AWPROT;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem3 AWQOS" *) output [3:0]m_axi_gmem3_AWQOS;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem3 AWREADY" *) input m_axi_gmem3_AWREADY;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem3 AWREGION" *) output [3:0]m_axi_gmem3_AWREGION;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem3 AWSIZE" *) output [2:0]m_axi_gmem3_AWSIZE;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem3 AWVALID" *) output m_axi_gmem3_AWVALID;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem3 BID" *) input [0:0]m_axi_gmem3_BID;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem3 BREADY" *) output m_axi_gmem3_BREADY;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem3 BRESP" *) input [1:0]m_axi_gmem3_BRESP;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem3 BVALID" *) input m_axi_gmem3_BVALID;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem3 RDATA" *) input [31:0]m_axi_gmem3_RDATA;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem3 RID" *) input [0:0]m_axi_gmem3_RID;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem3 RLAST" *) input m_axi_gmem3_RLAST;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem3 RREADY" *) output m_axi_gmem3_RREADY;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem3 RRESP" *) input [1:0]m_axi_gmem3_RRESP;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem3 RVALID" *) input m_axi_gmem3_RVALID;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem3 WDATA" *) output [31:0]m_axi_gmem3_WDATA;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem3 WID" *) output [0:0]m_axi_gmem3_WID;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem3 WLAST" *) output m_axi_gmem3_WLAST;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem3 WREADY" *) input m_axi_gmem3_WREADY;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem3 WSTRB" *) output [3:0]m_axi_gmem3_WSTRB;
  (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 m_axi_gmem3 WVALID" *) output m_axi_gmem3_WVALID;
endmodule
