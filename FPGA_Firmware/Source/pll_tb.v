
`timescale 1ns/1ns

module pll_tb;
	
	wire clk_o;
	wire clks_o;
	wire lock;
wire reset_g;
GSR 
GSR_INST (
	.GSR_N(1'b1),
	.CLK(1'b0)
);

wire hf_clk;
int_osc int_osc_ins1(.hf_out_en_i(1'b1), 
					 .hf_clk_out_o(hf_clk), 
					 .lf_clk_out_o(osc_clk));


out_pll inst1(	.clki_i(hf_clk),
						.rstn_i(1'b1),
						.clkop_o(clk_o),
						.clkos_o(clks_o),
						.lock_o(lock));
					



endmodule