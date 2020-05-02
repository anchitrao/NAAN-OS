#ifndef PAGING_INIT_ASM
#define PAGING_INIT_ASM

#ifndef ASM

/* Helper function for paging_init(), Enables paging */
extern void enable_paging();

/* Flushes all TLB entries of the non-global pages owned by the current process */
extern void flush_tlb();

#endif /* ASM */

#endif /* PAGING_INIT_ASM */
