/* pit.h - initializes PIT and uses interrupt handler of PIT for scheduling
 * vim:ts=4 noexpandtab
 */

#ifndef _PIT_H
#define _PIT_H

#include "types.h"
#include "i8259.h"
#include "lib.h"
#include "pit_handler.h"


/** MACROS **/

/* Ports for PIT */
#define CHANNEL0    0x40        // data reg for channel 0
#define PIC_CMD     0x43        // command reg on PIC
#define MODE3       0x36        // command byte for Mode 3 operation

/* Interrupt frequency generation divisors from maximum frequency of PIT */
#define MAX_FREQ    1193180     // clock frequency of PIT 
#define DIV_100HZ   11932       // divider to obtain 100Hz interrupt rate

/* Bitmasking constants */
#define LOWER_8     0x00FF      // gets lower 8 bits
#define UPPER_8     8           // shifts to get upper 8 bits

/* IRQ line for PIT */
#define PIT_IRQ     0           // PIT resides on IRQ 0 line


/** FUNCTION DECLARATIONS **/

/* Initializes PIT */
void init_pit();

/* Interrupt handler for PIT */
void pit_intr_handler();

#endif /* ensure .h file only read once */
