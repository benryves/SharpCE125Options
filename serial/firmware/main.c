#include <stdlib.h>
#include <stdbool.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/pgmspace.h>

#include <util/delay.h>

#include "sharp.h"
#include "uart.h"

const char rom[] PROGMEM = {
	0xE0, 0x10, 0x12, 0x51, 0x12, 0x1D, 0xC2, 0x12, 0x63, 0x64, 0x51, 0x62, 0x64, 0x59, 0x5E, 0x57, 0x11, 0x60, 0x5F, 0x59, 0x5E, 0x64, 0x34, 0x12, 0x1C, 0x66, 0x00,
	0xE0, 0x20, 0xC2, 0x12, 0x5D, 0x59, 0x5E, 0x65, 0x64, 0x55, 0x11, 0x59, 0x5E, 0x64, 0x55, 0x62, 0x66, 0x51, 0x5C, 0x34, 0x12, 0x1C, 0x51, 0x00,
	0xE0, 0x30, 0xC2, 0x12, 0x59, 0x5E, 0x64, 0x55, 0x62, 0x66, 0x51, 0x5C, 0x34, 0x12, 0x1C, 0x67, 0x00,
	0xE0, 0x40, 0x57, 0x34, 0x66, 0x1D, 0x56, 0x34, 0x66, 0x1D, 0x6A, 0x34, 0x40, 0x00,
	0xE0, 0x50, 0xD0, 0x6A, 0x34, 0x40, 0xD7, 0x47, 0x40, 0x00,
	0xE0, 0x60, 0x57, 0x34, 0x57, 0x36, 0x67, 0x1D, 0x53, 0x34, 0x57, 0x1D, 0xD7, 0x48, 0x40, 0x00,
	0xE0, 0x70, 0x53, 0x34, 0x57, 0x1D, 0x6A, 0x34, 0x41, 0x00,
	0xE0, 0x80, 0xD8, 0x43, 0x40, 0x40, 0x00,
	0xE0, 0x90, 0x56, 0x34, 0x56, 0x35, 0x67, 0x1D, 0x53, 0x34, 0x56, 0x00,
	0xE1, 0x00, 0xD8, 0x43, 0x40, 0x40, 0x00,
	0xE1, 0x10, 0xD7, 0x45, 0x40, 0x00,
	0xE1, 0x20, 0xD4, 0x00,
	0xE3, 0x00, 0x68, 0x34, 0x53, 0x1D, 0xD8, 0x45, 0x40, 0x40, 0x00,
	0xE3, 0x10, 0x69, 0x34, 0x52, 0x1D, 0x68, 0x34, 0x51, 0x35, 0x53, 0x00,
	0xE3, 0x20, 0xD8, 0x45, 0x40, 0x40, 0x00,
	0xE3, 0x30, 0x54, 0x34, 0x53, 0x1D, 0x53, 0x34, 0x54, 0x36, 0x51, 0x37, 0x69, 0x38, 0x30, 0x52, 0x36, 0x69, 0x31, 0x00,
	0xE3, 0x40, 0xD0, 0xAA, 0x30, 0x54, 0x36, 0x53, 0x31, 0x82, 0x4B, 0x36, 0x48, 0xD7, 0x43, 0x40, 0x40, 0x00,
	0xE3, 0x50, 0xDB, 0x43, 0x1D, 0xC1, 0x12, 0x51, 0x5E, 0x63, 0x67, 0x55, 0x62, 0x34, 0x12, 0x1B, 0x53, 0x00,
	0xE3, 0x60, 0xDE, 0x00,
	0xE5, 0x00, 0x52, 0x34, 0x30, 0x30, 0x68, 0x36, 0x42, 0x31, 0x37, 0x68, 0x36, 0x41, 0x31, 0x37, 0x68, 0x35, 0x42, 0x00,
	0xE5, 0x10, 0xDE, 0x00,
};

void sharp_serial_outstat(uint8_t value) {
	if (value & _BV(0)) {
		UART_PORT &= ~_BV(UART_DTR);
	} else {
		UART_PORT |= _BV(UART_DTR);
	}
	if (value & _BV(1)) {
		UART_PORT &= ~_BV(UART_RTS);
	} else {
		UART_PORT |= _BV(UART_RTS);
	}
}

uint8_t sharp_serial_instat(void) {
	uint8_t result = 0;
	uint8_t uart_state = ~UART_PIN;
	if (uart_state & _BV(UART_DTR)) result |= _BV(0);
	if (uart_state & _BV(UART_RTS)) result |= _BV(1);
	if (uart_state & _BV(UART_CTS)) result |= _BV(2);
	if (uart_state & _BV(UART_DCD)) result |= _BV(3);
	if (uart_state & _BV(UART_DSR)) result |= _BV(4);
	if (uart_state & _BV(UART_RI)) result |= _BV(5);
	return result;
}

