`timescale 1ns/1ns

module tb_mipi_rx_raw_depacker8b4_12bit();
	
		reg clk;
	reg bytes_valid;
	reg  [31:0]bytes_i;
	wire [55:0]bytes_o;
	wire synced;
	reg [2:0]packet_type;
wire raw_line_valid;

mipi_csi_rx_raw_depacker_8b4lane #(.PIXEL_WIDTH(14)) ins1(	.clk_i(clk),
															.data_valid_i(bytes_valid),
															.data_i(bytes_i),
															.packet_type_i(packet_type),
															.output_valid_o(synced),
															.raw_line_o(raw_line_valid),
															.output_o(bytes_o));
															
															

task sendbytes;
	input [31:0]bytes;
	begin
	bytes_i = bytes;
	clk = 1'b0;
	#4
	clk = 1'b1;
	#4;
	end
endtask

initial begin
		clk = 1'b0;
		bytes_valid = 4'h0;
		packet_type = 4; //12bit
		#50
		sendbytes(32'h0);
		sendbytes(32'h0);
		sendbytes(32'h0);
		
		bytes_valid = 1'h1;
		sendbytes( 32'h03000201);
		sendbytes( 32'h06050004);
		sendbytes( 32'h00080700);
		
		sendbytes( 32'h030F0201);
		sendbytes( 32'h0605F004);
		sendbytes( 32'h00080700);
		
		sendbytes( 32'h03000201);
		sendbytes( 32'h06050004);
		sendbytes( 32'hF008070F);
		
		sendbytes( 32'h03000201);
		sendbytes( 32'h06050004);
		sendbytes( 32'h00080700);
		
		sendbytes( 32'h03000201);
		sendbytes( 32'h06050004);
		sendbytes( 32'h00080700);
		sendbytes( 32'h03000201);
		sendbytes( 32'h06050004);
		sendbytes( 32'h00080700);
		sendbytes( 32'h03000201);
		sendbytes( 32'h06050004);
		sendbytes( 32'h00080700);
		sendbytes( 32'h03000201);
		sendbytes( 32'h06050004);
		sendbytes( 32'h00080700);
		
		bytes_valid = 1'h0;
		
		sendbytes(32'h0000);
		sendbytes(32'h0000);
		sendbytes(32'h0000);
		sendbytes(32'h0000);
		sendbytes(32'h0000);
		sendbytes(32'h0000);
		sendbytes(32'h0000);
		sendbytes(32'h0000);
		
		
		sendbytes(32'h0000);
		sendbytes(32'h0000);
		sendbytes(32'h0000);
		
		bytes_valid = 1'h1;

		sendbytes( 32'h03000201);
		sendbytes( 32'h00000004);
		sendbytes( 32'h00000000);
		
		sendbytes( 32'h03FFF2F1);
		sendbytes( 32'h0000FFF4);
		sendbytes( 32'h00000000);
		
		sendbytes( 32'h03000201);
		sendbytes( 32'h00000004);
		sendbytes( 32'h00000000);
		
		sendbytes( 32'h03000201);
		sendbytes( 32'h00000004);
		sendbytes( 32'h00000000);
		
		
		sendbytes( 32'h03000201);
		sendbytes( 32'h00000004);
		sendbytes( 32'h00000000);
		
		sendbytes( 32'h03000201);
		sendbytes( 32'h00000004);
		sendbytes( 32'h00000000);
		
		sendbytes( 32'h03000201);
		sendbytes( 32'h00000004);
		sendbytes( 32'h00000000);
		
		sendbytes( 32'h03000201);
		sendbytes( 32'h00000004);
		sendbytes( 32'h00000000);
		bytes_valid = 1'h0;

		sendbytes(32'h0000);
		sendbytes(32'h0000);
		sendbytes(32'h0000);
		sendbytes(32'h0000);
		sendbytes(32'h0000);
		sendbytes(32'h0000);
		sendbytes(32'h0000);
		sendbytes(32'h0000);
		sendbytes(32'h0000);
		sendbytes(32'h0000);
		sendbytes(32'h0000);
		
		
		sendbytes(32'h0000);
		sendbytes(32'h0000);
		sendbytes(32'h0000);
		sendbytes(32'h0000);
		
	
		$finish;
end
endmodule