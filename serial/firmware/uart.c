#include <stdlib.h>
#include <stdbool.h>

#include <avr/io.h>
#include <avr/interrupt.h>

#include <util/atomic.h>

#include "uart.h"

uint16_t uart_baud = 0;

volatile uint8_t uart_data_out = 0;
volatile uint8_t uart_data_out_counter = 0;
volatile uint8_t uart_data_out_parity = 0;

volatile uint8_t uart_data_in = 0;
volatile bool uart_data_in_get_parity = false;

volatile uint8_t uart_in_buffer[UART_IN_BUFFER_SIZE];
volatile uint8_t uart_in_buffer_read = 0;
volatile uint8_t uart_in_buffer_write = 0;
volatile uint8_t uart_in_buffer_count = 0;

volatile uint8_t uart_out_buffer[UART_OUT_BUFFER_SIZE];
volatile uint8_t uart_out_buffer_read = 0;
volatile uint8_t uart_out_buffer_write = 0;
volatile uint8_t uart_out_buffer_count = 0;

volatile uint8_t uart_previous_pin = 0;

uint8_t uart_timer_period;
uint8_t uart_timer_period_half;

uint8_t uart_data_bits;
uint8_t uart_data_and_parity_bits;
uint8_t uart_total_bits;
uint8_t uart_parity;

ISR(UART_RXD_PCINT_vect) {
	
	// Capture new state of the pins
	uint8_t uart_current_pin = UART_PIN;
	// What changed?
	uint8_t uart_changed_pin = (uart_current_pin ^ uart_previous_pin) & PCMSK;
	
	// Is it a start bit from the UART?
	if (uart_changed_pin & uart_current_pin & _BV(UART_RXD)) {
		// Disable UART pin change interrupts
		UART_RXD_PCINT_MSK &= ~_BV(UART_RXD_PCINT);
		
		// Get Timer0 running to capture the first bit
		OCR0A = uart_timer_period_half;
		TCNT0 = 0;
		TIFR = _BV(OCF0A);
		TIMSK |= _BV(OCIE0A);
	}
	
	// Store previous state of the pins
	uart_previous_pin = uart_current_pin;
}

ISR(TIMER0_COMPA_vect) {
	// We're now half a bit into receiving the data
	TIMSK &= ~_BV(OCIE0A);
	OCR0A = uart_timer_period;
	TCNT0 = 0;
	USICR = _BV(USIOIE) | _BV(USICS0);
	USISR = _BV(USIOIF) | ((16 - uart_data_bits) << USICNT0);
	uart_data_in_get_parity = uart_parity != UART_PARITY_NONE;
}

ISR(USI_OVERFLOW_vect) {
	
	// Capture USI data
	uint8_t value = USIBR;
	
	// Are we waiting for parity?
	if (uart_data_in_get_parity) {
		
		// Get the parity bit
		uart_data_in = value;
		USISR = _BV(USIOIF) | (15 << USICNT0);
		uart_data_in_get_parity = false;
		
	} else {
		
		// Switch off the USI
		USICR = 0;
		
		// We either just got the parity bit, or didn't need one.
		bool parity;
		if (uart_parity == UART_PARITY_NONE) {
			parity = true;
		} else {
			parity = value & 1;
			value = uart_data_in;
		}
		
		// Swap bit order
		value = (value & 0xF0) >> 4 | (value & 0x0F) << 4;
		value = (value & 0xCC) >> 2 | (value & 0x33) << 2;
		value = (value & 0xAA) >> 1 | (value & 0x55) << 1;
		
		// Invert and align
		value = ~value >> (8 - uart_data_bits);
		
		// Check parity
		if (uart_parity) {
			uint8_t parity_calc = uart_parity;
			uint8_t parity_data = value;
			uint8_t parity_len = uart_data_bits;
			while (parity_len -- > 0) {
				parity_calc += parity_data & 1;
				parity_data >>= 1;
			}
			parity = (parity_calc & 1) == parity;
		}
		
		// Invert and store
		if (parity && (uart_in_buffer_count < UART_IN_BUFFER_SIZE)) {
			uart_in_buffer[uart_in_buffer_write] = value;
			if (++uart_in_buffer_write == UART_IN_BUFFER_SIZE) {
				uart_in_buffer_write = 0;
			}
			++uart_in_buffer_count;
		}
		
		// Re-enable pin change interrupts to capture next byte
		GIFR |= _BV(UART_RXD_PCINT_IF);
		UART_RXD_PCINT_MSK |= _BV(UART_RXD_PCINT);
		uart_previous_pin = UART_PIN;
	}
}

