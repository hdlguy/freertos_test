//
module top (
    output  logic [15:0]    pmod_tri_o,
    output  logic [7:0]     leds_8bits_tri_o
);

    logic axi_aclk;
    logic[31:0] axi_count;
    
    always_ff @(posedge axi_aclk) axi_count <= axi_count + 1;

    system system_i (
        .leds_8bits_tri_o   (leds_8bits_tri_o),        
        .pmod_tri_o         (pmod_tri_o),
        .axi_aclk           (axi_aclk),
        .pl_ps_irq1         (axi_count[31:24]),
        .pl_ps_irq0         (axi_count[23:16])
    );    
    
endmodule
