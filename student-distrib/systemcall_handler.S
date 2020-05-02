# systemcall_handler.S - system call assembly linkage

#define ASM     1

#include "systemcall_handler.h"

.globl systemcall_handler

# int systemcall_handler (unsigned long system_call_num, unsigned long arg1, unsigned long arg2, unsigned long arg3);
#
# Interface: Register-based arguments (register-style)
#
#    Inputs: EAX - System Call Number
#            EBX - 1st argument
#            ECX - 2nd argument
#            EDX - 3rd argument
#   Outputs: -1 - Invalid command number
#
# Registers: EAX (clobbered) - holds return value for invalid command input
systemcall_handler:
    # decrements command number by 1
    decl    %eax

    # check valid command
    cmpl    $0, %eax
    jl      bad_params
    cmpl    $9, %eax
    jg      bad_params
    
    # callee + caller save regsters and flags
    pushfl
    pushl   %esi
    pushl   %edi
    pushl   %edx
    pushl   %ecx
    pushl   %ebx

    # _push arguments
    pushl   %edx
    pushl   %ecx
    pushl   %ebx


jump_to_call:
    # jumps to eax index in jump table
    call   *system_call_jumptable (, %eax, 4)

system_call_teardown:
    # _pop arguments
    popl   %ebx
    popl   %ecx
    popl   %edx

    # callee + caller restore registers and flags
    popl   %ebx
    popl   %ecx
    popl   %edx
    popl   %edi
    popl   %esi
    popfl
    
    # stack teardown
    iret

bad_params:
    # mark command as invalid
    movl   $-1, %eax
    iret
    
system_call_jumptable:
    # jump table for functions
    .long halt
    .long execute
    .long read
    .long write
    .long open
    .long close
    .long getargs
    .long vidmap
    .long set_handler
    .long sigreturn
