#include "idt.h"
#include "rtc.h"
#include "x86_desc.h"
#include "exception_handler.h"
#include "systemcall_handler.h"
#include "pit_handler.h"
#include "keyboard.h"

/* 
 * IDT_init
 *   DESCRIPTION: Initializes the IDT
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Initializes the IDT
 */
void IDT_init() {
    /* Load the IDT */
    lidt(idt_desc_ptr);

    int i;
    /* Loop thru all elements in the IDT */
    for(i = 0; i < NUM_VEC; i++) {
        /* Set the attributes for each vector in the IDT */
        idt[i].seg_selector = KERNEL_CS;
        idt[i].reserved4 = 0;
        idt[i].reserved3 = ((i < NUM_INTEL_DEFINED_VECTORS) | (i == SYSTEM_CALL_VECTOR)) ? 1 : 0;
        idt[i].reserved2 = 1;
        idt[i].reserved1 = 1;
        idt[i].size = 1;
        idt[i].reserved0 = 0;
        idt[i].dpl = (i == SYSTEM_CALL_VECTOR) ? USER_PRIVILEGE_LEVEL : 0;
        idt[i].present = 1;

        /* Assign each vector its corresponding handler */
        switch(i) {
            case 0:
                SET_IDT_ENTRY(idt[i], _0_divide_error_exception);
                break;
            case 1:
                SET_IDT_ENTRY(idt[i], _1_debug_exception);
                break;
            case 2:
                SET_IDT_ENTRY(idt[i], _2_nmi_interrupt);
                break;
            case 3:
                SET_IDT_ENTRY(idt[i], _3_breakpoint_exception);
                break;
            case 4:
                SET_IDT_ENTRY(idt[i], _4_overflow_exception);
                break;
            case 5:
                SET_IDT_ENTRY(idt[i], _5_bound_range_exceeded_exception);
                break;
            case 6:
                SET_IDT_ENTRY(idt[i], _6_invalid_opcode_exception);
                break;
            case 7:
                SET_IDT_ENTRY(idt[i], _7_device_not_available_exception);
                break;
            case 8:
                SET_IDT_ENTRY(idt[i], _8_double_fault_exception);
                break;
            case 9:
                SET_IDT_ENTRY(idt[i], _9_coprocessor_segment_overrun);
                break;
            case 10:
                SET_IDT_ENTRY(idt[i], _10_invalid_TSS_exception);
                break;
            case 11:
                SET_IDT_ENTRY(idt[i], _11_segment_not_present);
                break;
            case 12:
                SET_IDT_ENTRY(idt[i], _12_stack_fault_exception);
                break;
            case 13:
                SET_IDT_ENTRY(idt[i], _13_general_protection_exception);
                break;
            case 14:
                SET_IDT_ENTRY(idt[i], _14_page_fault_exception);
                break;
            case 15:
                SET_IDT_ENTRY(idt[i], reserved);
                break;
            case 16:
                SET_IDT_ENTRY(idt[i], _16_fpu_floating_point_error);
                break;
            case 17:
                SET_IDT_ENTRY(idt[i], _17_alignment_check_exception);
                break;
            case 18:
                SET_IDT_ENTRY(idt[i], _18_machine_check_exception);
                break;
            case 19:
                SET_IDT_ENTRY(idt[i], _19_simd_floating_point_exception);
                break;
            case SYSTEM_CALL_VECTOR:
                SET_IDT_ENTRY(idt[i], systemcall_handler);               
                break;
            case PIT_VECTOR:
                SET_IDT_ENTRY(idt[i], pit_handler);
                break;
            case KEYBOARD_VECTOR:
                SET_IDT_ENTRY(idt[i], keyboard_handler);               
                break;
            case RTC_VECTOR:
                SET_IDT_ENTRY(idt[i], rtc_handler);               
                break;
            default:
                /* Vectors 20-31 are reserved by Intel */
                if(i < NUM_INTEL_DEFINED_VECTORS) {
                    SET_IDT_ENTRY(idt[i], reserved);
                } else {
                    SET_IDT_ENTRY(idt[i], unreserved);
                }
        }
    }
}
