//
module top (
    output  logic [15:0]    pmod_tri_o,
    output  logic [7:0]     leds_8bits_tri_o
);

    logic[3:0] pulse;
    system system_i (
        .leds_8bits_tri_o   (leds_8bits_tri_o),        
        .pmod_tri_o         (pmod_tri_o),
        .axi_aclk           (axi_aclk),
        .pulse              (pulse)
    );
    
    intr_pulse_gen #(.max_count(   100000)) pulse_gen3 (.clk(axi_aclk), .pulse(pulse[3]));
    intr_pulse_gen #(.max_count(  1000000)) pulse_gen2 (.clk(axi_aclk), .pulse(pulse[2]));
    intr_pulse_gen #(.max_count( 10000000)) pulse_gen1 (.clk(axi_aclk), .pulse(pulse[1]));
    intr_pulse_gen #(.max_count(100000000)) pulse_gen0 (.clk(axi_aclk), .pulse(pulse[0]));
    
endmodule
