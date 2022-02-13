//
module top (
    output  logic[13:0] ddr3_sdram_addr,
    output  logic[2:0]  ddr3_sdram_ba,
    output  logic       ddr3_sdram_cas_n,
    output  logic[0:0]  ddr3_sdram_ck_n,
    output  logic[0:0]  ddr3_sdram_ck_p,
    output  logic[0:0]  ddr3_sdram_cke,
    output  logic[0:0]  ddr3_sdram_cs_n,
    output  logic[1:0]  ddr3_sdram_dm,
    inout   logic[15:0] ddr3_sdram_dq,
    inout   logic[1:0]  ddr3_sdram_dqs_n,
    inout   logic[1:0]  ddr3_sdram_dqs_p,
    output  logic[0:0]  ddr3_sdram_odt,
    output  logic       ddr3_sdram_ras_n,
    output  logic       ddr3_sdram_reset_n,
    output  logic       ddr3_sdram_we_n,
    //input   logic[3:0]  dip_switches_4bits_tri_i,
    output  logic       eth_mdio_mdc_mdc,
    inout   logic       eth_mdio_mdc_mdio_io,
    input   logic       eth_mii_col,
    input   logic       eth_mii_crs,
    output  logic       eth_mii_rst_n,
    input   logic       eth_mii_rx_clk,
    input   logic       eth_mii_rx_dv,
    input   logic       eth_mii_rx_er,
    input   logic[3:0]  eth_mii_rxd,
    input   logic       eth_mii_tx_clk,
    output  logic       eth_mii_tx_en,
    output  logic[3:0]  eth_mii_txd,
    output  logic       eth_refclk,
    output  logic[3:0]  led,
    //input   logic[3:0]  push_buttons_4bits_tri_i,
    inout   logic       qspi_flash_io0_io,
    inout   logic       qspi_flash_io1_io,
    inout   logic       qspi_flash_io2_io,
    inout   logic       qspi_flash_io3_io,
    inout   logic       qspi_flash_sck_io,
    inout   logic       qspi_flash_ss_io,
    input   logic       resetn,
    input   logic       sys_clock,
    input   logic       usb_uart_rxd,
    output  logic       usb_uart_txd
);

    IOBUF eth_mdio_mdc_mdio_iobuf     (.I(eth_mdio_mdc_mdio_o), .IO(eth_mdio_mdc_mdio_io), .O(eth_mdio_mdc_mdio_i), .T(eth_mdio_mdc_mdio_t));
    IOBUF qspi_flash_io0_iobuf        (.I(qspi_flash_io0_o), .IO(qspi_flash_io0_io), .O(qspi_flash_io0_i), .T(qspi_flash_io0_t));
    IOBUF qspi_flash_io1_iobuf        (.I(qspi_flash_io1_o), .IO(qspi_flash_io1_io), .O(qspi_flash_io1_i), .T(qspi_flash_io1_t));
    IOBUF qspi_flash_io2_iobuf        (.I(qspi_flash_io2_o), .IO(qspi_flash_io2_io), .O(qspi_flash_io2_i), .T(qspi_flash_io2_t));
    IOBUF qspi_flash_io3_iobuf        (.I(qspi_flash_io3_o), .IO(qspi_flash_io3_io), .O(qspi_flash_io3_i), .T(qspi_flash_io3_t));
    IOBUF qspi_flash_sck_iobuf        (.I(qspi_flash_sck_o), .IO(qspi_flash_sck_io), .O(qspi_flash_sck_i), .T(qspi_flash_sck_t));
    IOBUF qspi_flash_ss_iobuf         (.I(qspi_flash_ss_o),  .IO(qspi_flash_ss_io),  .O(qspi_flash_ss_i),  .T(qspi_flash_ss_t));

    logic [31:0]M06_AXI_araddr;
    logic [2:0]M06_AXI_arprot;
    logic [0:0]M06_AXI_arready;
    logic [0:0]M06_AXI_arvalid;
    logic [31:0]M06_AXI_awaddr;
    logic [2:0]M06_AXI_awprot;
    logic [0:0]M06_AXI_awready;
    logic [0:0]M06_AXI_awvalid;
    logic [0:0]M06_AXI_bready;
    logic [1:0]M06_AXI_bresp;
    logic [0:0]M06_AXI_bvalid;
    logic [31:0]M06_AXI_rdata;
    logic [0:0]M06_AXI_rready;
    logic [1:0]M06_AXI_rresp;
    logic [0:0]M06_AXI_rvalid;
    logic [31:0]M06_AXI_wdata;
    logic [0:0]M06_AXI_wready;
    logic [3:0]M06_AXI_wstrb;
    logic [0:0]M06_AXI_wvalid;
    logic axi_aclk;
    logic [0:0]axi_aresetn;

    system system_i (
        .ddr3_sdram_addr(ddr3_sdram_addr),
        .ddr3_sdram_ba(ddr3_sdram_ba),
        .ddr3_sdram_cas_n(ddr3_sdram_cas_n),
        .ddr3_sdram_ck_n(ddr3_sdram_ck_n),
        .ddr3_sdram_ck_p(ddr3_sdram_ck_p),
        .ddr3_sdram_cke(ddr3_sdram_cke),
        .ddr3_sdram_cs_n(ddr3_sdram_cs_n),
        .ddr3_sdram_dm(ddr3_sdram_dm),
        .ddr3_sdram_dq(ddr3_sdram_dq),
        .ddr3_sdram_dqs_n(ddr3_sdram_dqs_n),
        .ddr3_sdram_dqs_p(ddr3_sdram_dqs_p),
        .ddr3_sdram_odt(ddr3_sdram_odt),
        .ddr3_sdram_ras_n(ddr3_sdram_ras_n),
        .ddr3_sdram_reset_n(ddr3_sdram_reset_n),
        .ddr3_sdram_we_n(ddr3_sdram_we_n),
        // axi interface to register file.
        .M06_AXI_araddr(M06_AXI_araddr),
        .M06_AXI_arprot(M06_AXI_arprot),
        .M06_AXI_arready(M06_AXI_arready),
        .M06_AXI_arvalid(M06_AXI_arvalid),
        .M06_AXI_awaddr(M06_AXI_awaddr),
        .M06_AXI_awprot(M06_AXI_awprot),
        .M06_AXI_awready(M06_AXI_awready),
        .M06_AXI_awvalid(M06_AXI_awvalid),
        .M06_AXI_bready(M06_AXI_bready),
        .M06_AXI_bresp(M06_AXI_bresp),
        .M06_AXI_bvalid(M06_AXI_bvalid),
        .M06_AXI_rdata(M06_AXI_rdata),
        .M06_AXI_rready(M06_AXI_rready),
        .M06_AXI_rresp(M06_AXI_rresp),
        .M06_AXI_rvalid(M06_AXI_rvalid),
        .M06_AXI_wdata(M06_AXI_wdata),
        .M06_AXI_wready(M06_AXI_wready),
        .M06_AXI_wstrb(M06_AXI_wstrb),
        .M06_AXI_wvalid(M06_AXI_wvalid),
        .axi_aclk(axi_aclk),
        .axi_aresetn(axi_aresetn),
        //.dip_switches_4bits_tri_i(dip_switches_4bits_tri_i),
        .eth_mdio_mdc_mdc(eth_mdio_mdc_mdc),
        .eth_mdio_mdc_mdio_i(eth_mdio_mdc_mdio_i),
        .eth_mdio_mdc_mdio_o(eth_mdio_mdc_mdio_o),
        .eth_mdio_mdc_mdio_t(eth_mdio_mdc_mdio_t),
        .eth_mii_col(eth_mii_col),
        .eth_mii_crs(eth_mii_crs),
        .eth_mii_rst_n(eth_mii_rst_n),
        .eth_mii_rx_clk(eth_mii_rx_clk),
        .eth_mii_rx_dv(eth_mii_rx_dv),
        .eth_mii_rx_er(eth_mii_rx_er),
        .eth_mii_rxd(eth_mii_rxd),
        .eth_mii_tx_clk(eth_mii_tx_clk),
        .eth_mii_tx_en(eth_mii_tx_en),
        .eth_mii_txd(eth_mii_txd),
        .eth_refclk(eth_refclk),
        //.push_buttons_4bits_tri_i(push_buttons_4bits_tri_i),
        .qspi_flash_io0_i(qspi_flash_io0_i),
        .qspi_flash_io0_o(qspi_flash_io0_o),
        .qspi_flash_io0_t(qspi_flash_io0_t),
        .qspi_flash_io1_i(qspi_flash_io1_i),
        .qspi_flash_io1_o(qspi_flash_io1_o),
        .qspi_flash_io1_t(qspi_flash_io1_t),
        .qspi_flash_io2_i(qspi_flash_io2_i),
        .qspi_flash_io2_o(qspi_flash_io2_o),
        .qspi_flash_io2_t(qspi_flash_io2_t),
        .qspi_flash_io3_i(qspi_flash_io3_i),
        .qspi_flash_io3_o(qspi_flash_io3_o),
        .qspi_flash_io3_t(qspi_flash_io3_t),
        .qspi_flash_sck_i(qspi_flash_sck_i),
        .qspi_flash_sck_o(qspi_flash_sck_o),
        .qspi_flash_sck_t(qspi_flash_sck_t),
        .qspi_flash_ss_i(qspi_flash_ss_i),
        .qspi_flash_ss_o(qspi_flash_ss_o),
        .qspi_flash_ss_t(qspi_flash_ss_t),
        .sys_clock(sys_clock),
        .resetn(resetn),
        .usb_uart_rxd(usb_uart_rxd),
        .usb_uart_txd(usb_uart_txd)
    );
    
