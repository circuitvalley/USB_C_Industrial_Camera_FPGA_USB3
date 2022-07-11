`timescale 1ns/1ns

module tb_mipi_rx_raw_depacker14bit2();
	
		reg clk;
	reg bytes_valid;
	reg  [15:0]bytes_i;
	wire [23:0]bytes_o;
	wire synced;
	reg [2:0]packet_type;
wire reset_g;
wire raw_line_valid;


mipi_csi_rx_raw_depacker_8b2lane_2ppc #(.PIXEL_WIDTH(12)) ins1(	.clk_i(clk),
						.data_valid_i(bytes_valid),
						.data_i(bytes_i),
						.packet_type_i(packet_type),
						.output_valid_o(synced),
						.raw_line_o(raw_line_valid),
						.output_o(bytes_o));

task sendbytes;
	input [16:0]bytes;
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
		packet_type = 4;
		#50
		sendbytes(16'h0);
		sendbytes(16'h0);
		sendbytes(16'h0);
		sendbytes(16'h0);
		bytes_valid = 1'h1;
		sendbytes( 16'h0201);
		sendbytes( 16'h0300);
		sendbytes( 16'h0004);
		sendbytes( 16'h0605);
		sendbytes( 16'h0700);
		sendbytes( 16'h0008);
		sendbytes( 16'h0A09);
		sendbytes( 16'h0B00);
		sendbytes( 16'h000C);
		sendbytes( 16'h0E0D);
		sendbytes( 16'h0F00);
		sendbytes( 16'h0001);
		sendbytes( 16'h0302);
		sendbytes( 16'h0400);
		sendbytes( 16'h0005);
		sendbytes( 16'h0706);
		sendbytes( 16'h0900);
		sendbytes( 16'h000A);
		sendbytes( 16'h0C0B);
		sendbytes( 16'h0D00);
		sendbytes( 16'h000E);
		sendbytes( 16'h010F);
		
		bytes_valid = 1'h0;
		sendbytes(16'h0000);
		sendbytes(16'h0000);
		sendbytes(16'h0000);
		sendbytes(16'h0000);
		sendbytes(16'h0000);
		sendbytes(16'h0000);
		sendbytes(16'h0000);
		sendbytes(16'h0000);
		
		
		
		$finish;
end
endmodule