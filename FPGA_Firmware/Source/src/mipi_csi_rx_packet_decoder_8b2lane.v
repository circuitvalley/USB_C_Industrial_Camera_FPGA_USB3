`timescale 1ns/1ns
/*
MIPI CSI RX to Parallel Bridge (c) by Gaurav Singh www.CircuitValley.com

MIPI CSI RX to Parallel Bridge is licensed under a
Creative Commons Attribution 3.0 Unported License.

You should have received a copy of the license along with this
work.  If not, see <http://creativecommons.org/licenses/by/3.0/>.
*/

/*
Basically a packet Stripper, removes header and footer from packet 
Takes lane aligned data from lane aligner @ mipi byte clock
looks for specific packet types, in this case RAW10bit 0x2B RAW12bit 0x2C and RAW14bit 0x2D , Packet type is also output to be used in next modules
outputs Stripped bytes in exactly the way they were received.
this module also fetch packet length and output_valid is active as long as input data is valid and packet length is still valid.

V1.1 Sep 2020 Timing optimizations 
*/

module mipi_csi_rx_packet_decoder_8b2lane(
								clk_i,
								data_valid_i,
								data_i,
								output_valid_o,
								data_o,
								packet_length_o,
								packet_type_o
								);
localparam [7:0]MIPI_GEAR = 8;
localparam [3:0]LANES = 3'h2;

localparam [7:0]SYNC_BYTE = 8'hB8;
localparam [7:0]MIPI_CSI_PACKET_10bRAW = 8'h2B;
localparam [7:0]MIPI_CSI_PACKET_12bRAW = 8'h2C;
localparam [7:0]MIPI_CSI_PACKET_14bRAW = 8'h2D;

input clk_i;
input data_valid_i;
input 		[((MIPI_GEAR * LANES) - 1'h1) : 0]data_i;
output reg 	[((MIPI_GEAR * LANES) - 1'h1) : 0]data_o;
output reg output_valid_o;
output reg [15:0]packet_length_o;
output reg [2:0]packet_type_o;

reg output_valid_reg;

reg [15:0]packet_length_reg;
reg [((MIPI_GEAR * LANES) - 1'h1) :0]data_reg;



//packet format <SYNC_BYTE> <DataID> <WCount 8bit> <WCount8bit> <ECC8bit>
always @(posedge clk_i)
begin
	if (data_valid_i)
	begin
		output_valid_reg <= |packet_length_reg;
		output_valid_o <= output_valid_reg;
		
		if (packet_length_reg >= (LANES))
		begin
			packet_length_reg <= packet_length_reg - (LANES);
		end
		else if (data_o[7:0] == SYNC_BYTE && (data_reg[7:0] == MIPI_CSI_PACKET_10bRAW || data_reg[7:0] == MIPI_CSI_PACKET_12bRAW || data_reg[7:0] == MIPI_CSI_PACKET_14bRAW))
		begin
			packet_type_o     <= data_reg[2:0];
			packet_length_o   <=  {data_i[7:0], data_reg[15:8]};
			packet_length_reg <=  {data_i[7:0], data_reg[15:8]};
		end
		else
		begin
			packet_length_reg <= 15'h0;
			packet_type_o     <=  3'h0;
			packet_length_o   <= 15'h0;
		end
	end
	else 
	begin
		packet_type_o <= 3'h0;
		packet_length_o <= 15'h0;
		packet_length_reg <= 15'h0;
		output_valid_o <= 1'h0;
	end
end

always @(posedge clk_i)
begin
		data_reg <= data_i;
		data_o   <= data_reg;
end

endmodule
