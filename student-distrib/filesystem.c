/* filesystem.c - Defines structures to access filesystem in memory
 * vim:ts=4 noexpandtab
 */

#include "filesystem.h"
#include "lib.h"

/* Global variables to store starting addresses */
uint32_t boot_addr;                 // address pointing to start of boot block
uint32_t dentry_addr;               // address pointing to start of dentries in boot block
uint32_t inode_addr;                // address pointing to start of inode blocks
uint32_t data_addr;                 // address pointing to start of data blocks

/* Global variables to store information about file system */
dentry_t dentries[MAX_DENTRIES];    // list of dentries retrieved from boot block
uint32_t num_dentries;              // number of actual dentries as specified in boot block
uint32_t num_inodes;                // number of inode blocks (N)
uint32_t num_data;                  // number of data blocks (D)

/* 
 * init_fs
 * 
 * DESCRIPTION: initializes all global variables and file system structures
 * 
 * INPUT: the starting address of the filesystem
 * OUTPUT: none
 * RETURN VALUE: none
 * 
 * SIDE EFFECTS: none
 */
void init_fs(uint32_t mods_addr) {
    // initialize global pointers to boot, dentry, and inode blocks (data blocks initialized later)
    boot_addr = mods_addr; 
    dentry_addr = boot_addr + BYTE_64;     
    inode_addr = boot_addr + KBYTE_4;

    // parse boot block via boot_addr 
    memcpy(&num_dentries, (void *) boot_addr, BYTE_4);
    memcpy(&num_inodes, (void *) (boot_addr + BYTE_4), BYTE_4);
    memcpy(&num_data, (void *) (boot_addr + BYTE_8), BYTE_4);

    // copy in dentries to dentry array in boot
    memcpy(dentries, (void *) dentry_addr, BYTE_64 * MAX_DENTRIES);

    // initialize data block address
    data_addr = inode_addr + KBYTE_4 * num_inodes;

    // set dentries read to 0
    dentries_read = 0;
}

/* 
 * read_dentry_by_index
 * 
 * DESCRIPTION: takes in an index in boot block, fills dentry struct with correct
 * file information
 * 
 * INPUTS: index of dentry in boot block, dentry struct
 * OUTPUTS: none
 * RETURN VALUE: -1 for fail, 0 for success
 * 
 * SIDE EFFECTS: stores the directory entry specified by index into dentry
 */
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry) {
    // validate index
    if (index >= num_dentries) 
        return -1;

    // use index to find correct dentry in boot block
    *dentry = dentries[index];
    return 0; 
}

/* 
 * read_dentry_by_name
 * 
 * DESCRIPTION: gets dentry by name, fills dentry struct with 
 * 
 * INPUT: file name of dentry, dentry struct
 * OUTPUT: none
 * RETURN VALUE: -1 for fail, 0 for success
 * 
 * SIDE EFFECT: stores the directory entry specified by the file name 
 * into dentry 
 */
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry) {
    // temp string to record file name for each dentry in dentry array
    uint8_t temp_fname[BYTE_33]; 
    temp_fname[BYTE_32] = '\0';
    memset(temp_fname, '\0', BYTE_33);

    // iterate through all valid dentries 
    int i;
    for (i = 0; i < num_dentries; i++) {
        // store dentry's file name into temp_fname
        strncpy((int8_t*) temp_fname, (int8_t*) dentries[i].file_name, BYTE_32);
        
        if (strlen((int8_t*) fname) == strlen((int8_t*) temp_fname)) {
            if (strncmp((int8_t*) temp_fname, (int8_t*) fname, strlen((int8_t*) fname)) == 0) {
                // dentry found, set passed dentry to this dentry
                *dentry = dentries[i];
                return 0; 
            }
        }
    }
    
    // dentry not found
    return -1;
}

/* 
 * read_data
 * 
 * DESCRIPTION: read data from file given some inode, offset from start of the first
 * data block, and a length (number of bytes) into a buffer
 * 
 * INPUT: inode number, offset (in bytes from start of first data block), a buffer
 * of bytes of data, length = number of bytes to copy into buffer
 * OUTPUT: none
 * RETURN VALUE: returns number of bytes read
 * 
 * SIDE EFFECT: stores data in file into buffer
 */

