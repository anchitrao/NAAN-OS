# paging_init_asm.S - enables paging

#define ASM     1

#include "paging_init_asm.h"

.globl  enable_paging, flush_tlb

# void enable_paging(uint32_t* page_directory);
#
# Interface: C-style
#    Inputs: N/A
#   Outputs: N/A
#
# Registers: EAX (Clobbered) - used as a temp register
enable_paging:
    # stack buildup
    pushl   %ebp
    movl    %esp, %ebp

    # callee save
    pushl   %ebx
    pushl   %esi
    pushl   %edi

    # Set Page Directory Base Register (Upper 20 bits of CR3) to address of Page Directory
    movl $page_directory, %eax;
    movl %eax, %cr3;

    # Enable Page Size Extension by setting bit 4 of CR4 to 1
    movl %cr4, %eax;
    orl  $0x00000010, %eax;
    movl %eax, %cr4;

    # Enable Paging by setting bit 31 of CR0 to 1
    movl %cr0, %eax;
    orl  $0x80000000, %eax;
    movl %eax, %cr0;

    # callee restore
    popl    %edi
    popl    %esi
    popl    %ebx 
    
    # stack teardown
    leave
    ret


# void flush_tlb();
#
# Interface: C-style
#    Inputs: N/A
#   Outputs: N/A
# Registers: %eax - used as a temp register
#  Clobbers: %eax
flush_tlb:
    # stack buildup
    pushl   %ebp
    movl    %esp, %ebp

    # callee save
    pushl   %ebx
    pushl   %esi
    pushl   %edi

    # Set Page Directory Base Register (Upper 20 bits of CR3) to address of current page
    movl %cr3, %eax;
    movl %eax, %cr3;

    # callee restore
    popl    %edi
    popl    %esi
    popl    %ebx 
    
    # stack teardown
    leave
    ret
