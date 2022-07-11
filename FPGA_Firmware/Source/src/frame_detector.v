module frame_detector #(parameter MIPI_GEAR=8)( reset_i,
												clk_i,
												data_valid_i,
												data_lane0_i,
												detected_frame_sync_o
												);
				
localparam [7:0]SYNC_BYTE = 8'hB8;
localparam [7:0]MIPI_CSI_START = 8'h00;
localparam [7:0]MIPI_CSI_STOP = 8'h01;

input reset_i;
input clk_i;
input data_valid_i;
input [(MIPI_GEAR - 1'h1):0]data_lane0_i;
output reg detected_frame_sync_o;

reg [(MIPI_GEAR - 1'h1):0]last_data_lane0_i;
reg [1:0]packed_processed;
wire [((MIPI_GEAR*2) - 1'h1):0]pipe;

assign pipe = {data_lane0_i,last_data_lane0_i};
//packet format <SYNC_BYTE> <DataID> <WCount 8bit> <WCount8bit> <ECC8bit>
always @(posedge clk_i)
begin
	if (reset_i)
	begin
		last_data_lane0_i <= 0;
		detected_frame_sync_o <=0;
		packed_processed <= 0;
	end
	else
	begin
			last_data_lane0_i  <= data_lane0_i;

			if (data_valid_i)
			begin
				packed_processed[0] <= 1'b1;		//only check first two bytes/words having two bits variable allow bascially a counter till 2 
				packed_processed[1] <= packed_processed[0];

				if ( !packed_processed[1])
				begin
				
					if (pipe[7:0] == SYNC_BYTE && pipe[15:8] == MIPI_CSI_START)
					begin
						detected_frame_sync_o <= 1'b0;		//active low
					end
					else if (pipe[7:0] == SYNC_BYTE && pipe[15:8] == MIPI_CSI_STOP)
					begin
						detected_frame_sync_o <= 1'b1;
					end
				end
			end
			else
			begin
				packed_processed <= 2'b0;
			end
	end
end


endmodule