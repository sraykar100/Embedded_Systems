/*
 *
 * CSEE 4840 Lab 2 for 2019
 * Shiyao Marcus Lam (sml2286)
 * Siddarth Raykar (sr4102)
 */
#include "fbputchar.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include "usbkeyboard.h"
#include <pthread.h>

/* Update SERVER_HOST to be the IP address of
 * the chat server you are connecting to
 */
/* arthur.cs.columbia.edu */
#define SERVER_HOST "128.59.19.114"
#define SERVER_PORT 42000

#define BUFFER_SIZE 128

#define MSG_AREA_TOP 0       /* First row for messages */
#define MSG_AREA_BOTTOM 20   /* Last row for messages */
#define MSG_AREA_COLS 64     /* Characters per row */
#define INPUT_AREA_TOP 22    /* First row for user input */
#define DIVIDER_ROW 21       /* Row for the divider line */

/* Track row and col for the next character to go on */
int current_msg_row = MSG_AREA_TOP;
int current_msg_col = 0;

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

/* 
HIDs for special keys 
https://www.usb.org/sites/default/files/documents/hut1_12v2.pdf
*/
#define HID_ENTER     0x28
#define HID_ESCAPE    0x29
#define HID_CAPSLOCK  0x39
#define HID_BACKSPACE 0x2A
#define HID_TAB       0x2B

static int is_caps_on = 0;

/* Input buffer for user typing */
#define INPUT_BUFFER_SIZE 256
static char input_buffer[INPUT_BUFFER_SIZE];
static int input_length = 0;

/* Previous keyboard packet for edge detection */
static struct usb_keyboard_packet previous_packet = {0};
static uint8_t repeat_keycode = 0;
static uint8_t repeat_modifiers = 0;
static uint64_t next_repeat_ms = 0;

#define KEY_REPEAT_INITIAL_DELAY_MS 500
#define KEY_REPEAT_INTERVAL_MS 80
#define USB_POLL_TIMEOUT_MS 10

/*
 * References:
 *
 * https://web.archive.org/web/20130307100215/http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html
 *
 * http://www.thegeekstuff.com/2011/12/c-socket-programming/
 * 
 */

int sockfd; /* Socket file descriptor */

struct libusb_device_handle *keyboard;
uint8_t endpoint_address;

pthread_t network_thread;
void *network_thread_f(void *);

/* Function prototypes */
void clear_message_area(void);
void display_message(const char *msg, unsigned char r, unsigned char g, unsigned char b);
char keycode_to_ascii(uint8_t keycode, uint8_t modifiers);
int is_special_key(uint8_t keycode);
void handle_caps(uint8_t keycode);
void handle_keypress(uint8_t keycode, uint8_t modifiers);
void input_buf_add_char(char c);
void input_buf_delete_char(void);
void input_buf_clear(void);
void input_buf_debug_print(void);
int is_new_keypress(uint8_t keycode);
int is_repeatable_key(uint8_t keycode);
uint64_t monotonic_time_ms(void);
int send_input_buffer_to_server(void);
void display_user_input(void);

int main()
{
  int err;

  struct sockaddr_in serv_addr;

  struct usb_keyboard_packet packet;
  int transferred;
  char keystate[12];

  if ((err = fbopen()) != 0) {
    fprintf(stderr, "Error: Could not open framebuffer: %d\n", err);
    exit(1);
  }

  // /* Draw rows of asterisks across the top and bottom of the screen */
  // for (col = 0 ; col < 64 ; col++) {
  //   fbputchar('*', 0, col);
  //   fbputchar('*', 23, col);
  // }

  // fbputs("Hello CSEE 4840 World!", 4, 10);

  // -- NEW --
  fb_init_screen();
  // 3. Test: put some text in different areas to verify layout
  fbputs("=== MESSAGE AREA ===", 0, 0);      // Top of message area
  fbputs("Messages appear here", 2, 0);       // Message area
  fbputs("Last message row", 20, 0);          // Bottom of message area
  
  fbputs("Type here: ", 22, 0);               // Input area row 1
  fbputs("(input line 2)", 23, 0); 

  // -- END NEW --

  /* Open the keyboard */
  if ( (keyboard = openkeyboard(&endpoint_address)) == NULL ) {
    fprintf(stderr, "Did not find a keyboard\n");
    exit(1);
  }
    
  /* Create a TCP communications socket */
  if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
    fprintf(stderr, "Error: Could not create socket\n");
    exit(1);
  }

  /* Get the server address */
  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(SERVER_PORT);
  if ( inet_pton(AF_INET, SERVER_HOST, &serv_addr.sin_addr) <= 0) {
    fprintf(stderr, "Error: Could not convert host IP \"%s\"\n", SERVER_HOST);
    exit(1);
  }

  /* Connect the socket to the server */
  if ( connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
    fprintf(stderr, "Error: connect() failed.  Is the server running?\n");
    exit(1);
  }

  /* Start the network thread */
  pthread_create(&network_thread, NULL, network_thread_f, NULL);

  /* Look for and handle keypresses */
  for (;;) {
    int transfer_rc = libusb_interrupt_transfer(keyboard, endpoint_address,
			      (unsigned char *) &packet, sizeof(packet),
			      &transferred, USB_POLL_TIMEOUT_MS);
    if (transfer_rc == 0 && transferred == sizeof(packet)) {
      sprintf(keystate, "%02x %02x %02x", packet.modifiers, packet.keycode[0],
	      packet.keycode[1]);
      printf("%s\n", keystate);
      fbputs(keystate, 6, 0);

      /* Process each keycode slot - only handle NEW key presses */
      for (int i = 0; i < 6; i++) {
        uint8_t keycode = packet.keycode[i];
        if (keycode != 0 && is_new_keypress(keycode)) {
          handle_keypress(keycode, packet.modifiers);
        }
      }

      /* Held-key repeat behavior (after delay, at fixed rate) */
      uint8_t held_keycode = 0;
      for (int i = 0; i < 6; i++) {
        if (packet.keycode[i] != 0) {
          held_keycode = packet.keycode[i];
          break;
        }
      }

      uint64_t now_ms = monotonic_time_ms();
      if (held_keycode == 0) {
        repeat_keycode = 0;
      } else if (held_keycode != repeat_keycode || packet.modifiers != repeat_modifiers) {
        repeat_keycode = held_keycode;
        repeat_modifiers = packet.modifiers;
        next_repeat_ms = now_ms + KEY_REPEAT_INITIAL_DELAY_MS;
      } else if (is_repeatable_key(held_keycode) && now_ms >= next_repeat_ms) {
        handle_keypress(held_keycode, packet.modifiers);
        next_repeat_ms = now_ms + KEY_REPEAT_INTERVAL_MS;
      }

      /* Save current packet as previous for next iteration */
      previous_packet = packet;
    } else if (transfer_rc != LIBUSB_ERROR_TIMEOUT) {
      fprintf(stderr, "Keyboard read error: %d\n", transfer_rc);
    }

    /*
     * Run key repeat even without fresh keyboard packets.
     * Some devices report only state changes.
     */
    if (repeat_keycode != 0 && is_repeatable_key(repeat_keycode)) {
      uint64_t now_ms = monotonic_time_ms();
      if (now_ms >= next_repeat_ms) {
        handle_keypress(repeat_keycode, repeat_modifiers);
        next_repeat_ms = now_ms + KEY_REPEAT_INTERVAL_MS;
      }
    }
  }

  /* Terminate the network thread */
  pthread_cancel(network_thread);

  /* Wait for the network thread to finish */
  pthread_join(network_thread, NULL);

  return 0;
}

