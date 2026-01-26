#include <stdlib.h>
#include <stdbool.h>

#include <avr/io.h>
#include <avr/interrupt.h>

#include "uart.h"

#define SHARP_CE125_OPTION_OP2  (0)
#define SHARP_CE125_OPTION_ACL  (1)
#define SHARP_CE125_OPTION_BUSY (2)
#define SHARP_CE125_OPTION_DOUT (3)
#define SHARP_CE125_OPTION_DIN  (4)
#define SHARP_CE125_OPTION_ACK  (5)

#define SHARP_CE125_OPTION_PIN  (PIND)
#define SHARP_CE125_OPTION_PORT (PORTD)
#define SHARP_CE125_OPTION_DDR  (DDRD)

#define SHARP_CE125_OPTION_CMD_COMSTRING (0x2B)
#define SHARP_CE125_OPTION_CMD_KEYONOFF  (0x2E)
#define SHARP_CE125_OPTION_CMD_SETCOM    (0x70)
#define SHARP_CE125_OPTION_CMD_OUTSTAT   (0x71)
#define SHARP_CE125_OPTION_CMD_PRINT     (0x72)
#define SHARP_CE125_OPTION_CMD_LPRINT    (0x73)
#define SHARP_CE125_OPTION_CMD_LLIST     (0x74)
#define SHARP_CE125_OPTION_CMD_CSAVE     (0x75)
#define SHARP_CE125_OPTION_CMD_PRINTFILE (0x76)
#define SHARP_CE125_OPTION_CMD_INSTAT    (0x77)
#define SHARP_CE125_OPTION_CMD_INPUT     (0x78)
#define SHARP_CE125_OPTION_CMD_CLOAD     (0x7A)
#define SHARP_CE125_OPTION_CMD_INPUTFILE (0x7B)
#define SHARP_CE125_OPTION_CMD_ROM       (0x7C)
#define SHARP_CE125_OPTION_CMD_KEYGET    (0x7E)

uint8_t sharp_ce125_option_bits_received = 0;
uint8_t sharp_ce125_option_byte_in = 0;

void sharp_ce125_option_init(void) {
	
}

void sharp_ce125_option_acl(void) {
	sharp_ce125_option_bits_received = 0;
}

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

void sharp_ce125_option_received_byte(uint8_t value) {
	uart_write_byte('&');
	debug_hex_byte(value);
	uart_write_byte(' ');
}

int main(void) {
	
	uart_init();
	sharp_ce125_option_init();
	
	sei();
	
	for (;;) {
		
		if (!(SHARP_CE125_OPTION_PIN & _BV(SHARP_CE125_OPTION_OP2))) {
			// Addressing some other device, not us
			sharp_ce125_option_bits_received = 0;
		} else if (SHARP_CE125_OPTION_PIN & _BV(SHARP_CE125_OPTION_ACL)) {
			// ACL: Clear
			uart_write_byte('A');
			uart_write_byte('C');
			uart_write_byte('L');
			uart_write_byte(' ');
			while ((SHARP_CE125_OPTION_PIN & _BV(SHARP_CE125_OPTION_ACL)) && (SHARP_CE125_OPTION_PIN & _BV(SHARP_CE125_OPTION_OP2)));
			sharp_ce125_option_acl();
		} else if (SHARP_CE125_OPTION_PIN & _BV(SHARP_CE125_OPTION_BUSY)) {
			// Receiving a bit of data
			
			// Clock in the bit
			sharp_ce125_option_byte_in >>= 1;
			if (SHARP_CE125_OPTION_PIN & _BV(SHARP_CE125_OPTION_DOUT)) {
				sharp_ce125_option_byte_in |= _BV(7);
			}
			
			// Drive ACK high
			SHARP_CE125_OPTION_DDR |= _BV(SHARP_CE125_OPTION_ACK);
			SHARP_CE125_OPTION_PORT |= _BV(SHARP_CE125_OPTION_ACK);
			
			// Wait for BUSY to go back low
			while (SHARP_CE125_OPTION_PIN & _BV(SHARP_CE125_OPTION_BUSY));
			
			// Release ACK
			SHARP_CE125_OPTION_PORT &= ~_BV(SHARP_CE125_OPTION_ACK);
			SHARP_CE125_OPTION_DDR &= ~_BV(SHARP_CE125_OPTION_ACK);
			
			// Have we received a full byte?
			if (++sharp_ce125_option_bits_received == 8) {
				sharp_ce125_option_bits_received = 0;
				sharp_ce125_option_received_byte(sharp_ce125_option_byte_in);
			}
			
		}
		
	}
	
	
}