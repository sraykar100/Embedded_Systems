#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>
#include "usbkeyboard.h"

/* 
HIDs for special keys 
https://www.usb.org/sites/default/files/documents/hut1_12v2.pdf
*/
#define HID_ENTER     0x28
#define HID_ESCAPE    0x29
#define HID_CAPSLOCK  0x39
#define HID_BACKSPACE 0x2A
#define HID_TAB       0x2B
#define HID_RIGHT     0x4F
#define HID_LEFT      0x50

char keycode_to_ascii(uint8_t keycode, uint8_t modifiers);
int is_special_key(uint8_t keycode);
void handle_caps(uint8_t keycode);
void handle_keypress(uint8_t keycode, uint8_t modifiers);
int is_new_keypress(uint8_t keycode);
int is_repeatable_key(uint8_t keycode);
void keyboard_save_packet(struct usb_keyboard_packet *packet);

#endif