void *network_thread_f(void *ignored)
{
  char recvBuf[BUFFER_SIZE];
  int n;
  /* Receive data */
  while ( (n = read(sockfd, &recvBuf, BUFFER_SIZE - 1)) > 0 ) {
    recvBuf[n] = '\0';
    printf("%s", recvBuf);
    // fbputs(recvBuf, 8, 0);
    display_message(recvBuf, 255, 255, 255);
  }

  return NULL;
}

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
 * Handles wrapping and overflow
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

/* Add a character to the input buffer */
void input_buf_add_char(char c)
{
    if (input_length < INPUT_BUFFER_SIZE - 1) {
        input_buffer[input_length++] = c;
        input_buffer[input_length] = '\0';
        input_buf_debug_print();
    }
}

/* Delete the last character from the input buffer (backspace) */
void input_buf_delete_char(void)
{
    if (input_length > 0) {
        input_length--;
        input_buffer[input_length] = '\0';
        input_buf_debug_print();
    }
}

/* Clear the input buffer */
void input_buf_clear(void)
{
    input_length = 0;
    input_buffer[0] = '\0';
    input_buf_debug_print();
}

void input_buf_debug_print(void)
{
    printf("Input[%d]: \"%s\"\n", input_length, input_buffer);
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

    /* Handle Enter - for now just print, later will send to server */
    if (keycode == HID_ENTER) {
        printf("ENTER pressed. Buffer contents: \"%s\"\n", input_buffer);
        input_buf_clear();
        return;
    }

    /*
     * Handle Escape as "send message":
     * 1) transmit current input buffer to server
     * 2) clear local input only if send succeeds
     */
    if (keycode == HID_ESCAPE) {
        if (input_length > 0) {
            if (send_input_buffer_to_server() == 0) {
                input_buf_clear();
            } else {
                fprintf(stderr, "Failed to send message; buffer kept for retry.\n");
            }
        }
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
        case HID_ESCAPE:
        case HID_CAPSLOCK:
            return 0;
        default:
            return 1;
    }
}

uint64_t monotonic_time_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ((uint64_t)ts.tv_sec * 1000ULL) + ((uint64_t)ts.tv_nsec / 1000000ULL);
}

/*
 * Send the full input buffer to the chat server.
 * Uses a loop to handle partial writes, then sends a trailing newline.
 * Returns 0 on success and -1 on error.
 */
int send_input_buffer_to_server(void)
{
    int sent = 0;

    while (sent < input_length) {
        ssize_t n = write(sockfd, input_buffer + sent, input_length - sent);
        if (n <= 0) {
            return -1;
        }
        sent += (int)n;
    }

    /*
     * Server messages are line-oriented; send newline so the message
     * is treated as a complete chat line.
     */
    if (write(sockfd, "\n", 1) != 1) {
        return -1;
    }

    return 0;
}

void display_user_input(void)
{
  int row = INPUT_AREA_TOP + 1;  /* Just use the bottom row its easiest*/
  int col = 0;
  
  /* Clear the row and redraw the whole thing */
  for (int c = 0; c < MSG_AREA_COLS; c++) {
      fbputchar(' ', row, c);
  }
  
  /* Calculate the start of the "window" to display for long inputs */
  int start = 0;
  if (input_length > MSG_AREA_COLS) {
      start = input_length - MSG_AREA_COLS;
  }
  
  /* Draw visible portion of buffer */
  for (int i = start; i < input_length; i++) {
      fbputchar(input_buffer[i], row, col);
      col++;
  }
}
