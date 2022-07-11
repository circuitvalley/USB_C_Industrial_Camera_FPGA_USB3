`timescale 1ns/1ns

module tb_mipi_csi_rx_packet_decoder;
	reg clk;
	reg reset;
	reg input_valid;
	reg  [15:0]bytes_i;
	wire [15:0]bytes_o;
	wire synced;
	wire [15:0]packet_length;
	wire [2:0]packet_type;
	
wire reset_g;

GSR 
GSR_INST (
	.GSR_N(1'b1),
	.CLK(1'b0)
);

mipi_csi_rx_packet_decoder_8b2lane dec1(.clk_i(clk),
							 .data_valid_i(input_valid),
							 .data_i(bytes_i),
							 .output_valid_o(synced),
							 .data_o(bytes_o),
							 .packet_length_o(packet_length),
							 .packet_type_o(packet_type));

												
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
		clk = 1'b0;
		input_valid = 1'h0;
		#50
		reset = 1'b0;
		sendbytes(16'h0000);
		sendbytes(16'h0000);
		sendbytes(16'h0000);
		sendbytes(16'h0000);
		sendbytes(16'h0000);
		sendbytes(16'h0000);
		input_valid = 1'h1;
		sendbytes(16'hB8B8);
		sendbytes(16'h062B);
		sendbytes(16'hDD00);
		sendbytes(16'hFFEE);
		sendbytes(16'h3322);
		sendpacket();
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
