#include "tests.h"
#include "x86_desc.h"
#include "rtc.h"
#include "lib.h"
#include "idt.h"
#include "paging.h"
#include "terminal.h"
#include "filesystem.h"
#include "systemcalls.h"

#define PASS 1
#define FAIL 0

/* Used for paging tests */
#define UNDER_LOWER_BOUND 0
#define IN_BOUNDS 1
#define ABOVE_UPPER_BOUND 2

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test - Example
 * 
 * Asserts that all IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < NUM_VEC; ++i){
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}

	return result;
}

/* IDT Test - Divide By 0
 * 
 * Asserts that dividing by 0 throws an exception.
 * Inputs: None
 * Outputs: PASS
 * Side Effects: Should cause a divide by 0 error
 * 				 and freeze the screen.
 * Coverage: IDT definition
 * Files: idt.h/c, exception_handler.h/c
 */
int idt_divide_by_zero(){
	TEST_HEADER;

	int i;
	int remove_warning = 0;
	/* Should throw a divide by zero exception */
	i = 5/remove_warning;
	
	remove_warning = i;
	return PASS;
}

/* RTC Test - Periodic Interrupt Handling
 * 
 * Asserts that RTC interrupt handler is called regularly.
 * Inputs: None
 * Outputs: PASS
 * Side Effects: Should cause random characters to be written
 * 				 on the screen over and over again.
 * Coverage: RTC initialization and interrupt handler
 * Files: rtc.h/c, rtc_handler.h/S
 */
int rtc_handler_test() {
	TEST_HEADER;

	while(1) {
		test_interrupts();
	}
	return PASS;
}

/* Paging Test - Dereferencing NULL
 * 
 * Asserts that dereferencing a null pointer causes a page fault.
 * Inputs: None
 * Outputs: PASS
 * Side Effects: Should cause a page fault and freeze the screen.
 * Coverage: Paging initialization
 * Files: paging.h/c, paging_init_asm.h/S
 */
int paging_deref_null() {
	TEST_HEADER;

	int* ptr = NULL;
	/* Should Page Fault */
	int i = *ptr;

	int remove_warning;
	remove_warning = i;
	i = remove_warning;
	return PASS;
}

/* Paging Test - Accessing Kernel Memory
 * 
 * Asserts that kernel memory is accessible.
 * Inputs: None
 * Outputs: PASS
 * Side Effects: If testing UNDER_LOWER_BOUND or ABOVE_UPPER_BOUND,
 * 				 a page fault should occur and freeze the screen.
 * Coverage: Page Directory set up
 * Files: paging.h/c, paging_init_asm.h/S
 */
int paging_ker_mem(int test) {
	TEST_HEADER;

	int* ptr;
	int i;

	switch(test) {
		case UNDER_LOWER_BOUND:
			/* Should Page Fault */
			printf("UNDER_LOWER_BOUND: ");
			ptr = (int*) (KERNEL_MEM_START - 4);
			i = *ptr;
		case IN_BOUNDS:
			/* Check if kernel memory is accessible */
			printf("IN_BOUNDS: ");
			ptr = (int*) (KERNEL_MEM_START);
			i = *ptr;
			ptr = (int*) (KERNEL_MEM_START + _4MB_ - 4);
			i = *ptr;
			break;
		case ABOVE_UPPER_BOUND:
			/* Should Page Fault */
			printf("ABOVE_UPPER_BOUND: ");
			ptr = (int*) (KERNEL_MEM_START + _4MB_);
			i = *ptr;
	}

	int remove_warning;
	remove_warning = i;
	i = remove_warning;
	return PASS;
}

/* Paging Test - Accessing Video Memory
 * 
 * Asserts that video memory is accessible.
 * Inputs: None
 * Outputs: PASS
 * Side Effects: If testing UNDER_LOWER_BOUND or ABOVE_UPPER_BOUND,
 * 				 a page fault should occur and freeze the screen.
 * Coverage: Page Table set up
 * Files: paging.h/c, paging_init_asm.h/S
 */
