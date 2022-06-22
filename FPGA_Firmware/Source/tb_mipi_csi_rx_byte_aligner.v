`timescale 1ns/1ns

module tb_rx_byte_aligner;
	
	reg clk;
	reg reset;
	reg  [15:0]byte_i;
	wire [63:0]byte_o;

wire reset_g;
GSR 
GSR_INST (
	.GSR_N(1'b1),
	.CLK(1'b0)
);

wire lane_aligned;
wire [3:0]byte_synced;
wire [63:0]lane_bytes;
wire [127:0]unpacked_bytes;
wire [63:0]decoded_data;
wire decoded_valid;
wire unpacked_valid;
wire [2:0]packet_type;
mipi_csi_rx_byte_aligner0 byte_inst1(	.clk_i(clk),
									.reset_i(reset),
									.byte_i(byte_i),
									.byte_o(byte_o[15:0]),
									.byte_valid_o(byte_synced[0]));

mipi_csi_rx_byte_aligner1 byte_inst2(	.clk_i(clk),
									.reset_i(reset),
									.byte_i(byte_i),
									.byte_o(byte_o[31:16]),
									.byte_valid_o(byte_synced[1]));
						
mipi_csi_rx_byte_aligner2 byte_inst3(	.clk_i(clk),
									.reset_i(reset),
									.byte_i(byte_i),
									.byte_o(byte_o[47:32]),
									.byte_valid_o(byte_synced[2]));

mipi_csi_rx_byte_aligner3 byte_inst4(	.clk_i(clk),
									.reset_i(reset),
									.byte_i(byte_i),
									.byte_o(byte_o[63:48]),
									.byte_valid_o(byte_synced[3]));


mipi_csi_rx_lane_aligner lane_ins1(	.clk_i(clk),
						.reset_i(reset),
						.bytes_valid_i(byte_synced),
						.byte_i(byte_o),
						.lane_valid_o(lane_aligned),
						.lane_byte_o(lane_bytes));
						
mipi_csi_rx_packet_decoder dec1(.clk_i(clk),
							 .data_valid_i(lane_aligned),
							 .data_i(lane_bytes),
							 .output_valid_o(decoded_valid),
							 .data_o(decoded_data),
							 .packet_length_o(),
							 .packet_type_o(packet_type));
							 
mipi_csi_rx_raw_depacker depacker_ins1(	.clk_i(clk),
						.data_valid_i(decoded_valid),
						.data_i(decoded_data),
						.packet_type_i(packet_type),
						.output_valid_o(unpacked_valid),
						.output_o(unpacked_bytes));


wire rgb_output_valid;
wire [383:0]rgb_output;
debayer_filter debayer_ins1(	.clk_i(clk),
						.reset_i(reset),
						.line_valid_i(decoded_valid),
						.data_i(unpacked_bytes),
						.data_valid_i(unpacked_valid),
						.output_valid_o(rgb_output_valid),
						.output_o(rgb_output));
						
task sendbyte;
	input [15:0]byte;
	begin
	byte_i = byte;
	clk = 1'b1;
	#4
	clk = 1'b0;
	#4;
	end
endtask

initial begin
		clk = 1'b1;
		reset = 1'b1;
		clk = 1'b1;
		#4
		clk = 1'b0;
		#4;
		#50
		reset = 1'b0;
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'h2577);  //B8 2B 11 72 16 11 83 11
		sendbyte(16'hCE42);
		sendbyte(16'h2222);
		sendbyte(16'h6222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
				sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
			sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
				sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
			sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
				sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		reset = 1'h1;
		#5
		clk = 1'b1;
		#4
		clk = 1'b0;
		#4;
		#50
		
		reset = 1'b0;
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'h2BB8);	//B8 60 10
		sendbyte(16'h442E);
		sendbyte(16'h88A9);
		sendbyte(16'h0888);
		sendbyte(16'h0817);
		sendbyte(16'h0808);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
				sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
				sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
			sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
				sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
			sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
				sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
			sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
				sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		reset = 1'h1;
		#5
		clk = 1'b1;
		#4
		clk = 1'b0;
		#4;
		#50
	

		reset = 1'b0;
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'h5770);	//B8 60 10
		sendbyte(16'h4410);
		sendbyte(16'h88A9);
		sendbyte(16'h0888);
		sendbyte(16'h0817);
		sendbyte(16'h0808);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
			sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
				sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
			sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
				sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		reset = 1'h1;
		#5
		clk = 1'b1;
		#4
		clk = 1'b0;
		#4;
		#50		

		reset = 1'b0;
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'hAEE0);	//B8 60 10
		sendbyte(16'h4400);
		sendbyte(16'h88A9);
		sendbyte(16'h0888);
		sendbyte(16'h0817);
		sendbyte(16'h0808);
			sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
				sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
			sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
				sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
			sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
				sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
			sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
				sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		reset = 1'h1;
		#5
		clk = 1'b1;
		#4
		clk = 1'b0;
		#4;
		#50
		
		reset = 1'b0;
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'h5DC0);	//B8 60 10
		sendbyte(16'h4401);
		sendbyte(16'h88A9);
		sendbyte(16'h0888);
		sendbyte(16'h0817);
		sendbyte(16'h0808);
			sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
				sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
			sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
				sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		reset = 1'h1;
		#5
		clk = 1'b1;
		#4
		clk = 1'b0;
		#4;
		#50

		reset = 1'b0;
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'hBB80);	//B8 60 10
		sendbyte(16'h4402);
		sendbyte(16'h88A9);
		sendbyte(16'h0888);
		sendbyte(16'h0817);
		sendbyte(16'h0808);
			sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
				sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
			sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
				sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		reset = 1'h1;
		#5
		clk = 1'b1;
		#4
		clk = 1'b0;
		#4;
		#50
		
		
		reset = 1'b0;
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'h7700);	//B8 60 10
		sendbyte(16'h4405);
		sendbyte(16'h88A9);
		sendbyte(16'h0888);
		sendbyte(16'h0817);
		sendbyte(16'h0808);
			sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
				sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
			sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
				sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2222);
		sendbyte(16'h2230);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		sendbyte(16'h2202);
		reset = 1'h1;
		#5
		clk = 1'b1;
		#4
		clk = 1'b0;
		#4;
		#50
		
		reset = 1'b0;
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'hEE00);	//B8 60 10
		sendbyte(16'h440A);
		sendbyte(16'h88A9);
		sendbyte(16'h0888);
		sendbyte(16'h0817);
		sendbyte(16'h0808);
		reset = 1'h1;
		#5
		clk = 1'b1;
		#4
		clk = 1'b0;
		#4;
		#50
		
		reset = 1'b0;
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'hDC00);	//B8 60 10
		sendbyte(16'h4415);
		sendbyte(16'h88A9);
		sendbyte(16'h0888);
		sendbyte(16'h0817);
		sendbyte(16'h0808);
		reset = 1'h1;
		#5
		clk = 1'b1;
		#4
		clk = 1'b0;
		#4;
		#50




		reset = 1'b0;
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'hB800);	//B8 60 10
		sendbyte(16'h442B);
		sendbyte(16'h88A9);
		sendbyte(16'h0888);
		sendbyte(16'h0817);
		sendbyte(16'h0808);
		reset = 1'h1;
		#5
		clk = 1'b1;
		#4
		clk = 1'b0;
		#4;
		#50
		
		reset = 1'b0;
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'h7000);	//B8 60 10
		sendbyte(16'h4457);
		sendbyte(16'h88A9);
		sendbyte(16'h0888);
		sendbyte(16'h0817);
		sendbyte(16'h0808);
		reset = 1'h1;
		#5
		clk = 1'b1;
		#4
		clk = 1'b0;
		#4;
		#50


		reset = 1'b0;
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'hE000);	//B8 60 10
		sendbyte(16'h44AE);
		sendbyte(16'h88A9);
		sendbyte(16'h0888);
		sendbyte(16'h0817);
		sendbyte(16'h0808);
		reset = 1'h1;
		#5
		clk = 1'b1;
		#4
		clk = 1'b0;
		#4;
		#50
		
		
		
		
		reset = 1'b0;
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'hC000);	//B8 60 10
		sendbyte(16'h415D);
		sendbyte(16'h88A9);
		sendbyte(16'h0888);
		sendbyte(16'h0817);
		sendbyte(16'h0808);
		reset = 1'h1;
		#5
		clk = 1'b1;
		#4
		clk = 1'b0;
		#4;
		#50



		reset = 1'b0;
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'h8000);	//B8 60 10
		sendbyte(16'h42BB);
		sendbyte(16'h88A9);
		sendbyte(16'h0888);
		sendbyte(16'h0817);
		sendbyte(16'h0808);
		reset = 1'h1;
		#5
		clk = 1'b1;
		#4
		clk = 1'b0;
		#4;
		#50
		
		
		reset = 1'b0;
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'h0000);	//B8 60 10
		sendbyte(16'h4577);
		sendbyte(16'h88A9);
		sendbyte(16'h0888);
		sendbyte(16'h0817);
		sendbyte(16'h0808);
		reset = 1'h1;
		#5
		clk = 1'b1;
		#4
		clk = 1'b0;
		#4;
		#50



		reset = 1'b0;
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'h0000);	//B8 60 10
		sendbyte(16'h4AEE);
		sendbyte(16'h88A9);
		sendbyte(16'h0888);
		sendbyte(16'h0817);
		sendbyte(16'h0808);
		reset = 1'h1;
		#5
		clk = 1'b1;
		#4
		clk = 1'b0;
		#4;
		#50
		
		
		
		reset = 1'b0;
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'h0000);	//B8 60 10
		sendbyte(16'h15DC);
		sendbyte(16'h88A9);
		sendbyte(16'h0888);
		sendbyte(16'h0817);
		sendbyte(16'h0808);
		reset = 1'h1;
		#5
		clk = 1'b1;
		#4
		clk = 1'b0;
		#4;
		#50
	
		
		reset = 1'b0;
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'h0000);	//A packet with no sync
		sendbyte(16'h1500);
		sendbyte(16'h88A9);
		sendbyte(16'h0000);
		sendbyte(16'h0800);
		sendbyte(16'h0808);
		reset = 1'h1;
		#5
		clk = 1'b1;
		#4
		clk = 1'b0;
		#4;
		#50
		
		
		reset = 1'b0;
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'h00);
		sendbyte(16'h82E0); 	//B8 60 11
		sendbyte(16'h4045);
		sendbyte(16'h45C4);
		sendbyte(16'h0840);
		sendbyte(16'h0817);
		sendbyte(16'h0808);
		reset = 1'h1;	
		
end

endmodule