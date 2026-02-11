// CSEE 4840 Lab 1: Run and Display Collatz Conjecture Iteration Counts
//
// Spring 2023
//
// By: <your name here>
// Uni: <your uni here>

module lab1( input logic        CLOCK_50,  // 50 MHz Clock input
	     
	     input logic [3:0] 	KEY, // Pushbuttons; KEY[0] is rightmost

	     input logic [9:0] 	SW, // Switches; SW[0] is rightmost

	     // 7-segment LED displays; HEX0 is rightmost
	     output logic [6:0] HEX0, HEX1, HEX2, HEX3, HEX4, HEX5,

	     output logic [9:0] LEDR // LEDs above the switches; LED[0] on right
	     );

   logic 			clk, go, done;
   logic [31:0] 		start;
   logic [15:0] 		count;
   logic [21:0] 		counter;

   // Decimal digits (0-9) for display: extract from binary using / and %
   logic [3:0] 		n_ones, n_tens, n_hundreds;   // n = start[11:0]
   logic [3:0] 		c_ones, c_tens, c_hundreds;   // iteration count

   assign clk = CLOCK_50;

   range #(256, 8) // RAM_WORDS = 256, RAM_ADDR_BITS = 8)
         r ( .* ); // Connect everything with matching names

   // Extract decimal digits: ones = val % 10, tens = (val/10) % 10, hundreds = (val/100) % 10

      logic [11:0] start_display;

      assign start_display = SW[11:0];
      
   always_comb begin
      n_ones    = start_display % 10;
      n_tens    = (start_display / 10) % 10;
      n_hundreds = (start_display / 100) % 10;
      assign c_ones    = count % 10;
      assign c_tens    = (count / 10) % 10;
      assign c_hundreds = (count / 100) % 10;
   end

   // Left 3 digits (HEX5, HEX4, HEX3): n in decimal
   hex7seg seg5 (.a(n_hundreds), .y(HEX5));
   hex7seg seg4 (.a(n_tens),     .y(HEX4));
   hex7seg seg3 (.a(n_ones),     .y(HEX3));
   // Right 3 digits (HEX2, HEX1, HEX0): iteration count in decimal
   hex7seg seg2 (.a(c_hundreds), .y(HEX2));
   hex7seg seg1 (.a(c_tens),     .y(HEX1));
   hex7seg seg0 (.a(c_ones),     .y(HEX0));

   assign LEDR = SW;

   assign go = KEY[3];

   always_ff @(posedge clk) begin
      if (KEY[3]) begin
         start <= SW;
         go <= 1;
      end
      else begin
            go <= 0;
            if (done) begin
                  start <= 0;
            end
            // start <= 0;
            // counter <= counter + 1;
            // if (counter == 22'd1000000) begin
            //       counter <= 0;
            // end
            // if (KEY[0] && start < 255) begin
            //       start <= start + 1;
            // end
            // else if (KEY[1] && start > 0) begin
            //       start <= start - 1;
            // end
            // else if (KEY[2]) begin
            //       start <= 0;
            // end
      end
   end
  
endmodule
