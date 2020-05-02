/* rtc.c - Defines interactions with Real Time Clock (RTC)
 * vim:ts=4 noexpandtab
 */

// include .h files 
#include "rtc.h"
#include "lib.h"

/* mask for lower bits */
#define LOW_HEX_MASK 0xF0
/* masks lower 7-bits for non-maskable interrupts */
#define NMI_MASK 0x80

/*
 * init_rtc
 * 
 * DESCRIPTION: initializes RTC by notifying it to begin sending periodic
 * interrupts at a frequency specified in Register A. This is done by 
 * setting bit 6 in Register B. By default Register A values will be 
 * A[6:4] = 010 and A[3:0] = 0110, interrupts occur at frequency of 1024Hz
 * 
 * INPUTS: none
 * OUPTUTS: none
 * RETURN VALUE: none
 * 
 * SIDE EFFECTS: none
 */
void init_rtc(void) {
    // create a temporary value to store data in Register A and Register B
    uint8_t old_regA;
    uint8_t old_regB;

    // get Register A and mask NMI
    outb(NMI_MASK | RTC_REG_A, INDEX_PORT);
    old_regA = inb(CMOS_PORT);

    // set frequency on Register A to default values
    outb(NMI_MASK | RTC_REG_A, INDEX_PORT);
    outb(old_regA | DEFAULT_FREQ, CMOS_PORT); // sets RTC virtualization frequency to 512 Hz

    // store data from Register B and mask NMI 
    outb(NMI_MASK | RTC_REG_B, INDEX_PORT);
    old_regB = inb(CMOS_PORT);

    // set bit 6 in Register B to begin sending periodic interrupts to processor
    outb(NMI_MASK | RTC_REG_B, INDEX_PORT);
    outb(old_regB | 0x40, CMOS_PORT);           // 0x40 will set the 6th bit high

    // enable interrupt line (IRQ8) on which RTC resides
    enable_irq(RTC_IRQ);

}

/*
 * rtc_intr_handler
 * 
 * DESCRIPTION: test interrupt handling for RTC interrupts
 * 
 * INPUTS: none
 * OUTPUTS: prints random chars to the screen when test_interrupts is called
 * RETURN VALUE: none
 * 
 * SIDE EFFECTS: none 
 */
void rtc_intr_handler(void) {
    /* send EOI signal to PIC */
    send_eoi(RTC_IRQ);

    cli(); // LOCK
    /* read data from Register C to allow for next interrupt */
    outb(RTC_REG_C, INDEX_PORT);
    inb(CMOS_PORT);

    /* decrement iterations */
    if (terminal[sched_term].rtc_iterations != 0)
        terminal[sched_term].rtc_iterations--;
    sti(); // UNLOCK
}

/*
 * rtc_open
 * 
 * DESCRIPTION: gives illusion to calling process of a 2Hz RTC
 * 
 * INPUTS: const uint8_t* filename - filename of file to open
 * OUTPUTS: none
 * RETURN VALUE: -1 if named file does not exist or no descriptors are free
 * 
 * SIDE EFFECTS: sets RTC frequency to 2 Hz
 */
int32_t rtc_open(const uint8_t* filename) {
    /* set rtc frequency to 2 Hz for current process*/
    terminal[sched_term].rtc_constant = _512HZ_ / _2HZ_;

    return 0;
}

/*
 * rtc_read
 * 
 * DESCRIPTION: reads data from RTC
 * 
 * INPUTS: int32_t fd - file descriptor for device type
 *         void* buf - (user level) buffer to write values to
 *         int32_t nbytes - number of bytes to be read
 * OUTPUTS: none
 * RETURN VALUE: 0 - only AFTER an interrupt has occurred
 * 
 * SIDE EFFECTS: waits on interrupt handler to return
 */
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes) {
    // use a jump table referenced by the task's file array to call from a generic handler for this
    // call into a file-type-specific-function, jump-table should be inserted into the file array on 
    // the open system call. 

    /* wait for rtc_intr_handler to clear flag, then return 0 */
    sti();
    terminal[sched_term].rtc_iterations = terminal[sched_term].rtc_constant;
    while (terminal[sched_term].rtc_iterations != 0);
    cli();
    return 0;
}

/*
 * rtc_write
 * 
 * DESCRIPTION: writes data to the RTC
 * 
 * INPUTS: int32_t fd - file descriptor for device type
 *         const void* buf - (user level) buffer to write values from 
 *         int32_t nbytes - number of bytes to write to RTC
 * OUTPUTS: none
 * RETURN VALUE: number of bytes written 
 *               -1 - failure writing to regular files (read only)
 * 
 * SIDE EFFECTS: Updates RTC frequency
 */
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes) {
    /* check for valid argument */
    /* nbtyes cannont be 4 bytes */
    if (buf == NULL || nbytes != 4)
        return -1;

    /* casting argument to int32_t for access */
    /* 4-byte integer specifying interrupt rate in Hz */
    int32_t* buffer = (int32_t*) buf;

    /* check if frequency is not negative */
    if (*buffer > 0) {
        /* set RTC freq */
        terminal[sched_term].rtc_constant = _512HZ_ / (*buffer);
        return 0;
    } else {
        return -1;
    }
}

/*
 * rtc_close
 * 
 * DESCRIPTION: closes the specified file desciptor and makes it available for 
 *              return from later rtc_open calls
 * 
 * INPUTS: int32_t fd - file descriptor to close
 * OUTPUTS: none
 * RETURN VALUE: 0 - successful close
 *              -1 - attempting to close an invalid descriptor
 * 
 * SIDE EFFECTS: none
 */
int32_t rtc_close(int32_t fd) {
    return 0;
}

/*
 * rtc_update_freq
 * 
 * DESCRIPTION: updates the RTC frequency
 * 
 * INPUTS: uint32_t new_frequency - new integer frequency to set
 * OUTPUTS: none
 * RETURN VALUE: 0 - successful frequency set
 *               -1 - failure to change frequency (invalid arguments)
 * 
 * SIDE EFFECTS: frequency set in RTC registers
 */
int32_t rtc_update_freq(uint32_t new_frequency) {
    /* sets frequency back to 2 Hz (minimum) */
    uint32_t temp_frequency = _2HZ_SELECT_BITS;

    /* checks for valid argument (must be a power of 2) and not 0 or 1 and freq cannot be larger than 1024 since max freq is 512*/
    if ((new_frequency <= 1) || (new_frequency & (new_frequency - 1)) || (new_frequency > 1024))
        return -1;

    /* updates the value of new select to match register bits */
    while (new_frequency != _2HZ_) {
        /* double frequency by decrementing new_select */
        new_frequency >>= 1;
        temp_frequency--;
    }

    // get Register A and mask NMI
    outb(NMI_MASK | RTC_REG_A, INDEX_PORT);
    uint8_t old_regA = inb(CMOS_PORT);

    // set frequency on Register A value
    outb(NMI_MASK | RTC_REG_A, INDEX_PORT);
    outb((old_regA & LOW_HEX_MASK) | temp_frequency, CMOS_PORT);

    return 0;
}
