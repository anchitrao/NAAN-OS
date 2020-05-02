/* pit.c - initializes PIT and uses interrput handler of PIT for scheduling
 * vim:ts=4 noexpandtab
 */

#include "scheduler.h"
#include "paging.h"
#include "systemcalls.h"
#include "pit.h"

/* 
 * terminal_switch
 * 
 * DESCRIPTION: switches between current terminal to argument terminal
 * 
 * Input: uint8_t new_terminal_id - terminal id (0, 1, 2) to be switched into
 * Output: N/A
 * Return Values: N/A
 * 
 * SIDE EFFECTS: none
 */
void terminal_switch (uint8_t new_terminal) {
    /* Do nothing if argument terminal is current terminal */
    if (curr_term == new_terminal)
        return;

    /* save current video memory from screen into backup */
    memcpy((int8_t *) terminal[curr_term].video_mem, (int8_t *) (VIDEO), PAGE_SIZE);

    /* restore new video memory from backup onto screen */
    memcpy((int8_t *) (VIDEO), (int8_t *) terminal[new_terminal].video_mem, PAGE_SIZE);

    /* set current terminal as argument */
    curr_term = new_terminal;

    /* update cursor position based on current terminal */
    set_cursor(terminal[curr_term].screen_x, terminal[curr_term].screen_y, curr_term);
}

/* scheduler
 * 
 * DESCRIPTION: "schedules" process by switching from current process to next using round-robin method
 *              - saves ebp/esp of current process
 *              - switches process paging
 *              - sets task state segment
 *              - updates running video coordinates
 *              - restores ebp/esp of next process
 * 
 * Inputs: prev_term - terminal scheduler is switching from
 *         next_term - terminal scheduler is switching to
 * Outputs: none
 * Return values: none
 * 
 * SIDE EFFECTS: contexts switches into new process
 */
void scheduler(uint8_t prev_term, uint8_t next_term) {
    /* return if previous terminal is same as next terminal */
    if(prev_term == next_term) {
        send_eoi(PIT_IRQ);
        return;
    }

    /* 1. saves ebp/esp of current process
     *  - process that's on the screen
     *  - process running in the background */
    if (terminal[prev_term].active) {
        asm volatile ("      \n\
            movl %%esp, %0   \n\
            movl %%ebp, %1"
            : "=r" (terminal[prev_term].curr_pcb->esp), "=r" (terminal[prev_term].curr_pcb->ebp)
        );
    }

    /* set the scheduled terminal to next terminal */
    sched_term = next_term;

    /* execute new shell on terminal switch */
    if (terminal[next_term].active == 0) {
        send_eoi(PIT_IRQ);
        execute((uint8_t *) "shell");
        return;
    }

    /* 2. switches process paging */
    page_directory[USER_PAGE] = KERNEL_MEM_END + ((terminal[next_term].curr_pcb -> pid) * _4MB_);
    page_directory[USER_PAGE] |= FOUR_MB_PAGE | USER | RW | PRESENT;
    flush_tlb();

    /* 3. sets task state segment */
    tss.ss0 = KERNEL_DS;
    tss.esp0 = (uint32_t)(KERNEL_MEM_END - (terminal[next_term].curr_pcb -> pid) * _8KB_ - BYTE_4);

    /* 4. updates running video coordinates */
    if (terminal[next_term].curr_pcb -> terminal_id == curr_term) {
        /* write to screen */
        user_video_page_table[0] = VIDEO;
        user_video_page_table[0] |= (USER | RW | PRESENT);
    } else {
        /* write to backup */
        user_video_page_table[0] = VIDEO + ((terminal[next_term].curr_pcb -> terminal_id + 1) * PAGE_SIZE);
        user_video_page_table[0] |= (USER | RW | PRESENT);
    }
    flush_tlb();

    /* 5. restore EBP and ESP of next process */
    asm volatile ("         \n\
        movl %0, %%esp      \n\
        movl %1, %%ebp"
        : 
        : "r" (terminal[next_term].curr_pcb->esp), "r" (terminal[next_term].curr_pcb->ebp)
    );
}
