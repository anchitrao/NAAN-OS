#ifndef _SYSTEMCALLS_H
#define _SYSTEMCALLS_H

#include "types.h"
#include "systemcall_handler.h"
#include "filesystem.h"
#include "paging.h"
#include "rtc.h"
#include "x86_desc.h"
#include "exception_handler.h"

#define SPACE               32              /* Ascii value for space (' ') */
#define ELF_LENGTH          3               /* 3 bytes long, one per character in "ELF" */
#define ELF_OFFSET          1               /* first character is always 'del' before "ELF" */
#define PAGE_DIR_MASK       0xFFC00000      /* Mask to get just the highest 10 bits (page dir offset) of the address*/
#define _8KB_               0x00002000      /* 8KB = 8192 bytes */
#define PID_SIZE            8               /* Number of possible PIDs in our OS */
#define EXCEPTION_OCCURRED  256             /* Signifies exception occurred */
#define MAX_PROC            6               /* Maximum number of processes that can run at once */

/* Helper functions to get and set the current PCB */
pcb_t* get_curr_pcb();
void set_curr_pcb(pcb_t* new_pcb);

/* dummy function returns -1 for terminal_open */
int32_t bad_call_open(const uint8_t* filename);

/* dummy function returns -1 for terminal_close */
int32_t bad_call_close(int32_t fd);

/* exits the current process running */
int32_t halt (uint8_t status);

/* executes programs */
int32_t execute(const uint8_t* command);

/* [helper function] parses command into three seperate buffers*/
void execute_parse_args(uint8_t* filename_buf, uint8_t* args_buf, const uint8_t* command);

/* [helper function] checks elf file to see if executable program*/
int32_t execute_executable_check(uint8_t* filename);

/* [helper function] finds next available PID for new PCB */
int8_t execute_find_pid();

/* [helper function] sets up correct paging for shell / user function */
int32_t execute_program_paging(int8_t new_pid);

/* [helper function] maps the current program from virtual to physical memory */
void execute_user_level_program_loader();

/* [helper function] creates a new pcb for a new process */
int32_t execute_create_pcb(dentry_t* dentry, uint8_t* filename, uint8_t* args, int8_t new_pid);

/* [helper function] context switch (fool IRET) to run other process*/
int32_t execute_context_switch(uint8_t* filename);

/* Finds the file in the file system and assigns it an unassigned file descriptor */
int32_t open (const uint8_t* filename);

/* syscall function reads file based on the fd */
int32_t read (int32_t fd, void* buf, int32_t nbytes);

/* write system call, calls file specific write */
int32_t write (int32_t fd, const void* buf, int32_t nbytes);

/* Close the file descriptor passed in and set it to be available */
int32_t close (int32_t fd);

/* returns arguments passed to executable */
int32_t getargs (uint8_t* buf, int32_t nbytes);

/* sets a pointer to video memory */ 
int32_t vidmap (uint8_t** screen_start);

/* EXTRA CREDIT */
int32_t set_handler (int32_t signum, void* handler_address);

/* EXTRA CREDIT */
int32_t sigreturn (void);

#endif /* _SYSTEMCALLS_H */
