#include "types.h"

#ifndef EXCEPTION_HANDLER_H
#define EXCEPTION_HANDLER_H

/* Exception code used to return when a process hits exception */
#define EXCEPTION_CODE      255

volatile uint8_t exception_flag;

/* Exception handler for exception vector 0 */
void _0_divide_error_exception();

/* Exception handler for exception vector 1 */
void _1_debug_exception();

/* Exception handler for exception vector 2 */
void _2_nmi_interrupt();

/* Exception handler for exception vector 3 */
void _3_breakpoint_exception();

/* Exception handler for exception vector 4 */
void _4_overflow_exception();

/* Exception handler for exception vector 5 */
void _5_bound_range_exceeded_exception();

/* Exception handler for exception vector 6 */
void _6_invalid_opcode_exception();

/* Exception handler for exception vector 7 */
void _7_device_not_available_exception();

/* Exception handler for exception vector 8 */
void _8_double_fault_exception();

/* Exception handler for exception vector 9 */
void _9_coprocessor_segment_overrun();

/* Exception handler for exception vector 10 */
void _10_invalid_TSS_exception();

/* Exception handler for exception vector 11 */
void _11_segment_not_present();

/* Exception handler for exception vector 12 */
void _12_stack_fault_exception();

/* Exception handler for exception vector 13 */
void _13_general_protection_exception();

/* Exception handler for exception vector 14 */
void _14_page_fault_exception();

/* Exception handler for exception vector 16 */
void _16_fpu_floating_point_error();

/* Exception handler for exception vector 17 */
void _17_alignment_check_exception();

/* Exception handler for exception vector 18 */
void _18_machine_check_exception();

/* Exception handler for exception vector 19 */
void _19_simd_floating_point_exception();

/* Exception handler for exception vector 15, or 20 thru 31 */
void reserved();

/* Exception handler for exception vectors 32 thru 255 */
void unreserved();

#endif /* EXCEPTION_HANDLER_H */
