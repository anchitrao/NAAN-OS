#include "terminal.h"
#include "lib.h"

/* 
 * terminal_init
 * 
 * DESCRIPTION: initializes terminal array (that holds running processes)
 * 
 * Input: none
 * Output: none
 * Return Values: none
 * 
 * SIDE EFFECTS: none
 */
void terminal_init () {
    int i;
    for (i = 0; i < TERMINAL_COUNT; i++) {
        terminal[i].screen_x = 0;
        terminal[i].screen_y = 0;
        terminal[i].active = 0;
        terminal[i].buffer_index = 0;
        terminal[i].curr_pcb = NULL;
        terminal[i].rtc_constant = 0;
        terminal[i].rtc_iterations = 0;
        terminal[i].video_mem = (int8_t*) (VIDEO + ((i + 1) * PAGE_SIZE));
        memset(terminal[i].internal_buffer, '\0', MAX_BUFFER_SIZE);
        terminal[i].buffer_index = 0;
    }
    curr_term = 0;
    sched_term = 0;
}

/* 
 * terminal_open
 * 
 * DESCRIPTION: opens terminal providing access to terminal and initializing variables
 * 
 * Input: const uint8_t* filename - filename of file to be used
 * Output: N/A
 * Return Values: -1 if file does not exist
 * 
 * SIDE EFFECTS: initializes flags
 */
int32_t terminal_open (const uint8_t* filename) {
    return 0;
}

/* 
 * terminal_read
 * 
 * DESCRIPTION: function to read data from the keyboard, file, device (RTC), or directory
 * 
 * Input: int32_t fd - file descriptor of device being used
 *        void* buf - buffer being written to (user level)
 *        int32_t nbytes - number of bytes being read off of keyboard, file, device, or directory
 * Output: Writes values from device internal buffer onto argument buffer (user level)
 * Return Values: Number of bytes read
 * 
 * SIDE EFFECTS: N/A
 */
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes) {
    /* set read_flag to bypass enter clear in keyboard handler */
    read_flag = 1;

    int32_t num_bytes_read = 0;

     /* check for valid argument */
    if (buf == NULL || nbytes < 0)
        return -1;
    
    /* cast void* argument to int8_t* for modification */
    int8_t* buffer = (int8_t*) buf; 

    int i;
    /* special case for ctrl+L */
    if(ctrl_L_flag) {
        /* restore values of internal buffer and buffer index before ctrl+L */
        memcpy(terminal[sched_term].internal_buffer, placeholder_buf, MAX_BUFFER_SIZE);
        terminal[sched_term].buffer_index = placeholder_buf_index;
        /* Redisplay all characters previously in the buffer */
        for (i = 0; i < terminal[sched_term].buffer_index; i++)
            putc(terminal[sched_term].internal_buffer[i]);
        /* reset ctrl+L flag */
        ctrl_L_flag = 0;
    }

    sti();
    /* wait for enter press */
    while(!terminal[sched_term].enter_flag);
    terminal[sched_term].enter_flag = 0;
    cli();

    /* count number of bytes typed */
    for (i = 0; i < terminal[sched_term].buffer_index && i < (nbytes - 1); i++) {
        num_bytes_read++;
    }

    /* copies current internal buffer into argument; inserts NEWLINE at end */
    memcpy(buffer, terminal[sched_term].internal_buffer, num_bytes_read);
    buffer[num_bytes_read++] = NEWLINE;

    /* clears internal buffer and resets buffer index */
    memset(terminal[sched_term].internal_buffer, '\0', MAX_BUFFER_SIZE);
    terminal[sched_term].buffer_index = 0;

    /* reset read_flag for next enter press */
    read_flag = 0;

    /* return number of bytes read */
    return num_bytes_read;
}


/* 
 * terminal_write
 * 
 * DESCRIPTION: function to write data to the a device (RTC) or terminal
 * 
 * Input: int32_t fd - file descriptor of device being used
 *        const void* buf - buffer being written from
 *        int32_t nbytes - number of bytes being written from argument buffer to device/terminal
 * Output: Writes onto device/terminal using putc
 * Return Values: Number of bytes typed
 * 
 * SIDE EFFECTS: Updates value of internal buffer
 */
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes) {
    int32_t num_bytes_typed = 0;
    
    /* check for valid argument */
    if (buf == NULL || nbytes < 0)
        return -1;
    

    /* cast void* argument to int8_t* for modification */
    int8_t* buffer = (int8_t*) buf; 
    
    /* iterate through all characters to be written */
    int i;
    for (i = 0; i < nbytes; i++) {
        /* do not print null characters */
        if (buffer[i] == '\0')
            continue;
        
        /* write characters to screen */
        putc(buffer[i]);
        num_bytes_typed++;
    }

    /* return number of bytes typed */
    return num_bytes_typed;
}


/* 
 * terminal_close
 * 
 * DESCRIPTION: Closes the specified file descriptor for access by another open
 * 
 * Input: int32_t fd - file descriptor of device being used
 * Output: N/A
 * Return Values: 0 if successful close, -1 if unsuccesful
 * 
 * SIDE EFFECTS: closes the file descriptor passed in
 */
int32_t terminal_close (int32_t fd) {
    return 0;
}
