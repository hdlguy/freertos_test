`timescale 1ns/1ps

module stream_gen_tb ();

    localparam integer fsize = 4;

    localparam clk_period = 10; logic clk = 0; always #(clk_period/2) clk = ~clk;

    logic           aresetn;
    logic [15:0]    frame_size;
    logic [31:0]    tdata;
    logic  [3:0]    tkeep;
    logic           tlast;
    logic           tready;
    logic           tvalid;

    stream_gen uut (.*);

    assign frame_size = fsize-1;

    initial begin
        aresetn = 0;
        #(clk_period*10);
        aresetn = 1;
    end
    
    integer seed = 1;
    always_ff @(posedge clk) begin
        if (aresetn == 0) begin
            tready <= 0;
        end else begin
            tready <= $random(seed);
        end
    end

endmodule


