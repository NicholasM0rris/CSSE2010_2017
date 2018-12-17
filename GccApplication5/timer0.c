/*
 * timer0.c
 *
 * Author: Peter Sutton
 *
 * We setup timer0 to generate an interrupt every 1ms
 * We update a global clock tick variable - whose value
 * can be retrieved using the get_clock_ticks() function.
 */

#include <avr/io.h>
#include <avr/interrupt.h>

#include "timer0.h"
#include "snake.h"

/* Our internal clock tick count - incremented every 
 * millisecond. Will overflow every ~49 days. */
static volatile uint32_t clock_ticks;

// Defining the seven segment display appropriate values
uint8_t seven_seg[10] = {63,6,91, 79, 102, 109,125,7,127,111};
// Defining the digit (right if 0 or left if 1) being displayed
static volatile uint8_t digit;

/* Set up timer 0 to generate an interrupt every 1ms. 
 * We will divide the clock by 64 and count up to 124.
 * We will therefore get an interrupt every 64 x 125
 * clock cycles, i.e. every 1 milliseconds with an 8MHz
 * clock. 
 * The counter will be reset to 0 when it reaches it's
 * output compare value.
 */
void init_timer0(void) {
	// Reset clock tick count. L indicates a long (32 bit) constant. 
	clock_ticks = 0L;
	
	/* Clear the timer */
	TCNT0 = 0;

	/* Set the output compare value to be 124 */
	OCR0A = 124;
	
	/* Set the timer to clear on compare match (CTC mode)
	 * and to divide the clock by 64. This starts the timer
	 * running.
	 */
	TCCR0A = (1<<WGM01);
	TCCR0B = (1<<CS01)|(1<<CS00);

	/* Enable an interrupt on output compare match. 
	 * Note that interrupts have to be enabled globally
	 * before the interrupts will fire.
	 */
	TIMSK0 |= (1<<OCIE0A);
	
	/* Make sure the interrupt flag is cleared by writing a 
	 * 1 to it.
	 */
	TIFR0 &= (1<<OCF0A);
	
	// Setting things related to snake length display
	/* Set all the bits of PORTA to be outputs, because
	 * that's on PORTA that the SSD is connected
	 */
	DDRA = 0xFF;
	// Set pin 0 in PORTC to be an output
	DDRC = (1<<DDRC0);
	//starting with the 'right' number on the seven seg display
	digit = 0; 
	
	}

uint32_t get_clock_ticks(void) {
	uint32_t return_value;

	/* Disable interrupts so we can be sure that the interrupt
	 * doesn't fire when we've copied just a couple of bytes
	 * of the value. Interrupts are re-enabled if they were
	 * enabled at the start.
	 */
	uint8_t interrupts_were_on = bit_is_set(SREG, SREG_I);
	cli();
	return_value = clock_ticks;
	if(interrupts_were_on) {
		sei();
	}
	return return_value;
}

/* display_digit gets called every time the timer compare interrupt occurs.
 * every time it gets called, the digit displayed is changed
 */
void display_length(uint8_t snakeLength, uint8_t digit) {
	if(digit == 0) {
		//Right side of display
		PORTA = seven_seg[snakeLength % 10];
		// we toggle digit, so that next time display_digit is called
		// it will be the left number displayed
		digit = digit + 1;
	}
	else {
		//Left side of the display
		PORTA = seven_seg[snakeLength / 10];
		// we toggle digit, so that next time display_digit is called
		// it will be the right number displayed
		digit = 1 - digit; 
	}
	
}

/* Interrupt handler which fires when timer/counter 0 reaches 
 * the defined output compare value (i.e 124: every millisecond)
 */
ISR(TIMER0_COMPA_vect) {
	/* Increment our clock tick count */
	clock_ticks++;
	//toggle the CC pin to display the Snake length
	display_length(get_snake_length(), digit);
}
	

