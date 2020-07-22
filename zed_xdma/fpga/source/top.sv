//
module top (
    inout   logic [14:0]    DDR_addr,
    inout   logic [2:0]     DDR_ba,
    inout   logic           DDR_cas_n,
    inout   logic           DDR_ck_n,
    inout   logic           DDR_ck_p,
    inout   logic           DDR_cke,
    inout   logic           DDR_cs_n,
    inout   logic [3:0]     DDR_dm,
    inout   logic [31:0]    DDR_dq,
    inout   logic [3:0]     DDR_dqs_n,
    inout   logic [3:0]     DDR_dqs_p,
    inout   logic           DDR_odt,
    inout   logic           DDR_ras_n,
    inout   logic           DDR_reset_n,
    inout   logic           DDR_we_n,
    inout   logic           FIXED_IO_ddr_vrn,
    inout   logic           FIXED_IO_ddr_vrp,
    inout   logic [53:0]    FIXED_IO_mio,
    inout   logic           FIXED_IO_ps_clk,
    inout   logic           FIXED_IO_ps_porb,
    inout   logic           FIXED_IO_ps_srstb,
    //
    output  logic [15:0]    pmod_tri_o,
    output  logic [7:0]     leds_8bits_tri_o
);

    logic axi_aclk, axi_aresetn;
    logic[31:0] axi_count; 
    always_ff @(posedge axi_aclk) axi_count <= axi_count + 1;
    
    logic        dma_introut;
    logic [31:0] stream_tdata;
    logic [3:0]  stream_tkeep;
    logic        stream_tlast;
    logic        stream_tready;
    logic        stream_tvalid;
    logic [31:0] frame_size;
    logic [15:0] data_rate;

    system system_i (
        .DDR_addr           (DDR_addr),
        .DDR_ba             (DDR_ba),
        .DDR_cas_n          (DDR_cas_n),
        .DDR_ck_n           (DDR_ck_n),
        .DDR_ck_p           (DDR_ck_p),
        .DDR_cke            (DDR_cke),
        .DDR_cs_n           (DDR_cs_n),
        .DDR_dm             (DDR_dm),
        .DDR_dq             (DDR_dq),
        .DDR_dqs_n          (DDR_dqs_n),
        .DDR_dqs_p          (DDR_dqs_p),
        .DDR_odt            (DDR_odt),
        .DDR_ras_n          (DDR_ras_n),
        .DDR_reset_n        (DDR_reset_n),
        .DDR_we_n           (DDR_we_n),
        .FIXED_IO_ddr_vrn   (FIXED_IO_ddr_vrn),
        .FIXED_IO_ddr_vrp   (FIXED_IO_ddr_vrp),
        .FIXED_IO_mio       (FIXED_IO_mio),
        .FIXED_IO_ps_clk    (FIXED_IO_ps_clk),
        .FIXED_IO_ps_porb   (FIXED_IO_ps_porb),
        .FIXED_IO_ps_srstb  (FIXED_IO_ps_srstb),
        //
        .leds_8bits_tri_o   (leds_8bits_tri_o),        
        .pmod_tri_o         (pmod_tri_o),        
        .frame_size_tri_o   (frame_size),
        //
        .axi_aclk           (axi_aclk),
        .axi_aresetn        (axi_aresetn),
        //
        .irq                ({axi_count[31:17],dma_introut}),  // Interrupt ID = [91:84], [68:61]
        //
        .dma_introut(dma_introut),
        //
        .stream_tdata   (stream_tdata),
        .stream_tkeep   (stream_tkeep),
        .stream_tlast   (stream_tlast),
        .stream_tready  (stream_tready),
        .stream_tvalid  (stream_tvalid)
    );
    
    assign data_rate = 100-1;
    
    stream_gen stream_gen_inst(
        .clk        (axi_aclk),
        .aresetn    (axi_aresetn),
        .frame_size (frame_size),
        .data_rate  (data_rate),
        .tdata      (stream_tdata),
        .tkeep      (stream_tkeep),
        .tlast      (stream_tlast),
        .tready     (stream_tready),
        .tvalid     (stream_tvalid)
    );
    
endmodule

