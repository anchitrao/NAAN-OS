#ifndef IDT_H
#define IDT_H

#define NUM_INTEL_DEFINED_VECTORS   32      /* The first 32 exception vectors are reserved by Intel */
#define USER_PRIVILEGE_LEVEL        3       /* User privilege level is 3, Kernel privilege level is 0 */
#define SYSTEM_CALL_VECTOR          0x80    /* Exception vector associated with all system calls */
#define PIT_VECTOR                  0x20    /* Exception vector associated with all PIT interrupts */
#define KEYBOARD_VECTOR             0x21    /* Exception vector associated with all keyboard interrupts */
#define RTC_VECTOR                  0x28    /* Exception vector associated with all rtc interrupts */

/* Initializes the IDT */
void IDT_init();

#endif /* IDT_H */
