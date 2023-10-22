`timescale 1ns/1ns

/*
MIPI CSI RX to Parallel Bridge (c) by Gaurav Singh www.CircuitValley.com

MIPI CSI RX to Parallel Bridge is licensed under a
Creative Commons Attribution 3.0 Unported License.

You should have received a copy of the license along with this
work.  If not, see <http://creativecommons.org/licenses/by/3.0/>.
*/

/*
Takes 32bit 2pixel to 128bit 8pixel yuv input from rgb2yuv module @ mipi byte clock outputs 32bit 2pixel yuv output @output_clk_i , 
output_clk_i can be totoally independnt phease and frequency of mipi_byte_clock, but frequnecy be fast enough to have full line transmitted before next line comes.
This implementation of Output reformatter outputs data which which meant to send out of the system to a 32bit receiver
depending on requirement this will be need to be adapted as per the receiver 
*/

module  output_reformatter #(parameter PIXEL_PER_CLK=8 )(
						  clk_i, //data changes on negedge 
						  output_clk_i, //output clock
						  data_i,
						  data_in_valid_i, //expected active high
						  line_sync_i, //expected active high
						  frame_sync_i,  //expected active low
						  output_o,
						  output_valid_o //active high
						  );
				  
input line_sync_i;
input frame_sync_i;
input clk_i;
input data_in_valid_i; 
input [((PIXEL_PER_CLK * 8 * 2) - 1 ):0]data_i;
output reg output_valid_o;
output [31:0]output_o;
input output_clk_i;

reg [11:0] write_address;
reg [11:0] read_address;


wire [31:0]ram_even_o;
wire [31:0]ram_odd_o;

reg [10:0] input_pixel_count_clk_i; //under clk_i domain
reg [10:0] input_pixel_count_meta1;
reg [10:0] input_pixel_count_meta2;
reg [10:0] input_pixel_count_out_clk;

reg line_even_nodd_clk_i;				//select between two different RAM
reg line_even_nodd_meta1;
reg line_even_nodd_meta2;
reg line_even_nodd_meta3;
reg line_even_nodd_meta4;
reg line_even_nodd_out_clk; 

reg last_line_sync;				//helps to determine edge of line sync for write address reset
reg last_line_even_nodd;		//helps to determine edge of line sync for read address reset

//ebr ram_dp write address and data is latched on same rising edge of write clock
//read address is latached on rising edge of read clock and data outputed on same rising edge but after tCO_EBR so should be sampled on comming fallsing or next rising edge


out_line_ram_dp line_odd(	.wr_clk_i(clk_i),  					// write domain
							.rst_i(frame_sync_i), 
							.wr_clk_en_i(data_in_valid_i),
							.wr_en_i(!line_even_nodd_clk_i), 
							.wr_data_i(data_i),
							.wr_addr_i(write_address), 
							
							.rd_clk_i(output_clk_i), 			// read domain
							.rd_en_i(line_even_nodd_out_clk), 
							.rd_clk_en_i(1'b1), 
							.rd_addr_i(read_address),  
							.rd_data_o(ram_odd_o));


out_line_ram_dp line_even(	.wr_clk_i(clk_i), 					
							.rst_i(frame_sync_i),     			
							.wr_clk_en_i(data_in_valid_i),   	
							.wr_en_i(line_even_nodd_clk_i),  	
							.wr_data_i(data_i),					
							.wr_addr_i(write_address), 
							
							.rd_clk_i(output_clk_i),  			
							.rd_en_i(!line_even_nodd_out_clk), 	
							.rd_clk_en_i(1'b1),  				
							.rd_addr_i(read_address), 			
							.rd_data_o(ram_even_o)); 			

//assign output_o = line_even_nodd? ram_odd_o[((read_address[0])?6'd32:6'd0) +:32]: ram_even_o[((read_address[0])?6'd32:6'd0) +:32]; //depeding on line select even or odd ram , also select correct 32bit word from 64 bit ramoutput

assign output_o = line_even_nodd_out_clk? ram_odd_o:ram_even_o; //depeding on line select even or odd ram 



always @(posedge clk_i or posedge frame_sync_i)
begin
	if (frame_sync_i)
	begin
		line_even_nodd_clk_i <= 0;
		last_line_sync <= 0;
		input_pixel_count_clk_i <= 0;
		write_address <= 0;
	end
	else
	begin
		last_line_sync <= line_sync_i;
		
		if (!last_line_sync && line_sync_i) // on rising edge of line_sync_i
		begin
			write_address <= 9'b0;
			
			input_pixel_count_clk_i <= write_address << (PIXEL_PER_CLK >> 2) ; //x4 or x2 or also x1 write_address as each write_address has 128 bit or 64 bit or 32 bit while output width is 32bit
			line_even_nodd_clk_i <= !line_even_nodd_clk_i;
		end
		else
		begin
			write_address <= write_address + data_in_valid_i; 
		end
	end
end


always @(posedge output_clk_i)
begin
		line_even_nodd_meta1 <= line_even_nodd_clk_i;		//This is Sync signal need to have more flip-flops to get more delay to make sure sync always arrive after pixel count is already setlled
		line_even_nodd_meta2 <= line_even_nodd_meta1;
		line_even_nodd_meta3 <= line_even_nodd_meta2;
		line_even_nodd_meta4 <= line_even_nodd_meta3;
		line_even_nodd_out_clk <= line_even_nodd_meta4;
		
		
		input_pixel_count_meta1 <= input_pixel_count_clk_i;
		input_pixel_count_meta2 <= input_pixel_count_meta1;

		last_line_even_nodd <= line_even_nodd_out_clk;

		
		if (last_line_even_nodd != line_even_nodd_out_clk)	//reset read address for each new line
		begin
			 read_address <= 12'b0;
			 output_valid_o <= 1'b0;
			 input_pixel_count_out_clk <= input_pixel_count_meta2;
		end
		else
			begin
			if (read_address < input_pixel_count_out_clk)
			begin
				read_address <= read_address + 1'b1;
				output_valid_o <= 1'b1;
			end
			else
			begin
				output_valid_o <= 1'b0;
			end 
		end
end


endmodule