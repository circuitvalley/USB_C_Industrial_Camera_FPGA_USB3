`timescale 1ns/1ns
//64bit pipeline 
module tb_csi_rx_lane_aligner;
	reg clk;
	reg [3:0]bytes_valid;
	reg  [63:0]bytes_i;
	wire [63:0]bytes_o;
	wire synced;
	reg reset;
	
wire reset_g;
GSR 
GSR_INST (
	.GSR_N(1'b1),
	.CLK(1'b0)
);
mipi_csi_rx_lane_aligner ins1(	.clk_i(clk),
						.reset_i(reset),
						.bytes_valid_i(bytes_valid),
						.byte_i(bytes_i),
						.lane_valid_o(synced),
						.lane_byte_o(bytes_o));

task sendbytes;
	input [63:0]bytes;
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

		sendbytes(64'h00000000);
		reset = 1'h1;
		sendbytes(64'h00000000);
		reset = 1'h0;
		sendbytes(64'h00000000);
		sendbytes(64'h00000000);
		bytes_valid[1] = 1'h1;
		sendbytes(64'h0000000011B80000);
		sendbytes(64'h0000000033220000);
		bytes_valid[2] = 1'h1;
		bytes_valid[3] = 1'h1;
		bytes_valid[0] = 1'h1;
		sendbytes(64'h11B811B8554411B8);
		sendbytes(64'h3322332277663322);
		sendbytes(64'h5544554499885544);
		bytes_valid[1] = 1'h0;
		sendbytes(64'h7766776600007766);
		sendbytes(64'h9988998800009988);
		bytes_valid[2] = 1'h0;
		bytes_valid[3] = 1'h0;
		bytes_valid[0] = 1'h0;
		sendbytes(64'h00000000);
		sendbytes(64'h00000000);
		
		sendbytes(64'h00000000);
		sendbytes(64'h00000000);		
		sendbytes(64'h00000000);
		sendbytes(64'h00000000);
		sendbytes(64'h00000000);
		sendbytes(64'h00000000);
		sendbytes(64'h00000000);
		bytes_valid[1] = 1'h1;
		sendbytes(64'h0000000011B80000);
		sendbytes(64'h0000000033220000);
		bytes_valid[2] = 1'h1;
		bytes_valid[0] = 1'h1;
		sendbytes(64'h000011B8554411B8);
		bytes_valid[3] = 1'h1;
		sendbytes(64'h11B8332277663322);
		sendbytes(64'h3322554499885544);
		bytes_valid[1] = 1'h0;
		sendbytes(64'h5544776600007766);
		sendbytes(64'h7766998800009988);
		bytes_valid[0] = 1'h0;
		bytes_valid[2] = 1'h0;
		sendbytes(64'h9988000000000000);
		bytes_valid[3] = 1'h0;
		sendbytes(64'h00000000);
		sendbytes(64'h00000000);
		sendbytes(64'h00000000);
		sendbytes(64'h00000000);
		sendbytes(64'h00000000);
		sendbytes(64'h00000000);


end

endmodule