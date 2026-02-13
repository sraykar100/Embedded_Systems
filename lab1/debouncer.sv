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

logic [13:0] timer;
always @ (posedge clk) begin
if(next == sync_1) begin
        timer <=0;
end
else begin
        timer <= timer + 1;
        if(timer == 14'h3fff) next <= ~next;
end
end
endmodule











