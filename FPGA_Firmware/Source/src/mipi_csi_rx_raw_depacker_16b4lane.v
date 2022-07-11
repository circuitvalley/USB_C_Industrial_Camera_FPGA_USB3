`timescale 1ns/1ns

/*
MIPI CSI RX to Parallel Bridge (c) by Gaurav Singh www.CircuitValley.com

MIPI CSI RX to Parallel Bridge is licensed under a
Creative Commons Attribution 3.0 Unported License.

You should have received a copy of the license along with this
work.  If not, see <http://creativecommons.org/licenses/by/3.0/>.
*/

/*
Receives 4 lane raw mipi bytes from packet decoder, rearrange bytes to output 8 pixel upto 16bit each 
output is one clock cycle delayed, because the way , MIPI RAW is packed 
output come in chunk on each clock cycle, output_valid_o remains active only while 20 pixel chunk is outputted 
*/

module mipi_csi_rx_raw_depacker_16b4lane(	clk_i,
											data_valid_i,
											data_i,
											packet_type_i,
											raw_line_o,
											output_valid_o,
											output_o);


localparam [7:0]MIPI_CSI_PACKET_10bRAW = 8'h2B;
localparam [7:0]MIPI_CSI_PACKET_12bRAW = 8'h2C;
localparam [7:0]MIPI_CSI_PACKET_14bRAW = 8'h2D;

input clk_i;
input data_valid_i;
input [63:0]data_i;
input [2:0]packet_type_i;

output reg output_valid_o;
output reg [127:0]output_o; 
output raw_line_o;

reg [7:0]index_table_pixel_0[3:0];
reg [7:0]index_table_pixel_1[3:0];
reg [7:0]index_table_pixel_2[3:0];
reg [7:0]index_table_pixel_3[3:0];
reg [7:0]index_table_pixel_lsb1[3:0];
reg [7:0]index_table_pixel_4[3:0];
reg [7:0]index_table_pixel_5[3:0];
reg [7:0]index_table_pixel_6[3:0];
reg [7:0]index_table_pixel_7[3:0];
reg [7:0]index_table_pixel_lsb2[3:0];

reg [7:0]offset_pixel_0;
reg [7:0]offset_pixel_1;
reg [7:0]offset_pixel_2;
reg [7:0]offset_pixel_3;
reg [7:0]offset_pixel_lsb1;
reg [7:0]offset_pixel_4;
reg [7:0]offset_pixel_5;
reg [7:0]offset_pixel_6;
reg [7:0]offset_pixel_7;
reg [7:0]offset_pixel_lsb2;


reg [1:0]offset_index;

			
reg [2:0]byte_count;
reg [63:0]last_data_i[2:0];
reg [1:0]idle_count;

reg data_valid_reg;
reg [63:0]data_reg;
reg [7:0]offset_factor_reg;
reg [2:0]burst_length_reg;
reg [1:0]idle_length_reg;
reg [2:0]packet_type_reg;

wire [7:0]offset_factor;
wire [2:0]burst_length;
wire [1:0]idle_length;
wire [255:0]pipe; //combined current and last input data byte

reg [127:0]output_10b;
reg [127:0]output_12b;
reg [127:0]output_14b;
reg output_valid_reg;
reg output_valid_reg_2;

assign pipe = {data_reg , last_data_i[0], last_data_i[1], last_data_i[2]}; //would need last bytes as well as current data to get full 8 pixel , with 16x gering bytes are arrange as [bb,aa]

				
assign offset_factor = (packet_type_i == (MIPI_CSI_PACKET_10bRAW & 8'h07))? 8'd32: (packet_type_i == (MIPI_CSI_PACKET_12bRAW & 8'h07))? 8'd32:8'd48;
					   
assign burst_length =  ((packet_type_i == (MIPI_CSI_PACKET_10bRAW & 8'h07)) || (packet_type_i == (MIPI_CSI_PACKET_14bRAW & 8'h07)))? 8'd5:8'd3;		   
						
assign idle_length =  ((packet_type_i == (MIPI_CSI_PACKET_10bRAW & 8'h07)) || (packet_type_i == (MIPI_CSI_PACKET_12bRAW & 8'h07)))? 2'd1: 2'd3;


assign raw_line_o = data_valid_i| output_valid_reg | output_valid_reg_2 | output_valid_o;
 
always @(*)
begin
	//output_10b[127:112] = 	{pipe [offset_pixel_0	+:8], 	pipe [offset_pixel_lsb1 +:2], 6'h0}; 		//lane 1 first pixel on wire	
	//output_10b[111:96]  = 	{pipe [offset_pixel_1 	+:8], 	pipe [offset_pixel_lsb1 +:4]>>2, 6'h0};		
	//output_10b[95:80]   = 	{pipe [offset_pixel_2 	+:8], 	pipe [offset_pixel_lsb1 +:6]>>4 , 6'h0};
	//output_10b[79:64]   = 	{pipe [offset_pixel_3 	+:8], 	pipe [offset_pixel_lsb1	+:8]>>6, 6'h0};
	//output_10b[63:48] = 	{pipe [offset_pixel_4 	+:8], 	pipe [offset_pixel_lsb2 +:2], 6'h0};
	//output_10b[47:32] = 	{pipe [offset_pixel_5 	+:8], 	pipe [offset_pixel_lsb2 +:4]>>2, 6'h0};
	//output_10b[31:16] = 	{pipe [offset_pixel_6 	+:8], 	pipe [offset_pixel_lsb2 +:6]>>4, 6'h0};		
	//output_10b[15:0]  = 	{pipe [offset_pixel_7	+:8], 	pipe [offset_pixel_lsb2 +:8]>>6, 6'h0}; 		//lane 4 	
	
	
	output_10b[127:112] = 	{pipe [offset_pixel_7	+:8], 	pipe[(offset_pixel_lsb2+6) +:2], 6'h0}; 		//lane 4 	
	output_10b[111:96]  = 	{pipe [offset_pixel_6 	+:8], 	pipe[(offset_pixel_lsb2+4) +:2], 6'h0};		
	output_10b[95:80]   = 	{pipe [offset_pixel_5 	+:8], 	pipe[(offset_pixel_lsb2+2) +:2], 6'h0};
	output_10b[79:64]   = 	{pipe [offset_pixel_4 	+:8], 	pipe[offset_pixel_lsb2 +:2], 6'h0};
	output_10b[63:48] = 	{pipe [offset_pixel_3 	+:8], 	pipe[(offset_pixel_lsb1+6) +:2], 6'h0};
	output_10b[47:32] = 	{pipe [offset_pixel_2 	+:8], 	pipe[(offset_pixel_lsb1+4) +:2], 6'h0};
	output_10b[31:16] = 	{pipe [offset_pixel_1 	+:8], 	pipe[(offset_pixel_lsb1+2) +:2], 6'h0};		
	output_10b[15:0]  = 	{pipe [offset_pixel_0	+:8], 	pipe[offset_pixel_lsb1 +:2], 6'h0}; 		//lane 1 first pixel on wire	

/*
	output_12b[127:112] = 	{pipe [offset_79	-:8], 	pipe [offset_95 	-:4]} << 4; 		//lane 4 	TODO:Reverify 
	output_12b[111:96]  = 	{pipe [offset_71 	-:8], 	pipe [offset_91 	-:4]} << 4;		
	output_12b[95:80]   = 	{pipe [offset_63 	-:8], 	pipe [offset_87 	-:4]} << 4;
	output_12b[79:64]   = 	{pipe [offset_55 	-:8], 	pipe [offset_83 	-:4]} << 4;
	output_12b[63:48] = 	{pipe [offset_31  	-:8], 	pipe [offset_47 	-:4]} << 4;
	output_12b[47:32] = 	{pipe [offset_23 	-:8], 	pipe [offset_43 	-:4]} << 4;
	output_12b[31:16] = 	{pipe [offset_15 	-:8], 	pipe [offset_39 	-:4]} << 4;
	output_12b[15:0]  = 	{pipe [offset_7 	-:8], 	pipe [offset_35 	-:4]} << 4; 		//lane 1
	
	output_14b[127:112] = 	{pipe [offset_87	-:8], 	pipe [offset_111 	-:6]} << 2; 		//lane 4 	TODO:Reverify 
	output_14b[111:96]  = 	{pipe [offset_79 	-:8], 	pipe [offset_105 	-:6]} << 2;		
	output_14b[95:80]   = 	{pipe [offset_71 	-:8], 	pipe [offset_99 	-:6]} << 2;
	output_14b[79:64]   = 	{pipe [offset_63 	-:8], 	pipe [offset_93 	-:6]} << 2;			
	output_14b[63:48] =		{pipe [offset_31 	-:8], 	pipe [offset_55 	-:6]} << 2;			
	output_14b[47:32] = 	{pipe [offset_23 	-:8], 	pipe [offset_49 	-:6]} << 2;
	output_14b[31:16] = 	{pipe [offset_15 	-:8], 	pipe [offset_43 	-:6]} << 2;
	output_14b[15:0]  = 	{pipe [offset_7 	-:8], 	pipe [offset_37 	-:6]} << 2; 		//lane 1 
	*/
end

always @(posedge clk_i)
begin

	
	if (packet_type_reg == (MIPI_CSI_PACKET_10bRAW & 8'h07))
	begin
		output_o <= output_10b;
	end
	else if (packet_type_reg == (MIPI_CSI_PACKET_12bRAW & 8'h07))
	begin		
		output_o <= output_12b;
	end
	else // if (packet_type_i == (MIPI_CSI_PACKET_14bRAW & 8'h07))
	begin
		output_o <= output_14b;
	end
	
end



always @(posedge clk_i)
begin
	
		output_valid_reg_2 <= output_valid_reg;
		output_valid_o <= output_valid_reg_2;
		
		if (output_valid_reg_2)
		begin
			offset_index = offset_index + 1;
		end
		else
		begin
			offset_index = 0;
		end
		
		offset_pixel_0 <= index_table_pixel_0[offset_index];
		offset_pixel_1 <= index_table_pixel_1[offset_index];
		offset_pixel_2 <= index_table_pixel_2[offset_index];
		offset_pixel_3 <= index_table_pixel_3[offset_index];
		offset_pixel_lsb1 <= index_table_pixel_lsb1[offset_index];
		offset_pixel_4 <= index_table_pixel_4[offset_index];
		offset_pixel_5 <= index_table_pixel_5[offset_index];
		offset_pixel_6 <= index_table_pixel_6[offset_index];
		offset_pixel_7 <= index_table_pixel_7[offset_index];
		offset_pixel_lsb2 <= index_table_pixel_lsb2[offset_index];
end

always @(posedge clk_i )//or negedge data_valid_reg)
begin
	
	if (data_valid_reg)
	begin

		if (byte_count < (burst_length_reg))
		begin
			byte_count <= byte_count + 1'd1;
			idle_count <= idle_length_reg - 1'b1;			
			
			output_valid_reg <= 1'b1;
			
		end
		else
		begin
			idle_count <= idle_count - 1'b1;
			if (!idle_count)
			begin
				byte_count <= 4'b1;		//set to 1 to enable output_valid_o with next edge
			end

			output_valid_reg <= 1'h0;
		end


	end
	else
	begin
	
		byte_count <= burst_length;

		index_table_pixel_0[0] <= 0;  
		index_table_pixel_0[1] <= 32;
		index_table_pixel_0[2] <= 8;
		index_table_pixel_0[3] <= 40;
		
		index_table_pixel_1[0] <= 16;
		index_table_pixel_1[1] <= 48;
		index_table_pixel_1[2] <= 24;
		index_table_pixel_1[3] <= 56;
		
		index_table_pixel_2[0] <= 32;
		index_table_pixel_2[1] <= 8;
		index_table_pixel_2[2] <= 40;
		index_table_pixel_2[3] <= 64;
		
		index_table_pixel_3[0] <= 48;
		index_table_pixel_3[1] <= 24;
		index_table_pixel_3[2] <= 56;
		index_table_pixel_3[3] <= 80;
		
		index_table_pixel_lsb1[0] <= 8;
		index_table_pixel_lsb1[1] <= 40;
		index_table_pixel_lsb1[2] <= 64;
		index_table_pixel_lsb1[3] <= 96;
		
		index_table_pixel_4[0] <= 24;
		index_table_pixel_4[1] <= 56;
		index_table_pixel_4[2] <= 80;
		index_table_pixel_4[3] <= 112;
		
		index_table_pixel_5[0] <= 40;
		index_table_pixel_5[1] <= 64;
		index_table_pixel_5[2] <= 96;
		index_table_pixel_5[3] <= 72;
		
		index_table_pixel_6[0] <= 56;
		index_table_pixel_6[1] <= 80;
		index_table_pixel_6[2] <= 112;
		index_table_pixel_6[3] <= 88;
		
		index_table_pixel_7[0] <= 64;
		index_table_pixel_7[1] <= 96;
		index_table_pixel_7[2] <= 72;
		index_table_pixel_7[3] <= 104;
		
		index_table_pixel_lsb2[0] <= 80;
		index_table_pixel_lsb2[1] <= 112;
		index_table_pixel_lsb2[2] <= 88;
		index_table_pixel_lsb2[3] <= 120;
		
	//	if (packet_type_i == (MIPI_CSI_PACKET_14bRAW & 8'h07))		// for 14bit need to wait for 3 sample while 12bit and 10bit only need 1 sample delay
	//	begin
	//		idle_count <= 3'd2;
	//	end
	//	else
	//	begin 
			idle_count <= 3'b0;	//need to be zero to wait for 1 sample after data become valid	
	//	end
		
		output_valid_reg <= 1'h0;
		offset_factor_reg <= offset_factor;
		burst_length_reg <= burst_length;
		idle_length_reg <= idle_length;
		packet_type_reg <= packet_type_i;
	end
end

always @(posedge clk_i)
begin
		data_valid_reg <= data_valid_i;
		data_reg <= data_i;
		
		last_data_i[0] <= data_reg;
		last_data_i[1] <= last_data_i[0];
		last_data_i[2] <= last_data_i[1];
end

endmodule
