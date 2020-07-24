// A little data generator to test Stream to Memory Mapped (S2MM) DMA.
// A 32 bit count is produced with flow control.
module stream_gen (
    input   logic           clk,
    input   logic           aresetn,
    // parameters
    input   logic [31:0]    frame_size,  // number of words between tlast assertions.
    input   logic [15:0]    data_rate,   // (clock rate)/(word rate) - 1 = (100e6)/(Fs/4) - 1.
    // AXI Stream Interface
    output  logic [31:0]    tdata,
    output  logic  [3:0]    tkeep,
    output  logic           tlast,
    input   logic           tready,
    output  logic           tvalid   
);


    logic[15:0] ce_count = 0;
    logic ce;
    always_ff @(posedge clk) begin
        if (0 == ce_count) begin
            ce <= 1;
            ce_count <= data_rate;
        end else begin
            ce <= 0;
            ce_count <= ce_count - 1;
        end
    end

    logic s_axis_tvalid, s_axis_tlast, s_axis_tready;
    logic [31:0] s_axis_tdata, m_axis_tdata;

    assign tkeep = 4'b1111;
    logic [31:0] count;
    always_ff @(posedge clk) begin
        if (0 == aresetn) begin
            s_axis_tvalid <= 0;
            s_axis_tdata  <= 0;
            s_axis_tlast  <= 0;
            count <= frame_size;
        end else begin
            if (1 == ce) begin
                s_axis_tvalid <= 1;
                s_axis_tdata <= tdata + 1;
                if (0 == count) begin
                    count <= frame_size;
                end else begin
                    count <= count - 1;
                end
                if (1 == count) s_axis_tlast <= 1; else s_axis_tlast  <= 0;
            end else begin
                s_axis_tvalid <= 0;
                s_axis_tlast <= 0;
            end
        end
    end

    stream_gen_fifo fifo_inst (
        .s_aclk         (clk),
        .s_aresetn      (aresetn),
        //
        .s_axis_tvalid  (s_axis_tvalid),
        .s_axis_tready  (s_axis_tready),
        .s_axis_tdata   (s_axis_tdata),
        .s_axis_tlast   (s_axis_tlast),
        //
        .m_axis_tvalid  (tvalid),
        .m_axis_tready  (tready),
        .m_axis_tdata   (tdata),
        .m_axis_tlast   (tlast)
    );
    
endmodule


