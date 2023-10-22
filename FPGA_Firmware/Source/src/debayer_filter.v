`timescale 1ns/1ns

/*
MIPI CSI RX to Parallel Bridge (c) by Gaurav Singh www.CircuitValley.com

MIPI CSI RX to Parallel Bridge is licensed under a
Creative Commons Attribution 3.0 Unported License.

You should have received a copy of the license along with this
work.  If not, see <http://creativecommons.org/licenses/by/3.0/>.
*/

/*
Takes upto 8 pixel upto 16 bit from RAW depacker module @mipi byte clock output upto 16bit RGB for each pixel ,
Implement Basic Debayer filter, As debayer need pixel infrom neighboring pixel which may be on next or previous display line,
so input data is written onto RAM, only 4 lines are stored in RAM at one time and only three of the readable at any give time , RAM to which data is written to can not be read. 
as we have enough info in RAM, RAW pixel will be coverted to  RGB output
First line is expected to BGBG , second line GRGR Basically BGGR format  
*/

module debayer_filter #(parameter PIXEL_WIDTH=14 , parameter PIXEL_PER_CLK=8 )(clk_i,
								  reset_i,
								  line_valid_i,
								  data_i, 	//lane 0 is LSbits 
								  data_valid_i,
								  output_valid_o, //lane 0 is LSbits
								  output_o
								  );

localparam INPUT_WIDTH = PIXEL_PER_CLK * PIXEL_WIDTH;	//Upto 8 x 16bit pixels from raw depacker module  (if 16bit each channel)
localparam OUTPUT_WIDTH = PIXEL_WIDTH * PIXEL_PER_CLK * 3;  //Upto 8 x 48bit RGB output 



input clk_i;
input reset_i;
input line_valid_i;
input data_valid_i;
input [(INPUT_WIDTH -1):0] data_i;
output reg output_valid_o;
output reg [(OUTPUT_WIDTH-1):0]output_o;

reg line_counter; //only 1 bit so does not actually counts lines of the frame , needed determine if line is odd or even
reg data_valid_reg;
reg data_valid_reg2;

reg [(PIXEL_WIDTH - 1):0]R1[(PIXEL_PER_CLK -1):0];
reg [(PIXEL_WIDTH - 1):0]R2[(PIXEL_PER_CLK -1):0];
reg [(PIXEL_WIDTH - 1):0]R3[(PIXEL_PER_CLK -1):0];
reg [(PIXEL_WIDTH - 1):0]R4[(PIXEL_PER_CLK -1):0];


reg [(PIXEL_WIDTH - 1):0]B1[(PIXEL_PER_CLK -1):0];
reg [(PIXEL_WIDTH - 1):0]B2[(PIXEL_PER_CLK -1):0];
reg [(PIXEL_WIDTH - 1):0]B3[(PIXEL_PER_CLK -1):0];
reg [(PIXEL_WIDTH - 1):0]B4[(PIXEL_PER_CLK -1):0];

reg [(PIXEL_WIDTH - 1):0]G1[(PIXEL_PER_CLK -1):0];
reg [(PIXEL_WIDTH - 1):0]G2[(PIXEL_PER_CLK -1):0];
reg [(PIXEL_WIDTH - 1):0]G3[(PIXEL_PER_CLK -1):0];
reg [(PIXEL_WIDTH - 1):0]G4[(PIXEL_PER_CLK -1):0];


reg [(PIXEL_WIDTH - 1):0]R1_even[(PIXEL_PER_CLK -1):0];
reg [(PIXEL_WIDTH - 1):0]R2_even[(PIXEL_PER_CLK -1):0];
reg [(PIXEL_WIDTH - 1):0]R3_even[(PIXEL_PER_CLK -1):0];
reg [(PIXEL_WIDTH - 1):0]R4_even[(PIXEL_PER_CLK -1):0];


reg [(PIXEL_WIDTH - 1):0]B1_even[(PIXEL_PER_CLK -1):0];
reg [(PIXEL_WIDTH - 1):0]B2_even[(PIXEL_PER_CLK -1):0];
reg [(PIXEL_WIDTH - 1):0]B3_even[(PIXEL_PER_CLK -1):0];
reg [(PIXEL_WIDTH - 1):0]B4_even[(PIXEL_PER_CLK -1):0];

reg [(PIXEL_WIDTH - 1):0]G1_even[(PIXEL_PER_CLK -1):0];
reg [(PIXEL_WIDTH - 1):0]G2_even[(PIXEL_PER_CLK -1):0];
reg [(PIXEL_WIDTH - 1):0]G3_even[(PIXEL_PER_CLK -1):0];
reg [(PIXEL_WIDTH - 1):0]G4_even[(PIXEL_PER_CLK -1):0];

reg [(PIXEL_WIDTH - 1):0]R1_odd[(PIXEL_PER_CLK -1):0];
reg [(PIXEL_WIDTH - 1):0]R2_odd[(PIXEL_PER_CLK -1):0];
reg [(PIXEL_WIDTH - 1):0]R3_odd[(PIXEL_PER_CLK -1):0];
reg [(PIXEL_WIDTH - 1):0]R4_odd[(PIXEL_PER_CLK -1):0];

reg [(PIXEL_WIDTH - 1):0]B1_odd[(PIXEL_PER_CLK -1):0];
reg [(PIXEL_WIDTH - 1):0]B2_odd[(PIXEL_PER_CLK -1):0];
reg [(PIXEL_WIDTH - 1):0]B3_odd[(PIXEL_PER_CLK -1):0];
reg [(PIXEL_WIDTH - 1):0]B4_odd[(PIXEL_PER_CLK -1):0];


reg [(PIXEL_WIDTH - 1):0]G1_odd[(PIXEL_PER_CLK -1):0];
reg [(PIXEL_WIDTH - 1):0]G2_odd[(PIXEL_PER_CLK -1):0];
reg [(PIXEL_WIDTH - 1):0]G3_odd[(PIXEL_PER_CLK -1):0];
reg [(PIXEL_WIDTH - 1):0]G4_odd[(PIXEL_PER_CLK -1):0];



reg [1:0]read_ram_index_even; 	//which line RAM is being focused to read for even lines,  (not which address is being read from line RAM) Must be 2 bits only
reg [1:0]read_ram_index_odd; 	//which line RAM is being focused to read for odd lines,  (not which address is being read from line RAM)
reg [1:0]read_ram_index_even_plus_1; 
reg [1:0]read_ram_index_odd_plus_1; 
reg [1:0]read_ram_index_even_minus_1; 
reg [1:0]read_ram_index_odd_minus_1; 


reg [3:0]write_ram_select;	//which line RAM is begin written
reg [10:0]line_address; 		//which address is being read and written 


reg [1:0]not_used2b;


wire [(INPUT_WIDTH-1):0]RAM_out[3:0];
reg  [(INPUT_WIDTH-1):0]RAM_out_reg[3:0];
reg  [(INPUT_WIDTH-1):0]last_ram_outputs[3:0]; //one clock cycle delayed output of line RAMs
reg  [(INPUT_WIDTH-1):0]last_ram_outputs_stage2[3:0]; //two clock cycle delayed output of RAMs 

wire ram_write_enable;
wire ram_clk;

assign ram_clk = clk_i; 
assign ram_write_enable = line_valid_i;

integer i;
//line rams, total 4,
//The way debayer is implemented in this code. Depending on pixel, we need minimum 2 lines and  maximum 3 lines in the ram, To be able to have access to neighboring pixels from previous and next line
//There are many ways to implemented debayer, this code implement simplest possible bare minimum.
//IMX219/IMX477/IMX290/IMX462/IMX327 Camera only output BGGR as defined by Driver in linux repo MEDIA_BUS_FMT_SBGGR10_1X10,  Camera datasheet incrorrectly defines output as RGGB and GBRG. Data sheet is incorrect in this case.
//Bayer filter type does not affet test pattern. 

//ebr ram_dp write address and data is latched on same rising edge of write clock
//read address is latached on rising edge of read clock and data outputed on same rising edge but after tCO_EBR so should be sampled on comming fallsing or next rising edge

line_ram_dp line0(.wr_clk_i(ram_clk), 		//data and address latch in on rising edge 
				  .rd_clk_i(ram_clk), 
				  .rst_i(reset_i), 
				  .wr_clk_en_i(ram_write_enable),					
				  .rd_en_i(1'b1), 
				  .rd_clk_en_i(1'b1), 
				  .wr_en_i(write_ram_select[0]  ), 
				  .wr_data_i(data_i), 
				  .wr_addr_i(line_address), 
				  .rd_addr_i(line_address), 
				  .rd_data_o(RAM_out[0])) ;

line_ram_dp line1(.wr_clk_i(ram_clk), 
				  .rd_clk_i(ram_clk), 
				  .rst_i(reset_i), 
				  .wr_clk_en_i(ram_write_enable),
				  .rd_en_i(1'b1), 
				  .rd_clk_en_i(1'b1), 
				  .wr_en_i(write_ram_select[1]), 
				  .wr_data_i(data_i), 
				  .wr_addr_i(line_address), 
				  .rd_addr_i(line_address), 
				  .rd_data_o(RAM_out[1])) ;

line_ram_dp line2(.wr_clk_i(ram_clk), 
				  .rd_clk_i(ram_clk), 
				  .rst_i(reset_i), 
				  .wr_clk_en_i(ram_write_enable),
				  .rd_en_i(1'b1), 
				  .rd_clk_en_i(1'b1), 
				  .wr_en_i(write_ram_select[2]), 
				  .wr_data_i(data_i), 
				  .wr_addr_i(line_address), 
				  .rd_addr_i(line_address), 
				  .rd_data_o(RAM_out[2])) ;

line_ram_dp line3(.wr_clk_i(ram_clk), 	
				  .rd_clk_i(ram_clk), 
				  .rst_i(reset_i), 
				  .wr_clk_en_i(ram_write_enable),
				  .rd_en_i(1'b1), 
				  .rd_clk_en_i(1'b1), 
				  .wr_en_i(write_ram_select[3]), 
				  .wr_data_i(data_i), 
				  .wr_addr_i(line_address), 
				  .rd_addr_i(line_address), 
				  .rd_data_o(RAM_out[3])) ;



always @(posedge clk_i)	 
begin
	if (!line_valid_i )
	begin
		line_address <= 10'h0;
	end
	else
	begin
		if (data_valid_i)
		begin
			line_address <= line_address + 1'b1;
		end
	end
end


always @(posedge reset_i or posedge line_valid_i)
begin
	if (reset_i)
	begin
		write_ram_select <= 4'b1000;	//on first line ram[0] will be selected 
		line_counter <= 0;				//on first line line_counter --> 1 --> odd 
		read_ram_index_odd <= 2'd1;		//on first line read from ram 2 (1 + 1 at rising edge of line_valid_i)
		read_ram_index_even <= 2'd1;
		read_ram_index_odd_plus_1 <= 2'd2;
		read_ram_index_odd_minus_1 <= 2'd0;
		read_ram_index_even_plus_1 <= 2'd2;
		read_ram_index_even_minus_1 <= 2'd0;
	end
	else
	begin
		write_ram_select <= {write_ram_select[2:0], write_ram_select[3]};

			
		read_ram_index_odd 			<= read_ram_index_odd 			+ 1'b1;
		read_ram_index_even 		<= read_ram_index_even 			+ 1'b1;
		read_ram_index_odd_plus_1 	<= read_ram_index_odd_plus_1 	+ 1'b1;
		read_ram_index_odd_minus_1 	<= read_ram_index_odd_minus_1 	+ 1'b1;
		read_ram_index_even_plus_1 	<= read_ram_index_even_plus_1 	+ 1'b1;
		read_ram_index_even_minus_1 <= read_ram_index_even_minus_1 	+ 1'b1;

		line_counter <= !line_counter;
	end
end

always @(posedge ram_clk)
begin
	
	for ( i = 0; i < 4; i = i + 1)
	begin
		RAM_out_reg[i] 				<= RAM_out[i];
		last_ram_outputs[i] 		<= RAM_out_reg[i];
		last_ram_outputs_stage2[i] 	<= last_ram_outputs[i];
	end
end

always @(posedge clk_i or posedge reset_i )
begin
	if (reset_i)
	begin
		output_valid_o <= 1'b0;
		data_valid_reg <= 1'b0;
		data_valid_reg2 <= 1'b0;
	end
	else
	begin
		data_valid_reg <= data_valid_i; 
		data_valid_reg2 <= data_valid_reg; 
		output_valid_o <= data_valid_reg2;
	end
end



always @(*)
begin
		if (line_counter)	//odd
			begin
				
				for (i=0; i < PIXEL_PER_CLK; i = i + 1)
				begin
					R1[i] <= R1_odd[i];
					R2[i] <= R2_odd[i];
					R3[i] <= R3_odd[i];
					R4[i] <= R4_odd[i];
					
					G1[i] <= G1_odd[i];
					G2[i] <= G2_odd[i];
					G3[i] <= G3_odd[i];
					G4[i] <= G4_odd[i];
					
					B1[i] <= B1_odd[i];
					B2[i] <= B2_odd[i];
					B3[i] <= B3_odd[i];	
					B4[i] <= B4_odd[i];
				end				
				
			end 	//end odd rows
			else
			begin	//even rows  
				
				for (i=0; i < PIXEL_PER_CLK; i = i + 1)
				begin
					R1[i] <= R1_even[i];
					R2[i] <= R2_even[i];
					R3[i] <= R3_even[i];
					R4[i] <= R4_even[i];
					
					G1[i] <= G1_even[i];
					G2[i] <= G2_even[i];
					G3[i] <= G3_even[i];
					G4[i] <= G4_even[i];
					
					B1[i] <= B1_even[i];
					B2[i] <= B2_even[i];
					B3[i] <= B3_even[i];	
					B4[i] <= B4_even[i];
				end

			end 
end

always @(posedge clk_i)
begin
		for (i=0; i < PIXEL_PER_CLK ; i = i + 1)
		begin
			{not_used2b,output_o[((i*(PIXEL_WIDTH * 3)) + (PIXEL_WIDTH*2)) +:PIXEL_WIDTH]} <= {(({2'd0, R1[i]} + R2[i]) + R3[i]) + R4[i]} >> 2; //R
			{not_used2b,output_o[((i*(PIXEL_WIDTH * 3)) + (PIXEL_WIDTH  )) +:PIXEL_WIDTH]} <= {(({2'd0, G1[i]} + G2[i]) + G3[i]) + G4[i]} >> 2; //G
			{not_used2b,output_o[ (i*(PIXEL_WIDTH * 3)) 	   			   +:PIXEL_WIDTH]} <= {(({2'd0, B1[i]} + B2[i]) + B3[i]) + B4[i]} >> 2; //B
			
		end
end

wire [((INPUT_WIDTH*3)-1):0]ram_pipe[3:0];

assign ram_pipe[0] = {RAM_out_reg[0], last_ram_outputs[0], last_ram_outputs_stage2[0]}; //ram output delayed by 1 register , previous ram output(history level 1),  previous ram out history level 2
assign ram_pipe[1] = {RAM_out_reg[1], last_ram_outputs[1], last_ram_outputs_stage2[1]}; //packing pixels  {ff,ee,dd,cc,bb,aa}
assign ram_pipe[2] = {RAM_out_reg[2], last_ram_outputs[2], last_ram_outputs_stage2[2]};
assign ram_pipe[3] = {RAM_out_reg[3], last_ram_outputs[3], last_ram_outputs_stage2[3]};

always @(*)
begin
		for (i=0; i < PIXEL_PER_CLK; i = i +2)		// all pixles calculated for all possible combination but only the correct pixel is put to output,
		begin
			
			R1_odd[i] 		= ram_pipe[ read_ram_index_odd_minus_1 	][((INPUT_WIDTH+ ( i	*	PIXEL_WIDTH)) - PIXEL_WIDTH) 	+:PIXEL_WIDTH];
			R2_odd[i] 		= ram_pipe[ read_ram_index_odd_minus_1 	][ (INPUT_WIDTH+ ((i+1)	*	PIXEL_WIDTH)) 					+:PIXEL_WIDTH];
			R3_odd[i] 		= ram_pipe[ read_ram_index_odd_plus_1  	][((INPUT_WIDTH+ ( i 	*	PIXEL_WIDTH)) - PIXEL_WIDTH) 	+:PIXEL_WIDTH];
			R4_odd[i] 		= ram_pipe[ read_ram_index_odd_plus_1  	][ (INPUT_WIDTH+ ((i+1)	*	PIXEL_WIDTH)) 					+:PIXEL_WIDTH];

			R1_odd[i + 1] 	= ram_pipe[ read_ram_index_odd_minus_1	][ (INPUT_WIDTH+ ((i+1)	*	PIXEL_WIDTH))					+:PIXEL_WIDTH]; 
			R2_odd[i + 1] 	= ram_pipe[ read_ram_index_odd_plus_1	][ (INPUT_WIDTH+ ((i+1)	*	PIXEL_WIDTH))					+:PIXEL_WIDTH]; 
			R3_odd[i + 1] 	= ram_pipe[ read_ram_index_odd_minus_1	][ (INPUT_WIDTH+ ((i+1)	*	PIXEL_WIDTH))					+:PIXEL_WIDTH]; 
			R4_odd[i + 1] 	= ram_pipe[ read_ram_index_odd_plus_1	][ (INPUT_WIDTH+ ((i+1)	*	PIXEL_WIDTH))					+:PIXEL_WIDTH]; 

			G1_odd[i] 		= ram_pipe[ read_ram_index_odd_minus_1 	][ (INPUT_WIDTH+ ( i	*	PIXEL_WIDTH)) 	 			   	+:PIXEL_WIDTH];
			G2_odd[i] 		= ram_pipe[ read_ram_index_odd_plus_1  	][ (INPUT_WIDTH+ ( i	*	PIXEL_WIDTH)) 	 			   	+:PIXEL_WIDTH];
			G3_odd[i] 		= ram_pipe[ read_ram_index_odd    	 	][ (INPUT_WIDTH+ ((i+1)	*	PIXEL_WIDTH)) 			   		+:PIXEL_WIDTH];
			G4_odd[i] 		= ram_pipe[ read_ram_index_odd 		 	][((INPUT_WIDTH+ ( i	*	PIXEL_WIDTH)) - PIXEL_WIDTH) 	+:PIXEL_WIDTH];

			G1_odd[i + 1] 	= ram_pipe[ read_ram_index_odd 			][ (INPUT_WIDTH+ ((i+1) *	PIXEL_WIDTH)) 					+:PIXEL_WIDTH];
			G2_odd[i + 1] 	= ram_pipe[ read_ram_index_odd 			][ (INPUT_WIDTH+ ((i+1) *	PIXEL_WIDTH)) 					+:PIXEL_WIDTH];
			G3_odd[i + 1] 	= ram_pipe[ read_ram_index_odd 			][ (INPUT_WIDTH+ ((i+1) *	PIXEL_WIDTH)) 					+:PIXEL_WIDTH];
			G4_odd[i + 1] 	= ram_pipe[ read_ram_index_odd 			][ (INPUT_WIDTH+ ((i+1) *	PIXEL_WIDTH)) 					+:PIXEL_WIDTH];
			
			B1_odd[i] 		= ram_pipe[ read_ram_index_odd 			][ (INPUT_WIDTH+ ( i	*	PIXEL_WIDTH)) 					+:PIXEL_WIDTH];
			B2_odd[i] 		= ram_pipe[ read_ram_index_odd 			][ (INPUT_WIDTH+ ( i	*	PIXEL_WIDTH)) 					+:PIXEL_WIDTH];
			B3_odd[i] 		= ram_pipe[ read_ram_index_odd 			][ (INPUT_WIDTH+ ( i	*	PIXEL_WIDTH)) 					+:PIXEL_WIDTH];
			B4_odd[i] 		= ram_pipe[ read_ram_index_odd 			][ (INPUT_WIDTH+ ( i	*	PIXEL_WIDTH)) 					+:PIXEL_WIDTH];
			
			B1_odd[i + 1] 	= ram_pipe[ read_ram_index_odd 			][ (INPUT_WIDTH+ ( i	*	PIXEL_WIDTH))					+:PIXEL_WIDTH];
			B2_odd[i + 1] 	= ram_pipe[ read_ram_index_odd 			][ (INPUT_WIDTH+ ((i+2)	*	PIXEL_WIDTH))					+:PIXEL_WIDTH];
			B3_odd[i + 1] 	= ram_pipe[ read_ram_index_odd 			][ (INPUT_WIDTH+ ( i	*	PIXEL_WIDTH))					+:PIXEL_WIDTH];
			B4_odd[i + 1] 	= ram_pipe[ read_ram_index_odd 			][ (INPUT_WIDTH+ ((i+2)	*	PIXEL_WIDTH))					+:PIXEL_WIDTH];
			
			R1_even[i] 		= ram_pipe[ read_ram_index_even 		][ (INPUT_WIDTH+ ((i+1)	*	PIXEL_WIDTH)) 	 		  		+:PIXEL_WIDTH];
			R2_even[i] 		= ram_pipe[ read_ram_index_even 		][((INPUT_WIDTH+ ((i)	*	PIXEL_WIDTH)) - PIXEL_WIDTH) 	+:PIXEL_WIDTH];
			R3_even[i] 		= ram_pipe[ read_ram_index_even 		][((INPUT_WIDTH+ ((i)	*	PIXEL_WIDTH)) - PIXEL_WIDTH) 	+:PIXEL_WIDTH];
			R4_even[i] 		= ram_pipe[ read_ram_index_even 		][ (INPUT_WIDTH+ ((i+1)	*	PIXEL_WIDTH))    		  		+:PIXEL_WIDTH];
			
			R1_even[i + 1] 	= ram_pipe[ read_ram_index_even 		][ (INPUT_WIDTH+ ((i+1)	*	PIXEL_WIDTH)) 					+:PIXEL_WIDTH];
			R2_even[i + 1] 	= ram_pipe[ read_ram_index_even 		][ (INPUT_WIDTH+ ((i+1)	*	PIXEL_WIDTH)) 					+:PIXEL_WIDTH];
			R3_even[i + 1] 	= ram_pipe[ read_ram_index_even  		][ (INPUT_WIDTH+ ((i+1)	*	PIXEL_WIDTH)) 					+:PIXEL_WIDTH];
			R4_even[i + 1] 	= ram_pipe[ read_ram_index_even 		][ (INPUT_WIDTH+ ((i+1)	*	PIXEL_WIDTH)) 					+:PIXEL_WIDTH];
					
			G1_even[i] 		= ram_pipe[ read_ram_index_even 		][ (INPUT_WIDTH+ ( i	*	PIXEL_WIDTH)) 					+:PIXEL_WIDTH];
			G2_even[i] 		= ram_pipe[ read_ram_index_even 		][ (INPUT_WIDTH+ ( i	*	PIXEL_WIDTH)) 					+:PIXEL_WIDTH];
			G3_even[i] 		= ram_pipe[ read_ram_index_even 		][ (INPUT_WIDTH+ ( i	*	PIXEL_WIDTH)) 					+:PIXEL_WIDTH];
			G4_even[i] 		= ram_pipe[ read_ram_index_even 		][ (INPUT_WIDTH+ ( i	*	PIXEL_WIDTH)) 					+:PIXEL_WIDTH];

			G1_even[i + 1] 	= ram_pipe[ read_ram_index_even		  	][ (INPUT_WIDTH+ ( i	*	PIXEL_WIDTH))					+:PIXEL_WIDTH];
			G2_even[i + 1] 	= ram_pipe[ read_ram_index_even_minus_1	][ (INPUT_WIDTH+ ((i+1) *	PIXEL_WIDTH)) 					+:PIXEL_WIDTH];
			G3_even[i + 1] 	= ram_pipe[ read_ram_index_even_plus_1 	][ (INPUT_WIDTH+ ((i+1) *	PIXEL_WIDTH)) 					+:PIXEL_WIDTH];
			G4_even[i + 1] 	= ram_pipe[ read_ram_index_even		  	][ (INPUT_WIDTH+ ((i+2)	*	PIXEL_WIDTH)) 					+:PIXEL_WIDTH];

			B1_even[i] 		= ram_pipe[ read_ram_index_even_plus_1 	][ (INPUT_WIDTH+ ( i	*	PIXEL_WIDTH)) 					+:PIXEL_WIDTH]; 
			B2_even[i] 		= ram_pipe[ read_ram_index_even_plus_1 	][ (INPUT_WIDTH+ ( i	*	PIXEL_WIDTH)) 					+:PIXEL_WIDTH];
			B3_even[i] 		= ram_pipe[ read_ram_index_even_minus_1	][ (INPUT_WIDTH+ ( i	*	PIXEL_WIDTH)) 					+:PIXEL_WIDTH]; 
			B4_even[i] 		= ram_pipe[ read_ram_index_even_minus_1	][ (INPUT_WIDTH+ ( i	*	PIXEL_WIDTH)) 					+:PIXEL_WIDTH];
			
			B1_even[i+1] 	= ram_pipe[ read_ram_index_even_minus_1	][ (INPUT_WIDTH+ ( i	*	PIXEL_WIDTH)) 					+:PIXEL_WIDTH]; 
			B2_even[i+1] 	= ram_pipe[ read_ram_index_even_plus_1 	][ (INPUT_WIDTH+ ( i	*	PIXEL_WIDTH)) 					+:PIXEL_WIDTH];
			B3_even[i+1] 	= ram_pipe[ read_ram_index_even_minus_1	][ (INPUT_WIDTH+ ((i+2)	*	PIXEL_WIDTH)) 					+:PIXEL_WIDTH];
			B4_even[i+1] 	= ram_pipe[ read_ram_index_even_plus_1 	][ (INPUT_WIDTH+ ((i+2)	*	PIXEL_WIDTH))					+:PIXEL_WIDTH];


		end			
end
endmodule