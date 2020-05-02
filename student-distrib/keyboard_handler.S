/* keyboard_handler.S - wrapper for keyboard interrupt handler, saves regs
 * vim:ts=4 noexpandtab
 */

#define ASM     1
#include "keyboard_handler.h"

.globl  keyboard_handler

# void keyboard_handler();
#
# Interface: Interrupt Handler
#    Inputs: none
#   Outputs: none
# Registers: none
#  Clobbers: none
keyboard_handler:
    # save all registers
    pushl   %eax
    pushl   %ebx
    pushl   %ecx
    pushl   %edx
    pushl   %esi
    pushl   %edi

    # call interrupt handler for keyboard
    call   keyboard_intr_handler

    # restore all registers
    popl   %edi
    popl   %esi
    popl   %edx
    popl   %ecx
    popl   %ebx
    popl   %eax

    # return from interrupt
    iret 
