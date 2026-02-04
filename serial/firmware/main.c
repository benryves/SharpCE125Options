#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "sharp.h"
#include "uart.h"

uint8_t last_outstat = 3;

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
	last_outstat = value;
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

void uart_write_byte_handshaking(uint8_t value) {
	// CTS, DCD and DSR must all be high
	while (~UART_PIN & (_BV(UART_CTS) | _BV(UART_DCD) | _BV(UART_DSR))) {
		// Bail out of ACL signal received
		if (SHARP_PIN & _BV(SHARP_ACL)) return;
	}
	uart_write_byte(value);
}

void print(char c) {
	uart_write_byte_handshaking(c);
	if (c == '\r') {
		uart_write_byte_handshaking('\n');
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
			uart_write_byte_handshaking(value);
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

uint8_t last_serial_mode = 0;

void sharp_serial_data_begin(uint8_t mode) {
	llist_column_count = 0;
	if (mode != last_serial_mode) {
		if (mode != SHARP_CMD_KEYGET && mode != SHARP_CMD_INPUT) {
			uart_empty_read_buffer();
		}
		last_serial_mode = mode;
	}
	uart_empty_write_buffer();
}

void sharp_serial_data_end(uint8_t mode) {
	while (uart_is_busy() && !(SHARP_PIN & _BV(SHARP_ACL)));
	sharp_serial_outstat(last_outstat);
}

int main(void) {
	
	sharp_init();
	sharp_serial_setcom(0, 0);
	
	sei();
	
	for (;;) {
		if (!sharp_update()) {
			sharp_sleep();
		}
	}
	
}