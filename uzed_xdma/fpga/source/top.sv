//
module top (
    output  logic [15:0]    pmod_tri_o,
    output  logic [7:0]     leds_8bits_tri_o
);

    logic axi_aclk, axi_aresetn;
    
    logic[31:0] axi_count;    
    always_ff @(posedge axi_aclk) axi_count <= axi_count + 1;
    
    logic [31:0] stream_in_tdata;
    logic [3:0]  stream_in_tkeep;
    logic        stream_in_tlast;
    logic        stream_in_tready;
    logic        stream_in_tvalid;   

    system system_i (
        .axi_aclk           (axi_aclk),
        .axi_aresetn        (axi_aresetn),
        //
        .leds_8bits_tri_o   (leds_8bits_tri_o),        
        .pmod_tri_o         (pmod_tri_o),
        //
        .pl_ps_irq1         (axi_count[31:24]),
        //
        .stream_in_tdata    (stream_in_tdata),
        .stream_in_tkeep    (stream_in_tkeep),
        .stream_in_tlast    (stream_in_tlast),
        .stream_in_tready   (stream_in_tready),
        .stream_in_tvalid   (stream_in_tvalid)
    );    
    
    logic [31:0] frame_size;
    assign frame_size = 32'h0000_0800 - 1;
    logic [15:0] data_rate;
    assign data_rate = 100-1;
    
    stream_gen stream_gen_inst(
        .clk        (axi_aclk), 
        .aresetn    (axi_aresetn), 
        .frame_size (frame_size),
        .data_rate  (data_rate),
        .tdata      (stream_in_tdata), 
        .tkeep      (stream_in_tkeep),
        .tlast      (stream_in_tlast),
        .tready     (stream_in_tready),
        .tvalid     (stream_in_tvalid)
    );
    
endmodule

