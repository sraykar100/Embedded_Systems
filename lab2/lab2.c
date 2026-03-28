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
#include "keyboard.h"
#include "display.h"

/* Update SERVER_HOST to be the IP address of
 * the chat server you are connecting to
 */
/* arthur.cs.columbia.edu */
#define SERVER_HOST "128.59.19.114"
#define SERVER_PORT 42000

#define BUFFER_SIZE 128

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

static uint8_t repeat_keycode = 0;
static uint8_t repeat_modifiers = 0;
static uint64_t next_repeat_ms = 0;

uint64_t monotonic_time_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ((uint64_t)ts.tv_sec * 1000ULL) + ((uint64_t)ts.tv_nsec / 1000000ULL);
}

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
  fbputs("Type here: ", 22, 0);               // Input area row 1

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
      keyboard_save_packet(&packet);
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
