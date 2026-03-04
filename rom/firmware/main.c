#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "sharp.h"

const PROGMEM uint8_t rom_data[] = {
	0xE0, 0x10, 0xC1, 0x12, 0x58, 0x55, 0x5C, 0x5C, 0x5F, 0x1B, 0x11, 0x67, 0x5F, 0x62, 0x5C, 0x54, 0x14, 0x12, 0x00,
};

bool sharp_rom(bool (*ret)(uint8_t)) {
	for (size_t i = 0; i < sizeof(rom_data); ++i) {
		if (!ret(pgm_read_byte(rom_data + i))) return false;
	}
	return true;
}

int main(void) {
	
	sharp_init();
	
	sei();
	
	for (;;) {
		if (!sharp_update()) {
			sharp_sleep();
		}
	}
	
}