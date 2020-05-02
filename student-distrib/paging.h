#ifndef PAGING_H
#define PAGING_H

#include "lib.h"
#include "paging_init_asm.h"

#define MAX_ENTRIES         1024        /* Max entries in the Page Directory as well as the Page Table */

#define PRESENT                 0x00000001      /* If bit 0 is set, the page is located in physical memory */
#define RW                      0x00000002      /* If bit 1 is set, the page is writeable */
#define USER                    0x00000004      /* If bit 2 is set, the page is accessible to User and Supervisor */
#define FOUR_MB_PAGE            0x00000080      /* If bit 7 is set, the page size becomes 4MB */

#define PROGRAM_IMAGE_ADDR      0x8048000       /* Address of program image */
#define USER_STACK              0x83FFFFC       /* Address of user stack for program */
#define MAX_FILE_SIZE           36164           /* Max size of file */


#define VIDEO_MEM_PAGE          (VIDEO >> 12)   /* VIDEO = 0xB8000, obtain most significant bits by right shifting by 12 */
#define KERNEL_MEM_START        0x00400000      /* Kernel memory starting address (4MB) */
#define KERNEL_MEM_END          0x00800000      /* Kernel memory ending address */

#define PAGE_BASE_ADDR_OFFSET   22      /* Only the first 10 bits [31:22] are the page base address */

#define USER_PAGE           (PROGRAM_IMAGE_ADDR >> PAGE_BASE_ADDR_OFFSET) /* Page where the program image is stored */
#define USER_VID_MEM_PAGE   (USER_PAGE + 1)   /* The page after the program image page is where the user video memory pages should be */

/* Page Directory */
uint32_t page_directory[MAX_ENTRIES] __attribute__((aligned(PAGE_SIZE)));
/* Page Table */
uint32_t page_table[MAX_ENTRIES] __attribute__((aligned(PAGE_SIZE)));
/* User Video Page Table */
uint32_t user_video_page_table[MAX_ENTRIES] __attribute__((aligned(PAGE_SIZE)));

/* Initializes paging */
void paging_init();

#endif /* PAGING_H */
