#include "keyboard.h"
#include "usbkeyboard.h"
#include "display.h"
#include <stdio.h>
#include <unistd.h>

/* Lookup table: Unshifted characters */
static const char keycode_to_ascii_unshifted[128] = {
  0, 0, 0, 0, 'a', 'b', 'c', 'd',      // 0x00-0x07
  'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l',  // 0x08-0x0F
  'm', 'n', 'o', 'p', 'q', 'r', 's', 't',  // 0x10-0x17
  'u', 'v', 'w', 'x', 'y', 'z', '1', '2',  // 0x18-0x1F
  '3', '4', '5', '6', '7', '8', '9', '0',  // 0x20-0x27
  '\n', 0, 0, '\t', ' ', '-', '=', '[',    // 0x28-0x2F (Enter, Esc, Backspace, Tab, Space, -, =, [)
  ']', '\\', 0, ';', '\'', '`', ',', '.',  // 0x30-0x37
  '/', 0, 0, 0, 0, 0, 0, 0,                // 0x38-0x3F (/, CapsLock, F1-F5)
  // rest are 0 
};

/* Lookup table: Shifted characters */
static const char keycode_to_ascii_shifted[128] = {
  0, 0, 0, 0, 'A', 'B', 'C', 'D',         // 0x00-0x07
  'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', // 0x08-0x0F
  'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', // 0x10-0x17
  'U', 'V', 'W', 'X', 'Y', 'Z', '!', '@', // 0x18-0x1F
  '#', '$', '%', '^', '&', '*', '(', ')', // 0x20-0x27
  '\n', 0, 0, '\t', ' ', '_', '+', '{',   // 0x28-0x2F
  '}', '|', 0, ':', '"', '~', '<', '>',   // 0x30-0x37
  '?', 0, 0, 0, 0, 0, 0, 0,               // 0x38-0x3F
  // rest are 0
};

static int is_caps_on = 0;

/* Previous keyboard packet for edge detection */
static struct usb_keyboard_packet previous_packet = {0};

/* Input buffer for user typing */
#define INPUT_BUFFER_SIZE 256
static char input_buffer[INPUT_BUFFER_SIZE];
static int input_length = 0;
static int cursor_position = 0;  /* Cursor position within input buffer */

extern int sockfd;

void handle_caps(uint8_t keycode){
  if (keycode == HID_CAPSLOCK){
      is_caps_on = !is_caps_on;
    }
}

char keycode_to_ascii(uint8_t keycode, uint8_t modifiers)
{
    /* no key pressed */
    if (keycode == 0) {
        return 0;
    }
    
    /* keycode is out of range */
    if (keycode >= 128) {
        return 0;  
    }

    /* either Shift key is pressed, then look up in appropriate table */
    int shift_pressed = 0;
    if ((modifiers & USB_LSHIFT) || (modifiers & USB_RSHIFT)) {
        shift_pressed = 1;
    }
    int layer_toggle = 0; 
    if (keycode >= 0x04 && keycode <= 0x1d){ // if letter, toggle caps.
      layer_toggle = shift_pressed ^ is_caps_on;
    }
    else{
      layer_toggle = shift_pressed;
    }

    if (layer_toggle) {
        return keycode_to_ascii_shifted[keycode];
    } else {
        return keycode_to_ascii_unshifted[keycode];
    }
}

int is_special_key(uint8_t keycode)
{
    switch (keycode) {
        case HID_ENTER:
        case HID_ESCAPE:
        case HID_CAPSLOCK:
        case HID_BACKSPACE:
        case HID_TAB:
            return 1;
        default:
            return 0;
    }
}

/* Insert char at cursor */
void input_buf_add_char(char c)
{
    if (input_length < INPUT_BUFFER_SIZE - 1) {
        /* Shift right to make room */
        for (int i = input_length; i > cursor_position; i--) {
            input_buffer[i] = input_buffer[i - 1];
        }
        input_buffer[cursor_position] = c;
        input_length++;
        cursor_position++;
        input_buffer[input_length] = '\0';
        printf("Input[%d] cursor@%d: \"%s\"\n", input_length, cursor_position, input_buffer);
        display_user_input(input_buffer, input_length, cursor_position);
    }
}

