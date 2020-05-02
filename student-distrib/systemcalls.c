#include "systemcalls.h"
#include "terminal.h"
#include "lib.h"

/* Keeps track of the current number of processes active */
static uint32_t pid_array[MAX_PROC] = {0, 0, 0, 0, 0, 0};

/* OPERATION TABLES */
static fops_t terminal_ops_table = {bad_call_open, terminal_read, terminal_write, bad_call_close};
static fops_t rtc_ops_table = {rtc_open, rtc_read, rtc_write, rtc_close};
static fops_t directory_ops_table = {fs_open, fs_read, fs_write, fs_close};
static fops_t file_ops_table = {fs_open, fs_read, fs_write, fs_close};

/* 
 * get_curr_pcb
 * 
 * DESCRIPTION: returns the pcb_t of the currently running
 * process
 * 
 * Input: none
 * Output: none
 * Return Values: a pointer to the pcb_t of the currently
 * running process
 * 
 * SIDE EFFECTS: N/A
 */
pcb_t* get_curr_pcb() {
    return terminal[sched_term].curr_pcb;
}

/* 
 * set_curr_pcb
 * 
 * DESCRIPTION: sets the current pcb as the argument
 * 
 * Input: pcb_t* new_pcb - pcb struct pointer to set to 
 * Output: none
 * Return Values: none
 * running process
 * 
 * SIDE EFFECTS: N/A
 */
void set_curr_pcb(pcb_t* new_pcb) {
    terminal[sched_term].curr_pcb = new_pcb;
}

/* 
 * bad_call_open
 * 
 * DESCRIPTION: dummy functions, returns -1
 * 
 * Input: const uin8_t* filename - match open parameters
 * Output: none
 * Return Values: -1 always
 * 
 * SIDE EFFECTS: N/A
 */
int32_t bad_call_open(const uint8_t* filename) {
    return -1;
}

/* 
 * bad_call_close
 * 
 * DESCRIPTION: dummy functions, returns -1
 * 
 * Input: int32_t fd - match open parameters
 * Output: none
 * Return Values: -1 always
 * 
 * SIDE EFFECTS: N/A
 */
int32_t bad_call_close(int32_t fd) {
    return -1;
}

/* 
 * halt
 * 
 * DESCRIPTION: system call that halts a child process
 * and returns control back to parent process
 * 
 * Input: status (whether program ended with exception
 * or not)
 * Output: none
 * Return Values: jumps to execute so technically none
 * 
 * SIDE EFFECTS: N/A
 */
int32_t halt (uint8_t status) {
    // clear FD array of process
    int i; 
    for (i = 0; i < FD_ARRAY_SIZE; i++) {
        close(i);
    }
    
    /* Restore PID array */
    pid_array[ terminal[sched_term].curr_pcb -> pid ] = 0;

    /* execute shell if no processes are running */
    if (terminal[sched_term].curr_pcb->parent_pcb == NULL) {
        terminal[sched_term].curr_pcb = NULL;
        execute((uint8_t*)"shell");
    }

    /* restore parent PCB and set it in terminal_proc */
    terminal[sched_term].curr_pcb = terminal[sched_term].curr_pcb -> parent_pcb;

    /* Set page base address */
    page_directory[USER_PAGE] = KERNEL_MEM_END + ((terminal[sched_term].curr_pcb -> pid) * _4MB_);
    /* Set attributes of new page */
    page_directory[USER_PAGE] |= FOUR_MB_PAGE | USER | RW | PRESENT;
    /* Flush the TLB */
    flush_tlb();
    
    /* Load TSS segment with kernel stack for parent process */
    tss.ss0 = KERNEL_DS;
    tss.esp0 = (uint32_t)(KERNEL_MEM_END - (terminal[sched_term].curr_pcb ->pid) * _8KB_) - BYTE_4;

    /* store 256 into status if exception has been raised */
    uint32_t status_exp = (uint32_t) status;
    if (exception_flag) 
        status_exp++;

    /* reset the number of dentries read to 0 */
    dentries_read = 0;

    /* clears internal buffer and resets buffer index */
    memset(terminal[sched_term].internal_buffer, '\0', MAX_BUFFER_SIZE);
    terminal[sched_term].buffer_index = 0;

    /* restore ESP and EBP of parent process (now current process) then jump to end of execute */
    asm volatile ("     \n\
        movl %0, %%esp  \n\
        movl %1, %%ebp  \n\
        movl %2, %%eax  \n\
        jmp EXEC_FIN"
        :
        : "r" (terminal[sched_term].curr_pcb -> esp), "r" (terminal[sched_term].curr_pcb -> ebp), "r" (status_exp)
    );

    return 0;
}

