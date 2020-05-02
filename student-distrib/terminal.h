#ifndef _TERMINAL_H
#define _TERMINAL_H

#include "types.h"

#define MAX_BUFFER_SIZE 128
#define NEWLINE 0xA

/* Internal buffer */
uint8_t internal_buffer[TERMINAL_COUNT][MAX_BUFFER_SIZE];
/* Placeholder buffer for ctrl+L special case */
uint8_t placeholder_buf[TERMINAL_COUNT][MAX_BUFFER_SIZE];
/* Number of elements in the internal buffer */
volatile uint32_t buffer_index;
/* Placeholder buffer index for ctrl+L special case */
volatile uint32_t placeholder_buf_index;
/* Identifies if terminal read has been called */
volatile uint32_t read_flag;
/* Identifies if ctrl+L has been pressed */
volatile uint32_t ctrl_L_flag;

/* initialize terminal */
void terminal_init();

/* Provides access to terminal and initializes variables */
int32_t terminal_open (const uint8_t* filename);

/* reads data from keyboard, file, device, or directory */
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes);

/* writes data to terminal or device */
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes);

/* Closes the specified file descriptor for access by another open */
int32_t terminal_close (int32_t fd);

#endif  /* end if for _TERMINAL_H */