int paging_video_mem(int test) {
	TEST_HEADER;

	int* ptr;
	int i;

	switch(test) {
		case UNDER_LOWER_BOUND:
			/* Should Page Fault */
			printf("UNDER_LOWER_BOUND: ");
			ptr = (int*) (VIDEO - 4);
			i = *ptr;
		case IN_BOUNDS:
			/* Check if video memory is accessible */
			printf("IN_BOUNDS: ");
			ptr = (int*) (VIDEO);
			i = *ptr;
			ptr = (int*) (VIDEO + KBYTE_4 - 4);
			i = *ptr;
			break;
		case ABOVE_UPPER_BOUND:
			/* Should Page Fault */
			printf("ABOVE_UPPER_BOUND: ");
			ptr = (int*) (VIDEO + KBYTE_4);
			i = *ptr;
	}
	
	int remove_warning;
	remove_warning = i;
	i = remove_warning;
	return PASS;
}

/* Paging Test - Page Directory Entries are marked Present
 * 
 * Asserts that first two page directory entries are present.
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Page Directory attributes set up
 * Files: paging.h/c, paging_init_asm.h/S
 */
int paging_pd_entries() {
	TEST_HEADER;

	/* Get last bit of each page directory page */
	if(((page_directory[0] & 0x01) & (page_directory[1] & 0x01)) != 1) {
		return FAIL;
	}
	return PASS;
}

/* Checkpoint 2 tests */

/* Terminal Read Test
 * 
 * Asserts that terminal_read correctly writes to its input buffer
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: terminal_init and terminal_read
 * Files: terminal.c/h keyboard.c/h
 */
int terminal_read_test() {
	TEST_HEADER;

	char* ptr = NULL;
	/* Test invalid parameters */
	if(terminal_read(0, ptr, 5) != -1) return FAIL;

	char buf[200];
	/* Make sure to type at least 10 chars */
	memset(buf, 0, 200);
	if(terminal_read(0, buf, 10) != 10) return FAIL;
	if(buf[9] != NEWLINE) return FAIL;
	/* Make sure to type a full buffer */
	memset(buf, 0, 200);
	if(terminal_read(0, buf, 128) != 128) return FAIL;
	if(buf[127] != NEWLINE) return FAIL;
	/* Make sure to type a full buffer */
	memset(buf, 0, 200);
	if(terminal_read(0, buf, 200) != 128) return FAIL;
	if(buf[127] != NEWLINE) return FAIL;

	printf("Passed!\n");
	return PASS;
}

/* Terminal Write Test
 * 
 * Asserts that terminal_write correctly writes from input buffer to terminal
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: terminal_init and terminal_write
 * Files: terminal.c/h keyboard.c/h
 */
int terminal_write_test() {
	TEST_HEADER;
	/* check for appropriate return values of bytes typed */
	if (terminal_write(0, "asdf", 4) != 4) return FAIL;

	if (terminal_write(0, "asdf", 3) != 3) return FAIL;
	
	if (terminal_write(0, "asdf", 5) != 4) return FAIL;

	if (terminal_write(0, "asdf", -1) != -1) return FAIL;

	if (terminal_write(0, "12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", 200) != 200)
		return FAIL;

	printf("\nPassed!\n");
	return PASS;
}

/* Terminal Read/Write Test
 * 
 * Asserts that terminal_read and terminal_write work together in all cases
 * Inputs: None
 * Outputs: PASS
 * Side Effects: None
 * Coverage: terminal_init, terminal_read, and terminal_write
 * Files: terminal.c/h keyboard.c/h
 */
int terminal_read_write_test() {
	TEST_HEADER;
	uint8_t buf[200];
	while (1) {
		memset(buf, 0, 200);
		/* echo whatever is typed */
		terminal_write(0, buf, terminal_read(0, buf, 200));
		//printf("\n");
	}
	return PASS;
}

/* RTC Write Test
 * 
 * Asserts that rtc correctly reads and writes from device
 * Inputs: None
 * Outputs: None
 * Side Effects: Continously changes video memory
 * Coverage: rtc_init, rtc_read, and rtc_write
 * Files: rtc.c/h 
 */
