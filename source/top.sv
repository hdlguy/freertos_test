//
module top ();

    logic[31:0] S_AXIS_S2MM_0_tdata;
    logic[3:0]  S_AXIS_S2MM_0_tkeep;
    logic       S_AXIS_S2MM_0_tlast;
    logic       S_AXIS_S2MM_0_tready;
    logic       S_AXIS_S2MM_0_tvalid;
    logic       axi_aclk;
    logic[0:0]  axi_areset;

    system system_i(
        .S_AXIS_S2MM_0_tdata    (S_AXIS_S2MM_0_tdata),
        .S_AXIS_S2MM_0_tkeep    (S_AXIS_S2MM_0_tkeep),
        .S_AXIS_S2MM_0_tlast    (S_AXIS_S2MM_0_tlast),
        .S_AXIS_S2MM_0_tready   (S_AXIS_S2MM_0_tready),
        .S_AXIS_S2MM_0_tvalid   (S_AXIS_S2MM_0_tvalid),
        .axi_aclk               (axi_aclk),
        .axi_aresetn            (axi_aresetn)
    );
    
    assign S_AXIS_S2MM_0_tvalid = 1;
    assign S_AXIS_S2MM_0_tkeep  = 4'hf;
    assign S_AXIS_S2MM_0_tdata  = count;
    logic[31:0] count;
    always_ff @(posedge axi_aclk) begin
        if (0 == axi_aresetn) begin
            count <= 0;
        end else begin
            if (1 == S_AXIS_S2MM_0_tready) begin
                count <= count + 1;
            end
        end       
    end    
    always_comb if (1 == (&count[3:0])) S_AXIS_S2MM_0_tlast = 1; else S_AXIS_S2MM_0_tlast = 0;
            
endmodule
