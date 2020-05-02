/* rtc.h - Defines interactions with Real Time Clock (RTC)
 * vim:ts=4 noexpandtab
 */

// do not include header file twice
#ifndef _RTC_H
#define _RTC_H

// include .h files crucial to RTC operation 
#include "i8259.h"
#include "rtc_handler.h"

// RTC ports are 0x70 and 0x71 
#define INDEX_PORT  0x70
#define CMOS_PORT   0x71

// Registers in RTC
#define RTC_REG_A   0x0A
#define RTC_REG_B   0x0B
#define RTC_REG_C   0x0C 

// NMI mask and default Interrupt Freq
#define DEFAULT_FREQ        0x27
#define _2HZ_               0x02
#define _2HZ_SELECT_BITS    0xF
#define _512HZ_             0x200             

// RTC is at IRQ 8 on PIC
#define RTC_IRQ     8

/* Externally visible functions in RTC driver */
// initializes RTC
void init_rtc(void);

// RTC interrupt handler
void rtc_intr_handler(void);


/* NEW & MODIFIED FUNCTIONS */

/* opens the rtc */
int32_t rtc_open(const uint8_t* filename);
/* (user-level) allows user to read from the rtc */
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes);
/* (user-level) allows user to write to the rtc */
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes);
/* closes the rtc */
int32_t rtc_close(int32_t fd);
/* updates the frequency of the rtc */
int32_t rtc_update_freq(uint32_t new_frequency);

#endif // _RTC_H is now defined