int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length){
    // given inode index, obtain address of inode in memory
    uint32_t current_inode_addr = inode_addr + inode * KBYTE_4;
    inode_t current_inode;

    // validate if inode is within defined inode range (as in boot block)
    if (inode >= num_inodes)
        return 0;
    
    // obtain the inode length and the number of data blocks in the inode
    current_inode.length = *((uint32_t*)current_inode_addr);
    current_inode.num_data_blocks = (current_inode.length / KBYTE_4) + 1; 

    // if the offset is greater than the total bytes then return -1    
    if (offset > current_inode.length) 
        return 0;  
    
    // gets the address of the first data block to read from
    uint32_t start_block_index = offset / KBYTE_4; 
    uint32_t start_block_num = *((uint32_t*)(current_inode_addr + (start_block_index + 1) * BYTE_4)); 
    uint32_t start_block_addr = data_addr + (KBYTE_4 * start_block_num);
    
    // sets current_block_addr to the address of the first data block
    uint32_t current_block_addr = start_block_addr;  
    
    // iterate through data in the data blocks
    int bytes_read;
    int next_block_index = 1; 
    int block_offset = start_block_index;
    for (bytes_read = 0; bytes_read < length; bytes_read++) {
        // check if EOF has occurred (current bytes read + offset is larger than length of file)
        if (bytes_read + offset > current_inode.length) 
            return bytes_read;

        // if offset + current length read is over the current data block, move to the next data block
        if ((offset + bytes_read) / KBYTE_4 != block_offset) {
            // update the current data block address
            uint32_t current_block_num = *((uint32_t*)(current_inode_addr + BYTE_4 + (start_block_index + next_block_index) * BYTE_4));
            current_block_addr = data_addr + (KBYTE_4 * current_block_num);
            next_block_index++;

            // update the previous offset
            block_offset = (offset + bytes_read) / KBYTE_4;
        }
        
        // write data into buffer
        buf[bytes_read] = *((uint8_t*)(current_block_addr + ((offset + bytes_read) % KBYTE_4)));
    }

    // return the number of bytes read
    return bytes_read;
}


/* SYSTEM CALLS FOR FILES */

/* 
 * open_file
 *
 * DESCRIPTION: does nothing (for now)
 * 
 * INPUT: a filename stored in an array of chars
 * OUTPUT: none
 * RETURN VALUE: 0 for success and -1 for write
 * 
 * SIDE EFFECTS: none
 */
int32_t open_file(const uint8_t* filename) {
    return 0;
}

/* 
 * close_file
 *
 * DESCRIPTION: does nothing (for now)
 * 
 * INPUT: a file descriptor
 * OUTPUT: none
 * RETURN VALUE: 0 for success and -1 for write
 * 
 * SIDE EFFECTS: none
 */
int32_t close_file(int32_t fd) {
    return 0;
}

/* 
 * write_file
 * 
 * DESCRIPTION: does nothing (for now)
 * 
 * INPUT: a file descriptor, buffer of things to write, number of bytes to be written
 * OUTPUT: none
 * RETURN VALUE: 0 for success and -1 for write
 * 
 * SIDE EFFECTS: none
 */
int32_t write_file(int32_t fd, const void* buf, int32_t nbytes) {
    return 0;
}

/* 
 * read_file
 * 
 * DESCRIPTION: reads file, reads nbytes of data from file into buf
 * utilizes the read_data function
 * 
 * INPUT: filename, offset (in bytes), buffer, and number of bytes to be read
 * OUTPUT: none
 * RETURN VALUE: number of bytes read
 * 
 * SIDE EFFECTS: reads nbytes of file and stores it into buffer
 */
int32_t read_file(const uint8_t* filename, uint32_t offset, uint8_t* buf, int32_t nbytes) {
    // record dentry object for filename
    dentry_t dentry;

    // if dentry doesn't exist, return -1
    if (read_dentry_by_name(filename, &dentry) == -1) 
        return -1;
    
    // dentry exists, call read_data to extract data 
    return read_data(dentry.inode_num, offset, buf, nbytes);
}


/* SYSTEM CALLS FOR DIRECTORIES */

/* 
 * open_directory
 *
 * DESCRIPTION: does nothing (for now)
 * 
 * INPUT: a filename stored in an array of chars
 * OUTPUT: none
 * RETURN VALUE: 0 for success and -1 for write
 * 
 * SIDE EFFECTS: none
 */
int32_t open_directory(const uint8_t* filename) {
    return 0;
}

/* 
 * close_directory
 *
 * DESCRIPTION: does nothing (for now)
 * 
 * INPUT: a file descriptor
 * OUTPUT: none
 * RETURN VALUE: 0 for success and -1 for write
 * 
 * SIDE EFFECTS: none
 */
int32_t close_directory(int32_t fd) {
    return 0;
}

/* 
 * write_directory
 * 
 * DESCRIPTION: does nothing (for now)
 * 
 * INPUT: a file descriptor, buffer of things to write, number of bytes to be written
 * OUTPUT: none
 * RETURN VALUE: 0 for success and -1 for write
 * 
 * SIDE EFFECTS: none
 */
