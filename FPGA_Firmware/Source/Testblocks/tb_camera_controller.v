`timescale 1ns/1ns

module tb_camera_controller;
	reg clk;
	reg reset;
	
	reg ctrl_in;
GSR 
GSR_INST (
	.GSR_N(1'b1),
	.CLK(1'b0)
);

always #1 clk = !clk;
	
camera_controller ins0( .sclk_i(clk), //a basic low freqency slow clock, should not be comming from MIPI block/Camera 
						.reset_i(reset), //global reset
						.cam_ctrl_in(ctrl_in), //control camera control input from host
						.cam_pwr_en_o(), //enable camera power 
						.cam_reset_o(),  //camera reset to camera
						.cam_xmaster_o() //camera master or slave 
						);



initial begin
		clk =0;
		ctrl_in=0;
		reset = 1'b1;
		#30;
		reset = 1'b0;
		#140000;
		ctrl_in = 1'b1;
		#1400000;
		ctrl_in = 1'b0;
		#140000;
		ctrl_in = 1'b1;
		#140000;
		reset = 1'b1;
		#500;
		$stop;

end

endmodule
