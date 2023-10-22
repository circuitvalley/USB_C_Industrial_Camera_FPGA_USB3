//this module handles basic camera functions, such enabling regulators on the camera board, setting camera reset

module camera_controller(sclk_i, //a basic low freqency slow clock, should not be comming from MIPI block/Camera 
						reset_i, //global reset
						cam_ctrl_in, //control camera control input from host
						cam_xce_o,
						cam_pwr_en_o, //enable camera power 
						cam_reset_o,  //camera reset to camera
						cam_xmaster_o //camera master or slave 
						);
input sclk_i;
input reset_i;
input cam_ctrl_in;
output reg cam_pwr_en_o;
output reg cam_reset_o;
output reg cam_xmaster_o;
output reg cam_xce_o;

reg [1:0]camera_state;
parameter state_reset = 2'h0;
parameter state_power_on = 2'h1;
parameter state_active = 2'h2;
parameter state_idle = 2'h3;

parameter delay_bewteen_state = 16'd1280; //around 10ms //on Crosslink nx slow internal oscillator is around 128Khz



reg [15:0]state_time_counter;	

always @(posedge sclk_i or posedge reset_i)
begin
	if (reset_i || !cam_ctrl_in)
	begin
		state_time_counter <= delay_bewteen_state;
		camera_state <= state_reset;
		cam_pwr_en_o <= 1'b0;
		cam_reset_o <= 1'b0;
		cam_xmaster_o <= 1'b0;
		cam_xce_o <= 1'b1;
	end
	else
	begin

		state_time_counter <= state_time_counter - 1'b1;
		
		if (state_time_counter == 0)
		begin
			camera_state <= camera_state + (camera_state != state_idle); //go to next state if state is not equal to state_idle
			
			case(camera_state)
			state_reset:
			begin
				cam_pwr_en_o <= 1'b0;
				cam_reset_o <= 1'b0;
				cam_xmaster_o <= 1'b0;
				cam_xce_o <= 1'b1;
				state_time_counter <= delay_bewteen_state;
			end
			state_power_on:
			begin
				cam_pwr_en_o <= 1'b1;
				cam_reset_o <= 1'b0;
				cam_xmaster_o <= 1'b0;
				state_time_counter <= delay_bewteen_state;
			end
			state_active:
			begin
				cam_pwr_en_o <= 1'b1;
				cam_reset_o <= 1'b1;
				cam_xmaster_o <= 1'b0;		
				state_time_counter <= delay_bewteen_state;
				
			end
			state_idle:
			begin
				cam_pwr_en_o <= 1'b1;
				cam_reset_o <= 1'b1;
				cam_xmaster_o <= 1'b0;							
			end
			default:
			begin
				cam_pwr_en_o <= 1'b0;
				cam_reset_o <= 1'b0;
				cam_xmaster_o <= 1'b0;
			end
			endcase			
		end	
			
	end
end

endmodule