/* 
 * execute
 * 
 * DESCRIPTION: creates a new child process and executes it 
 * 
 * Input: a command used for execution
 * Output: none
 * Return Values: returns status of child process when it ends 
 * (stored in EAX from halt inline assembly)
 * 
 * SIDE EFFECTS: creates a new process (PCB) and executes it
 */
int32_t execute(const uint8_t* command) {
    /* lock so execute cant be interrupted */
    cli();

    /* validate command is valid */
    if (command == NULL)
        return -1;
    
    /* Reset exception flag */
    exception_flag = 0;

    /* store the esp and ebp of the current process */
    if (terminal[sched_term].active && terminal[sched_term].curr_pcb != NULL) {
        asm volatile ("      \n\
            movl %%esp, %0   \n\
            movl %%ebp, %1"
            : "=r" (terminal[sched_term].curr_pcb -> esp), "=r" (terminal[sched_term].curr_pcb -> ebp) 
        );
    }

    terminal[sched_term].active = 1;

    /* Set up filename and args variables */
    uint8_t filename[FILE_NAME_CHAR];
    uint8_t args[MAX_BUFFER_SIZE];
    dentry_t dentry;
    uint32_t retval;
    
    /* parse command arguments into filename and args */
    execute_parse_args(filename, args, command);

    /* if read_dentry_by_name unsuccessful, return -1 */
    if (read_dentry_by_name(filename, &dentry) == -1)
        return -1;

    /* check if file is an executable [ELF] */
    if (execute_executable_check(filename))
        return -1;

    /* find next available PID for process */
    int8_t new_pid;
    if ((new_pid = execute_find_pid()) == -1) {
        printf("PID Array is Full\n");
        return -1;
    }

    /* sets up correct paging for shell / user function */
    execute_program_paging(new_pid);

    /* copying program image from filename to PROGRAM_IMAGE_ADDR */
    execute_user_level_program_loader(filename);
    
    /* create a new PCB for process */
    execute_create_pcb(&dentry, filename, args, new_pid);

    /* context switch (trick IRET) to run other process */
    execute_context_switch(filename);

    /* child process has called "halt" with status code, return control to parent */
    asm volatile (" \n\
        EXEC_FIN:   \n\
        leave       \n\
        ret"
        : "=a" (retval)
    );

    /* checks for exception flag, returns 256 if set */
    if (exception_flag)
        retval = EXCEPTION_OCCURRED;

    return retval;
}

/* 
 * execute_parse_args
 * 
 * DESCRIPTION: helper function for execute system call, retrieves
 * filename, args, and command given a buffer
 * 
 * Input: stores the filename in filename_buf
 * Output: none
 * Return Values: none
 * 
 * SIDE EFFECTS: N/A
 */
void execute_parse_args(uint8_t* filename_buf, uint8_t* args_buf, const uint8_t* command) {
    /* fill filename and args with nulls */
    memset(filename_buf, '\0', FILE_NAME_CHAR);
    memset(args_buf, '\0', MAX_BUFFER_SIZE);

    /* Set up string iterator */
    uint8_t* command_copy = (uint8_t*)command;
    uint8_t command_length = strlen((int8_t *) command_copy);

    /* set up looping variables */
    int i = 0;
    int filename_idx = 0;
    int args_idx = 0;

    /* strip leading spaces */
    while (command_copy[i] == SPACE && i < command_length) 
        i++;

    /* fill up filename buffer command */
    while (command_copy[i] != SPACE && i < command_length && filename_idx < FILE_NAME_CHAR) {
        memcpy(filename_buf + filename_idx, command_copy + i, 1);
        i++;
        filename_idx++;
    }

    /* strip spaces in middle */
    while (command_copy[i] == SPACE && i < command_length) 
        i++;

    /* fill up args_buf with arguments */
    while (i < command_length && args_idx < MAX_BUFFER_SIZE) {
        memcpy(args_buf + args_idx, command_copy + i, 1);
        i++;
        args_idx++;
    }
}

/* 
 * execute_executable_check
 * 
 * DESCRIPTION: helper function for system call execute,
 * checks that the filename passed in refers
 * to an executable. It does this by checking the first 4 bytes
 * are equal to the magic numbers specified in documentation
 * 
 * Input: filename
 * Output: none
 * Return Values: 0 if executable, else if not
 * 
 * SIDE EFFECTS: N/A
 */