int rtc_write_test() {
	/* set a test frequency */
	uint32_t test_freq = 2;
	int i;
	
	rtc_open(0);

	while(test_freq <= 8192) {
		for (i = 0; i < 100; i++) {
			rtc_read(0, &test_freq, 0);
			test_interrupts();
		}
		test_freq <<= 1;
		rtc_write(0, &test_freq, 4);
	}

	return PASS;
}

/* 
 * read_directory_test - reads all filename correctly
 * 
 * Prints all filenames in filesystem including "." 
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side effects: none
 * Coverage: read sys call on directories
 * Files: filesystem.c/h
 */
int read_directory_test() {
	TEST_HEADER;

	// create a 32 * 63 byte buffer
	uint8_t buf[32];

	// call read_directory
	uint32_t bytes_read = 0;
	while (1) {
		// read file name
		bytes_read = read_directory(buf, 32);

		// we outtie bois
		if (bytes_read == -1)
			break;

		// print the buffer
		int i;
		for (i = 0; i < bytes_read; i++) {
			putc(buf[i]);
		}
		printf("\n");
	}

	return PASS;
}


/* 
 * read_text_test - reads file contents
 * 
 * Prints all file contents of given filename
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side effects: none
 * Coverage: read sys call on directories
 * Files: filesystem.c/h
 */
int read_text_test() {
	TEST_HEADER;

	// create a 32 * 63 byte buffer
	uint8_t buf[187];

	// call read_directory - ACTUAL FILESIZE IS 174 BYTES
	uint32_t bytes_read = read_file((uint8_t*)"frame1.txt", 0, buf, 187);

	// read failed
	if (bytes_read <= 0)
		return FAIL;

	int i;
	for (i = 0; i < bytes_read; i++) {
		// if(buf[i] != 0)
		putc(buf[i]);
	}

	return PASS;
}


/* 
 * read_non_text_test - reads file contents of non text file
 * 
 * Prints all file contents of given filename
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side effects: none
 * Coverage: read sys call on directories
 * Files: filesystem.c/h
 */
int read_non_text_test() {
	TEST_HEADER;

	// create a 32 * 63 byte buffer
	uint8_t buf[6000];

	// call read_directory
	uint32_t bytes_read = read_file((uint8_t*)"hello", 0, buf, 5349);

	// read failed
	if (bytes_read <= 0)
		return FAIL;

	int i;
	for (i = 0; i < bytes_read; i++) {
		putc(buf[i]);
	}

	return PASS;
}


/* 
 * read_partial_test - reads partial contents of a file
 * 
 * Prints all file contents of given filename
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side effects: none
 * Coverage: read sys call on directories
 * Files: filesystem.c/h
 */
int read_partial_test() {
	TEST_HEADER;

	// create a 32 * 63 byte buffer
	uint8_t buf[200];

	// call read_directory
	uint32_t bytes_read = read_file((uint8_t*)"frame0.txt", 93, buf, 187);

	// read failed
	if (bytes_read <= 0)
		return FAIL;

	int i;
	for (i = 0; i < bytes_read; i++) {
		// if(buf[i] != 0)
			putc(buf[i]);
	}

	return PASS;
}


/* 
 * read_big_test - reads file contents of syserror (8kB)
 * 
 * Prints all file contents of given filename
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side effects: none
 * Coverage: read sys call on directories
 * Files: filesystem.c/h
 */
int read_big_test() {
	TEST_HEADER;

	// create a 32 * 63 byte buffer
	uint8_t buf[37000];

	// call read_directory
	uint32_t bytes_read = read_file((uint8_t*)"syserr", 0, buf, 36164);

	// read failed
	if (bytes_read <= 0)
		return FAIL;

	int i;
	for (i = 0; i < bytes_read; i++) {
		putc(buf[i]);
	}
	// printf("\n%d\n", bytes_read);

	return PASS;
}

/* Checkpoint 3 tests */

/* 
 * test_system_call - only works if body of system calls is commented out, tests system call handler
 * 
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side effects: none
 * Coverage: checks system call handler
 * Files: systemcalls.c/h, systemcall_handler.S/h
 */
