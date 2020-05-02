/* rtc_handler.S - RTC interrupt handler
 * vim:ts=4 noexpandtab
 */

#define ASM     1
#include "rtc_handler.h"

.globl  rtc_handler

# void rtc_handler();
#
# Interface: Interrupt Handler
#    Inputs: none
#   Outputs: none
# Registers: none
#  Clobbers: none
rtc_handler:
    # save all registers
    pushl   %eax
    pushl   %ebx
    pushl   %ecx
    pushl   %edx
    pushl   %esi
    pushl   %edi

    # call interrupt handler
    call   rtc_intr_handler

    # restore all registers
    popl   %edi
    popl   %esi
    popl   %edx
    popl   %ecx
    popl   %ebx
    popl   %eax

    # return
    iret 
