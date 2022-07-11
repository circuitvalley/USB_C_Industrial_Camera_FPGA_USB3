`timescale 1ns/1ns
//64bit pipeline 
module tb_csi_rx_lane_aligner_8b2lane;
	reg clk;
	reg [2:0]bytes_valid;
	reg  [15:0]bytes_i;
	wire [15:0]bytes_o;
	wire synced;
	reg reset;
	
wire reset_g;
GSR 
GSR_INST (
	.GSR_N(1'b1),
	.CLK(1'b0)
);
mipi_csi_rx_lane_aligner #(.MIPI_GEAR(8), .MIPI_LANES(2))  ins1(	.clk_i(clk),
						.reset_i(reset),
						.bytes_valid_i(bytes_valid),
						.byte_i(bytes_i),
						.lane_valid_o(synced),
						.lane_byte_o(bytes_o));

task sendbytes;
	input [15:0]bytes;
	begin
	bytes_i = bytes;
	clk = 1'b0;
	#4
	clk = 1'b1;
	#4;
	end
endtask

initial begin
		reset = 1'b0;
		clk = 1'b0;
		bytes_valid = 4'h0;
		#50

		sendbytes(16'h0000);
		reset = 1'h1;
		sendbytes(16'h0000);
		reset = 1'h0;
		sendbytes(16'h0000);
		sendbytes(16'h0000);
		bytes_valid[0] = 1'h1;
		sendbytes(16'h00B8);
		bytes_valid[1] = 1'h1;
		sendbytes(16'hB811);
		sendbytes(16'h1122);
		sendbytes(16'h2233);
		sendbytes(16'h3344);
		sendbytes(16'h4455);
		sendbytes(16'h5566);
		bytes_valid[0] = 1'h0;
		sendbytes(16'h6677);
		bytes_valid[1] = 1'h0;
		sendbytes(16'h0000);
		sendbytes(16'h0000);

		sendbytes(16'h0000);
		reset = 1'h1;
		sendbytes(16'h0000);
		reset = 1'h0;
		sendbytes(16'h0000);
		sendbytes(16'h0000);
		bytes_valid[0] = 1'h1;
		bytes_valid[1] = 1'h1;
		sendbytes(16'hB8B8);
		sendbytes(16'h1111);
		sendbytes(16'h2222);
		sendbytes(16'h3333);
		sendbytes(16'h4444);
		sendbytes(16'h5555);
		sendbytes(16'h6666);
		bytes_valid[0] = 1'h0;
		bytes_valid[1] = 1'h0;
		sendbytes(16'h7777);
		sendbytes(16'h0000);
		sendbytes(16'h0000);


		sendbytes(16'h0000);
		reset = 1'h1;
		sendbytes(16'h0000);
		reset = 1'h0;
		sendbytes(16'h0000);
		sendbytes(16'h0000);
		
		bytes_valid[1] = 1'h1;
		sendbytes(16'hB800);
		sendbytes(16'h1100);
		bytes_valid[0] = 1'h1;
		sendbytes(16'h22B8);
		sendbytes(16'h3311);
		sendbytes(16'h4422);
		sendbytes(16'h5533);
		sendbytes(16'h6644);
		bytes_valid[1] = 1'h0;
		sendbytes(16'h7755);
		sendbytes(16'h8866);
		bytes_valid[0] = 1'h0;
		sendbytes(16'h0000);
		sendbytes(16'h0000);
		sendbytes(16'h0000);
		
end

endmodule