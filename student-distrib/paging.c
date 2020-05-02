#include "paging.h"

/* 
 * paging_init
 *   DESCRIPTION: Initializes paging
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Initializes paging
 */
void paging_init() {
    int i;
    
    /* Initialize Page Directory and Page Table */
    for (i = 0; i < MAX_ENTRIES; i++) {
        /* Set default attributes */
        page_directory[i] = RW & ~PRESENT;
        /* Set default attributes */
        page_table[i] = (i * PAGE_SIZE) | (RW & ~PRESENT);
        /* Set default attributes */
        user_video_page_table[i] = (i * PAGE_SIZE) | (RW & ~PRESENT);
    }

    /* The first entry of the page directory should hold the page table */
    page_directory[0] = (uint32_t) page_table;
    page_directory[0] |= (RW | PRESENT);

    /* allocates physical memory video and backups */
    for (i = 0; i <= TERMINAL_COUNT; i++) {
        page_table[VIDEO_MEM_PAGE + i] |= (RW | PRESENT);
    }

    /* Set second entry in page directory to be start of kernel memory */
    page_directory[1] = KERNEL_MEM_START;
    page_directory[1] |= (FOUR_MB_PAGE | RW | PRESENT);
    
    /* The page after the user page should hold the user video page table */
    page_directory[USER_VID_MEM_PAGE] = (uint32_t) user_video_page_table;
    page_directory[USER_VID_MEM_PAGE] |= (USER | RW | PRESENT);

    /* maps user space virtual memory to physical memory video backups */
    for (i = 0; i <= TERMINAL_COUNT; i++) {
        user_video_page_table[i] = VIDEO + (i * PAGE_SIZE);
        user_video_page_table[i] |= (USER | RW | PRESENT);
    }

    /* Enable paging using assembly code */
    enable_paging();
}
