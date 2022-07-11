
`timescale 1ns/1ns

module tb_debayer_filter_file();
	reg clk;
	reg [383:0]rgb_input;
	reg rgb_valid;
	reg line_valid;
	reg frame_sync;
	reg out_clock;
	wire [31:0]data_out;
PUR PUR_INST (.PUR (reset_g)); 

GSR 
GSR_INST (
	.GSR_N(1'b1),
	.CLK(1'b0)
);


					

wire osc_clk;

wire output_clock; 
wire mipi_byte_clock; //byte clock from mipi phy


reg is_raw_line_valid;
reg is_unpacked_valid;
wire is_rgb_valid;
wire is_yuv_valid;
wire is_yuv_line_valid;

wire mipi_out_clk;


reg [127:0]unpacked_data;
wire [383:0]rgb_data;
wire [127:0]yuv_data;


//wire frame_sync_in;




wire ready; 
wire [15:0]debug_16;
wire [7:0]debug_aligner;




debayer_filter debayer_filter_0(.clk_i(clk),
								.reset_i(frame_sync),
								.line_valid_i(is_raw_line_valid),
								.data_i(unpacked_data),
								.data_valid_i(is_unpacked_valid),
								.output_o(rgb_data),
								.output_valid_o(is_rgb_valid));

rgb_to_yuv rgb_to_yuv_0(.clk_i(clk),
					    .rgb_i(rgb_data),
					    .rgb_valid_i(is_rgb_valid),
						.line_valid_i(is_raw_line_valid),
					    .yuv_o(yuv_data),
					    .yuv_valid_o(is_yuv_valid),
						.yuv_line_o(yuv_line_valid));


output_reformatter out_reformatter_0(.clk_i(clk),
									 .line_sync_i(yuv_line_valid),
									 .frame_sync_i(frame_sync),
									 .output_clk_i(out_clock),
									 .data_i(yuv_data),
									 .data_in_valid_i(is_yuv_valid),
									 .output_o(data_out),
									 .output_valid_o(lsync_o),
									 .debug_16());


assign pclk_o = mipi_out_clk; 	//output clock always available
assign fsync_o = !frame_sync;	 //activate fsync Active high



task sendclock;
	begin
	unpacked_data = 0;
	clk = 1'b1;
	#10;
	clk = 1'b0;
	#10;
	clk = 1'b1;
	#10;
	clk = 1'b0;
	#10;
	end
endtask

task sendbayer;
	input [127:0]bayer;
	begin
	unpacked_data = bayer;
	clk = 1'b1;
	out_clock = 1'b0;
	#10
	out_clock = 1'b1;
	#10
	
	$fwrite(write_yuv_fd, "%u", {data_out[7:0], data_out[15:8], data_out[23:16], data_out[31:24]});
	//$display("%h", data_out);
	out_clock = 1'b0;
	#10
	out_clock = 1'b1;
	#10
	$fwrite(write_yuv_fd, "%u", {data_out[7:0], data_out[15:8], data_out[23:16], data_out[31:24]});
	//$display("%h", data_out);	
	out_clock = 1'b0;
	#10
	out_clock = 1'b1;
	#10
	$fwrite(write_yuv_fd, "%u", {data_out[7:0], data_out[15:8], data_out[23:16], data_out[31:24]});
	//$display("%h", data_out);	
	out_clock = 1'b0;
	clk = 1'b0;
	#10
	out_clock = 1'b1;
	#10;
	$fwrite(write_yuv_fd, "%u", {data_out[7:0], data_out[15:8], data_out[23:16], data_out[31:24]});
	//$display("%h", data_out);		
	end
endtask



integer i;
integer j;
integer verify_fd;
integer read_fd;
integer write_fd;		 
integer write_yuv_fd;
reg[63:0] read_bytes;
reg[127:0] send_read_bytes;
reg[383:0] rgb_data_reordered;
initial begin
	clk = 1;
	out_clock = 0;
	rgb_valid = 0;
	read_fd = $fopen("C:\\Users\\gaurav\\Documents\\FPGA\\Lattice\\MIPI_CSI_Parallel_16_nx\\csi_16_nx\\source\\csi_16_nx\\italy.bmp768x512.raw","rb");
	write_fd = $fopen("C:\\Users\\gaurav\\Documents\\FPGA\\Lattice\\MIPI_CSI_Parallel_16_nx\\csi_16_nx\\source\\csi_16_nx\\italy.bmp768x512.raw.rgb","wb");
	write_yuv_fd = $fopen("C:\\Users\\gaurav\\Documents\\FPGA\\Lattice\\MIPI_CSI_Parallel_16_nx\\csi_16_nx\\source\\csi_16_nx\\italy.bmp768x512.raw.yuv","wb");
	verify_fd = $fopen("C:\\Users\\gaurav\\Documents\\FPGA\\Lattice\\MIPI_CSI_Parallel_16_nx\\csi_16_nx\\source\\csi_16_nx\\italy.bmp768x512.raw.verify","wb");
	
	$display("read_fd=%d",read_fd);
 	$display("write_fd=%d",write_fd);
	
	rgb_input = 0;
	frame_sync = 1; //active low
	sendclock();
	
	frame_sync = 0; //active low
		
    for (i = 0; i < 512; i = i + 1)		
	begin		 
	rgb_valid = 1;
	line_valid = 1;
	is_raw_line_valid  = 1;
	is_unpacked_valid = 1;
		for (j=0; j < 96; j = j + 1)
		begin
		$fread(read_bytes, read_fd);
		
		send_read_bytes[127:112] = 	{read_bytes [0	+:8], 	8'h0}; 		//lane 1 first pixel on wire	
		send_read_bytes[111:96]  = 	{read_bytes [8 	+:8], 	8'h0};
		send_read_bytes[95:80]   = 	{read_bytes [16	+:8], 	8'h0};
		send_read_bytes[79:64]   = 	{read_bytes [24	+:8], 	8'h0};
		send_read_bytes[63:48] = 	{read_bytes [32 +:8], 	8'h0};
		send_read_bytes[47:32] = 	{read_bytes [40 +:8], 	8'h0};
		send_read_bytes[31:16] = 	{read_bytes [48 +:8], 	8'h0};
		send_read_bytes[15:0]  = 	{read_bytes [56 +:8], 	8'h0}; 		//lane 4 	

		$fwrite(verify_fd, "%u", { send_read_bytes[15:0], send_read_bytes[31:16], send_read_bytes[47:32], send_read_bytes[63:48], send_read_bytes[79:64], send_read_bytes[95:80], send_read_bytes[111:96],send_read_bytes[127:112]} );
		sendbayer(send_read_bytes);
		$fwrite(write_fd, "%u", rgb_data); //outputs little endian 

		
		//$display("%h", rgb_data_reordered);	
		end	  
		rgb_valid = 0;	  
		line_valid = 0;
		is_raw_line_valid  = 0;
		is_unpacked_valid = 0;
		sendclock();
		sendclock();
		sendclock();
		sendclock();
	end
	/*
	  rgb_valid = 1;
	while (!$feof(read_fd)) begin 
		
		$fread(read_bytes, read_fd);

		send_read_bytes ={read_bytes[383:336], read_bytes[335:288], read_bytes[287:240], read_bytes[239:192], read_bytes[191:144], read_bytes[143:96], read_bytes[95:48], read_bytes[47:0]};
		sendrgb(send_read_bytes);		
		
		$fwrite(write_fd, "%u", { {yuv_data[7:0], yuv_data[15:8]} , {yuv_data[23:16], yuv_data[31:24]}, 
		{yuv_data[39:32], yuv_data[47:40]}, { yuv_data[55:48] ,yuv_data[63:56]}, 
		{yuv_data[71:64], yuv_data[79:72]}, {yuv_data[87:80], yuv_data[95:88]}, 
		{yuv_data[103:96], yuv_data[111:104]}, {yuv_data[119:112], yuv_data[127:120]} } );
		 #10;
		//$fwrite(write_fd, "%u", {{yuv_data[119:112], yuv_data[127:120]}, {yuv_data[103:96], yuv_data[111:104]},{yuv_data[87:80], yuv_data[95:88]},{yuv_data[71:64], yuv_data[79:72]},{ yuv_data[55:48] ,yuv_data[63:56]}, 			{yuv_data[39:32], yuv_data[47:40]},			  {yuv_data[23:16], yuv_data[31:24]},{yuv_data[7:0], yuv_data[15:8]}} );
	end
	*/
	frame_sync = 1; //active low
	sendclock();
	sendclock();
	sendclock();
	sendclock();
	rgb_valid = 0;	  
	$fclose(read_fd);
	$fclose(write_fd);	 
	$fclose(write_yuv_fd);
	$fclose(verify_fd);
	
end


					   
endmodule