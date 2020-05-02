/* keyboard.h - handles interactions with PS/2 keyboard 
 * vim:ts=4 noexpandtab
 */

#include "keyboard.h"
#include "lib.h"
#include "terminal.h"
#include "types.h"
#include "scheduler.h"

/* Scancode associated with the L and function keys */
#define L_KEY       0x26
#define F_ONE       0x3B
#define F_TWO       0x3C
#define F_THREE     0x3D

/* Flags for modifier keys */
volatile int ctrl_flag;
volatile int alt_flag;
volatile int caps_flag;
volatile int shft_flag;

/* { normal key press, shift key press, capslock key press } */
/* 2 modifier keys + base ASCIIs = 3 */
uint8_t scancodes[NUM_PRESSED_SCANCODES][3] =
{
    {'\0', '\0', '\0'}, // No Key
    {'\0', '\0', '\0'}, // Escape
    {'1' , '!' , '1' }, 
    {'2' , '@' , '2' }, 
    {'3' , '#' , '3' }, 
    {'4' , '$' , '4' }, 
    {'5' , '%' , '5' }, 
    {'6' , '^' , '6' }, 
    {'7' , '&' , '7' }, 
    {'8' , '*' , '8' }, 
    {'9' , '(' , '9' }, 
    {'0' , ')' , '0' }, 
    {'-' , '_' , '-' }, 
    {'=' , '+' , '=' }, 
    {'\0', '\0', '\0'}, // Backspace
    {' ' , ' ' , ' ' }, // Tab
    {'q' , 'Q' , 'Q' },
    {'w' , 'W' , 'W' },
    {'e' , 'E' , 'E' }, 
    {'r' , 'R' , 'R' }, 
    {'t' , 'T' , 'T' }, 
    {'y' , 'Y' , 'Y' }, 
    {'u' , 'U' , 'U' }, 
    {'i' , 'I' , 'I' }, 
    {'o' , 'O' , 'O' }, 
    {'p' , 'P' , 'P' }, 
    {'[' , '{' , '[' }, 
    {']' , '}' , ']' }, 
    {'\0', '\0', '\0'}, // Enter
    {'\0', '\0', '\0'}, // Left Ctrl
    {'a' , 'A' , 'A' }, 
    {'s' , 'S' , 'S' }, 
    {'d' , 'D' , 'D' }, 
    {'f' , 'F' , 'F' }, 
    {'g' , 'G' , 'G' }, 
    {'h' , 'H' , 'H' }, 
    {'j' , 'J' , 'J' }, 
    {'k' , 'K' , 'K' }, 
    {'l' , 'L' , 'L' }, 
    {';' , ':' , ';' }, 
    {'\'', '\"', '\''}, 
    {'`' , '~' , '`' }, 
    {'\0', '\0', '\0'}, // Left Shift
    {'\\', '|' , '\\'}, 
    {'z' , 'Z' , 'Z' }, 
    {'x' , 'X' , 'X' }, 
    {'c' , 'C' , 'C' }, 
    {'v' , 'V' , 'V' }, 
    {'b' , 'B' , 'B' }, 
    {'n' , 'N' , 'N' }, 
    {'m' , 'M' , 'M' }, 
    {',' , '<' , ',' }, 
    {'.' , '>' , '.' }, 
    {'/' , '?' , '/' }, 
    {'\0', '\0', '\0'}, // Right Shift
    {'\0', '\0', '\0'}, // (Keypad) *
    {'\0', '\0', '\0'}, // Left Alt
    {' ' , ' ' , ' ' }, 
    {'\0', '\0', '\0'}, // CapsLock
    {'\0', '\0', '\0'}, // F1
    {'\0', '\0', '\0'}, // F2
    {'\0', '\0', '\0'}, // F3
    {'\0', '\0', '\0'}, // F4
    {'\0', '\0', '\0'}, // F5
    {'\0', '\0', '\0'}, // F6
    {'\0', '\0', '\0'}, // F7
    {'\0', '\0', '\0'}, // F8
    {'\0', '\0', '\0'}, // F9
    {'\0', '\0', '\0'}, // F10
    {'\0', '\0', '\0'}, // NumberLock
    {'\0', '\0', '\0'}, // ScrollLock
    {'\0', '\0', '\0'}, // (Keypad) 7
    {'\0', '\0', '\0'}, // (Keypad) 8
    {'\0', '\0', '\0'}, // (Keypad) 9
    {'\0', '\0', '\0'}, // (Keypad) -
    {'\0', '\0', '\0'}, // (Keypad) 4
    {'\0', '\0', '\0'}, // (Keypad) 5
    {'\0', '\0', '\0'}, // (Keypad) 6 
    {'\0', '\0', '\0'}, // (Keypad) + 
    {'\0', '\0', '\0'}, // (Keypad) 1 
    {'\0', '\0', '\0'}, // (Keypad) 2 
    {'\0', '\0', '\0'}, // (Keypad) 3 
    {'\0', '\0', '\0'}, // (Keypad) 0 
    {'\0', '\0', '\0'}, // (Keypad) . 
    {'\0', '\0', '\0'}, // No Key 
    {'\0', '\0', '\0'}, // No Key
    {'\0', '\0', '\0'}, // F11
    {'\0', '\0', '\0'}  // F12
};

