// CSEE 4840 Lab 1: Run and Display Collatz Conjecture Iteration Counts
//
// Spring 2026
//
// By: Siddharth Raykar & Shiyao Marcus Lam
// Uni: sr4102 & sml2286

// I got the code for the following debouncer from the University of Michigan
// website. https://www.eecs.umich.edu/courses/eecs270/270lab/270_docs/debounce.html 
// It's slightly modified, but the skeleton's the same. 

module debouncer (input clk,
	input switch,
	output logic next);

// This section synchronizes the input to the clk
logic sync_0;
logic sync_1;
always @(posedge clk) sync_0 <= switch;
always @(posedge clk) sync_1 <= sync_0;

// Now we debounce: we wait until count (a 16 bit integer) reaches its max
// value. Since it's a 50 MHz clock, this should happen in milliseconds. 

logic [15:0] timer; 
always @ (posedge clk) begin
if(next == sync_1) begin
	timer <=0;
end
else begin
	timer <= timer + 1;
	if(timer == 16'hffff) next = ~next;
end
end
endmodule

// Takes the debounced button level and turns it into single-cycle pulses.
// A quick tap gives you exactly one pulse. If you hold the button down,
// it waits ~0.5s then starts firing pulses every ~0.1s for scrubbing.
module button_press #(
    parameter HOLD   = 25_000_000, // cycles before auto-repeat starts (~0.5s at 50 MHz)
    parameter REPEAT =  5_000_000 // cycles between repeats (~0.1s at 50 MHz)
)(
    input  logic clk,
    input  logic btn, // debounced level from debouncer
    output logic pulse // single-cycle pulse
);

    logic prev;
    logic [24:0] cnt;
    logic held;

    always_ff @(posedge clk) begin
        prev  <= btn;
        pulse <= 0;

        if (!btn) begin 
            cnt <= 0; held <= 0; // button not pressed, reset everything
        end 
        else if (!prev) 
            pulse <= 1; // rising edge, so return an immediate pulse
        else if (cnt < (held ? REPEAT : HOLD) - 1) // button is pressed AND pressed last cycle too, but counter is not at threshold yet
            cnt <= cnt + 1; // counting toward threshold
        else begin 
            pulse <= 1; cnt <= 0; held <= 1; // threshold hit: pulse & reset
      end  
    end

endmodule

module lab1( input logic        CLOCK_50,  // 50 MHz Clock input
	     
	     input logic [3:0] 	KEY, // Pushbuttons; KEY[0] is rightmost

	     input logic [9:0] 	SW, // Switches; SW[0] is rightmost

	     // 7-segment LED displays; HEX0 is rightmost
	     output logic [6:0] HEX0, HEX1, HEX2, HEX3, HEX4, HEX5,

	     output logic [9:0] LEDR // LEDs above the switches; LED[0] on right
	     );

   logic 			clk, done;
   logic [31:0] 		start;
   logic [15:0] 		count;
   

   // Decimal digits (0-9) for display: extract from binary using / and %
   logic [3:0] 		n_ones, n_tens, n_hundreds;   // n = start[11:0]
   logic [3:0] 		c_ones, c_tens, c_hundreds;   // iteration count

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
         r (clk, go, start, done, count); // Connect everything with matching names

      logic [11:0] start_display; // Value whose collatz number is shown on the display. 
      logic [7:0] start_index; // Start index is what's changed upon increment/decrement

      assign start_display = SW + start_index;
   
   // Left 3 digits: n in decimal
   hex7seg seg5 (.a(start_display[11:8]), .y(HEX5));
   hex7seg seg4 (.a(start_display[7:4]),     .y(HEX4));
   hex7seg seg3 (.a(start_display[3:0]),     .y(HEX3));
   // Right 3 digits: iteration count in decimal
   hex7seg seg2 (.a(count[11:8]), .y(HEX2));
   hex7seg seg1 (.a(count[7:4]),     .y(HEX1));
   hex7seg seg0 (.a(count[3:0]),     .y(HEX0));
	
   logic ready;
   assign LEDR = SW;
   	
   always_ff @(posedge clk) begin
	// always update display values 
	// Extract decimal digits: ones = val % 10, tens = (val/10) % 10, hundreds = (val/100) % 10
      if (go) begin
	      start <= SW;
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
