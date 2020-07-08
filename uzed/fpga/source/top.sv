//
module top (
    output  logic [15:0]    pmod_tri_o,
    output  logic [7:0]     leds_8bits_tri_o
);

    localparam axi_div = 1000000;
    logic axi_aclk, pps;
    logic[31:0] axi_count; 
    always_ff @(posedge axi_aclk) begin
        if (0 == axi_count) begin
            axi_count <= axi_div-1;
            pps <= 1;
        end else begin
            axi_count <= axi_count - 1;
            pps <= 0;
        end
    end

    system system_i (
        .leds_8bits_tri_o   (leds_8bits_tri_o),        
        .pmod_tri_o         (pmod_tri_o),
        //
        .axi_aclk           (axi_aclk),
        .pps                (pps)
    );
    
endmodule
