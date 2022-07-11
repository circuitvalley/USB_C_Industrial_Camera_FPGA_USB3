`timescale 1ns/1ns
//64bit pipeline 
module tb_frame_sync_generator;
	reg clk;
	reg clk_lp;
	wire frame_sync_output;
	
GSR 
GSR_INST (
	.GSR_N(1'b1),
	.CLK(1'b0)
);

frame_sync_generator ins1(.lp_clk_i(clk_lp),
						  .out_clk_i(clk),
						  .frame_sync_o(frame_sync_output));
							

initial begin
	clk = 1'b0;
end
always begin
	#5 clk =  ~clk;
end


task send_frame;
	reg [16:0]i;
	begin
		for(i = 0; i< 2000; i = i+1) begin
				if (i > 1000)
				begin
					clk_lp = 1;
				end
				#2;

		end 
	end	
endtask

initial begin
		
		clk_lp = 0;
	
		send_frame();
		clk_lp = 0;
		#1000;
		send_frame();
		clk_lp = 0;
		#1000;
		send_frame();
end

endmodule