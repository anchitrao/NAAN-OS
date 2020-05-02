/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask = 0xFF; /* IRQs 0-7  , bitmask all interrupt lines on master */
uint8_t slave_mask = 0xFF;  /* IRQs 8-15 , bitmask all interrupt lines on slave */

/* 
 * i8259_init 
 *
 * Description: Initialize the 8259 PIC, send control words to correct ports for 
 * both master and slave 
 *  
 * Inputs: none
 * Return value: none
 */
void i8259_init(void) {
	// save flags and clear interrupt flag 
	uint32_t flags;
	cli_and_save(flags);

	// ICW1
	outb(ICW1, MASTER_COM_PORT);
	outb(ICW1, SLAVE_COM_PORT);
	
	// ICW2
	outb(ICW2_MASTER, MASTER_DATA_PORT);
	outb(ICW2_SLAVE, SLAVE_DATA_PORT);
	
	// ICW3
	outb(ICW3_MASTER, MASTER_DATA_PORT);
	outb(ICW3_SLAVE, SLAVE_DATA_PORT);

	// ICW4
	outb(ICW4, MASTER_DATA_PORT);
	outb(ICW4, SLAVE_DATA_PORT);

	// mask all interrupt lines on both PICs -> 0xFF = bitmask to mask all interrupt lines
	outb(0xFF, MASTER_DATA_PORT);
	outb(0xFF, SLAVE_DATA_PORT);	

	// unmask interrupt line the Slave PIC (IRQ 2) is connected to on Master PIC
	enable_irq(SLAVE_LINE);

	// restore flags and interrupt flag
	restore_flags(flags);
}

/* 
 * enable_irq
 * 
 * Description: Enable (unmask) the specified IRQ. Edits the master/slave bitmask
 * 
 * Inputs: irq_num = a value that can be between 0 and 15 and corresponds to which interrupt line was risen
 * Return value: none
 */
void enable_irq(uint32_t irq_num) {
	// save flags and clear interrupt flags
	uint32_t flags;
	cli_and_save(flags);

	// iteration variable
	int i;

	// validate irq_num to be within range of IRQ values (0 - 15)
	if (irq_num > 15 || irq_num < 0) {
		// restore flags and re-enable interrupts
		restore_flags(flags);

		// invalid irq_num, return
		return;
	}

	/* given IRQ_num, determine if it's in Master (IRQ's 0-7) or Slave PIC (IRQs 8-15) */
	// interrupt came from Master PIC
	if (irq_num >= 0 && irq_num <= 7) {
		// use temporary mask (1111 1110) and shift it by irq_num to get the interrupt line needing unmasking
		uint8_t temp_mask = 0xFE;	
		for (i = 0; i < irq_num; i++) 
			temp_mask = (temp_mask << 1) + 1;	

		// AND the temp_mask with the Master PIC bitmask to update the Master PIC bitmask
		master_mask = master_mask & temp_mask;
		
		// write the new master_mask to the Master PIC
		outb(master_mask, MASTER_DATA_PORT);	
	} 
	
	// interrupt came from Slave PIC
	else {
		// use temporary mask (1111 1110) and shift it by irq_num to get the interrupt line needing unmasking
		uint8_t temp_mask = 0xFE;	
		for (i = 0; i < (irq_num & 7); i++) 	// AND with 7 to get IRQ value between 0-7 relative to Slave PIC
			temp_mask = (temp_mask << 1) + 1;	

		// AND the temp_mask with the Master PIC bitmask to update the Master PIC bitmask
		slave_mask = slave_mask & temp_mask;
		
		// write the new master_mask to the Master PIC
		outb(slave_mask, SLAVE_DATA_PORT);	
	}

	// restore flags and re-enable interrupts
	restore_flags(flags);
}

/* 
 * disable_irq
 * 
 * Description: Disable (mask) the specifie//d IRQ. Edits master/slave bitmask respectively
 * 
 * Inputs: irq_num = a value that can be between 0 and 15 and corresponds to which interrupt line was risen
 * Return value: none
 */
void disable_irq(uint32_t irq_num) {
	// save flags and clear interrupt flags
	uint32_t flags;
	cli_and_save(flags);

	// iteration variable
	int i;

	// validate irq_num to be within range of irq values on Master/Slave PIC (0 - 15)
	if (irq_num > 15 || irq_num < 0) {
		// restore flags and reenable interrupts
		restore_flags(flags);

		// invalid irq_num, return
		return;
	}

	/* given IRQ_num, determine if it's in Master (IRQ's 0-7) or Slave PIC (IRQs 8-15) */
	// interrupt came from Master PIC
	if (irq_num >= 0 && irq_num <= 7) {
		// use temporary mask (0000 0001) and shift it by irq_num to get the interrupt line needing masking
		uint8_t temp_mask = 0x01;	
		for (i = 0; i < irq_num; i++) 
			temp_mask = temp_mask << 1;	

		// AND the temp_mask with the Master PIC bitmask to update the Master PIC bitmask
		master_mask = master_mask | temp_mask;
		
		// write the new master_mask to the Master PIC
		outb(master_mask, MASTER_DATA_PORT);	
	} 
	
	// interrupt came from Slave PIC
	else {
		// use temporary mask (0000 0001) and shift it by irq_num to get the interrupt line needing unmasking
		uint8_t temp_mask = 0x01;	
		for (i = 0; i < (irq_num & 7); i++) 	// AND with 7 to get IRQ value between 0-7 relative to Slave PIC
			temp_mask = temp_mask << 1;	

		// AND the temp_mask with the Master PIC bitmask to update the Master PIC bitmask
		slave_mask = slave_mask | temp_mask;
		
		// write the new master_mask to the Master PIC
		outb(slave_mask, SLAVE_DATA_PORT);	
	}

	// restore flags and re-enable interrupts
	restore_flags(flags);
}

/* 
 * send_eoi 
 * 
 * Description: Send end-of-interrupt signal for the specified IRQ. If interrupt came from Master, just send 
 * EOI signal to Master. If it came from Slave, send EOI to both Slave and Master
 * 
 * Inputs: irq_num = a value that can be between 0 and 15 and corresponds to which interrupt line was risen
 * Return value: none
 */
void send_eoi(uint32_t irq_num) {
	// save flags and clear interrupt flags
	uint32_t flags;
	cli_and_save(flags);

	// validate irq_num to be within range of irq values (0 - 15)
	if (irq_num > 15 || irq_num < 0) {
		// restore flags and re-enable interrupts
		restore_flags(flags);

		// invalid irq_num, return
		return;
	}

	// check whether irq_num came from Master PIC (IRQ's 0-7) or Slave PIC (IRQs 8-15)
	if (irq_num >= 8) {
		outb((EOI | (irq_num & 7)), SLAVE_COM_PORT);		// came from Slave PIC, OR EOI with IRQ 0-7 relative to Slave PIC
		outb(EOI + 2, MASTER_COM_PORT);						// send EOI signal to interrupt line Slave PIC is attached to on Master PIC
	} else {
		outb((EOI | irq_num) , MASTER_COM_PORT);			// came from Master PIC, send EOI OR'd with irq_num to Master PIC 	
	}

	// restore flags and re-enable interrupts
	restore_flags(flags);
	sti();
}
