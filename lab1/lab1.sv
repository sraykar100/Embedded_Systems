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

   logic [11:0] 		n;
   
   assign clk = CLOCK_50;
 
   range #(256, 8) // RAM_WORDS = 256, RAM_ADDR_BITS = 8)
         r ( .* ); // Connect everything with matching names

   // One hex7seg instance per 7-segment display (each digit is 4 bits)
   hex7seg seg0 (.a(count[3:0]),   .y(HEX0));  
   hex7seg seg1 (.a(count[7:4]),   .y(HEX1));
   hex7seg seg2 (.a(count[11:8]),  .y(HEX2));
   
   // Unused, should be blank
   hex7seg seg3 (.a(4'h0),   .y(HEX3));
   hex7seg seg4 (.a(4'h0),   .y(HEX4));
   hex7seg seg5 (.a(4'h0),  .y(HEX5));
   

   range #(.RAM_WORDS(256), .RAM_ADDR_BITS(8)) r1(
      .clk(clk), 
      .go(go), 
      .start(start), 
      .done(done), 
      .count(count)
   );

   assign go = KEY[3];

   always_ff @(posedge clk) begin
      if (go) begin
         start <= SW;
      end
      else begin
            start <= 0;
            counter <= counter + 1;
            if (counter == 22'd1000000) begin
                  counter <= 0;
            end
            if (KEY[0] && start < 255) begin
                  start <= start + 1;
            end
            else if (KEY[1] && start > 0) begin
                  start <= start - 1;
            end
            else if (KEY[2]) begin
                  start <= 0;
            end
      end
   end
  
endmodule
