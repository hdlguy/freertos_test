//
module stream_gen (
    input   logic           clk,
    input   logic           aresetn,
    output  logic [31:0]    tdata,
    output  logic  [3:0]    tkeep,
    output  logic           tlast,
    input   logic           tready,
    output  logic           tvalid   
);

    always_ff @(posedge clk) begin
        if (0 == aresetn) begin
            tvalid <= 0;
            tdata <= 0;
        end else begin
            tvalid <= 1;
            if ( (1==tvalid) && (1==tready) ) begin
                dtata <= tdata + 1;
            end
        end
    end

endmodule

