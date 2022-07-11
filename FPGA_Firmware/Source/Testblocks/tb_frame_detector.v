`timescale 1ns/1ns

module tb_frame_detector;
	reg clk;
	reg reset;
	reg input_valid;
	reg  [15:0]bytes_i;
	wire synced;
	
wire reset_g;

GSR 
GSR_INST (
	.GSR_N(1'b1),
	.CLK(1'b0)
);

frame_detector #(.MIPI_GEAR(16)) dec1(.reset_i(reset),
							 .clk_i(clk),
							 .data_valid_i(input_valid),
							 .data_lane0_i(bytes_i),
							 .detected_frame_sync_o(synced));

												
task sendbytes;
	input [15:0]bytes;
	begin
	bytes_i = bytes;
	clk = 1'b1;
	#4
	clk = 1'b0;
	#4;
	end
endtask

task sendpacket;
	reg [16:0]i;
	for ( i= 32'b0; i < 32'h4C0; i = i + 4)
	begin
		sendbytes(i*1000);
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
		sendbytes(16'h0000);
		sendbytes(16'h0000);
		sendbytes(16'h0000);
		sendbytes(16'h0000);
		sendbytes(16'h0000);
		sendbytes(16'h0000);
		input_valid = 1'h1;
		sendbytes(16'h00B8);
		sendbytes(16'h01B8);
		sendbytes(16'h01B8);
		sendbytes(16'h0201);
		sendbytes(16'h0201);
		sendbytes(16'h0201);
		sendbytes(16'h0201);
		sendbytes(16'h01B8);
		sendbytes(16'h0003);
		input_valid = 1'h0;
		sendbytes(16'h00000000);
		sendbytes(16'h00000000);
		sendbytes(16'h00000000);
		sendbytes(16'h0000);
		sendbytes(16'h0000);
		input_valid = 1'h1;
		sendbytes(16'h2BB8);
		sendbytes(16'h0201);
		sendbytes(16'h0003);
		input_valid = 1'h0;
		sendbytes(16'h00000000);
		sendbytes(16'h00000000);
		sendbytes(16'h0000);
		sendbytes(16'h0000);
		input_valid = 1'h1;
		sendbytes(16'h2CB8);
		sendbytes(16'h0201);
		sendbytes(16'h0003);
		input_valid = 1'h0;
		sendbytes(16'h00000000);
		sendbytes(16'h00000000);
		sendbytes(16'h0000);
		sendbytes(16'h0000);
		input_valid = 1'h1;
		sendbytes(16'h2DB8);
		sendbytes(16'h0201);
		sendbytes(16'h0003);
		input_valid = 1'h0;
		sendbytes(16'h00000000);
		sendbytes(16'h00000000);
		sendbytes(16'h00000000);
		sendbytes(16'h00000000);
		sendbytes(16'h0000);
		sendbytes(16'h0000);
		input_valid = 1'h1;
		sendbytes(16'h01B8);
		sendbytes(16'h0205);
		sendbytes(16'h0003);
		input_valid = 1'h0;
		sendbytes(16'h00000000);
		sendbytes(16'h00000000);
		sendbytes(16'h00000000);
		sendbytes(16'h00000000);
		sendbytes(16'h00000000);
		sendbytes(16'h00000000);
		sendbytes(16'h00000000);
		sendbytes(16'h00000000);
		
		reset = 1'b1;
		
end

endmodule
