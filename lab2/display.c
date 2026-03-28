#include "display.h"
#include "fbputchar.h"

/* Track row and col for the next character to go on */
static int current_msg_row = MSG_AREA_TOP;
static int current_msg_col = 0;

/*
When messages fill up the display area, clear the message area (just rows 0-20) and reset cursor position to the top.
*/
void clear_message_area(void)
{
    int row, col;
    
    for (row = MSG_AREA_TOP; row <= MSG_AREA_BOTTOM; row++) {
        for (col = 0; col < MSG_AREA_COLS; col++) {
            fbputchar(' ', row, col);
        }
    }
    
    /* Reset position to top-left of message area */
    current_msg_row = MSG_AREA_TOP;
    current_msg_col = 0;
}


/*
 * Display a message in the message area with the given color.
 * Handles wrapping and overflow.
 * If displaying the message would overflow, clear and redisplay from the start.
 */
void display_message(const char *msg, unsigned char r, unsigned char g, unsigned char b)
{
    const char *p = msg;
    char c;
    
    while ((c = *p++) != '\0') {
        /* Handle newline: move to start of next row */
        if (c == '\n') {
            current_msg_col = 0;
            current_msg_row++;

            /* Check if we've gone past the message area */
            if (current_msg_row > MSG_AREA_BOTTOM) {
                clear_message_area();
                p = msg;  /* Restart displaying from beginning of message */
            }
            continue;
        }

        /* Check if we need to wrap to next line */
        if (current_msg_col >= MSG_AREA_COLS) {
            current_msg_col = 0;
            current_msg_row++;

            /* Check if we've gone past the message area */
            if (current_msg_row > MSG_AREA_BOTTOM) {
                clear_message_area();
                p = msg;  /* Restart displaying from beginning of message */
                continue;
            }
        }

        /* Display the character */
        fbputchar(c, current_msg_row, current_msg_col);
        current_msg_col++;
    }

    /* Only advance to next line if we printed something on current line */
    if (current_msg_col > 0) {
        current_msg_col = 0;
        current_msg_row++;

        if (current_msg_row > MSG_AREA_BOTTOM) {
            clear_message_area();
        }
    }
}

void display_user_input(const char *buffer, int length, int cursor_pos)
{
  int row = INPUT_AREA_TOP + 1;
  
  /* Clear row */
  for (int c = 0; c < MSG_AREA_COLS; c++) {
      fbputchar(' ', row, c);
  }
  
  /* Keep cursor visible by sliding window */
  int visible_width = MSG_AREA_COLS - 1;
  int start = 0;
  if (cursor_pos > visible_width) {
      start = cursor_pos - visible_width;
  }
  
  /* Draw text */
  int col = 0;
  for (int i = start; i < length && col < visible_width; i++) {
      fbputchar(buffer[i], row, col);
      col++;
  }
  
  /* Draw cursor */
  int cursor_display_col = cursor_pos - start;
  if (cursor_display_col >= 0 && cursor_display_col < MSG_AREA_COLS) {
      fbputchar('|', row, cursor_display_col);
  }
}