int32_t execute_executable_check(uint8_t* filename) {
    /* create and clear a buffer of size 3 to store ELF */
    uint8_t buf[ELF_LENGTH];
    memset(buf, '\0', ELF_LENGTH);

    /* check for correct bytes read (should be 3) */
    uint32_t bytes_read = read_file(filename, ELF_OFFSET, buf, ELF_LENGTH);
    if (bytes_read < ELF_LENGTH) 
        return -1;

    /* return 0 if ELF matches buffer */
    return strncmp((int8_t*)buf, "ELF", ELF_LENGTH) ? -1 : 0;
}

/*
 * execute_find_pid
 * 
 * DESCRIPTION: execute helper function, gets the lowest available PID
 * from the PID array
 * 
 * Input: none
 * Output: none
 * Return value: lowest available PID, -1 if no PID is available
 * 
 * SIDE EFFECTS: none
 */
int8_t execute_find_pid() {
    // parse PID array, find first PID with flag 0 (not in use)
    int i; 
    for (i = 0; i < MAX_PROC; i++) {
        if (pid_array[i] == 0) {
            // available PID found, set flag to 1 (in use)
            pid_array[i] = 1;
            return i;
        }
    }

    // no slots for PID array available
    return -1;
}

/* 
 * execute_program_paging
 * 
 * DESCRIPTION: helper function for system call execute,
 * maps virtual address to physical space corresponding to 
 * the new process
 * 
 * Input: PID for new process
 * Output: none
 * Return Values: none
 * 
 * SIDE EFFECTS: Remaps virtual address 0x8048000 to physical 
 * address space of new process
 */
int32_t execute_program_paging(int8_t new_pid) {   
    /* Set page base address */
    page_directory[USER_PAGE] = KERNEL_MEM_END + (new_pid * _4MB_);
    /* Set attributes of new page */
    page_directory[USER_PAGE] |= FOUR_MB_PAGE | USER | RW | PRESENT;
    /* Flush the TLB */
    flush_tlb();

    return 0;
}

/* 
 * execute_user_level_program_loader
 * 
 * DESCRIPTION: helper function for execute,
 * loads the executable specified by filename into memory
 * 
 * Input: filename of executable
 * Output: none
 * Return Values: none
 * 
 * SIDE EFFECTS: loads program to memory
 */
void execute_user_level_program_loader(uint8_t* filename) {
    /* reads file program image into PROGRAM_IMAGE_ADDR */
    read_file(filename, 0, (uint8_t*)PROGRAM_IMAGE_ADDR, MAX_FILE_SIZE);
}

/* 
 * execute_create_pcb
 * 
 * DESCRIPTION: creates a new Process Control Block (PCB)
 * given a dentry. 
 * 
 * Input: dentry block of executable, filename buffer, args buffer, PID for new process
 * Output: none
 * Return Values: returns 0 if successful
 * 
 * SIDE EFFECTS: creates new pcb 
 */
int32_t execute_create_pcb(dentry_t* dentry, uint8_t* filename, uint8_t* args, int8_t new_pid) {
    int i;

    /* initialize starting location of PCB */
    pcb_t* new_pcb = (pcb_t*)(KERNEL_MEM_END - (new_pid + 1) * _8KB_);
    
    for (i = 0; i < FD_ARRAY_SIZE; i++) {
        /* set terminal table and flags if stdin or stdout */
        if (i == 0 || i == 1) {
            new_pcb -> fd_array[i].file_operations_table_ptr = terminal_ops_table;
            new_pcb -> fd_array[i].flags = 1;
        } else {
            new_pcb -> fd_array[i].file_operations_table_ptr = (fops_t) {NULL, NULL, NULL, NULL};
            new_pcb -> fd_array[i].flags = 0;
        }
        /* initialize inode and file position */
        new_pcb -> fd_array[i].inode = -1;
        new_pcb -> fd_array[i].file_position = 0;
    }
    
    /* updates parent_pcb based on current process running per terminal */
    new_pcb -> parent_pcb = terminal[sched_term].curr_pcb == NULL ? NULL : terminal[sched_term].curr_pcb;
    new_pcb -> pid = new_pid; 
    new_pcb -> terminal_id = sched_term;

    /* set starting address for kernel stack and kernel base pointers */
    /* esp and ebp held 4 behind the program image */
    new_pcb -> esp = (PROGRAM_IMAGE_ADDR & PAGE_DIR_MASK) + _4MB_ - BYTE_4;
    new_pcb -> ebp = (PROGRAM_IMAGE_ADDR & PAGE_DIR_MASK) + _4MB_ - BYTE_4;
    
    // memcpy(new_pcb->filename, filename, FILE_NAME_CHAR);
    strcpy((int8_t*)(new_pcb->args), (const int8_t*)args);

    /* assigns newly created pcb as the current pcb */
    terminal[sched_term].curr_pcb = new_pcb;

    return 0;
}

