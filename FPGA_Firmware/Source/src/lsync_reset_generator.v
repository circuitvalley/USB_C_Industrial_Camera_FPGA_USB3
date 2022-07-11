//MIPI Data line HS to LP transition can cause gragage data into bus,
//This moduel holds reset a little longer before stable data comes throw 

module line_reset_generator(clk_i, lp_data_i, line_reset_o);
input clk_i;
input lp_data_i;
output line_reset_o;

reg [10:0]shift_reg;
always @(posedge clk_i)
begin
	shift_reg <= {shift_reg[9:0] , lp_data_i};
end

assign line_reset_o = shift_reg[10];

endmodule