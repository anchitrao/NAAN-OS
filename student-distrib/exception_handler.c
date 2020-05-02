#include "exception_handler.h"
#include "lib.h"
#include "systemcalls.h"

/* 
 * _0_divide_error_exception
 *   DESCRIPTION: Prints a description of exception vector 0 and
 *                returns to the shell.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Writes a message to the screen and returns to the shell
 */
void _0_divide_error_exception() {
    printf("Divide Error Exception\n"); 
    exception_flag = 1;
    halt(EXCEPTION_CODE);
}

/* 
 * _1_debug_exception
 *   DESCRIPTION: Prints a description of exception vector 1 and
 *                returns to the shell.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Writes a message to the screen and returns to the shell
 */
void _1_debug_exception() {
    printf("Debug Exception\n");
    exception_flag = 1;
    halt(EXCEPTION_CODE);
}

/* 
 * _2_nmi_interrupt
 *   DESCRIPTION: Prints a description of exception vector 2 and
 *                returns to the shell.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Writes a message to the screen and returns to the shell
 */
void _2_nmi_interrupt() {
    printf("NMI Interrupt\n");
    exception_flag = 1;
    halt(EXCEPTION_CODE);
    
}

/* 
 * _3_breakpoint_exception
 *   DESCRIPTION: Prints a description of exception vector 3 and
 *                returns to the shell.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Writes a message to the screen and returns to the shell
 */
void _3_breakpoint_exception() {
    printf("Breakpoint Exception\n");
    exception_flag = 1;
    halt(EXCEPTION_CODE);
}

/* 
 * _4_overflow_exception
 *   DESCRIPTION: Prints a description of exception vector 4 and
 *                returns to the shell.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Writes a message to the screen and returns to the shell
 */
void _4_overflow_exception() {
    printf("Overflow Exception\n");
    exception_flag = 1;
    halt(EXCEPTION_CODE);
}

/* 
 * _5_bound_range_exceeded_exception
 *   DESCRIPTION: Prints a description of exception vector 5 and
 *                returns to the shell.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Writes a message to the screen and returns to the shell
 */
void _5_bound_range_exceeded_exception() {
    printf("BOUND Range Exceeded Exception\n");
    exception_flag = 1;
    halt(EXCEPTION_CODE);
}

/* 
 * _6_invalid_opcode_exception
 *   DESCRIPTION: Prints a description of exception vector 6 and
 *                returns to the shell.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Writes a message to the screen and returns to the shell
 */
void _6_invalid_opcode_exception() {
    printf("Invalid Opcode Exception\n");
    exception_flag = 1;
    halt(EXCEPTION_CODE);
}

/* 
 * _7_device_not_available_exception
 *   DESCRIPTION: Prints a description of exception vector 7 and
 *                returns to the shell.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Writes a message to the screen and returns to the shell
 */
void _7_device_not_available_exception() {
    printf("Device Not Available Exception\n");
    exception_flag = 1;
    halt(EXCEPTION_CODE);
}

/* 
 * _8_double_fault_exception
 *   DESCRIPTION: Prints a description of exception vector 8 and
 *                returns to the shell.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Writes a message to the screen and returns to the shell
 */
void _8_double_fault_exception() {
    printf("Double Fault Exception\n");
    exception_flag = 1;
    halt(EXCEPTION_CODE);
}

/* 
 * _9_coprocessor_segment_overrun
 *   DESCRIPTION: Prints a description of exception vector 9 and
 *                returns to the shell.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Writes a message to the screen and returns to the shell
 */
void _9_coprocessor_segment_overrun() {
    printf("Coprocessor Segment Overrun\n");
    exception_flag = 1;
    halt(EXCEPTION_CODE);
}

/* 
 * _10_invalid_TSS_exception
 *   DESCRIPTION: Prints a description of exception vector 10 and
 *                returns to the shell.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Writes a message to the screen and returns to the shell
 */
void _10_invalid_TSS_exception() {
    printf("Invalid TSS Exception\n");
    exception_flag = 1;
    halt(EXCEPTION_CODE);
}

/* 
 * _11_segment_not_present
 *   DESCRIPTION: Prints a description of exception vector 11 and
 *                returns to the shell.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Writes a message to the screen and returns to the shell
 */
void _11_segment_not_present() {
    printf("Segmant Not Present\n");
    exception_flag = 1;
    halt(EXCEPTION_CODE);
}

/* 
 * _12_stack_fault_exception
 *   DESCRIPTION: Prints a description of exception vector 12 and
 *                returns to the shell.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Writes a message to the screen and returns to the shell
 */
void _12_stack_fault_exception() {
    printf("Stack Fault Exception\n");
    exception_flag = 1;
    halt(EXCEPTION_CODE);
}

/* 
 * _13_general_protection_exception
 *   DESCRIPTION: Prints a description of exception vector 13 and
 *                returns to the shell.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Writes a message to the screen and returns to the shell
 */
void _13_general_protection_exception() {
    printf("General Protection Exception\n");
    exception_flag = 1;
    halt(EXCEPTION_CODE);
}

/* 
 * _14_page_fault_exception
 *   DESCRIPTION: Prints a description of exception vector 14 and
 *                returns to the shell.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Writes a message to the screen and returns to the shell
 */
void _14_page_fault_exception() {
    printf("Page Fault Exception\n");
    exception_flag = 1;
    halt(EXCEPTION_CODE);
}

/* 
 * _16_fpu_floating_point_error
 *   DESCRIPTION: Prints a description of exception vector 16 and
 *                returns to the shell.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Writes a message to the screen and returns to the shell
 */
void _16_fpu_floating_point_error() {
    printf("Floating-point error\n");
    exception_flag = 1;
    halt(EXCEPTION_CODE);
} 

/* 
 * _17_alignment_check_exception
 *   DESCRIPTION: Prints a description of exception vector 17 and
 *                returns to the shell.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Writes a message to the screen and returns to the shell
 */
void _17_alignment_check_exception() {
    printf("Alignment Check Exception\n");
    exception_flag = 1;
    halt(EXCEPTION_CODE);
}

/* 
 * _18_machine_check_exception
 *   DESCRIPTION: Prints a description of exception vector 18 and
 *                returns to the shell.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Writes a message to the screen and returns to the shell
 */
void _18_machine_check_exception() {
    printf("Machine Check Exception\n");
    exception_flag = 1;
    halt(EXCEPTION_CODE);
}

/* 
 * _19_simd_floating_point_exception
 *   DESCRIPTION: Prints a description of exception vector 19 and
 *                returns to the shell.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Writes a message to the screen and returns to the shell
 */
void _19_simd_floating_point_exception() {
    printf("SIMD Floating-Point Exception\n");
    exception_flag = 1;
    halt(EXCEPTION_CODE);
}

/* 
 * reserved
 *   DESCRIPTION: Prints a description of the exception 15, or 20-31 and
 *                returns to the shell.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Writes a message to the screen and returns to the shell
 */
void reserved() {
    printf("Reserved\n");
    exception_flag = 1;
    halt(EXCEPTION_CODE);
}

/* 
 * unreserved
 *   DESCRIPTION: Prints a description of the exception 32-255.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Writes a message to the screen
 */
void unreserved() {
    printf("Unreserved\n");
    exception_flag = 1;
    halt(EXCEPTION_CODE);
}