//    eth_ila eth_ila_inst (.clk(axi_aclk), 
//        .probe0({eth_mdio_mdc_mdc,eth_mdio_mdc_mdio_i,eth_mdio_mdc_mdio_o,eth_mdio_mdc_mdio_t}),
//        .probe1({eth_mii_col, eth_mii_crs, eth_mii_rst_n, 1'b0, eth_mii_rx_dv, eth_mii_rx_er, eth_mii_rxd, 1'b0, eth_mii_tx_en, eth_mii_txd}) // 16
//    );



    localparam int Nreg = 16;
    localparam int Nreg_addr = $clog2(Nreg) + 2;

    logic [Nreg-1:0][31:0] slv_reg, slv_read;


    assign slv_read[0] = 32'h07010100;
    assign slv_read[1] = 32'hdeadbeef;
//    assign slv_read[0] = 32'h53524556; // VERS
//    assign slv_read[1] = 32'h314e4f49; // ION1
    
    assign led = slv_reg[2][3:0];

    assign slv_read[Nreg-1:2]    = slv_reg[Nreg-1:2];


	axi_regfile_v1_0_S00_AXI #	(
		.C_S_AXI_DATA_WIDTH(32),
		.C_S_AXI_ADDR_WIDTH(Nreg_addr)
	) axi_regfile_inst (
        // register interface
        .slv_read(slv_read), 
        .slv_reg (slv_reg),  
        .slv_wr_pulse(),
        // axi interface
		.S_AXI_ACLK    (axi_aclk),
		.S_AXI_ARESETN (axi_aresetn),
        //
		.S_AXI_ARADDR  (M06_AXI_araddr ),
		.S_AXI_ARPROT  (M06_AXI_arprot ),
		.S_AXI_ARREADY (M06_AXI_arready),
		.S_AXI_ARVALID (M06_AXI_arvalid),
		.S_AXI_AWADDR  (M06_AXI_awaddr ),
		.S_AXI_AWPROT  (M06_AXI_awprot ),
		.S_AXI_AWREADY (M06_AXI_awready),
		.S_AXI_AWVALID (M06_AXI_awvalid),
		.S_AXI_BREADY  (M06_AXI_bready ),
		.S_AXI_BRESP   (M06_AXI_bresp  ),
		.S_AXI_BVALID  (M06_AXI_bvalid ),
		.S_AXI_RDATA   (M06_AXI_rdata  ),
		.S_AXI_RREADY  (M06_AXI_rready ),
		.S_AXI_RRESP   (M06_AXI_rresp  ),
		.S_AXI_RVALID  (M06_AXI_rvalid ),
		.S_AXI_WDATA   (M06_AXI_wdata  ),
		.S_AXI_WREADY  (M06_AXI_wready ),
		.S_AXI_WSTRB   (M06_AXI_wstrb  ),
		.S_AXI_WVALID  (M06_AXI_wvalid )
	);


endmodule
