#include <stdlib.h>
#include <stdbool.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/pgmspace.h>

#include <util/delay.h>

#include "sharp.h"
#include "uart.h"



void debug_hex_nybble(uint8_t value) {
	value &= 0xF;
	if (value < 10) {
		uart_write_byte('0' + value);
	} else {
		uart_write_byte('A' + value - 10);
	}
}
void debug_hex_byte(uint8_t value) {
	debug_hex_nybble(value >> 4);
	debug_hex_nybble(value);
}


void sharp_outstat(uint8_t value) {
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

uint8_t sharp_instat(void) {
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

void sharp_setcom(uint8_t mode, uint8_t devices) {
	
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

void sharp_comstring(void) {
	
	
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
		if (!sharp_send_byte(*p++)) return;
	}
	
	// Send the list of enabled devices
	for (uint8_t mask = 0x01, device = 0; device < sizeof(device_names); ++device, mask <<= 1) {
		if (enabled_devices & mask) {
			uint8_t device_name = pgm_read_byte(&device_names[device]);
			if (!sharp_send_byte(0x1B) ||
				!sharp_send_byte(device_name & 0x7F) ||
				!sharp_send_byte(device_name & 0x80 ? 0x59 : 0x5F)
			) return;
		}
	}
	
}

void sharp_print(uint8_t token) {
	char c;
	if (token == 0x0D) {
		sharp_data_out('\r');
		sharp_data_out('\n');
	} else if (sharp_token_to_char(token, &c)) {
		sharp_data_out(c);
	}
}

void sharp_data_out(uint8_t value) {
	uart_write_byte(value);
}

int main(void) {
	
	sharp_init();
	sharp_setcom(0, 0);
	
	sei();
	
	uint8_t value;
	
	for (;;) {
		
		sharp_update();
		
		if (uart_read_byte(&value)) {
			// Transmit a byte of data back to the computer
			switch (sharp_send_mode) {
				case SHARP_CMD_KEYGET:
				case SHARP_CMD_INPUT:
					if (sharp_char_to_token(value, &value)) {
						sharp_return_byte(value);
					}
					break;
				default:
					sharp_return_byte(value);
					break;
			}
		}
		
	}
	
	
}