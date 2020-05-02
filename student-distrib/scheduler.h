/* pit.h - initializes PIT and uses interrupt handler of PIT for scheduling
 * vim:ts=4 noexpandtab
 */

#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#include "types.h"

/* Switches between current terminal and terminal given */
void terminal_switch (uint8_t new_terminal_id);

/* schedules between one process to the next */
void scheduler(uint8_t prev_term, uint8_t new_terminal);

#endif /* ensure .h file only read once */
