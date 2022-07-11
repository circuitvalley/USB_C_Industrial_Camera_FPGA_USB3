`timescale 1ns/1ns

module tb_debayer_filter();
	reg clk;
	reg reset;
	reg line_valid;
	wire [127:0]unpacked_data;
	wire unpacked_valid;
	wire [383:0]rgb_output;
	wire output_valid;
	wire [127:0] yuv_data;
	wire is_yuv_valid;
	
	wire [31:0]data_out;
	wire lsync_out;
	reg out_clock;
	reg frame_sync;
	
	reg bytes_valid;
	reg  [63:0]bytes_i;
	wire synced;


wire reset_g;
//GSR GSR_INST (.GSR (reset_g));
//PUR PUR_INST (.PUR (reset_g)); 

//GSR 
//GSR_INST (
//	.GSR_N(1'b1),
//	.CLK(1'b0)
//);

debayer_filter ins1(	.clk_i(clk),
						.reset_i(frame_sync),
						.line_valid_i(line_valid),
						.data_i(unpacked_data),
						.data_valid_i(unpacked_valid),
						.output_valid_o(output_valid),
						.output_o(rgb_output));

rgb_to_yuv rgb_to_yuv1(.clk_i(clk),
					   .rgb_i(rgb_output),
					   .rgb_valid_i(output_valid),
					   .yuv_o(yuv_data),
					   .yuv_valid_o(is_yuv_valid));

output_reformatter out_reformat1(
								 .clk_i(clk),
								 .line_sync_i(line_valid),
								 .output_clk_i(out_clock),
								 .data_i(yuv_data),
								 .data_in_valid_i(is_yuv_valid),
								 .output_o(data_out),
								 .output_valid_o(lsync_out),
								 .frame_sync_i(frame_sync));
								 



mipi_csi_rx_raw_depacker_8b2lane_2ppc ins2(	.clk_i(clk),
						.packet_type_i(3'h3), //3 --> 10bit
						.data_valid_i(bytes_valid),
						.data_i(bytes_i),
						.output_valid_o(unpacked_valid),
						.output_o(unpacked_data));

task sendbytes;
	input [63:0]bytes;
	begin
	bytes_i = bytes;
	clk = 1'b0;
	out_clock = 1'b0;
	#10
	out_clock = 1'b1;
	#10
	out_clock = 1'b0;
	#10
	out_clock = 1'b1;
	#10
	out_clock = 1'b0;
	clk = 1'b1;
	#10
	out_clock = 1'b1;
	#10;
	end
endtask

integer i; 
initial begin
		clk = 1'b0;
		out_clock = 1'b0;
		line_valid = 1'h0;
		bytes_valid = 4'h0;
		frame_sync = 1; //active low in system
		reset = 1'h1;
		sendbytes( 64'h0);
		sendbytes( 64'h0);
		sendbytes( 64'h0);
		sendbytes( 64'h0);
		sendbytes( 64'h0);
		sendbytes( 64'h0);
		reset = 1'h0;
		sendbytes( 64'h0);
		sendbytes( 64'h0);
		sendbytes( 64'h0);
		sendbytes( 64'h0);
		sendbytes( 64'h0);
		sendbytes( 64'h0);
		sendbytes( 64'h0);
		sendbytes( 64'h0);
		sendbytes( 64'h0);
		sendbytes( 64'h0);
		#50
		frame_sync = 0;
		#10;
		sendbytes( 64'h0);
		sendbytes( 64'h0);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
	
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
				
		sendbytes( 64'h0);
		sendbytes( 64'h0);
		bytes_valid = 1'h1;
		line_valid = 1'h1;

		for (i = 0; i <64; i = i + 1)
		begin
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		end

		for (i = 0; i <64; i = i + 1)
		begin
		sendbytes( 64'hFF0000FFFF0033FF);
		sendbytes( 64'hFF0033FF0033FF00);
		sendbytes( 64'h0033FF0000FFFF00);
		sendbytes( 64'h00FFFF0033FF0033);
		sendbytes( 64'h33FF0033FF0000FF);
		end
		
		bytes_valid = 1'h0;
		line_valid = 1'h0;
		
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		bytes_valid = 1'h1;
		line_valid = 1'h1;
		
		
		
		for (i = 0; i <64; i = i + 1)
		begin
		sendbytes( 64'h00FFFF0000FFCC00); //red pixel on even line
		sendbytes( 64'h00FFCC00FFCC00FF);
		sendbytes( 64'hFFCC00FFFF0000FF);
		sendbytes( 64'hFF0000FFCC00FFCC);
		sendbytes( 64'hCC00FFCC00FFFF00);
		end
		for (i = 0; i <64; i = i + 1)
		begin
		sendbytes( 64'h00000000);			//rest of the pixels black
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		end
		
		
		bytes_valid = 1'h0;
		line_valid = 1'h0;
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
				
		sendbytes( 64'h0);
		sendbytes( 64'h0);
		bytes_valid = 1'h1;
		line_valid = 1'h1;

		for (i = 0; i <64; i = i + 1)
		begin
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		end

		for (i = 0; i <64; i = i + 1)
		begin
		sendbytes( 64'hFF0000FFFF0033FF);
		sendbytes( 64'hFF0033FF0033FF00);
		sendbytes( 64'h0033FF0000FFFF00);
		sendbytes( 64'h00FFFF0033FF0033);
		sendbytes( 64'h33FF0033FF0000FF);
		end
		
		bytes_valid = 1'h0;
		line_valid = 1'h0;
		
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h0);
		sendbytes( 64'h0);
		bytes_valid = 1'h1;
		line_valid = 1'h1;
		for (i = 0; i <64; i = i + 1)
		begin
		sendbytes( 64'h00FFFF0000FFCC00); //red pixel on even line
		sendbytes( 64'h00FFCC00FFCC00FF);
		sendbytes( 64'hFFCC00FFFF0000FF);
		sendbytes( 64'hFF0000FFCC00FFCC);
		sendbytes( 64'hCC00FFCC00FFFF00);
		end
		for (i = 0; i <64; i = i + 1)
		begin
		sendbytes( 64'h00000000);			//rest of the pixels black
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		end
		
		bytes_valid = 1'h0;
		line_valid = 1'h0;
		
		
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h0);
		sendbytes( 64'h0);
		bytes_valid = 1'h1;
		line_valid = 1'h1;
		for (i = 0; i <64; i = i + 1)
		begin
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		end

		for (i = 0; i <64; i = i + 1)
		begin
		sendbytes( 64'hFF0000FFFF0033FF);
		sendbytes( 64'hFF0033FF0033FF00);
		sendbytes( 64'h0033FF0000FFFF00);
		sendbytes( 64'h00FFFF0033FF0033);
		sendbytes( 64'h33FF0033FF0000FF);
		end
		bytes_valid = 1'h0;
		line_valid = 1'h0;
		
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h0);
		sendbytes( 64'h0);
		bytes_valid = 1'h1;
		line_valid = 1'h1;
		for (i = 0; i <64; i = i + 1)
		begin
		sendbytes( 64'h00FFFF0000FFCC00); //red pixel on even line
		sendbytes( 64'h00FFCC00FFCC00FF);
		sendbytes( 64'hFFCC00FFFF0000FF);
		sendbytes( 64'hFF0000FFCC00FFCC);
		sendbytes( 64'hCC00FFCC00FFFF00);
		end
		for (i = 0; i <64; i = i + 1)
		begin
		sendbytes( 64'h00000000);			//rest of the pixels black
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		end
		
		bytes_valid = 1'h0;
		line_valid = 1'h0;
		
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h0);
		sendbytes( 64'h0);
		bytes_valid = 1'h1;
		line_valid = 1'h1;
		for (i = 0; i <64; i = i + 1)
		begin
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		end

		for (i = 0; i <64; i = i + 1)
		begin
		sendbytes( 64'hFF0000FFFF0033FF);
		sendbytes( 64'hFF0033FF0033FF00);
		sendbytes( 64'h0033FF0000FFFF00);
		sendbytes( 64'h00FFFF0033FF0033);
		sendbytes( 64'h33FF0033FF0000FF);
		end
		bytes_valid = 1'h0;
		line_valid = 1'h0;
		
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h0);
		sendbytes( 64'h0);
		bytes_valid = 1'h1;
		line_valid = 1'h1;
		
		
	
		for (i = 0; i <64; i = i + 1)
		begin
		sendbytes( 64'h00FFFF0000FFCC00); //red pixel on even line
		sendbytes( 64'h00FFCC00FFCC00FF);
		sendbytes( 64'hFFCC00FFFF0000FF);
		sendbytes( 64'hFF0000FFCC00FFCC);
		sendbytes( 64'hCC00FFCC00FFFF00);
		end
		for (i = 0; i <64; i = i + 1)
		begin
		sendbytes( 64'h00000000);			//rest of the pixels black
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		end
		
		
		bytes_valid = 1'h0;
		line_valid = 1'h0;
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		#10;
		frame_sync = 1;	   
		
		
		
				sendbytes( 64'h0);
		sendbytes( 64'h0);
		sendbytes( 64'h0);
		sendbytes( 64'h0);
		sendbytes( 64'h0);
		sendbytes( 64'h0);
		sendbytes( 64'h0);
		sendbytes( 64'h0);
		#50
		frame_sync = 0;
		#10;
		sendbytes( 64'h0);
		sendbytes( 64'h0);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
	
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
				
		sendbytes( 64'h0);
		sendbytes( 64'h0);
		bytes_valid = 1'h1;
		line_valid = 1'h1;

		for (i = 0; i <64; i = i + 1)
		begin
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		end

		for (i = 0; i <64; i = i + 1)
		begin
		sendbytes( 64'hFF0000FFFF0033FF);
		sendbytes( 64'hFF0033FF0033FF00);
		sendbytes( 64'h0033FF0000FFFF00);
		sendbytes( 64'h00FFFF0033FF0033);
		sendbytes( 64'h33FF0033FF0000FF);
		end
		
		bytes_valid = 1'h0;
		line_valid = 1'h0;
		
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		bytes_valid = 1'h1;
		line_valid = 1'h1;
		
		
		
		for (i = 0; i <64; i = i + 1)
		begin
		sendbytes( 64'h00FFFF0000FFCC00); //red pixel on even line
		sendbytes( 64'h00FFCC00FFCC00FF);
		sendbytes( 64'hFFCC00FFFF0000FF);
		sendbytes( 64'hFF0000FFCC00FFCC);
		sendbytes( 64'hCC00FFCC00FFFF00);
		end
		for (i = 0; i <64; i = i + 1)
		begin
		sendbytes( 64'h00000000);			//rest of the pixels black
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		end
		
		
		bytes_valid = 1'h0;
		line_valid = 1'h0;
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
				
		sendbytes( 64'h0);
		sendbytes( 64'h0);
		bytes_valid = 1'h1;
		line_valid = 1'h1;

		for (i = 0; i <64; i = i + 1)
		begin
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		end

		for (i = 0; i <64; i = i + 1)
		begin
		sendbytes( 64'hFF0000FFFF0033FF);
		sendbytes( 64'hFF0033FF0033FF00);
		sendbytes( 64'h0033FF0000FFFF00);
		sendbytes( 64'h00FFFF0033FF0033);
		sendbytes( 64'h33FF0033FF0000FF);
		end
		
		bytes_valid = 1'h0;
		line_valid = 1'h0;
		
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h0);
		sendbytes( 64'h0);
		bytes_valid = 1'h1;
		line_valid = 1'h1;
		for (i = 0; i <64; i = i + 1)
		begin
		sendbytes( 64'h00FFFF0000FFCC00); //red pixel on even line
		sendbytes( 64'h00FFCC00FFCC00FF);
		sendbytes( 64'hFFCC00FFFF0000FF);
		sendbytes( 64'hFF0000FFCC00FFCC);
		sendbytes( 64'hCC00FFCC00FFFF00);
		end
		for (i = 0; i <64; i = i + 1)
		begin
		sendbytes( 64'h00000000);			//rest of the pixels black
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		end
		
		bytes_valid = 1'h0;
		line_valid = 1'h0;
		
		
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h0);
		sendbytes( 64'h0);
		bytes_valid = 1'h1;
		line_valid = 1'h1;
		for (i = 0; i <64; i = i + 1)
		begin
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		end

		for (i = 0; i <64; i = i + 1)
		begin
		sendbytes( 64'hFF0000FFFF0033FF);
		sendbytes( 64'hFF0033FF0033FF00);
		sendbytes( 64'h0033FF0000FFFF00);
		sendbytes( 64'h00FFFF0033FF0033);
		sendbytes( 64'h33FF0033FF0000FF);
		end
		bytes_valid = 1'h0;
		line_valid = 1'h0;
		
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h0);
		sendbytes( 64'h0);
		bytes_valid = 1'h1;
		line_valid = 1'h1;
		for (i = 0; i <64; i = i + 1)
		begin
		sendbytes( 64'h00FFFF0000FFCC00); //red pixel on even line
		sendbytes( 64'h00FFCC00FFCC00FF);
		sendbytes( 64'hFFCC00FFFF0000FF);
		sendbytes( 64'hFF0000FFCC00FFCC);
		sendbytes( 64'hCC00FFCC00FFFF00);
		end
		for (i = 0; i <64; i = i + 1)
		begin
		sendbytes( 64'h00000000);			//rest of the pixels black
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		end
		
		bytes_valid = 1'h0;
		line_valid = 1'h0;
		
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h0);
		sendbytes( 64'h0);
		bytes_valid = 1'h1;
		line_valid = 1'h1;
		for (i = 0; i <64; i = i + 1)
		begin
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		end

		for (i = 0; i <64; i = i + 1)
		begin
		sendbytes( 64'hFF0000FFFF0033FF);
		sendbytes( 64'hFF0033FF0033FF00);
		sendbytes( 64'h0033FF0000FFFF00);
		sendbytes( 64'h00FFFF0033FF0033);
		sendbytes( 64'h33FF0033FF0000FF);
		end
		bytes_valid = 1'h0;
		line_valid = 1'h0;
		
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h0);
		sendbytes( 64'h0);
		bytes_valid = 1'h1;
		line_valid = 1'h1;
		
		
	
		for (i = 0; i <64; i = i + 1)
		begin
		sendbytes( 64'h00FFFF0000FFCC00); //red pixel on even line
		sendbytes( 64'h00FFCC00FFCC00FF);
		sendbytes( 64'hFFCC00FFFF0000FF);
		sendbytes( 64'hFF0000FFCC00FFCC);
		sendbytes( 64'hCC00FFCC00FFFF00);
		end
		for (i = 0; i <64; i = i + 1)
		begin
		sendbytes( 64'h00000000);			//rest of the pixels black
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		end
		
		
		bytes_valid = 1'h0;
		line_valid = 1'h0;
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		#10;
		frame_sync = 1;	   
		
		
		
				sendbytes( 64'h0);
		sendbytes( 64'h0);
		sendbytes( 64'h0);
		sendbytes( 64'h0);
		sendbytes( 64'h0);
		sendbytes( 64'h0);
		sendbytes( 64'h0);
		sendbytes( 64'h0);
		#50
		frame_sync = 0;
		#10;
		sendbytes( 64'h0);
		sendbytes( 64'h0);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
	
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
				
		sendbytes( 64'h0);
		sendbytes( 64'h0);
		bytes_valid = 1'h1;
		line_valid = 1'h1;

		for (i = 0; i <64; i = i + 1)
		begin
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		end

		for (i = 0; i <64; i = i + 1)
		begin
		sendbytes( 64'hFF0000FFFF0033FF);
		sendbytes( 64'hFF0033FF0033FF00);
		sendbytes( 64'h0033FF0000FFFF00);
		sendbytes( 64'h00FFFF0033FF0033);
		sendbytes( 64'h33FF0033FF0000FF);
		end
		
		bytes_valid = 1'h0;
		line_valid = 1'h0;
		
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		bytes_valid = 1'h1;
		line_valid = 1'h1;
		
		
		
		for (i = 0; i <64; i = i + 1)
		begin
		sendbytes( 64'h00FFFF0000FFCC00); //red pixel on even line
		sendbytes( 64'h00FFCC00FFCC00FF);
		sendbytes( 64'hFFCC00FFFF0000FF);
		sendbytes( 64'hFF0000FFCC00FFCC);
		sendbytes( 64'hCC00FFCC00FFFF00);
		end
		for (i = 0; i <64; i = i + 1)
		begin
		sendbytes( 64'h00000000);			//rest of the pixels black
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		end
		
		
		bytes_valid = 1'h0;
		line_valid = 1'h0;
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
				
		sendbytes( 64'h0);
		sendbytes( 64'h0);
		bytes_valid = 1'h1;
		line_valid = 1'h1;

		for (i = 0; i <64; i = i + 1)
		begin
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		end

		for (i = 0; i <64; i = i + 1)
		begin
		sendbytes( 64'hFF0000FFFF0033FF);
		sendbytes( 64'hFF0033FF0033FF00);
		sendbytes( 64'h0033FF0000FFFF00);
		sendbytes( 64'h00FFFF0033FF0033);
		sendbytes( 64'h33FF0033FF0000FF);
		end
		
		bytes_valid = 1'h0;
		line_valid = 1'h0;
		
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h0);
		sendbytes( 64'h0);
		bytes_valid = 1'h1;
		line_valid = 1'h1;
		for (i = 0; i <64; i = i + 1)
		begin
		sendbytes( 64'h00FFFF0000FFCC00); //red pixel on even line
		sendbytes( 64'h00FFCC00FFCC00FF);
		sendbytes( 64'hFFCC00FFFF0000FF);
		sendbytes( 64'hFF0000FFCC00FFCC);
		sendbytes( 64'hCC00FFCC00FFFF00);
		end
		for (i = 0; i <64; i = i + 1)
		begin
		sendbytes( 64'h00000000);			//rest of the pixels black
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		end
		
		bytes_valid = 1'h0;
		line_valid = 1'h0;
		
		
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h0);
		sendbytes( 64'h0);
		bytes_valid = 1'h1;
		line_valid = 1'h1;
		for (i = 0; i <64; i = i + 1)
		begin
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		end

		for (i = 0; i <64; i = i + 1)
		begin
		sendbytes( 64'hFF0000FFFF0033FF);
		sendbytes( 64'hFF0033FF0033FF00);
		sendbytes( 64'h0033FF0000FFFF00);
		sendbytes( 64'h00FFFF0033FF0033);
		sendbytes( 64'h33FF0033FF0000FF);
		end
		bytes_valid = 1'h0;
		line_valid = 1'h0;
		
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h0);
		sendbytes( 64'h0);
		bytes_valid = 1'h1;
		line_valid = 1'h1;
		for (i = 0; i <64; i = i + 1)
		begin
		sendbytes( 64'h00FFFF0000FFCC00); //red pixel on even line
		sendbytes( 64'h00FFCC00FFCC00FF);
		sendbytes( 64'hFFCC00FFFF0000FF);
		sendbytes( 64'hFF0000FFCC00FFCC);
		sendbytes( 64'hCC00FFCC00FFFF00);
		end
		for (i = 0; i <64; i = i + 1)
		begin
		sendbytes( 64'h00000000);			//rest of the pixels black
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		end
		
		bytes_valid = 1'h0;
		line_valid = 1'h0;
		
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h0);
		sendbytes( 64'h0);
		bytes_valid = 1'h1;
		line_valid = 1'h1;
		for (i = 0; i <64; i = i + 1)
		begin
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		end

		for (i = 0; i <64; i = i + 1)
		begin
		sendbytes( 64'hFF0000FFFF0033FF);
		sendbytes( 64'hFF0033FF0033FF00);
		sendbytes( 64'h0033FF0000FFFF00);
		sendbytes( 64'h00FFFF0033FF0033);
		sendbytes( 64'h33FF0033FF0000FF);
		end
		bytes_valid = 1'h0;
		line_valid = 1'h0;
		
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h0);
		sendbytes( 64'h0);
		bytes_valid = 1'h1;
		line_valid = 1'h1;
		
		
	
		for (i = 0; i <64; i = i + 1)
		begin
		sendbytes( 64'h00FFFF0000FFCC00); //red pixel on even line
		sendbytes( 64'h00FFCC00FFCC00FF);
		sendbytes( 64'hFFCC00FFFF0000FF);
		sendbytes( 64'hFF0000FFCC00FFCC);
		sendbytes( 64'hCC00FFCC00FFFF00);
		end
		for (i = 0; i <64; i = i + 1)
		begin
		sendbytes( 64'h00000000);			//rest of the pixels black
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		end
		
		
		bytes_valid = 1'h0;
		line_valid = 1'h0;
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		#10;
		frame_sync = 1;	   
		
				sendbytes( 64'h0);
		sendbytes( 64'h0);
		sendbytes( 64'h0);
		sendbytes( 64'h0);
		sendbytes( 64'h0);
		sendbytes( 64'h0);
		sendbytes( 64'h0);
		sendbytes( 64'h0);
		#50
		frame_sync = 0;
		#10;
		sendbytes( 64'h0);
		sendbytes( 64'h0);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
	
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
				
		sendbytes( 64'h0);
		sendbytes( 64'h0);
		bytes_valid = 1'h1;
		line_valid = 1'h1;

		for (i = 0; i <64; i = i + 1)
		begin
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		end

		for (i = 0; i <64; i = i + 1)
		begin
		sendbytes( 64'hFF0000FFFF0033FF);
		sendbytes( 64'hFF0033FF0033FF00);
		sendbytes( 64'h0033FF0000FFFF00);
		sendbytes( 64'h00FFFF0033FF0033);
		sendbytes( 64'h33FF0033FF0000FF);
		end
		
		bytes_valid = 1'h0;
		line_valid = 1'h0;
		
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		bytes_valid = 1'h1;
		line_valid = 1'h1;
		
		
		
		for (i = 0; i <64; i = i + 1)
		begin
		sendbytes( 64'h00FFFF0000FFCC00); //red pixel on even line
		sendbytes( 64'h00FFCC00FFCC00FF);
		sendbytes( 64'hFFCC00FFFF0000FF);
		sendbytes( 64'hFF0000FFCC00FFCC);
		sendbytes( 64'hCC00FFCC00FFFF00);
		end
		for (i = 0; i <64; i = i + 1)
		begin
		sendbytes( 64'h00000000);			//rest of the pixels black
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		end
		
		
		bytes_valid = 1'h0;
		line_valid = 1'h0;
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
				
		sendbytes( 64'h0);
		sendbytes( 64'h0);
		bytes_valid = 1'h1;
		line_valid = 1'h1;

		for (i = 0; i <64; i = i + 1)
		begin
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		end

		for (i = 0; i <64; i = i + 1)
		begin
		sendbytes( 64'hFF0000FFFF0033FF);
		sendbytes( 64'hFF0033FF0033FF00);
		sendbytes( 64'h0033FF0000FFFF00);
		sendbytes( 64'h00FFFF0033FF0033);
		sendbytes( 64'h33FF0033FF0000FF);
		end
		
		bytes_valid = 1'h0;
		line_valid = 1'h0;
		
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h0);
		sendbytes( 64'h0);
		bytes_valid = 1'h1;
		line_valid = 1'h1;
		for (i = 0; i <64; i = i + 1)
		begin
		sendbytes( 64'h00FFFF0000FFCC00); //red pixel on even line
		sendbytes( 64'h00FFCC00FFCC00FF);
		sendbytes( 64'hFFCC00FFFF0000FF);
		sendbytes( 64'hFF0000FFCC00FFCC);
		sendbytes( 64'hCC00FFCC00FFFF00);
		end
		for (i = 0; i <64; i = i + 1)
		begin
		sendbytes( 64'h00000000);			//rest of the pixels black
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		end
		
		bytes_valid = 1'h0;
		line_valid = 1'h0;
		
		
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h0);
		sendbytes( 64'h0);
		bytes_valid = 1'h1;
		line_valid = 1'h1;
		for (i = 0; i <64; i = i + 1)
		begin
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		end

		for (i = 0; i <64; i = i + 1)
		begin
		sendbytes( 64'hFF0000FFFF0033FF);
		sendbytes( 64'hFF0033FF0033FF00);
		sendbytes( 64'h0033FF0000FFFF00);
		sendbytes( 64'h00FFFF0033FF0033);
		sendbytes( 64'h33FF0033FF0000FF);
		end
		bytes_valid = 1'h0;
		line_valid = 1'h0;
		
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h0);
		sendbytes( 64'h0);
		bytes_valid = 1'h1;
		line_valid = 1'h1;
		for (i = 0; i <64; i = i + 1)
		begin
		sendbytes( 64'h00FFFF0000FFCC00); //red pixel on even line
		sendbytes( 64'h00FFCC00FFCC00FF);
		sendbytes( 64'hFFCC00FFFF0000FF);
		sendbytes( 64'hFF0000FFCC00FFCC);
		sendbytes( 64'hCC00FFCC00FFFF00);
		end
		for (i = 0; i <64; i = i + 1)
		begin
		sendbytes( 64'h00000000);			//rest of the pixels black
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		end
		
		bytes_valid = 1'h0;
		line_valid = 1'h0;
		
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h0);
		sendbytes( 64'h0);
		bytes_valid = 1'h1;
		line_valid = 1'h1;
		for (i = 0; i <64; i = i + 1)
		begin
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		end

		for (i = 0; i <64; i = i + 1)
		begin
		sendbytes( 64'hFF0000FFFF0033FF);
		sendbytes( 64'hFF0033FF0033FF00);
		sendbytes( 64'h0033FF0000FFFF00);
		sendbytes( 64'h00FFFF0033FF0033);
		sendbytes( 64'h33FF0033FF0000FF);
		end
		bytes_valid = 1'h0;
		line_valid = 1'h0;
		
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h0);
		sendbytes( 64'h0);
		bytes_valid = 1'h1;
		line_valid = 1'h1;
		
		
	
		for (i = 0; i <64; i = i + 1)
		begin
		sendbytes( 64'h00FFFF0000FFCC00); //red pixel on even line
		sendbytes( 64'h00FFCC00FFCC00FF);
		sendbytes( 64'hFFCC00FFFF0000FF);
		sendbytes( 64'hFF0000FFCC00FFCC);
		sendbytes( 64'hCC00FFCC00FFFF00);
		end
		for (i = 0; i <64; i = i + 1)
		begin
		sendbytes( 64'h00000000);			//rest of the pixels black
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		end
		
		
		bytes_valid = 1'h0;
		line_valid = 1'h0;
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		sendbytes( 64'h00000000);
		#10;
		frame_sync = 1;	   
		
		
		
		
		
		
		
		
		
end
	
endmodule 