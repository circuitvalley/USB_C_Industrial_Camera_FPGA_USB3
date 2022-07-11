`timescale 1ns/1ns

module tb_frame_detector8;
	reg clk;
	reg reset;
	reg input_valid;
	reg  [7:0]bytes_i;
	wire synced;
	
wire reset_g;

GSR 
GSR_INST (
	.GSR_N(1'b1),
	.CLK(1'b0)
);

frame_detector #(.MIPI_GEAR(8)) dec1(.reset_i(reset),
							 .clk_i(clk),
							 .data_valid_i(input_valid),
							 .data_lane0_i(bytes_i),
							 .detected_frame_sync_o(synced));

												
task sendbytes;
	input [7:0]bytes;
	begin
	bytes_i = bytes;
	clk = 1'b1;
	#4
	clk = 1'b0;
	#4;
	end
endtask

initial begin
		reset = 1'b1;
		clk = 1'b0;
		input_valid = 1'h0;
		#50
		clk = 1'b1;
		#4
		clk = 1'b0;
		#4
		clk = 1'b1;
		reset = 1'b0;
		sendbytes(8'h00);
		sendbytes(8'h00);
		sendbytes(8'h00);
		sendbytes(8'h00);
		sendbytes(8'h00);
		sendbytes(8'h00);
		sendbytes(8'h00);
		sendbytes(8'h00);
		input_valid = 1'h1;
		sendbytes(8'hB8);
		sendbytes(8'h00);
		sendbytes(8'hB8);
		sendbytes(8'h01);
		sendbytes(8'hB8);
		sendbytes(8'h01);
		sendbytes(8'hB8);
		sendbytes(8'h01);
		sendbytes(8'h03);
		sendbytes(8'hB8);
		sendbytes(8'h01);
		sendbytes(8'h03);
		sendbytes(8'h03);
		input_valid = 1'h0;
		sendbytes(8'h00);
		sendbytes(8'h00);
		sendbytes(8'h00);
		sendbytes(8'h00);
		sendbytes(8'h00);
		sendbytes(8'h00);
		sendbytes(8'h00);
		sendbytes(8'h00);
		input_valid = 1'h1;
		sendbytes(8'hB8);
		sendbytes(8'h2B);
		sendbytes(8'h03);
		input_valid = 1'h0;
		sendbytes(8'h00);
		sendbytes(8'h00);
		sendbytes(8'h00);
		sendbytes(8'h00);
		sendbytes(8'h00);
		sendbytes(8'h00);
		sendbytes(8'h00);
		sendbytes(8'h00);
		input_valid = 1'h1;
		sendbytes(8'hB8);
		sendbytes(8'h2C);
		sendbytes(8'h03);
		input_valid = 1'h0;
		sendbytes(8'h00);
		sendbytes(8'h00);
		sendbytes(8'h00);
		sendbytes(8'h00);
		sendbytes(8'h00);
		sendbytes(8'h00);
		sendbytes(8'h00);
		sendbytes(8'h00);
		input_valid = 1'h1;
		sendbytes(8'hB8);
		sendbytes(8'h2D);
		sendbytes(8'h03);
		sendbytes(8'h03);
		sendbytes(8'h03);
		sendbytes(8'h03);
		sendbytes(8'h03);
		sendbytes(8'hB8);
		sendbytes(8'h01);
		sendbytes(8'h03);
		input_valid = 1'h0;
		sendbytes(8'h00);
		sendbytes(8'h00);
		sendbytes(8'h00);
		sendbytes(8'h00);
		sendbytes(8'h00);
		sendbytes(8'h00);
		sendbytes(8'h00);
		sendbytes(8'h00);
		input_valid = 1'h1;
		sendbytes(8'hB8);
		sendbytes(8'h01);
		sendbytes(8'h03);
		input_valid = 1'h0;
		sendbytes(8'h00);
		sendbytes(8'h00);
		sendbytes(8'h00);
		sendbytes(8'h00);
		sendbytes(8'h00);
		sendbytes(8'h00);
		sendbytes(8'h00);
		sendbytes(8'h00);
		
		reset = 1'b1;
		
end

endmodule
