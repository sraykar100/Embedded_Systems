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