/* 
 * execute_context_switch
 * 
 * DESCRIPTION: sets us TSS, finds entry point into executable,
 * and pushes user context onto stack for IRET. Once IRET is called,
 * child process will begin to run
 * 
 * Input: filename of executable
 * Output: none
 * Return Values: technically none, "return 0" is never reached
 * 
 * SIDE EFFECTS: N/A
 */
int32_t execute_context_switch(uint8_t* filename) {
    // find entry point into file (bytes 24-27 in file executable)
    uint8_t entry_point_string[BYTE_4];
    uint32_t entry_point;
    read_file(filename, ENTRY_POINT, entry_point_string, BYTE_4);
    entry_point = *((uint32_t *) entry_point_string);

    // Load TSS segment with kernel stack for the process about to run
    tss.ss0 = KERNEL_DS;
    tss.esp0 = (uint32_t)(KERNEL_MEM_END - (terminal[sched_term].curr_pcb -> pid) * _8KB_ - BYTE_4);

    // push kernel DS, ESP, EFLAG, kernel CS
    asm volatile (" \n\
        pushl %0    \n\
        pushl %1    \n\
        sti         \n\
        pushfl      \n\
        pushl %2    \n\
        pushl %3    \n\
        iret"
        :
        : "r" (USER_DS), "r" (USER_STACK), "r" (USER_CS), "r" (entry_point)
    );

    // return success
    return 0;
}

/* 
 * open 
 * 
 * DESCRIPTION: open system call, opens a file and allocates
 * a new file descriptor in current_pcb
 * 
 * Input: filename of file to open
 * Output: none
 * Return Values: -1 on failure, 0 on success
 * 
 * SIDE EFFECTS: N/A
 */
int32_t open (const uint8_t* filename) {
    dentry_t dentry;
    int32_t fd;

    /* fill in dentry by searching dentries via filename */
    if (read_dentry_by_name(filename, &dentry) == -1)
        return -1;


    /* find an fd that is not in use */
    for (fd = 0; fd < FD_ARRAY_SIZE; fd++) {
        if (terminal[sched_term].curr_pcb -> fd_array[fd].flags == 0) {
            break;
        }
    }

    /* check if an available slot in FD array was found */
    if(fd == FD_ARRAY_SIZE)
        return -1;

    /* set relevant jump table based on file type */
    switch(dentry.file_type) {
        case RTC_TYPE:
            terminal[sched_term].curr_pcb -> fd_array[fd].file_operations_table_ptr = rtc_ops_table;
            break;
        case DIR_TYPE:
            terminal[sched_term].curr_pcb -> fd_array[fd].file_operations_table_ptr = directory_ops_table;
            break;
        case FILE_TYPE:
            terminal[sched_term].curr_pcb -> fd_array[fd].file_operations_table_ptr = file_ops_table;
            break;
        default:
            break;
    }

    /* initialize inode and flags */
    terminal[sched_term].curr_pcb -> fd_array[fd].inode = dentry.inode_num;
    terminal[sched_term].curr_pcb -> fd_array[fd].file_position = 0;
    terminal[sched_term].curr_pcb -> fd_array[fd].flags = 1;

    /* Call function specific open, return -1 if it fails */
    if (terminal[sched_term].curr_pcb -> fd_array[fd].file_operations_table_ptr.open(filename) == -1)
        return -1;

    return fd;
}

/* 
 * read
 * 
 * DESCRIPTION: read system call, reads a file
 * 
 * Input: file descriptor (index into file descriptor array of currently
 * running process), a buffer to read into, number of bytes to read
 * Output: none
 * Return Values: number of bytes read, -1 on fail
 * 
 * SIDE EFFECTS: N/A
 */
int32_t read (int32_t fd, void* buf, int32_t nbytes) {
    /* Check if given file descriptor is in bounds */
    if (fd < 0 || fd >= FD_ARRAY_SIZE)
        return -1;

    /* checks for fd != stdout */
    if (fd == 1)
        return -1;
    
    /* checks if not in use */
    if (terminal[sched_term].curr_pcb -> fd_array[fd].flags == 0)
        return -1;

    /* returns function call for given file descriptor with function parameters */
    return terminal[sched_term].curr_pcb -> fd_array[fd].file_operations_table_ptr.read(fd, buf, nbytes);
}

