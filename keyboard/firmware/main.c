#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include <avr/io.h>
#include <avr/interrupt.h>

#include "sharp.h"
#include "keyboard.h"

ISR(PCINT0_vect) {
	switch (keyboard_transmission_direction) {
		case KEYBOARD_TRANSMISSION_DEVICE_TO_HOST:
			if (!(KEYBOARD_CLOCK_PIN & _BV(KEYBOARD_CLOCK))) {
				if (TIFR & _BV(TOV0)) keyboard_clear_receive_buffer();
				keyboard_bit_received(KEYBOARD_DATA_PIN & _BV(KEYBOARD_DATA));
				TCNT0 = 128;
				TIFR |= _BV(TOV0);
			}
			break;
		case KEYBOARD_TRANSMISSION_HOST_TO_DEVICE:
			keyboard_send_bit();
			break;
	}
}

void keyboard_disable_clock_interrupt() {
	GIMSK &= ~_BV(PCIE0);
	PCMSK &= ~_BV(PCINT1);
}

void keyboard_enable_clock_interrupt() {
	GIMSK |= _BV(PCIE0);
	PCMSK |= _BV(PCINT1);
}


bool sharp_key_in(uint8_t *value) {
	if (keyboard_key_received_flag) {
		char c = keyboard_buffer_dequeue();
		return sharp_char_to_token(c, value);
	} else {
		return false;
	}
}

int main(void) {
	
	TCCR0B = 0b010 << CS00; // Prescaler = 8; timer overflows after 256us
	
	sharp_init();
	keyboard_reset();
	
	sei();
	
	for (;;) {
		if (!sharp_update()) {
			sharp_sleep();
		}
	}
	
}