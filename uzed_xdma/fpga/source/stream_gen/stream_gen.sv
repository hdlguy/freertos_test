//
module stream_gen (
    input   logic           clk,
    input   logic           aresetn,
    //
    input   logic [15:0]    frame_size,
    //
    output  logic [31:0]    tdata,
    output  logic  [3:0]    tkeep,
    output  logic           tlast,
    input   logic           tready,
    output  logic           tvalid   
);

    assign tkeep = 4'b1111;
    
    logic [15:0] count;
    always_ff @(posedge clk) begin
        if (0 == aresetn) begin
            tvalid <= 0;
            tdata <= 0;
            count <= frame_size;
            tlast <= 0;
        end else begin
            tvalid <= 1;
            if ( (1==tvalid) && (1==tready) ) begin
                tdata <= tdata + 1;
                if (0 == count) begin
                    count <= frame_size;
                end else begin
                    count <= count - 1;
                end
                if (1 == count) tlast <= 1; else tlast  <= 0;
            end
        end
    end
    
    //assign tlast = (0 == count)  ? 1'b1  : 1'b0;

endmodule