/* 
 * init_keyboard
 * 
 * DESCRIPTION: Enables PIC Interrupt line and sets flags
 * 
 * Input: none
 * Output: none
 * Return Values: none
 * 
 * SIDE EFFECTS: sets flags; enables keyboard interrupt line on PIC and clears screen
 */
void init_keyboard(void) {
    /* Initialize flags */
    ctrl_flag = 0;
    alt_flag = 0;
    caps_flag = 0;
    shft_flag = 0;

    /* unmasks keyboard interrupt line (IRQ1) */
    enable_irq(KEYBOARD_IRQ);
}

/* 
 * keyboard_intr_handler
 * 
 * DESCRIPTION: main C keyboard interrupt handler, handles key presses and prints chars to screen
 * Input: none
 * Output: none
 * Return Values: none
 * 
 * SIDE EFFECTS: none
 */
void keyboard_intr_handler() {
    // obtain the scan code the keyboard sent
    uint8_t keyboard_scancode = inb(DATA_PORT); 

    /* send EOI signal to PIC */
    send_eoi(KEYBOARD_IRQ);
    /* handles key release scancodes */
    if (keyboard_scancode > RELEASED_OFFSET) {
        /* if shift is released, update flag */
        if ((keyboard_scancode - RELEASED_OFFSET == LEFT_SHIFT) || (keyboard_scancode - RELEASED_OFFSET == RIGHT_SHIFT)) {
            shft_flag = 0; 
        }

        /* if control is released, update flag */
        if ((keyboard_scancode - RELEASED_OFFSET == LEFT_CTRL)) { 
            ctrl_flag = 0; 
        }

        /* if alt is released, update flag */
        if (keyboard_scancode - RELEASED_OFFSET== LEFT_ALT)
            alt_flag = 0; 

        /* if enter is released, update flag */
        if ((keyboard_scancode - RELEASED_OFFSET == ENTER)) { 
            terminal[curr_term].enter_flag = 0; 
        }

        return;
    } 

    /* handles key press scancodes */
    /* holds ASCII of keyboard output */
    uint8_t keyboard_output = scancodes[keyboard_scancode][0];

    /* if caps lock has been pressed, update flag */
    if (keyboard_scancode == CAPS_LOCK)
        caps_flag ^= 1;

    if ((keyboard_scancode == LEFT_SHIFT) || (keyboard_scancode == RIGHT_SHIFT))
        shft_flag = 1; 

    if (keyboard_scancode == LEFT_CTRL)
        ctrl_flag = 1; 
           
    if (keyboard_scancode == LEFT_ALT)
        alt_flag = 1; 

    /* handles print output if caps lock is active */
    if (caps_flag)
        /* indexing value from caps lock sub-array (2) */
        keyboard_output = scancodes[keyboard_scancode][2];

    /* handles print output if shift is pressed */
    if (shft_flag)
        /* reverses output if caps pressed with shift */
        keyboard_output = caps_flag ? scancodes[keyboard_scancode][0] : scancodes[keyboard_scancode][1];

    /* handles output if ctrl+L is pressed */
    if (ctrl_flag && (keyboard_scancode == L_KEY)) {
        /* make a copy of values in internal buffer and the buffer index */
        memcpy(placeholder_buf, terminal[curr_term].internal_buffer, MAX_BUFFER_SIZE);
        placeholder_buf_index = terminal[curr_term].buffer_index;
        /* clears screen and the internal buffer */
        clear();
        memset(terminal[curr_term].internal_buffer, '\0', MAX_BUFFER_SIZE);
        /* set flags for terminal_read() */
        ctrl_L_flag = 1;
        terminal[curr_term].enter_flag = 1;

        return;
    }

    /* handles output if ALT + F1 is pressed */
    if (alt_flag && (keyboard_scancode == F_ONE)) {
        /* switch into corresponding terminal */
        terminal_switch(0);
        return;
    }
	
    /* handles output if ALT + F2 is pressed */
    if (alt_flag && (keyboard_scancode == F_TWO)) {
        /* switch into corresponding terminal */
        terminal_switch(1);
        return;
    }
	
    /* handles output if ALT + F3 is pressed */
    if (alt_flag && (keyboard_scancode == F_THREE)) {
        /* switch into corresponding terminal */
        terminal_switch(2);
	    return;
    }

    /* handles output if enter is pressed */
    if (keyboard_scancode == ENTER) {
        /* Mark that enter has been pressed */
        terminal[curr_term].enter_flag = 1;

        /* scroll screen if at bottom */
        if (terminal[curr_term].screen_y == NUM_ROWS - 1)
            scroll(curr_term);
        /* otherwise, move to next line */
        else
            set_cursor(0, terminal[curr_term].screen_y + 1, curr_term);

        return;
    }

    /* handles output if backspace is pressed */
    if (keyboard_scancode == BACKSPACE && (terminal[curr_term].buffer_index != 0)) {
        /* check if wraparound is active; backspacing on previous line */
        if (terminal[curr_term].screen_x == NUM_COLS) {
            /* clear value in video memory of previous typed character in previous line*/
            *(uint8_t *) ((char*)VIDEO + (( (terminal[curr_term].screen_y - 1) * NUM_COLS + (NUM_COLS - 1) ) << 1)) = ' ';
            *(uint8_t *) ((char*)VIDEO + (( (terminal[curr_term].screen_y - 1) * NUM_COLS + (NUM_COLS - 1) ) << 1) + 1) = ATTRIB;

            /* move cursor to last position of previous line */
            set_cursor(NUM_COLS - 1, terminal[curr_term].screen_y - 1, curr_term);
        } 

        /* if backspacing on current line */
        else { 
            /* clear value in video memory of previous typed character in current line*/
            *(uint8_t *) ((char*)VIDEO + ((terminal[curr_term].screen_y * NUM_COLS + (terminal[curr_term].screen_x - 1)) << 1)) = ' ';
            *(uint8_t *) ((char*)VIDEO + ((terminal[curr_term].screen_y * NUM_COLS + (terminal[curr_term].screen_x - 1)) << 1) + 1) = ATTRIB;

            /* move cursor back one x position */
            set_cursor(terminal[curr_term].screen_x - 1, terminal[curr_term].screen_y, curr_term);
        }

        /* decrement buffer index and clear character from internal buffer */
        terminal[curr_term].internal_buffer[ terminal[curr_term].buffer_index-- ] = ' ';
    }

    /* prints keyboard output to screen*/
    if (keyboard_output != '\0') {
        if(keyboard_scancode == TAB) {
            /* A tab input consists of 4 spaces - so 4 iterations */
            int i;
            for(i = 0; i < 4; i++) {
                /* check if max characters for current line has not been reached */
                if (terminal[curr_term].buffer_index < MAX_BUFFER_SIZE - 1) {
                    keyboard_putc(keyboard_output);
                    terminal[curr_term].internal_buffer[ terminal[curr_term].buffer_index++ ] = keyboard_output;
                }
            }
        } else {
            /* check if max characters for current line has not been reached */
            if (terminal[curr_term].buffer_index < MAX_BUFFER_SIZE - 1) {
                keyboard_putc(keyboard_output);
                terminal[curr_term].internal_buffer[ terminal[curr_term].buffer_index++ ] = keyboard_output;
            }
        }
    }
}
