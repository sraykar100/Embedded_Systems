#ifndef DISPLAY_H
#define DISPLAY_H

#define MSG_AREA_TOP 0       /* First row for messages */
#define MSG_AREA_BOTTOM 20   /* Last row for messages */
#define MSG_AREA_COLS 64     /* Characters per row */
#define INPUT_AREA_TOP 22    /* First row for user input */
#define DIVIDER_ROW 21       /* Row for the divider line */

void clear_message_area(void);
void display_message(const char *msg, unsigned char r, unsigned char g, unsigned char b);
void display_user_input(const char *buffer, int length, int cursor_pos);

#endif