int32_t write_directory(int32_t fd, const void* buf, int32_t nbytes) {
    return 0;
}

/* 
 * read_directory
 * 
 * DESCRIPTION: reads directory, prints files filename by filename
 * utilizes the read_data_by_index
 * 
 * INPUT: buffer to store a file names (buffer could be of any length), the length of the buffer
 * OUTPUT: none
 * RETURN VALUE: number of bytes read, 0 on completion of reading directory
 * 
 * SIDE EFFECTS: buffer will hold a new filename
 */

int32_t read_directory(uint8_t* buf, uint32_t nbytes) {
    // make sure the number of dentries read is not over number of actual dentries
    if (dentries_read == num_dentries) {
        dentries_read = 0;  // reset number of dentries read
        return 0;           // return successful read of directory
    }

    // obtain size of buffer
    // uint32_t len_buf = strlen((int8_t*) buf);
    uint32_t len_filename = strlen((int8_t*) dentries[dentries_read].file_name);

    // find the number of bytes to copy into the buffer
    uint32_t num_bytes_copy;
    if (nbytes > BYTE_32)
        num_bytes_copy = BYTE_32;
    else {
        if (nbytes > len_filename)
            num_bytes_copy = len_filename;
        else
            num_bytes_copy = nbytes;
    }

    // copy filename into buffer at 32-byte intervals 
    strncpy((int8_t*) (buf), (int8_t *) dentries[dentries_read].file_name, num_bytes_copy);             

    // increment dentries_read
    dentries_read++;

    return num_bytes_copy;
}



/* SYSTEM CALLS FOR FILESYSTEM DRIVER */

/* 
 * fs_open
 * 
 * DESCRIPTION: opens a filesystem (but really does nothing)
 * 
 * INPUT: filename of file to open
 * OUTPUT: none
 * RETURN VALUE: 0 for success 
 * 
 * SIDE EFFECTS: none
 */
int32_t fs_open(const uint8_t* filename) {
   
    dentry_t dummy_dentry;
    if (read_dentry_by_name(filename, &dummy_dentry) == -1) {
        return -1;
    }

    return 0;
}

/* 
 * fs_close
 * 
 * DESCRIPTION: does nothing
 * 
 * INPUT: a file descriptor
 * OUTPUT: none
 * RETURN VALUE: 0 for success
 * 
 * SIDE EFFECTS: none
 */
int32_t fs_close(int32_t fd) {
    return 0;
}

/* 
 * fs_write
 * 
 * DESCRIPTION: does nothing
 * 
 * INPUT: file descriptor, buffer, number of bytes to write
 * OUTPUT: none
 * RETURN VALUE: -1 for failure
 * 
 * SIDE EFFECTS: none
 */
int32_t fs_write(int32_t fd, const void* buf, int32_t nbytes) {
    return -1;
}

/* 
 * fs_read
 * 
 * DESCRIPTION: given a file descriptor, fs_read will decide whether
 * to redirect to a directory read or a file read based on the file type
 * of the file specified
 * 
 * INPUT: file descriptor 
 * OUTPUT: none
 * RETURN VALUE: returns number of bytes read 
 * 
 * SIDE EFFECTS: none
 */
int32_t fs_read(int32_t fd, void* buf, int32_t nbytes) {
    // obtain inode number from file descriptor
    pcb_t* curr_pcb = get_curr_pcb();
    uint32_t inode_num = curr_pcb -> fd_array[fd].inode;
    uint32_t offset = curr_pcb -> fd_array[fd].file_position;
    dentry_t* dentry = NULL;  
    
    // obtain dentry given an inode
    int i;
    for (i = 0; i < num_dentries; i++) {
        // inode found, store dentry
        if (inode_num == dentries[i].inode_num) {
            dentry = &(dentries[i]); 
            break;
        }
    }

    // check if the dentry was found
    if (dentry == NULL) 
        return -1;

    // obtain filename terminated by NULL character
    uint8_t filename[BYTE_33];
    memset(filename, '\0', BYTE_33);
    strncpy((int8_t*) filename, (int8_t*) dentry->file_name, BYTE_32);

    // redirect the read based on wehther we read a directory or file
    int32_t bytes_read;
    if (dentry->file_type == FILE_TYPE || dentry->file_type == RTC_TYPE){
        // type of file is normal file
        bytes_read = read_file(filename, offset, buf, nbytes);
    } else if (dentry->file_type == DIR_TYPE) {
        // type of file is directory
        bytes_read = read_directory(buf, nbytes);
    } else {
        // file type was invalid
        return -1;
    }

    // update the current process's file offset
    if(bytes_read > 0)
        curr_pcb->fd_array[fd].file_position += bytes_read;

    // return the number of bytes read from the file
    return bytes_read;
}
