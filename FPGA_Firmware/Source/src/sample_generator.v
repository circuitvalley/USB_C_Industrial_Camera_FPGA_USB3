module sample_generator(clk_i,
						reset_i,
						framesync_i,
						byte_o,
						byte_valid_o);
input reset_i;
input clk_i;
input framesync_i;
output [15:0]byte_o;
output reg byte_valid_o;
reg [11:0]sample_counter;
wire [15:0]output_first;
wire [15:0]output_second;

rom_sec romsec_ins(.rd_clk_i(clk_i), 
        .rst_i(reset_i), 
        .rd_en_i(1'b1), 
        .rd_clk_en_i(1'b1), 
        .rd_addr_i(sample_counter), 
        .rd_data_o(output_second)) ;

rom_first debug_rom_ins(.rd_clk_i(clk_i), 
        .rst_i(reset_i), 
        .rd_en_i(1'b1), 
        .rd_clk_en_i(1'b1), 
        .rd_addr_i(sample_counter), 
        .rd_data_o(output_first)) ;
		
reg [9:0]line_counter;

assign byte_o = line_counter[0]?output_second:output_first;

always @(posedge framesync_i or negedge reset_i)
begin
	if (framesync_i)
	begin
		line_counter <= 0;
	end 
	else
	begin
		line_counter <= line_counter + 1'h1;
	end
end

always @(posedge clk_i)
begin
	if (reset_i)
	begin
		sample_counter <= 16'h0;
	end
	else
	begin
		sample_counter <= sample_counter + 1'h1;
	end
end

always @(*)
begin
	if (reset_i)
	begin
		byte_valid_o = 1'b0;
	end
	else
	begin
		if ((line_counter > 10'h3) && (line_counter < 10'd994) && byte_o[7:0] == 8'hB8)
		begin
			byte_valid_o = 1'b1;
		end

	end
end

endmodule