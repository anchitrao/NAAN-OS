/* types.h - Defines to use the familiar explicitly-sized types in this
 * OS (uint32_t, int8_t, etc.).  This is necessary because we don't want
 * to include <stdint.h> when building this OS
 * vim:ts=4 noexpandtab
 */

#ifndef _TYPES_H
#define _TYPES_H

#define NULL 0

#ifndef ASM

/* Types defined here just like in <stdint.h> */
typedef int int32_t;
typedef unsigned int uint32_t;

typedef short int16_t;
typedef unsigned short uint16_t;

typedef char int8_t;
typedef unsigned char uint8_t;


/** MACROS **/
/* multiterminal.h */
#define TERMINAL_COUNT      3           /* maximum number of terminals open */

/* systemcalls.h */
#define FD_ARRAY_SIZE       8           /* Upto 8 open files at any given point */

/* paging.h */
#define PAGE_SIZE           KBYTE_4       /* 4096 bytes = 4KB per page */
#define PHYS_ADDR           0xFFFFF000    /* bitmask to switch screens */     

/* filesystem.h */
#define FILE_NAME_CHAR      32          /* file name is 32 characters */

/* byte size definitions */
#define _4MB_               0x00400000  /* 4MB = 4194304 bytes */
#define KBYTE_4             4096        /* 4kB block in bytes */
#define BYTE_64             64          /* size of 64 bytes in bytes */
#define BYTE_33             33          /* size of 33 bytes in bytes */
#define BYTE_32             32          /* size of 32 bytes in bytes */
#define BYTE_4              4           /*  size of 4 bytes in bytes */
#define BYTE_8              8           /* size of 8 bytes in bytes */

/* constants for file types */
#define RTC_TYPE            0       // rtc has a file type of 0
#define DIR_TYPE            1       // directory has file type of 1
#define FILE_TYPE           2       // normal file has file fype of 2

/* terminal.h */
#define MAX_BUFFER_SIZE     128         /* maximum size for internal buffer */

/** Structs **/
/* file operations table for system calls read, write, open, and close */
typedef struct file_operations_table {
	int32_t (*open)(const uint8_t* filename);
    int32_t (*read)(int32_t fd, void* buf, int32_t nbytes);
	int32_t (*write)(int32_t fd, const void* buf, int32_t nbytes);
	int32_t (*close)(int32_t fd);
} fops_t;

/* file descriptor struct */
typedef struct fd_struct {
    fops_t file_operations_table_ptr;
    uint32_t inode;
    uint32_t file_position;
    uint32_t flags;
} fd_array_t;

/* process control block (PCB) struct */
typedef struct process_control_block {
    fd_array_t fd_array[FD_ARRAY_SIZE];
    uint8_t filename[FILE_NAME_CHAR];
    uint8_t args[MAX_BUFFER_SIZE];
    uint32_t pid;
    struct process_control_block* parent_pcb;
    uint32_t esp;
    uint32_t ebp;
    uint8_t terminal_id;
} pcb_t;

/* struct to define the directory entries */
typedef struct {
    uint8_t file_name[FILE_NAME_CHAR];
    uint32_t file_type;
    uint32_t inode_num;
    uint8_t reserved[24];
} dentry_t;

/* struct to define the inode blocks */
typedef struct {
    uint32_t length;
    uint32_t num_data_blocks;
} inode_t;


/* MULTI TERMINAL */
volatile uint8_t curr_term;   // terminal currently being displayed
volatile uint8_t sched_term;  // terminal currently running

typedef struct terminal {
    /* library */
    int screen_x;
    int screen_y;
    char* video_mem;
    
    /* keyboard */
    uint8_t internal_buffer[MAX_BUFFER_SIZE];
    uint32_t buffer_index;
    uint8_t enter_flag;

    /* rtc */
    uint32_t rtc_constant;
    uint32_t rtc_iterations;

    /* processes */
    pcb_t* curr_pcb;
    uint8_t active;
} term_t;

term_t terminal[TERMINAL_COUNT];

#endif /* ASM */

#endif /* _TYPES_H */