int test_system_call() {
	TEST_HEADER;

	asm volatile (
		"movl $0, %eax;"
		"int $0x80;"
		"cmpl $-1, %eax;"

		"movl $1, %eax;"
		"int $0x80;"
		"movl $2, %eax;"
		"int $0x80;"
		"movl $3, %eax;"
		"int $0x80;"
		"movl $4, %eax;"
		"int $0x80;"
		"movl $5, %eax;"
		"int $0x80;"
		"movl $6, %eax;"
		"int $0x80;"
		"movl $7, %eax;"
		"int $0x80;"
		"movl $8, %eax;"
		"int $0x80;"
		"movl $9, %eax;"
		"int $0x80;"
		"movl $10, %eax;"
		"int $0x80;"
	);
	return PASS;
}

/* Checkpoint 4 tests */

/* 
 * open_read_test - used for debugging open and read on verylargetextwithverylongname.txt
 * 
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side effects: none
 * Coverage: checks system call handler
 * Files: systemcalls.c/h, systemcall_handler.S/h
 */
int open_read_test() {
	// test read_dentry_by_name
	dentry_t dentry;
	if (read_dentry_by_name((uint8_t*) "verylargetextwithverylongname.tx", &dentry) != -1) {
		printf("read_dentry_by_name success\n");

		int i = 0;
		while(dentry.file_name[i] != '\0') {
			putc(dentry.file_name[i]);
			i++;
		}
		putc('\n');

		printf("%d\n", dentry.inode_num);
	}
	else {
		printf("read_dentry_by_name fail\n");
		return FAIL;	
	}

	/*
	// test open
	if (fs_open((uint8_t*) "verylargetextwithverylongname.tx") != -1)
		printf("open success\n");
	else {
		printf("open fail\n");
		return FAIL;
	}

	// test read
	uint8_t buffer[5000];
	uint32_t bytes_read;
	if ((bytes_read = read_file((uint8_t*) "verylargetextwithverylongname.tx", 0, buffer, 5000)) != 0) 
		printf("read success\n");
	else {
		printf("read fail\n");	
		return FAIL;
	}

	// print buffer
	int i;
	for (i = 0; i < bytes_read; i++) {
		putc(buffer[i]);
	}*/


	return PASS;
}

/* Checkpoint 5 tests */

/* Test suite entry point */
void launch_tests() {
	/* Checkpoint 1 tests */
	// TEST_OUTPUT("idt_test", idt_test());
	// TEST_OUTPUT("idt_divide_by_zero", idt_divide_by_zero());
	// TEST_OUTPUT("rtc_handler_test", rtc_handler_test());
	// TEST_OUTPUT("paging_deref_null", paging_deref_null());
	// TEST_OUTPUT("paging_ker_mem", paging_ker_mem(UNDER_LOWER_BOUND));
	// TEST_OUTPUT("paging_ker_mem", paging_ker_mem(IN_BOUNDS));
	// TEST_OUTPUT("paging_ker_mem", paging_ker_mem(ABOVE_UPPER_BOUND));
	// TEST_OUTPUT("paging_video_mem", paging_video_mem(UNDER_LOWER_BOUND));
	// TEST_OUTPUT("paging_video_mem", paging_video_mem(IN_BOUNDS));
	// TEST_OUTPUT("paging_video_mem", paging_video_mem(ABOVE_UPPER_BOUND));
	// TEST_OUTPUT("paging_pd_entries", paging_pd_entries());

	/* Checkpoint 2 tests */
	// terminal_read_test();
	// terminal_write_test();
	// terminal_read_write_test();
	// rtc_write_test();
	// TEST_OUTPUT("read_directory_test", read_directory_test());
	// TEST_OUTPUT("read_text_test", read_text_test());
	// TEST_OUTPUT("read_non_text_test", read_non_text_test());
	// TEST_OUTPUT("read_big_test", read_big_test());
	// TEST_OUTPUT("read_partial_test", read_partial_test());

	/* Checkpoint 3 tests */
	// test_system_call();

	/* Checkpoint 4 tests */
	// open_read_test();

	/* Checkpoint 5 tests */
}