ISR(UART_TXD_COMP_vect) {
	
	// Is it the first bit? If so, fetch the byte to transmit.
	if (uart_data_out_counter == 0) {
		uart_data_out = uart_out_buffer[uart_out_buffer_read] ^ (0xFF >> (8 - uart_data_bits));
		if (++uart_out_buffer_read == UART_OUT_BUFFER_SIZE) {
			uart_out_buffer_read = 0;
		}
		--uart_out_buffer_count;
		uart_data_out_parity = uart_parity;
	}
	
	// Value of next bit to clock out
	if (uart_data_out & 1) {
		#ifdef TCCR1A
		TCCR1A |= _BV(UART_TXD_COM0);
		#endif
		#ifdef TCCR1
		TCCR1 |= _BV(UART_TXD_COM0);
		#endif
	} else {
		#ifdef TCCR1A
		TCCR1A &= ~_BV(UART_TXD_COM0);
		#endif
		#ifdef TCCR1
		TCCR1 |= _BV(UART_TXD_COM0);
		#endif
		++uart_data_out_parity;
	}
	
	// Check end of byte
	if (uart_data_out_counter <= uart_data_and_parity_bits) {
		// Advance to next bit
		uart_data_out >>= 1;
		++uart_data_out_counter;
		// If we've reached the last bit and have parity enabled, 
		if (uart_parity && uart_data_out_counter == uart_data_bits && (uart_data_out_parity & 1)) {
			uart_data_out |= 1;
		}
	} else {
		if (uart_out_buffer_count == 0) {
			if (uart_data_out_counter > uart_total_bits) {
				// Buffer is empty and we've finished sending the stop bit, so disable interrupts
				TIMSK &= ~_BV(UART_TXD_OCIE);
			} else {
				// Pad out the final stop bit to ensure gaps between bytes
				++uart_data_out_counter;
			}
		} else if (uart_data_out_counter < uart_total_bits) {
			// Still more in the buffer, but we're working on the stop bit
			++uart_data_out_counter;
		} else {
			// Still more in the buffer, so prepare the next iteration
			uart_data_out_counter = 0;
			#ifdef TCCR1A
			TCCR1A |= _BV(UART_TXD_COM0);
			#endif
			#ifdef TCCR1
			TCCR1 |= _BV(UART_TXD_COM0);
			#endif
			
		}
	}
}

void uart_write_byte(uint8_t value) {
	
	// Wait for free space to appear in the buffer
	while (uart_out_buffer_count >= UART_OUT_BUFFER_SIZE);
	
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		
		// Enqueue the byte to send
		uart_out_buffer[uart_out_buffer_write] = value;
		if (++uart_out_buffer_write == UART_OUT_BUFFER_SIZE) {
			uart_out_buffer_write = 0;
		}
		++uart_out_buffer_count;
		
		// If not currently sending, trigger the first bit to kick off the transfer
		if (!(TIMSK & _BV(UART_TXD_OCIE))) {
			
			// Start bit
			uart_data_out_counter = 0;
			TCNT1 = 0;
			
			#ifdef TCCR1A
			TCCR1A |= _BV(UART_TXD_COM0);
			#endif
			#ifdef TCCR1
			TCCR1 |= _BV(UART_TXD_COM0);
			#endif
			
			// Enable the interrupt to prepare the next bit
			TIFR |= _BV(UART_TXD_OCF);
			TIMSK |= _BV(UART_TXD_OCIE);
			
			// Trigger the interrupt
			TCNT1 = uart_timer_period - 1;
		}
	}
	
}

