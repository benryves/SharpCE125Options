#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include <avr/io.h>
#include <avr/interrupt.h>

#include "sharp.h"
#include "keyboard.h"

bool sharp_key_in(uint8_t *value) {
	if (keyboard_key_received_flag) {
		*value = keyboard_buffer_dequeue();
		return true;
	} else {
		return false;
	}
}

int main(void) {
	
	sharp_init();
	keyboard_reset();
	
	sei();
	
	for (;;) {
		if (!sharp_update()) {
			sharp_sleep();
		}
	}
	
}