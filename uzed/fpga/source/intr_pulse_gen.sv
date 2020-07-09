
module intr_pulse_gen #(
    parameter max_count = 100000000
)(
    input  logic  clk,
    output logic  pulse
);

    logic[31:0] count;
    always_ff @(posedge clk) begin
        if (0 == count) begin
            count <= max_count - 1;
        end else begin
            count <= count - 1;
        end
        if (count < 8) pulse <= 1; else pulse <= 0;
    end
 
endmodule
