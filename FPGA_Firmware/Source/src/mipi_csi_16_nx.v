`timescale 1ns/1ns

/*
MIPI CSI RX to Parallel Bridge (c) by Gaurav Singh www.CircuitValley.com

MIPI CSI RX to Parallel Bridge is licensed under a
Creative Commons Attribution 3.0 Unported License.

You should have received a copy of the license along with this
work.  If not, see <http://creativecommons.org/licenses/by/3.0/>.
*/

/* 
MIPI CSI upto 4 Lane Receiver To Parallel Bridge 
Tested with Lattice MachXO3LF-6900 with IMX219 Camera  and also with Crosslink NX with IMX290 IMX462 IMX219 IMX477
Takes MIPI Clock and 4 Data lane as input convert into Parallel YUV output 
Ouputs 32bit YUV data with Frame sync, lsync and pixel clock 
*/

module mipi_csi_16_nx(	//reset_in,
						mipi_clk_p_in,
						mipi_clk_n_in,
						mipi_data_p_in,
						mipi_data_n_in,


						pclk_o,  //data output on pos edge , should be latching into receiver on negedge
						data_o,
						fsync_o, //active high 
						lsync_o, //active high
						
						//these pins may or many not be needed depeding on hardware config
						//cam_ctrl_in, //control camera control input from host
						cam_xce_o, //camera interface selection 
						cam_pwr_en_o, //enable camera power 
						cam_reset_o,  //camera reset to camera
						cam_xmaster_o //camera master or slave 
						);
					
parameter MIPI_LANES = 2;			//number of mipi lanes with camera. Only 2 or 4
parameter MIPI_GEAR = 8;			//deserializer gearing ratio. Only 8 or 16 
parameter MIPI_PIXEL_PER_CLOCK = 2; //number of pixels pipeline process in one clock cycle. With 2 Lanes and Gear 8 only 2 or 4 with gear 16 only 4 , With 4 Lanes only 4 or 8 
parameter MAX_PIXEL_WIDTH = 12;   	//max pixel width , 14bit (RAW14) , IMX219 has 10bit while IMX477 has 12bit and IMX294 has 14bit
parameter FRAME_DETECT = 0; 		//if 1 MIPI start frame packet will be detected and used as frame sync rather than mipi_clk_lp , used for sensor whoes clock does not go into lp while frame sync is active

//this should never be active unless testing 
parameter SAMPLE_GENERATOR = 0;		//if 1 a ROM based sample generator will be activated for testing, Sample generator uses 2 ROM lines as that is what mipimum needed to an image with correct enough colors, ROM file are rom_first.rom and rom_second.rom files are in source dir

wire reset_in;
wire cam_ctrl_in;

input mipi_clk_p_in;
input mipi_clk_n_in;
input [MIPI_LANES-1:0]mipi_data_p_in;
input [MIPI_LANES-1:0]mipi_data_n_in;


output cam_xce_o;
output cam_pwr_en_o;
output cam_reset_o;
output cam_xmaster_o;

output pclk_o;
output [31:0]data_o;
output fsync_o;
output lsync_o;

wire osc_clk;

wire output_clock; 
wire mipi_byte_clock; //byte clock from mipi phy


wire [MIPI_LANES-1:0]lp_rx_data_p;
wire lp_rx_clk_p;
wire [MIPI_LANES-1:0]is_byte_valid;
wire is_lane_aligned_valid;
wire is_decoded_valid;
wire is_raw_line_valid;
wire is_unpacked_valid;
wire is_rgb_valid;
wire is_rgb_corrected_valid;
wire is_yuv_valid;
wire is_yuv_line_valid;

wire mipi_out_clk;
wire [((MIPI_LANES * MIPI_GEAR) - 1) :0]mipi_data_raw_hw;    
wire [((MIPI_LANES * MIPI_GEAR) - 1) :0]mipi_data_raw;    


wire [((MIPI_LANES * MIPI_GEAR) - 1) :0]byte_aligned;
wire [((MIPI_LANES * MIPI_GEAR) - 1) :0]lane_aligned;
wire [((MIPI_LANES * MIPI_GEAR) - 1) :0]decoded_data;
wire [2:0]packet_type;
wire [15:0]packet_length;
wire [((MAX_PIXEL_WIDTH * MIPI_PIXEL_PER_CLOCK	  ) - 1 ):0]unpacked_data;
wire [((MAX_PIXEL_WIDTH * MIPI_PIXEL_PER_CLOCK * 3) - 1 ):0]rgb_data;
wire [((MAX_PIXEL_WIDTH * MIPI_PIXEL_PER_CLOCK * 3) - 1 ):0]rgb_corrected;
wire [((MIPI_PIXEL_PER_CLOCK * 8 * 2 	          ) - 1 ):0]yuv_data;

wire line_reset;
wire frame_sync;
wire [1:0] sync_pulse;

int_osc int_osc_ins1(.hf_out_en_i(1'b1), 
					 .hf_clk_out_o(mipi_out_clk), 
					 .lf_clk_out_o(osc_clk));




//Lattice FPGA Engineering sample chip does not allow manual PHY pin assignment* for PHY and our hardware uses PHY1 rather than PHY0, this instance with go to PHY0 while our actual instance will go to PHY 1
//*Lattice Does allow manual pin assignment but when you assign manually PHY will not work at all. Appears to be ES chip bug.
//As i am using Lattice ES chips i need to use Radiant 2.0 , DPHY IP has been changed a little with Radiant 3.1
// If using ES Devices a dummy dphy_ip need to inferred as a place holder
/*
dphy_dummy mipi_csi_phy_inst0(.sync_clk_i(osc_clk), 
							.sync_rst_i(1'b1), 
							.lmmi_clk_i(1'b0), 
							.lmmi_resetn_i(1'b0), 
							.lmmi_wdata_i(4'b0), 
							.lmmi_wr_rdn_i(1'b0), 
							.lmmi_offset_i(5'b0), 
							.lmmi_request_i(1'b0), 
							.lmmi_ready_o(), 
							.lmmi_rdata_o(), 
							.lmmi_rdata_valid_o(), 
							.hs_rx_en_i(1'b1), 
							//.hs_rx_clk_en_i(1'b0),  
							//.hs_rx_data_en_i(1'b0), 
							//.hs_data_des_en_i(1'b0), 
							.hs_rx_data_o(dummy_out), 
							//.hs_rx_data_sy nc_o(), 
							.lp_rx_en_i(1'b0), 
							.lp_rx_data_p_o(), 
							.lp_rx_data_n_o(), 
							.lp_rx_clk_p_o(), 
							.lp_rx_clk_n_o(), 
							.pll_lock_i(1'b0), 
							.clk_p_io(mipi_clk_p_in1), 
							.clk_n_io(mipi_clk_n_in1), 
							.data_p_io(mipi_data_p_in1), 
							.data_n_io(mipi_data_n_in1), 
							.pd_dphy_i(1'b0), 
							.clk_byte_o(), 
							.ready_o()) ;
*/

csi_dphy  mipi_csi_phy_inst1(.sync_clk_i(osc_clk), 
							.sync_rst_i(1'b0), 
							.lmmi_clk_i(1'b0), 
							.lmmi_resetn_i(1'b0), 
							.lmmi_wdata_i(4'b0), 
							.lmmi_wr_rdn_i(1'b0), 
							.lmmi_offset_i(5'b0), 
							.lmmi_request_i(1'b0), 
							.lmmi_ready_o(), 
							.lmmi_rdata_o(), 
							.lmmi_rdata_valid_o(), 
							//.hs_rx_en_i(1'b1), 
							.hs_rx_clk_en_i(1'b1),  //new
							.hs_rx_data_en_i(1'b1), //new
							.hs_data_des_en_i(1'b1), //new
							.hs_rx_data_o(mipi_data_raw_hw), 
							.hs_rx_data_sync_o(sync_pulse), 
							.lp_rx_en_i(1'b1), 
							.lp_rx_data_p_o(lp_rx_data_p), 
							.lp_rx_data_n_o(), 
							.lp_rx_clk_p_o(lp_rx_clk_p), 
							.lp_rx_clk_n_o(), 
							.pll_lock_i(1'b1), 
							.clk_p_io(mipi_clk_p_in), 
							.clk_n_io(mipi_clk_n_in), 
							.data_p_io(mipi_data_p_in), 
							.data_n_io(mipi_data_n_in), 
							.pd_dphy_i(1'b0), 
							.clk_byte_o(mipi_byte_clock), 
							.ready_o()) ;

/*
generate  //if hardware mipi lane n does not match to FPGA PHY lane n then adjust here because Lattice ES chip does not allow manual pin assigment  
	if ( (MIPI_GEAR == 16) && (MIPI_LANES == 4))
	begin
		assign mipi_data_raw = {mipi_data_raw_hw[15:0], mipi_data_raw_hw[31:16], mipi_data_raw_hw[63:48] ,mipi_data_raw_hw[47:32]}; //If schematic lane 0 may not connected PHY lane 0 , May need to swap here because its not possible to do before PHY IP, because Radiant auto assign PHY ports
	end
	else if ( (MIPI_GEAR == 16) && (MIPI_LANES == 2))
	begin
		assign mipi_data_raw = {mipi_data_raw_hw[31:16], mipi_data_raw_hw[15:0]}; 
	end
	else if ( (MIPI_GEAR == 8) && (MIPI_LANES == 4))
	begin
		assign mipi_data_raw = {mipi_data_raw_hw[7:0], mipi_data_raw_hw[15:8], mipi_data_raw_hw[31:24] ,mipi_data_raw_hw[23:16]}; 
	end
	else if ( (MIPI_GEAR == 8) && (MIPI_LANES == 2))
	begin
		assign mipi_data_raw = mipi_data_raw_hw;
	end
endgenerate */

assign mipi_data_raw = mipi_data_raw_hw;


camera_controller camera_controller_ins0(	.sclk_i(osc_clk),
											.reset_i(reset_in),
											.cam_ctrl_in(cam_ctrl_in), 
											.cam_xce_o(cam_xce_o),
											.cam_pwr_en_o(cam_pwr_en_o), 
											.cam_reset_o(cam_reset_o),  
											.cam_xmaster_o(cam_xmaster_o)
										);



line_reset_generator line_reset_generator_ins0(.clk_i(mipi_byte_clock),
											   .lp_data_i(lp_rx_data_p[0]),
											   .line_reset_o(line_reset));



generate 
genvar i;

	if (SAMPLE_GENERATOR)
	begin
		wire dummy_byte_valid;	//sample generator should never be active unless debugging 
		assign is_byte_valid = {dummy_byte_valid,dummy_byte_valid};			
		sample_generator sample_generator_ins( .framesync_i(frame_sync),
												.clk_i(mipi_byte_clock),
											   .reset_i(line_reset),
											   .byte_o(byte_aligned),
											   .byte_valid_o(dummy_byte_valid));
		
	end
	else
	begin
		for (i = 0;i < MIPI_LANES; i = i +1) 
		begin
			
			mipi_csi_rx_byte_aligner #(.MIPI_GEAR(MIPI_GEAR))mipi_rx_byte_aligner_0(.clk_i(mipi_byte_clock),
																				 .reset_i(line_reset),
																				 .byte_i(mipi_data_raw[(MIPI_GEAR * i) +: MIPI_GEAR]),
																				 .byte_o( byte_aligned[(MIPI_GEAR * i) +: MIPI_GEAR]),
																				 .byte_valid_o(is_byte_valid[i]));
		end
		
	end
endgenerate 


generate 

if (FRAME_DETECT)
begin
	frame_detector #(.MIPI_GEAR(MIPI_GEAR)) frame_detector_ins0( 	.reset_i(reset_in),
																	.clk_i(mipi_byte_clock),
																	.data_valid_i(is_byte_valid[0]),
																	.data_lane0_i(byte_aligned[0 +: MIPI_GEAR]),
																	.detected_frame_sync_o(frame_sync));
	
end
else
begin
	assign frame_sync = lp_rx_clk_p;
end

endgenerate

				
									   
mipi_csi_rx_lane_aligner #(	.MIPI_GEAR(MIPI_GEAR), 
							.MIPI_LANES(MIPI_LANES))
	   mipi_rx_lane_aligner(.clk_i(mipi_byte_clock),
							.reset_i(line_reset),
							.bytes_valid_i(is_byte_valid),
							.byte_i(byte_aligned),
							.lane_valid_o(is_lane_aligned_valid),
							.lane_byte_o(lane_aligned));
											   




generate 
	if ( (MIPI_GEAR == 16) && (MIPI_LANES == 4))
	begin
		mipi_csi_rx_packet_decoder_16b4lane mipi_csi_packet_decoder_0(	.clk_i(mipi_byte_clock),
																		.data_valid_i(is_lane_aligned_valid),
																		.data_i(lane_aligned),
																		.output_valid_o(is_decoded_valid),
																		.data_o(decoded_data),
																		.packet_length_o(),
																		.packet_type_o(packet_type));
															
		mipi_csi_rx_raw_depacker_16b4lane #(.PIXEL_WIDTH(MAX_PIXEL_WIDTH)) mipi_csi_rx_raw_depacker_0(	.clk_i(mipi_byte_clock),
																										.data_valid_i(is_decoded_valid),
																										.data_i(decoded_data),
																										.packet_type_i(packet_type),
																										.output_o(unpacked_data),
																										.output_valid_o(is_unpacked_valid),
																										.raw_line_o(is_raw_line_valid));	
														
	end
	else if ( (MIPI_GEAR == 16) && (MIPI_LANES == 2))
	begin
		mipi_csi_rx_packet_decoder_16b2lane mipi_csi_packet_decoder_0(	.clk_i(mipi_byte_clock),
																		.data_valid_i(is_lane_aligned_valid),
																		.data_i(lane_aligned),
																		.output_valid_o(is_decoded_valid),
																		.data_o(decoded_data),
																		.packet_length_o(),
																		.packet_type_o(packet_type));
																		
																		

		mipi_csi_rx_raw_depacker_16b2lane #(.PIXEL_WIDTH(MAX_PIXEL_WIDTH)) mipi_csi_rx_raw_depacker_0(	.clk_i(mipi_byte_clock),
																										.data_valid_i(is_decoded_valid),
																										.data_i(decoded_data),
																										.packet_type_i(packet_type),
																										.output_o(unpacked_data),
																										.output_valid_o(is_unpacked_valid),
																										.raw_line_o(is_raw_line_valid));
	end
	else if ( (MIPI_GEAR == 8) && (MIPI_LANES == 4))	//only 4 or 8 PPC
	begin
		
		mipi_csi_rx_packet_decoder_8b4lane mipi_csi_packet_decoder_0(	.clk_i(mipi_byte_clock),
																		.data_valid_i(is_lane_aligned_valid),
																		.data_i(lane_aligned),
																		.output_valid_o(is_decoded_valid),
																		.data_o(decoded_data),
																		.packet_length_o(),
																		.packet_type_o(packet_type));
		if (( MIPI_PIXEL_PER_CLOCK == 4) )
		begin															
		mipi_csi_rx_raw_depacker_8b4lane #(.PIXEL_WIDTH(MAX_PIXEL_WIDTH)) mipi_csi_rx_raw_depacker_0(	.clk_i(mipi_byte_clock),
																										.data_valid_i(is_decoded_valid),
																										.data_i(decoded_data),
																										.packet_type_i(packet_type),
																										.output_o(unpacked_data),
																										.output_valid_o(is_unpacked_valid),
																										.raw_line_o(is_raw_line_valid));
		end
		else if(( MIPI_PIXEL_PER_CLOCK == 8) )
		begin
		mipi_csi_rx_raw_depacker_8b4lane_8ppc #(.PIXEL_WIDTH(MAX_PIXEL_WIDTH)) mipi_csi_rx_raw_depacker_0(	.clk_i(mipi_byte_clock),
																											.data_valid_i(is_decoded_valid),
																											.data_i(decoded_data),
																											.packet_type_i(packet_type),
																											.output_o(unpacked_data),
																											.output_valid_o(is_unpacked_valid),
																											.raw_line_o(is_raw_line_valid));			
		end
	end

	else if ( (MIPI_GEAR == 8) && (MIPI_LANES == 2)) //only 2 or 4 PPC
	begin
		
		mipi_csi_rx_packet_decoder_8b2lane  mipi_csi_packet_decoder_0(	.clk_i(mipi_byte_clock),
																	.data_valid_i(is_lane_aligned_valid),
																	.data_i(lane_aligned),
																	.output_valid_o(is_decoded_valid),
																	.data_o(decoded_data),
																	.packet_length_o(),
																	.packet_type_o(packet_type));


		if (( MIPI_PIXEL_PER_CLOCK == 4) )
		begin
			mipi_csi_rx_raw_depacker_8b2lane #(.PIXEL_WIDTH(MAX_PIXEL_WIDTH)) mipi_csi_rx_raw_depacker_0(	.clk_i(mipi_byte_clock),
																											.data_valid_i(is_decoded_valid),
																											.data_i(decoded_data),
																											.packet_type_i(packet_type),
																											.output_o(unpacked_data),
																											.output_valid_o(is_unpacked_valid),
																											.raw_line_o(is_raw_line_valid));	
		end
		else if (( MIPI_PIXEL_PER_CLOCK == 2) )
		begin
			mipi_csi_rx_raw_depacker_8b2lane_2ppc #(.PIXEL_WIDTH(MAX_PIXEL_WIDTH)) mipi_csi_rx_raw_depacker_0(	.clk_i(mipi_byte_clock),
																			.data_valid_i(is_decoded_valid),
																			.data_i(decoded_data),
																			.packet_type_i(packet_type),
																			.output_o(unpacked_data),
																			.output_valid_o(is_unpacked_valid),
																			.raw_line_o(is_raw_line_valid));
		end
			

														
	end

endgenerate 



debayer_filter #(.PIXEL_WIDTH(MAX_PIXEL_WIDTH), .PIXEL_PER_CLK(MIPI_PIXEL_PER_CLOCK))debayer_filter_0(	 .clk_i(mipi_byte_clock),
																										 .reset_i(frame_sync),
																										 .line_valid_i(is_raw_line_valid),
																										 .data_i(unpacked_data),
																										 .data_valid_i(is_unpacked_valid),
																										 .output_o(rgb_data),
																										 .output_valid_o(is_rgb_valid));
/*
color_correction_matrix #(.PIXEL_WIDTH(MAX_PIXEL_WIDTH), .PIXEL_PER_CLK(MIPI_PIXEL_PER_CLOCK))color_correction_matrix_0(	 
																										 .clk_i(mipi_byte_clock),
																										 .reset_i(frame_sync),
																										 .line_valid_i(is_raw_line_valid),
																										 .data_i(rgb_data),
																										 .data_valid_i(is_rgb_valid),
																										 .output_o(rgb_corrected),
																										 .output_valid_o(is_rgb_corrected_valid));


rgb_to_yuv #(.PIXEL_WIDTH(MAX_PIXEL_WIDTH), .PIXEL_PER_CLK(MIPI_PIXEL_PER_CLOCK))rgb_to_yuv_0(	.clk_i(mipi_byte_clock),
																								.rgb_i(rgb_corrected),
																								.rgb_valid_i(is_rgb_corrected_valid),
																								.line_valid_i(is_raw_line_valid),
																								.yuv_o(yuv_data),
																								.yuv_valid_o(is_yuv_valid),
																								.yuv_line_o(yuv_line_valid));
*/	

rgb_to_yuv #(.PIXEL_WIDTH(MAX_PIXEL_WIDTH), .PIXEL_PER_CLK(MIPI_PIXEL_PER_CLOCK))rgb_to_yuv_0(	.clk_i(mipi_byte_clock),
																								.rgb_i(rgb_data),
																								.rgb_valid_i(is_rgb_valid),
																								.line_valid_i(is_raw_line_valid),
																								.yuv_o(yuv_data),
																								.yuv_valid_o(is_yuv_valid),
																								.yuv_line_o(yuv_line_valid));
																							
//wire line_wire;
output_reformatter #(.PIXEL_PER_CLK(MIPI_PIXEL_PER_CLOCK))out_reformatter_0( .clk_i(mipi_byte_clock),
																			 .line_sync_i(yuv_line_valid),
																			 .frame_sync_i(frame_sync),
																			 .output_clk_i(mipi_out_clk),
																			 .data_i(yuv_data),
																			 .data_in_valid_i(is_yuv_valid),
																			 .output_o(data_o),
																			 .output_valid_o(lsync_o));

assign reset_in = 1'b0;
assign cam_ctrl_in = 1'b1;
assign pclk_o = mipi_out_clk; 	//output clock always available
assign fsync_o = !frame_sync;	 //activate fsync Active high
//assign lsync_o = is_yuv_valid;
endmodule