/* pit_handler.S - PIT rtc interrupt handler
 * vim:ts=4 noexpandtab
 */

#define ASM     1
#include "pit_handler.h"

.globl  pit_handler

# void pit_handler();
#
# Interface: Interrupt Handler
#    Inputs: none
#   Outputs: none
# Registers: none
#  Clobbers: none
pit_handler:
    # save all registers
    pushl   %eax
    pushl   %ebx
    pushl   %ecx
    pushl   %edx
    pushl   %esi
    pushl   %edi

    # call interrupt handler
    call   pit_intr_handler

    # restore all registers
    popl   %edi
    popl   %esi
    popl   %edx
    popl   %ecx
    popl   %ebx
    popl   %eax

    # return
    iret 
