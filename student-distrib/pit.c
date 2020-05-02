/* pit.c - initializes PIT and uses interrput handler of PIT for scheduling
 * vim:ts=4 noexpandtab
 */

#include "pit.h"
#include "scheduler.h"
#include "systemcalls.h"
#include "types.h"

/* init_PIT
 * 
 * DESCRIPTION: initializes PIT 
 * 
 * Inputs: none
 * Outputs: none
 * Return values: none
 * 
 * SIDE EFFECTS: initializes PIC, sets interrupt rate to 100 Hz
 */
void init_pit() {
    // Obtain interrupt frequency divider
    uint16_t div = DIV_100HZ;

    // Begin operating in Mode 3
    outb(MODE3, PIC_CMD);

    // Push frequency to PIC channel 0
    outb((uint8_t) (div && LOWER_8), CHANNEL0);
    outb((uint8_t) (div >> UPPER_8), CHANNEL0);

    // Enable IRQ line 0
    enable_irq(PIT_IRQ);
}

/* pit_intr_handler
 * 
 * DESCRIPTION: the PIT interrupt handler, sets up for task scheduling
 * 
 * Inputs: none
 * Outputs: none
 * Return values: none
 * 
 * SIDE EFFECTS: switches tasks in and out of memory
 */
void pit_intr_handler() {
    /* check if any terminals are running */
    if (get_curr_pcb() == NULL) {
        send_eoi(PIT_IRQ);
        return;
    }

    /* schedules next process using round robin scheduling */
    scheduler(sched_term, (sched_term + 1) % TERMINAL_COUNT);

    /* send EOI to PIC */
    send_eoi(PIT_IRQ);
}