/* Backspace: delete char before cursor */
void input_buf_delete_char(void)
{
    if (cursor_position > 0) {
        /* Shift left to fill gap */
        for (int i = cursor_position - 1; i < input_length - 1; i++) {
            input_buffer[i] = input_buffer[i + 1];
        }
        input_length--;
        cursor_position--;
        input_buffer[input_length] = '\0';
        printf("Input[%d] cursor@%d: \"%s\"\n", input_length, cursor_position, input_buffer);
        display_user_input(input_buffer, input_length, cursor_position);
    }
}

/* Clear the input buffer */
void input_buf_clear(void)
{
    input_length = 0;
    cursor_position = 0;
    input_buffer[0] = '\0';
    printf("Input[%d] cursor@%d: \"%s\"\n", input_length, cursor_position, input_buffer);
    display_user_input(input_buffer, input_length, cursor_position);
}

void cursor_move_left(void)
{
    if (cursor_position > 0) {
        cursor_position--;
        display_user_input(input_buffer, input_length, cursor_position);
    }
}

void cursor_move_right(void)
{
    if (cursor_position < input_length) {
        cursor_position++;
        display_user_input(input_buffer, input_length, cursor_position);
    }
}

/*
 * Send the full input buffer to the chat server.
 * Uses a loop to handle partial writes.
 * Returns 0 on success and -1 on error.
 */
static int send_input_buffer_to_server(void)
{
    int sent = 0;

    while (sent < input_length) {
        ssize_t n = write(sockfd, input_buffer + sent, input_length - sent);
        if (n <= 0) {
            return -1;
        }
        sent += (int)n;
    }

    return 0;
}

void handle_keypress(uint8_t keycode, uint8_t modifiers)
{
    /* Handle Caps Lock toggle */
    if (keycode == HID_CAPSLOCK) {
        handle_caps(keycode);
        return;
    }

    /* Handle Backspace - delete last character */
    if (keycode == HID_BACKSPACE) {
        input_buf_delete_char();
        return;
    }

    /* Handle Tab - add 4 spaces */
    if (keycode == HID_TAB) {
        for (int i = 0; i < 4; i++) {
            input_buf_add_char(' ');
        }
        return;
    }

    /*
     * Handle Enter as "send message":
     * 1) transmit current input buffer to server
     * 2) clear local input only if send succeeds
     */
    if (keycode == HID_ENTER) {
        if (input_length > 0) {
            if (send_input_buffer_to_server() == 0) {
                input_buf_clear();
            } else {
                fprintf(stderr, "Failed to send message; buffer kept for retry.\n");
            }
        }
        return;
    }

    /* Handle Escape: do nothing */
    if (keycode == HID_ESCAPE) {
        return;
    }

    /* Arrow keys move cursor */
    if (keycode == HID_LEFT) {
        cursor_move_left();
        return;
    }
    if (keycode == HID_RIGHT) {
        cursor_move_right();
        return;
    }

    /* Convert to ASCII and add to buffer */
    char ascii = keycode_to_ascii(keycode, modifiers);
    if (ascii != 0 && ascii != '\n' && ascii != '\t') {
        input_buf_add_char(ascii);
    }
}

/*
 * Essentially detect the rising edge of a keypress by comparing the current packet to the previous packet.
 */
int is_new_keypress(uint8_t keycode)
{
    if (keycode == 0) { // no key pressed / release 
        return 0;
    }
    
    /* Check if this keycode was in any slot of the previous packet */
    for (int i = 0; i < 6; i++) {
        if (previous_packet.keycode[i] == keycode) {
            return 0;  
        }
    }
    
    // New keypress 
    return 1;  
}

int is_repeatable_key(uint8_t keycode)
{
    switch (keycode) {
        case HID_ENTER:
        case HID_ESCAPE:
        case HID_CAPSLOCK:
            return 0;
        default:
            return 1;
    }
}

void keyboard_save_packet(struct usb_keyboard_packet *packet)
{
    previous_packet = *packet;
}