void uart_empty_write_buffer(void) {
	if (uart_out_buffer_count > 0) {
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
			uart_out_buffer_count = uart_out_buffer_read = uart_out_buffer_write = 0;
		}
	}
}

bool uart_read_byte(uint8_t *value) {
	bool read = false;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		if (uart_in_buffer_count > 0) {
			*value = uart_in_buffer[uart_in_buffer_read];
			if (++uart_in_buffer_read == UART_IN_BUFFER_SIZE) {
				uart_in_buffer_read = 0;
			}
			--uart_in_buffer_count;
			read = true;
		}
	}
	return read;
}

void uart_empty_read_buffer(void) {
	if (uart_in_buffer_count > 0) {
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
			uart_in_buffer_count = uart_in_buffer_read = uart_in_buffer_write = 0;
		}
	}
}

void uart_init(uint16_t baud, uint8_t data_bits, uint8_t parity, uint8_t stop_bits) {
	
	// Calculate the timer settings based on the baud rate
	uint32_t period = (uint32_t)F_CPU / baud;
	uint8_t prescaler = 1;
	
	while (period > 256 && prescaler < 5) {
		period >>= 2;
		if (prescaler < 3) {
			period >>= 1;
		}
		++prescaler;
	}
	
	uart_timer_period = (uint8_t)(period - 1);
	uart_timer_period_half = (uart_timer_period / 2) - UART_TIMER_SETUP_CYCLES; // TODO: Fix fudge factor
	uart_baud = baud;
	
	// Set the number of data bits, parity and stop bits
	uart_data_bits = data_bits;
	uart_data_and_parity_bits = data_bits;
	if ((uart_parity = parity) != UART_PARITY_NONE) {
		++uart_data_and_parity_bits;
	}
	uart_total_bits = uart_data_and_parity_bits + stop_bits;
	
	// Drive UART outputs
	UART_PORT &= ~(_BV(UART_TXD) | _BV(UART_DTR) | _BV(UART_RTS));
	UART_PORT |= _BV(UART_DCD) | _BV(UART_DSR) | _BV(UART_RI);
	UART_DDR |= _BV(UART_TXD) | _BV(UART_DTR) | _BV(UART_RTS);
	
	// Configure Timer0 as the UART receiver clock
	TCCR0A = (2 << WGM00);
	TCCR0B = (prescaler << CS00);
	#ifdef PSR0
	GTCCR |= _BV(PSR0);
	#endif
	
	// Configure Timer1 as the UART transmitter clock
	#ifdef TCCR1A
	TCCR1A = _BV(UART_TXD_COM1);
	#endif
	#ifdef TCCR1B
	TCCR1B = _BV(WGM12) | (prescaler << CS10);
	#endif
	#ifdef TCCR1
	TCCR1 = _BV(CTC1) | _BV(UART_TXD_COM1) | (UART_TIMER1_CLOCKSEL << CS10);
	#endif
	
	OCR1A = uart_timer_period;
	#ifdef OCR1C
	OCR1C = uart_timer_period;
	#endif
	
	#ifdef PSR10
	GTCCR |= _BV(PSR10);
	#endif

	// Enable pin-change interrupts
	uart_previous_pin = UART_PIN;
	UART_RXD_PCINT_MSK |= _BV(UART_RXD_PCINT);
	GIMSK |= _BV(UART_RXD_PCINT_IE);
}

void uart_get_settings(uint16_t *baud, uint8_t *data_bits, uint8_t *parity, uint8_t *stop_bits) {
	if (baud) *baud = uart_baud;
	if (data_bits) *data_bits = uart_data_bits;
	if (parity) *parity = uart_parity;
	if (stop_bits) *stop_bits = uart_total_bits - uart_data_and_parity_bits;
}