/* 
 * write
 * 
 * DESCRIPTION: write system call, calls file specific write 
 * onto a file
 * 
 * Input: file descriptor (index into file descriptor array of currently
 * running process), a buffer to write from, number of bytes to read
 * Output: none
 * Return Values: -1 on failure, 0 on success
 * 
 * SIDE EFFECTS: N/A
 */
int32_t write (int32_t fd, const void* buf, int32_t nbytes) {
    /* Check if given file descriptor is in bounds */
    if (fd < 0 || fd >= FD_ARRAY_SIZE)
        return -1;
    /* checks for fd != stdin */
    if (fd == 0)
        return -1;
    /* checks if not in use */
    if (terminal[sched_term].curr_pcb -> fd_array[fd].flags == 0)
        return -1;
    /* returns function call for given file descriptor with function parameters */
    return terminal[sched_term].curr_pcb -> fd_array[fd].file_operations_table_ptr.write(fd, buf, nbytes);
}

/* 
 * close
 * 
 * DESCRIPTION: closes a file that was previously open
 * 
 * Input: file descriptor (index into file descriptor array)
 * Output: none
 * Return Values: -1 on failure, 0 on success
 * 
 * SIDE EFFECTS: N/A
 */
int32_t close (int32_t fd) {
    /* Check if given file descriptor is in bounds */
    if (fd < 0 || fd >= FD_ARRAY_SIZE)
        return -1;
    /* checks for fd != stdin or stdout */
    if (fd == 0 || fd == 1)
        return -1;
    /* checks if not in use */
    if (terminal[sched_term].curr_pcb -> fd_array[fd].flags == 0)
        return -1;
    /* function call for given file descriptor with function parameters */
    if(terminal[sched_term].curr_pcb -> fd_array[fd].file_operations_table_ptr.close(fd))
        return -1;

    /* Set as not in use */
    terminal[sched_term].curr_pcb -> fd_array[fd].flags = 0;

    return 0;
}

/* 
 * getargs
 * 
 * DESCRIPTION: reads the program's command line 
 *              arguments into a user-level buffer
 * 
 * Input: buf - user level buffer to read arguments into
 *        nbytes - number of bytes to be read
 * Output: none
 * Return Values: 0 for success, -1 for failure
 * 
 * SIDE EFFECTS: N/A
 */
int32_t getargs (uint8_t* buf, int32_t nbytes) {
    /* check if buffer is null or bytes to be read is less than 0 */
    if (buf == NULL || nbytes <= 0)
        return -1;

    /* check if args buffer length is no 0 */
    if (strlen((const int8_t*) terminal[sched_term].curr_pcb -> args) == 0) 
        return -1; 

    /* check if whole args can fit into buffer */
    if (strlen((const int8_t*)(terminal[sched_term].curr_pcb -> args)) > nbytes)
        return -1;

    /* copy arguments into user level buffer */
    strcpy((int8_t*)buf, (const int8_t*)(terminal[sched_term].curr_pcb -> args));
    return 0;
}

/* 
 * vidmap
 * 
 * DESCRIPTION: maps the text-mode video memory into 
 *              user space at a pre-set virtual address
 * 
 * Input: screen_start - pre-set virtual address in user space
 * Output: none
 * Return Values: 0 for success, -1 for failure
 * 
 * SIDE EFFECTS: N/A
 */
int32_t vidmap (uint8_t** screen_start) {
    /* check if argument is not null */
    if(screen_start == NULL)
        return -1;
    /* address should not fall in the user-level page */
    if(((uint32_t)screen_start & PAGE_DIR_MASK) != (PROGRAM_IMAGE_ADDR & PAGE_DIR_MASK))
        return -1;

    /* map 4kB video memory page */
    *screen_start = (uint8_t *)(USER_VID_MEM_PAGE << PAGE_BASE_ADDR_OFFSET);

    /* Flush the TLB */
    flush_tlb();
    return 0;
}

/* 
 * set_handler
 * 
 * DESCRIPTION: none
 * 
 * Input: none
 * Output: none
 * Return Values: none
 * 
 * SIDE EFFECTS: N/A
 */
int32_t set_handler (int32_t signum, void* handler_address) {
    /* Extra Credit */
    printf("Set_handler\n");
    return 0;
}

/* 
 * sig_return
 * 
 * DESCRIPTION: none
 * 
 * Input: none
 * Output: none
 * Return Values: none
 * 
 * SIDE EFFECTS: N/A
 */
int32_t sigreturn (void) {
    /* Extra Credit */
    printf("Sigreturn\n");
    return 0;
}
