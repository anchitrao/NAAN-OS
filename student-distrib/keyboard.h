/* keyboard.h - handles interactions with PS/2 keyboard 
 * vim:ts=4 noexpandtab
 */

#ifndef _KEYBOARD_H
#define _KEYBOARD_H

// include .h files crucial to keyboard function
#include "i8259.h"
#include "keyboard_handler.h"

/* number of scan codes associated with key presses */
#define NUM_PRESSED_SCANCODES 0x58
/* keyboard IRQ line on PIC */
#define KEYBOARD_IRQ 1
/* offset between start and release scancodes */
#define RELEASED_OFFSET 0x80

/* SCANCODE MACROS (PRESSED) */
#define LEFT_CTRL 0x1D
#define LEFT_ALT 0x38
#define LEFT_SHIFT 0x2A
#define RIGHT_SHIFT 0x36
#define CAPS_LOCK 0x3A
#define ENTER 0x1C
#define BACKSPACE 0x0E
#define TAB 0x0F

/* data port to communicate with keyboard */
#define DATA_PORT 0x60

/* initializes keyboard */
void init_keyboard(void);

/* main C handler for keyboard */
void keyboard_intr_handler(void); 

#endif  /* end if for _KEYBOARD_H */
