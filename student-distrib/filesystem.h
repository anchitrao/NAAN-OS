/* filesystem.h - Defines structures to access filesystem in memory
 * vim:ts=4 noexpandtab
 */

#include "types.h"
#include "multiboot.h"
#include "systemcalls.h"

#ifndef _FILESYS_H
#define _FILESYS_H

/* constants used to store arrays */
#define MAX_DENTRIES    63      // 63 dentries possible

/* constants used to denote metadata regarding start of program */
#define ENTRY_POINT     24      // EIP metadata is stored from bytes 24-27

/* Global variable for storing the number of dentries currently read */
uint32_t dentries_read;

/* function in order to take given data and organize it*/
void init_fs(uint32_t mods_addr);

/* function loads dentry information from a given file name */
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry);

/* loads dentry information given a index offset in build block */
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry);

/* reads data from a file and puts it in a buffer */
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

/* System calls to open, close, write, and read from a file */
int32_t open_file(const uint8_t* filename);
int32_t close_file(int32_t fd);
int32_t write_file(int32_t fd, const void* buf, int32_t nbytes);
int32_t read_file(const uint8_t* filename, uint32_t offset, uint8_t* buf, int32_t nbytes);

/* System Calls to open, close, write, and read from a directory */
int32_t open_directory(const uint8_t* filename);
int32_t close_directory(int32_t fd);
int32_t write_directory(int32_t fd, const void* buf, int32_t nbytes);
int32_t read_directory(uint8_t* buf, uint32_t nbytes);

/* System calls to open, close, write, and read the file system */
int32_t fs_open(const uint8_t* filename);
int32_t fs_close(int32_t fd);
int32_t fs_write(int32_t fd, const void* buf, int32_t nbytes);
int32_t fs_read(int32_t fd, void* buf, int32_t nbytes);

#endif /* _FILESYS_H defined */
