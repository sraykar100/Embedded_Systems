// CSEE 4840 Lab 1: Run and Display Collatz Conjecture Iteration Counts
//
// Spring 2026
//
// By: Siddharth Raykar & Shiyao Marcus Lam
// Uni: sr4102 & sml2286

module lab1( input logic        CLOCK_50,  // 50 MHz Clock input
	     
	     input logic [3:0] 	KEY, // Pushbuttons; KEY[0] is rightmost

	     input logic [9:0] 	SW, // Switches; SW[0] is rightmost

	     // 7-segment LED displays; HEX0 is rightmost
	     output logic [6:0] HEX0, HEX1, HEX2, HEX3, HEX4, HEX5,

	     output logic [9:0] LEDR // LEDs above the switches; LED[0] on right
	     );

   logic 			clk, done;
   logic [31:0] 		start;
   logic [11:0] 		count;
   logic [3:0] 			unused;

   assign clk = CLOCK_50;
   logic add; // This is tied to Key[0] via a debouncer
   logic sub; // This is tied to Key[1] via a debouncer
   logic reset; // This is tied to Key[2] via a debouncer, and resets the display so that the collatz number of SW is shown. 
   logic go; // This is tied to Key[3] via a debouncer, and starts the range module from the initial value on SW. 

   debouncer add_debounce(clk, ~KEY[0], add);
   debouncer sub_debounce(clk, ~KEY[1], sub); 
   debouncer reset_debounce(clk, ~KEY[2], reset);
   debouncer go_debounce(clk, ~KEY[3], go);

   // Typematic style for add/sub buttons. (hold vs. single press)
   logic add_pulse, sub_pulse;
   button_press add_ctrl(.clk(clk), .btn(add), .pulse(add_pulse));
   button_press sub_ctrl(.clk(clk), .btn(sub), .pulse(sub_pulse));

   range #(256, 8) // RAM_WORDS = 256, RAM_ADDR_BITS = 8)
         r (
		 .clk(clk), 
		 .go(go), 
		 .start(start), 
		 .done(done), 
		 .count({unused,count})
	 ); 

      logic [11:0] start_display; // Value whose collatz number is shown on the display. 
      logic [7:0] start_index; // Start index is what's changed upon increment/decrement

      assign start_display = {2'b0, SW} + {4'b0, start_index};
   
   // Left 3 digits: n in decimal
   hex7seg seg5 (.a(start_display[11:8]), .y(HEX5));
   hex7seg seg4 (.a(start_display[7:4]),     .y(HEX4));
   hex7seg seg3 (.a(start_display[3:0]),     .y(HEX3));
   // Right 3 digits: iteration count in decimal
   hex7seg seg2 (.a(count[11:8]), .y(HEX2));
   hex7seg seg1 (.a(count[7:4]),     .y(HEX1));
   hex7seg seg0 (.a(count[3:0]),     .y(HEX0));
	
   logic ready;
   assign LEDR[8:0]= SW[8:0];
   assign LEDR[9] = SW[9] ^ (^unused & 1'b0);
   	
   always_ff @(posedge clk) begin
	// always update display values 
	// Extract decimal digits: ones = val % 10, tens = (val/10) % 10, hundreds = (val/100) % 10
      if (go) begin
	      start <= {22'b0, SW};
	      ready <= 0;
	      start_index <= 0;
      end
      if (done) begin
             start <= 0;
	     ready <= 1;
     end
     if (ready) begin
		if (add_pulse && start < 255) begin
			start <= start + 1;
			start_index <= start_index + 1;
		end
		else if (sub_pulse && start > 0) begin
			start <= start - 1;
			start_index <= start_index - 1;
		end
		else if (reset) begin
			start <= 0;
			start_index <= 0;
		end
     end
   end
endmodule