uint16_t baud_rates[] = {
	300,
	110,
	1200,
};

uint8_t enabled_devices = 0;

void sharp_serial_setcom(uint8_t mode, uint8_t devices) {
	
	uint8_t baud_index = mode & 3;
	uint8_t parity = (mode >> 2) & 3;
	uint8_t data_bits = 8 - ((mode >> 4) & 1);
	uint8_t stop_bits = 1 + ((mode >> 5) & 1);
	
	if (baud_index < sizeof(baud_rates) && parity < 3) {
		uart_empty_read_buffer();
		uart_empty_write_buffer();
		uart_init(baud_rates[baud_index], data_bits, parity, stop_bits);
	}
	
	enabled_devices = devices;
}

const uint8_t device_names[] PROGMEM = {
	'K' + 0x90, // KI
	'D' + 0x10, // DO
	'P' + 0x10, // PO
	'C' + 0x90, // CI
	'C' + 0x10, // CO
};

void sharp_serial_comstring(bool (*ret)(uint8_t)) {
	
	uint16_t baud;
	uint8_t data_bits, parity, stop_bits;
	
	uart_get_settings(&baud, &data_bits, &parity, &stop_bits);
	
	// Build up the string
	char com[24];
	itoa(baud, com, 10);
	
	char *p = com;
	while (*p) {
		*p += (0x40 - '0');
		++p;
	}
	*p++ = 0x1B;
	
	if (parity < 3) {
		switch (parity) {
			case UART_PARITY_NONE: *p++ = 0x5E; break;
			case UART_PARITY_EVEN: *p++ = 0x55; break;
			case UART_PARITY_ODD:  *p++ = 0x5F; break;
		}
		*p++ = 0x1B;
	}
	
	*p++ = data_bits + 0x40;
	*p++ = 0x1B;
	*p++ = stop_bits + 0x40;
	
	*p++ = 0;
	
	// Transmit the string
	p = com;
	while (*p) {
		if (!ret(*p++)) return;
	}
	
	// Send the list of enabled devices
	for (uint8_t mask = 0x01, device = 0; device < sizeof(device_names); ++device, mask <<= 1) {
		if (enabled_devices & mask) {
			uint8_t device_name = pgm_read_byte(&device_names[device]);
			if (!ret(0x1B) ||
				!ret(device_name & 0x7F) ||
				!ret(device_name & 0x80 ? 0x59 : 0x5F)
			) return;
		}
	}
}

uint8_t llist_column_count = 0;
uint8_t llist_column_limit = 80;
uint8_t llist_column_indent = 4;

void print(char c) {
	uart_write_byte(c);
	if (c == '\r') {
		uart_write_byte('\n');
	}
}

void sharp_serial_data_out(uint8_t mode, uint8_t value) {
	char c;
	switch (mode) {
		case SHARP_CMD_PRINT:
		case SHARP_CMD_LPRINT:
			if (sharp_token_to_char(value, &c)) {
				print(c);
			}
			break;
		case SHARP_CMD_LLIST:
			if (value != 0x0D) {
				if (value == 0x00 || value == 0x03) {
					print('\r');
					llist_column_count = 0;
				} else if (sharp_token_to_char(value, &c)) {
					// Do we need to force wrap?
					if (llist_column_limit && llist_column_count == llist_column_limit) {
						print('\r');
						// Indent the new line
						for (llist_column_count = 0; llist_column_count < llist_column_indent; ++llist_column_count) {
							print(' ');
						}
					}
					// Print the character
					if (c != ' ' || llist_column_count < 24 || llist_column_count > 27) {
						print(c);
					}
					// Count the number of printed columns
					++llist_column_count;
					if (!llist_column_limit && llist_column_count == 48) {
						llist_column_count = 24;
					}
				}
			}
			break;
		default:
			uart_write_byte(value);
			break;
	}
}

bool sharp_serial_data_in(uint8_t mode, uint8_t *value) {
	if (uart_read_byte(value)) {
		switch (mode) {
			case SHARP_CMD_KEYGET:
			case SHARP_CMD_INPUT:
				if (sharp_char_to_token((char)*value, value)) return true;
				break;
			default:
				return true;
		}
	}
	return false;
}

bool sharp_key_in(uint8_t *value) {
	return sharp_serial_data_in(SHARP_CMD_KEYGET, value);
}

bool sharp_rom(bool (*ret)(uint8_t)) {
	for (size_t i = 0; i < sizeof(rom); ++i) {
		if (!ret(pgm_read_byte(&rom[i]))) return false;
	}
	return true;
}

void sharp_serial_data_begin(uint8_t mode) {
	llist_column_count = 0;
}

void sharp_serial_data_end(uint8_t mode) {
}


int main(void) {
	
	sharp_init();
	sharp_serial_setcom(0, 0);
	
	sei();
	
	for (;;) {
		sharp_update();
	}
	
}