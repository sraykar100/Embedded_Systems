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

   logic 			clk, done;
   logic 			go;                    // one-cycle pulse to start range
   logic 			key3_prev = 1'b1;      // for KEY[3] edge detect
   logic 			go_next;               // go pulse one cycle after latching start
   logic [31:0] 		start;                 // latched n: display + range read addr (keep after run)
   logic [15:0] 		count;
   logic [21:0] 		counter;

   // CUSTOM: Decimal digits (0-9) for display: extract from binary using / and %
   logic [3:0] 		n_ones, n_tens, n_hundreds;   // n = start[11:0]
   logic [3:0] 		c_ones, c_tens, c_hundreds;   // iteration count

   assign clk = CLOCK_50;

   range #(256, 8) // RAM_WORDS = 256, RAM_ADDR_BITS = 8)
         r ( .* ); // Connect everything with matching names

   // Extract decimal digits: ones = val % 10, tens = (val/10) % 10, hundreds = (val/100) % 10
   always_comb begin
      n_ones    = start[11:0] % 10;
      n_tens    = (start[11:0] / 10) % 10;
      n_hundreds = (start[11:0] / 100) % 10;
      c_ones    = count % 10;
      c_tens    = (count / 10) % 10;
      c_hundreds = (count / 100) % 10;
   end

   // Left 3 digits (HEX5, HEX4, HEX3): n in decimal
   hex7seg seg5 (.a(n_hundreds), .y(HEX5));
   hex7seg seg4 (.a(n_tens),     .y(HEX4));
   hex7seg seg3 (.a(n_ones),     .y(HEX3));
   // Right 3 digits (HEX2, HEX1, HEX0): iteration count in decimal
   hex7seg seg2 (.a(c_hundreds), .y(HEX2));
   hex7seg seg1 (.a(c_tens),     .y(HEX1));
   hex7seg seg0 (.a(c_ones),     .y(HEX0));

   assign LEDR = SW; // mirror switches
   assign go = go_next;

   // KEY[3] rising edge: latch start from SW and pulse go next cycle. Do NOT zero start when done.
   always_ff @(posedge clk) begin
      key3_prev <= KEY[3];
      if (KEY[3] && ~key3_prev) begin
         start   <= 32'(SW);
         go_next <= 1;
      end else begin
         go_next <= 0;
         counter <= counter + 1;
         if (counter == 22'd1000000)
            counter <= 0;
         if (KEY[0] && start < 255)
            start <= start + 1;
         else if (KEY[1] && start > 0)
            start <= start - 1;
         else if (KEY[2])
            start <= 0;
      end
   end
  
endmodule
