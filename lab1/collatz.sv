module collatz( input logic         clk,   // Clock
		input logic 	    go,    // Load value from n; start iterating
		input logic  [31:0] n,     // Start value; only read when go = 1
		output logic [31:0] dout,  // Iteration value: true after go = 1
		output logic 	    done); // True when dout reaches 1

   always_ff @(posedge clk) begin
	   if (go) begin 
		   dout <= n;
		   done <= 1'b0;
	   end
	   else if (done);
	   else if (dout == 32'd1)	done <= 1'b1;	
	   else if(dout[0])	dout <= (dout * 3) + 1;
	   else			dout <= dout >> 1;
   end
endmodule
