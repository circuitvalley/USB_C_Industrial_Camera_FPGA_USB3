`timescale 1ns/1ns

/*
MIPI CSI RX to Parallel Bridge (c) by Gaurav Singh www.CircuitValley.com

MIPI CSI RX to Parallel Bridge is licensed under a
Creative Commons Attribution 3.0 Unported License.

You should have received a copy of the license along with this
work.  If not, see <http://creativecommons.org/licenses/by/3.0/>.
*/

/*
received upto 8 pixel RGB from the Debayer filter output upto 8 pixel yuv422 
Calculation is done based on integer YUV formula from the YUV wiki page 
*/

module rgb_to_yuv #(parameter PIXEL_WIDTH=16 , parameter PIXEL_PER_CLK=8 )(clk_i, //data changes on rising edge , latched in on falling edge
																		  rgb_i,
																		  rgb_valid_i,
																		  line_valid_i,
																		  yuv_o,
																		  yuv_valid_o,
																		  yuv_line_o);
				  input clk_i;
input [((PIXEL_WIDTH * PIXEL_PER_CLK * 3) - 1'd1):0]rgb_i;
input rgb_valid_i;
input line_valid_i;

output reg [((PIXEL_PER_CLK * 2 * 8) - 1'd1):0]yuv_o;
output reg yuv_valid_o;
output yuv_line_o;

integer i;
reg rgb_valid_stage1;
reg rgb_valid_stage2;

reg [7:0]Y[(PIXEL_PER_CLK - 1 ) :0]; // result 8 pixesl , 8 bit per channel , ultimately 16bit per pixel 
reg [7:0]U[(PIXEL_PER_CLK - 1 ) :0];
reg [7:0]V[(PIXEL_PER_CLK - 1 ) :0];

reg [24:0]Y_R[(PIXEL_PER_CLK - 1 ) :0]; //stores result  of  16bit input * ~8bit Const
reg [24:0]Y_G[(PIXEL_PER_CLK - 1 ) :0];
reg [24:0]Y_B[(PIXEL_PER_CLK - 1 ) :0];

reg [24:0]U_R[(PIXEL_PER_CLK - 1 ) :0];  //calculated only for alternate pixel so 4 pixel per chunk for 16bit pipeline  while Y is 8 pixel per chunk
reg [24:0]U_G[(PIXEL_PER_CLK - 1 ) :0];
reg [24:0]U_B[(PIXEL_PER_CLK - 1 ) :0];

reg [24:0]V_R[(PIXEL_PER_CLK - 1 ) :0];
reg [24:0]V_G[(PIXEL_PER_CLK - 1 ) :0];
reg [24:0]V_B[(PIXEL_PER_CLK - 1 ) :0];


reg [24:0]Y_ADD[(PIXEL_PER_CLK - 1 ) :0]; //intermediate Y result from pipeline  
reg [24:0]U_ADD[(PIXEL_PER_CLK - 1 ) :0]; //intermediate U result from pipeline  
reg [24:0]V_ADD[(PIXEL_PER_CLK - 1 ) :0]; //intermediate V result from pipeline  

reg [24:0]Y_B_STAGE_ADD[(PIXEL_PER_CLK - 1 ) :0];
reg [24:0]U_B_STAGE_ADD[(PIXEL_PER_CLK - 1 ) :0];
reg [24:0]V_B_STAGE_ADD[(PIXEL_PER_CLK - 1 ) :0];

reg [7:0]Y_ADD_STAGE2[(PIXEL_PER_CLK - 1 ) :0];
reg [7:0]U_ADD_STAGE2[(PIXEL_PER_CLK - 1 ) :0];
reg [7:0]V_ADD_STAGE2[(PIXEL_PER_CLK - 1 ) :0];


assign yuv_line_o = line_valid_i | rgb_valid_i | yuv_valid_o;

//from YUV wiki page full swing
// Y = ((77 R + 150G + 29B + 128) >>10)
// U = ((-43R - 84G + 127B + 128) >>10) + 128
// V = ((127R -106G -21B + 128) >>10) + 128

reg [23:0]not_used24; //to suppress warning from the tool 
always @(posedge  clk_i)
begin
	rgb_valid_stage1 <= rgb_valid_i; 
	rgb_valid_stage2 <= rgb_valid_stage1;
	yuv_valid_o <= rgb_valid_i;
	
	for (i=0;i<PIXEL_PER_CLK; i = i + 1)
	begin

		Y_R[i] <= ( 77 * rgb_i[((i* (PIXEL_WIDTH*3)) + (PIXEL_WIDTH*2)) +: PIXEL_WIDTH]);
		Y_G[i] <= (150 * rgb_i[((i* (PIXEL_WIDTH*3)) + (PIXEL_WIDTH  )) +: PIXEL_WIDTH]);
		Y_B[i] <= ( 29 * rgb_i[ (i* (PIXEL_WIDTH*3)) 					+: PIXEL_WIDTH]); 

		Y_ADD[i] <= Y_R[i] + Y_G[i];
		Y_B_STAGE_ADD[i] <= Y_B[i] + 24'd128;
		
		Y_ADD_STAGE2[i] <=  (Y_ADD[i] + Y_B_STAGE_ADD[i]) >> PIXEL_WIDTH ;
		Y[i] = Y_ADD_STAGE2[i];

	end
	
	for (i=0;i<PIXEL_PER_CLK; i = i + 2)	//only alternate U and V needed so loop with +2
	begin

		U_R[i] <= (43 *  rgb_i[((i* (PIXEL_WIDTH*3)) + (PIXEL_WIDTH*2)) +: PIXEL_WIDTH]);
		U_G[i] <= (84  * rgb_i[((i* (PIXEL_WIDTH*3)) + (PIXEL_WIDTH  )) +: PIXEL_WIDTH]);
		U_B[i] <= {		 rgb_i[( i* (PIXEL_WIDTH*3))  					+: PIXEL_WIDTH], 7'b0} - rgb_i[(i*(PIXEL_WIDTH*3)) 					   +: PIXEL_WIDTH]; // B*127 is converted to  val << 7 - val to save dsp * operation

		V_R[i] <= {		 rgb_i[((i* (PIXEL_WIDTH*3)) + (PIXEL_WIDTH*2)) +: PIXEL_WIDTH], 7'b0} - rgb_i[((i* (PIXEL_WIDTH*3)) + (PIXEL_WIDTH*2)) +: PIXEL_WIDTH];
		V_G[i] <= (106 * rgb_i[((i* (PIXEL_WIDTH*3)) + (PIXEL_WIDTH  )) +: PIXEL_WIDTH]);
		V_B[i] <= ( 21 * rgb_i[ (i* (PIXEL_WIDTH*3)) 					+: PIXEL_WIDTH]);

		U_ADD[i] <= U_R[i] + U_G[i];
		V_ADD[i] <= V_B[i] + V_G[i];

		U_B_STAGE_ADD[i] <= U_B[i] + 24'd128;
		V_B_STAGE_ADD[i] <= V_R[i] + 24'd128;

		U_ADD_STAGE2[i] <=  (U_B_STAGE_ADD[i] - U_ADD[i]) >> PIXEL_WIDTH;
		U[i] = U_ADD_STAGE2[i]  + 8'd128;

		V_ADD_STAGE2[i] <=  (V_B_STAGE_ADD[i] - V_ADD[i]) >> PIXEL_WIDTH;
		V[i] = V_ADD_STAGE2[i] + 8'd128;

	end

	for (i=0;i<PIXEL_PER_CLK; i = i + 2)	//only alternate U and V needed
	begin
		yuv_o[( (i*2)    * 8) +: 8] = V[i];
		yuv_o[(((i*2)+1) * 8) +: 8] = Y[i+1];
		yuv_o[(((i*2)+2) * 8) +: 8] = U[i];
		yuv_o[(((i*2)+3) * 8) +: 8] = Y[i];
	end
	//GPIF configed as big endian so expects first bytes at MSbyte
end

endmodule