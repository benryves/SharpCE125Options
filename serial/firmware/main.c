#include <stdlib.h>
#include <stdbool.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/pgmspace.h>

#include <util/delay.h>

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

const uint8_t program[] PROGMEM = {
	0xE0, 0x05, 0x12, 0x51, 0x12, 0x1D, 0xC5, 0x1D, 0xC4, 0x1D, 0x9F, 0x12, 0x12, 0x00,
	0xE0, 0x10, 0xC2, 0x12, 0x5E, 0x51, 0x5D, 0x55, 0x34, 0x12, 0x1C, 0x54, 0x18, 0x00,
	0xE0, 0x20, 0x9F, 0x12, 0x5E, 0x51, 0x5D, 0x55, 0x1D, 0x12, 0x1C, 0x54, 0x18, 0x00,
	0xE0, 0x30, 0xC2, 0x12, 0x54, 0x51, 0x64, 0x55, 0x11, 0x5F, 0x56, 0x11, 0x52, 0x59, 0x62, 0x64, 0x58, 0x1D, 0x69, 0x55, 0x51, 0x62, 0x34, 0x12, 0x1C, 0x65, 0x18, 0x1B, 0x12, 0x5D, 0x5F, 0x5E, 0x64, 0x58, 0x34, 0x12, 0x1C, 0x66, 0x1B, 0x12, 0x54, 0x51, 0x69, 0x34, 0x12, 0x1C, 0x67, 0x00,
	0xE0, 0x35, 0x56, 0x18, 0x34, 0x8B, 0x66, 0x1D, 0x57, 0x18, 0x34, 0x8B, 0x67, 0x00,
	0xE0, 0x40, 0x9F, 0x12, 0x54, 0x51, 0x64, 0x55, 0x11, 0x5F, 0x56, 0x11, 0x52, 0x59, 0x62, 0x64, 0x58, 0x1D, 0x12, 0x1D, 0x9F, 0x12, 0x11, 0x11, 0x11, 0x11, 0x12, 0x1C, 0x65, 0x18, 0x1C, 0x12, 0x1B, 0x12, 0x1C, 0x56, 0x18, 0x1C, 0x12, 0x1B, 0x12, 0x1C, 0x57, 0x18, 0x00,
	0xE0, 0x50, 0xD8, 0x45, 0x40, 0x40, 0x1D, 0x52, 0x34, 0x68, 0x00,
	0xE0, 0x60, 0xC2, 0x12, 0x54, 0x55, 0x63, 0x59, 0x62, 0x55, 0x54, 0x1D, 0x69, 0x55, 0x51, 0x62, 0x34, 0x12, 0x1C, 0x65, 0x18, 0x1B, 0x12, 0x5D, 0x5F, 0x5E, 0x64, 0x58, 0x34, 0x12, 0x1C, 0x66, 0x1D, 0x67, 0x34, 0x40, 0x00,
	0xE0, 0x65, 0x56, 0x18, 0x34, 0x8B, 0x66, 0x00,
	0xE0, 0x70, 0x9F, 0x12, 0x54, 0x55, 0x63, 0x59, 0x62, 0x55, 0x54, 0x11, 0x69, 0x55, 0x51, 0x62, 0x1B, 0x5D, 0x5F, 0x5E, 0x64, 0x58, 0x1D, 0x12, 0x1D, 0x9F, 0x12, 0x11, 0x11, 0x11, 0x11, 0x12, 0x1C, 0x65, 0x18, 0x1C, 0x12, 0x1B, 0x12, 0x1C, 0x56, 0x18, 0x00,
	0xE0, 0x75, 0x9F, 0x12, 0x12, 0x00,
	0xE0, 0x80, 0xD8, 0x45, 0x40, 0x40, 0x1D, 0x51, 0x34, 0x68, 0x00,
	0xE1, 0x00, 0x53, 0x34, 0x51, 0x36, 0x52, 0x00,
	0xE1, 0x10, 0xD8, 0x47, 0x40, 0x40, 0x00,
	0xE1, 0x20, 0xD8, 0x48, 0x40, 0x40, 0x00,
	0xE1, 0x30, 0x54, 0x34, 0x53, 0x36, 0xA9, 0x30, 0x53, 0x38, 0x42, 0x43, 0x31, 0x37, 0x42, 0x43, 0x00,
	0xE1, 0x40, 0x55, 0x34, 0x53, 0x36, 0xA9, 0x30, 0x53, 0x38, 0x42, 0x48, 0x31, 0x37, 0x42, 0x48, 0x00,
	0xE1, 0x50, 0x56, 0x34, 0x53, 0x36, 0xA9, 0x30, 0x53, 0x38, 0x43, 0x43, 0x31, 0x37, 0x43, 0x43, 0x00,
	0xE1, 0x60, 0xCA, 0x52, 0x18, 0x30, 0x42, 0x31, 0x37, 0x42, 0x41, 0x00,
	0xE1, 0x70, 0x52, 0x18, 0x30, 0x41, 0x31, 0x34, 0x12, 0x59, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x35, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x59, 0x12, 0x00,
	0xE1, 0x80, 0x52, 0x18, 0x30, 0x42, 0x31, 0x34, 0x12, 0x59, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x35, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x59, 0x12, 0x00,
	0xE2, 0x00, 0x9F, 0x12, 0x11, 0x11, 0x11, 0x11, 0x11, 0x36, 0x11, 0x11, 0x11, 0x11, 0x40, 0x11, 0x11, 0x11, 0x11, 0x35, 0x12, 0x00,
	0xE2, 0x10, 0x9F, 0x52, 0x18, 0x30, 0x41, 0x31, 0x00,
	0xE2, 0x15, 0xD1, 0x5B, 0x34, 0x41, 0x90, 0x6A, 0x00,
	0xE2, 0x20, 0x5C, 0x34, 0x40, 0x1D, 0xD0, 0x5B, 0x34, 0x45, 0x37, 0xA9, 0x30, 0x5B, 0x38, 0x45, 0x31, 0xD2, 0x5C, 0x34, 0x41, 0x00,
	0xE2, 0x30, 0xD0, 0x5C, 0x34, 0x41, 0xD2, 0x52, 0x18, 0x30, 0x40, 0x31, 0x34, 0x52, 0x18, 0x30, 0x41, 0x31, 0x1D, 0xD7, 0x42, 0x45, 0x40, 0x00,
	0xE2, 0x40, 0x52, 0x18, 0x30, 0x40, 0x31, 0x34, 0x52, 0x18, 0x30, 0x42, 0x31, 0x00,
	0xE2, 0x50, 0x57, 0x34, 0xA0, 0x30, 0x30, 0x5B, 0x35, 0x54, 0x31, 0x38, 0x42, 0x43, 0x37, 0x43, 0x46, 0x40, 0x31, 0x1D, 0x60, 0x18, 0x34, 0x12, 0x63, 0x12, 0x1D, 0xD8, 0x49, 0x40, 0x40, 0x00,
	0xE2, 0x60, 0x57, 0x34, 0xA0, 0x30, 0x30, 0x5B, 0x35, 0x55, 0x31, 0x38, 0x42, 0x48, 0x37, 0x43, 0x46, 0x40, 0x31, 0x1D, 0x60, 0x18, 0x34, 0x12, 0x5B, 0x12, 0x1D, 0xD8, 0x49, 0x40, 0x40, 0x00,
	0xE2, 0x70, 0x57, 0x34, 0xA0, 0x30, 0x30, 0x5B, 0x35, 0x56, 0x31, 0x38, 0x43, 0x43, 0x37, 0x43, 0x46, 0x40, 0x31, 0x1D, 0x60, 0x18, 0x34, 0x12, 0x53, 0x12, 0x1D, 0xD8, 0x49, 0x40, 0x40, 0x00,
	0xE2, 0x90, 0x9F, 0x52, 0x18, 0x30, 0x40, 0x31, 0x1C, 0xDD, 0x12, 0x15, 0x15, 0x15, 0x12, 0x1C, 0x5B, 0x00,
	0xE3, 0x00, 0xD5, 0x5B, 0x00,
	0xE3, 0x10, 0x9F, 0x52, 0x18, 0x30, 0x41, 0x31, 0x00,
	0xE3, 0x20, 0xD4, 0x00,
	0xE5, 0x00, 0x64, 0x34, 0x7E, 0x65, 0x18, 0x00,
	0xE5, 0x60, 0xD0, 0x66, 0x82, 0x43, 0xD2, 0x61, 0x34, 0x64, 0x1D, 0x62, 0x34, 0x66, 0x35, 0x41, 0x1D, 0x63, 0x34, 0x67, 0x1D, 0xD7, 0x45, 0x48, 0x40, 0x00,
	0xE5, 0x70, 0x61, 0x34, 0x64, 0x36, 0x41, 0x1D, 0x62, 0x34, 0x41, 0x43, 0x35, 0x66, 0x1D, 0x63, 0x34, 0x67, 0x00,
	0xE5, 0x80, 0x68, 0x34, 0xA9, 0x30, 0x43, 0x46, 0x45, 0x4A, 0x42, 0x45, 0x37, 0x61, 0x31, 0x35, 0xA9, 0x30, 0x43, 0x40, 0x4A, 0x46, 0x37, 0x62, 0x31, 0x35, 0x63, 0x36, 0x41, 0x42, 0x42, 0x00,
	0xE5, 0x90, 0x68, 0x34, 0x68, 0x36, 0xA9, 0x30, 0x61, 0x38, 0x41, 0x40, 0x40, 0x31, 0x35, 0xA9, 0x30, 0x61, 0x38, 0x44, 0x40, 0x40, 0x31, 0x00,
	0xE6, 0x00, 0xDE, 0x00,
	0xE7, 0x00, 0xD0, 0x64, 0x84, 0x44, 0x37, 0xA9, 0x30, 0x64, 0x38, 0x44, 0x31, 0xD2, 0x69, 0x34, 0x40, 0x1D, 0xDE, 0x00,
	0xE7, 0x10, 0xD0, 0x64, 0x84, 0x41, 0x40, 0x40, 0x37, 0xA9, 0x30, 0x64, 0x38, 0x41, 0x40, 0x40, 0x31, 0xD2, 0x69, 0x34, 0x41, 0x1D, 0xDE, 0x00,
	0xE7, 0x20, 0xD0, 0x64, 0x84, 0x44, 0x40, 0x40, 0x37, 0xA9, 0x30, 0x64, 0x38, 0x44, 0x40, 0x40, 0x31, 0xD2, 0x69, 0x34, 0x40, 0x1D, 0xDE, 0x00,
	0xE7, 0x30, 0x69, 0x34, 0x41, 0x1D, 0xDE, 0x00,
	0xE8, 0x00, 0xD0, 0x30, 0x66, 0x34, 0x44, 0x31, 0x35, 0x30, 0x66, 0x34, 0x46, 0x31, 0x35, 0x30, 0x66, 0x34, 0x49, 0x31, 0x35, 0x30, 0x66, 0x34, 0x41, 0x41, 0x31, 0x34, 0x41, 0xD2, 0x6A, 0x34, 0x43, 0x40, 0x1D, 0xDE, 0x00,
	0xE8, 0x10, 0xD0, 0x66, 0x34, 0x42, 0xD2, 0x6A, 0x34, 0x42, 0x48, 0x35, 0x69, 0x1D, 0xDE, 0x00,
	0xE8, 0x20, 0x6A, 0x34, 0x43, 0x41, 0x1D, 0xDE, 0x00,
	0xE9, 0x00, 0x5E, 0x34, 0x41, 0x41, 0x35, 0xA9, 0x30, 0x48, 0x37, 0xAA, 0x57, 0x31, 0x37, 0xAB, 0x57, 0x00,
	0xE9, 0x10, 0x52, 0x18, 0x30, 0x40, 0x31, 0x34, 0x8C, 0x30, 0x52, 0x18, 0x30, 0x40, 0x31, 0x1B, 0x5E, 0x36, 0x41, 0x31, 0x35, 0x60, 0x18, 0x35, 0x8D, 0x30, 0x52, 0x18, 0x30, 0x40, 0x31, 0x1B, 0x42, 0x41, 0x36, 0x5E, 0x31, 0x00,
	0xE9, 0x20, 0xDE, 0x00,
};

uint8_t sharp_ce125_option_receive_mode = 0;
uint8_t sharp_ce125_option_send_mode = 0;

uint8_t sharp_ce125_option_receive_buffer[4];
uint8_t sharp_ce125_option_receive_buffer_count = 0;

uint8_t sharp_ce125_option_bits_received = 0;
uint8_t sharp_ce125_option_byte_in = 0;

volatile bool sharp_ce125_option_timeout = false;

ISR(WDT_OVERFLOW_vect) {
	WDTCR |= _BV(WDIE);
	sharp_ce125_option_timeout = true;
}

void sharp_ce125_option_init(void) {
	wdt_enable(WDTO_15MS);
	wdt_reset();
	WDTCR |= _BV(WDIE);
}

void sharp_ce125_option_acl(void) {
	sharp_ce125_option_bits_received = 0;
	sharp_ce125_option_receive_buffer_count = 0;
	sharp_ce125_option_receive_mode = 0;
	sharp_ce125_option_send_mode = 0;
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

bool sharp_ce125_option_send_byte(uint8_t value) {
	
	bool sent = false;

	for (uint8_t i = 0; i < 8; ++i) {
		
		_delay_us(500);
		
		if (value & 1) {
			// Drive Din high
			SHARP_CE125_OPTION_PORT |= _BV(SHARP_CE125_OPTION_DIN);
			SHARP_CE125_OPTION_DDR |= _BV(SHARP_CE125_OPTION_DIN);
		} else {
			// Release Din low
			SHARP_CE125_OPTION_DDR &= ~_BV(SHARP_CE125_OPTION_DIN);
			SHARP_CE125_OPTION_PORT &= ~_BV(SHARP_CE125_OPTION_DIN);
		}
		value >>= 1;
		
		// Drive ACK high
		SHARP_CE125_OPTION_PORT |= _BV(SHARP_CE125_OPTION_ACK);
		SHARP_CE125_OPTION_DDR |= _BV(SHARP_CE125_OPTION_ACK);

		// Wait for BUSY to go high
		wdt_reset();
		sharp_ce125_option_timeout = false;
		while (!(SHARP_CE125_OPTION_PIN & _BV(SHARP_CE125_OPTION_BUSY))) {
			if (sharp_ce125_option_timeout) goto timeout;
		}

		// Release ACK low
		SHARP_CE125_OPTION_DDR &= ~_BV(SHARP_CE125_OPTION_ACK);
		SHARP_CE125_OPTION_PORT &= ~_BV(SHARP_CE125_OPTION_ACK);
		
		// Wait for BUSY to go low
		wdt_reset();
		sharp_ce125_option_timeout = false;
		while (SHARP_CE125_OPTION_PIN & _BV(SHARP_CE125_OPTION_BUSY)) {
			if (sharp_ce125_option_timeout) goto timeout;
		}
	}

	sent = true;

timeout:;
	// Ensure ACK is released
	SHARP_CE125_OPTION_DDR &= ~_BV(SHARP_CE125_OPTION_ACK);
	SHARP_CE125_OPTION_PORT &= ~_BV(SHARP_CE125_OPTION_ACK);

	// Release Din
	SHARP_CE125_OPTION_DDR &= ~_BV(SHARP_CE125_OPTION_DIN);
	SHARP_CE125_OPTION_PORT &= ~_BV(SHARP_CE125_OPTION_DIN);
	
	return sent;
}

void sharp_ce125_option_outstat(uint8_t value) {
	
}

uint8_t sharp_ce125_option_instat(void) {
	return 123;
}

void sharp_ce125_option_setcom(uint8_t mode, uint8_t devices) {
}


uint8_t sharp_ce125_option_checksum;
uint8_t sharp_ce125_option_checksum_count;
uint8_t sharp_ce125_option_checksum_reset = 80;
void sharp_ce125_option_reset_checksum(void) {
  sharp_ce125_option_checksum = 0;
  sharp_ce125_option_checksum_count = 0;
  sharp_ce125_option_checksum_reset = 80;
}

bool sharp_ce125_option_send_byte_with_checksum(uint8_t value) {
  
  // Send the byte
  if (!sharp_ce125_option_send_byte(value)) return false;

  // Update the checksum
  uint16_t new_checksum = sharp_ce125_option_checksum;
  new_checksum += value & 0xF;
  if (new_checksum > 0xFF) ++new_checksum;
  new_checksum += value >> 4;
  sharp_ce125_option_checksum = new_checksum;

  // Is it time to send the checksum?
  ++sharp_ce125_option_checksum_count;
  if ((sharp_ce125_option_checksum_count & 7) == 0) {
    if (!sharp_ce125_option_send_byte(sharp_ce125_option_checksum)) return false;
    // Reset the checksum every 80 bytes
    if (sharp_ce125_option_checksum_count == sharp_ce125_option_checksum_reset) {
      sharp_ce125_option_checksum = 0;
      sharp_ce125_option_checksum_count = 0;
    }
  }

  return true;
}

const char sharp_ce125_option_token_to_ascii[] PROGMEM = {
  '\0', ' ', '"', '?', '!', '#', '%', '\\', '$', ']', '[', ',', ';', ':', '@', '&',
  '(', ')', '>', '<', '=', '+', '-', '*', '/', '^', '\0', '\0', '\0', '\0', '\0', '\0',
  '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '.', '`', '\0', '~', '_', ' ',
  ' ', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
  'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'
};

const char sharp_ce125_option_ascii_to_token[] PROGMEM = {
  /*  !"#$%&'()*+,-./ */ 0x11, 0x14, 0x12, 0x15, 0x18, 0x16, 0x1E, 0x10, 0x30, 0x31, 0x37, 0x35, 0x1B, 0x36, 0x4A, 0x38,
  /* 0123456789:;<=>? */ 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x1D, 0x1C, 0x33, 0x34, 0x32, 0x13,
  /* @ABCDEFGHIJKLMNO */ 0x1E, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
  /* PQRSTUVWXYZ[\]^_ */ 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x1A, 0x17, 0x19, 0x39, 0x4E,
  /* ` */                0x4B,
};

uint8_t sharp_ce125_option_llist_column_count;

void sharp_ce125_option_print(uint8_t token) {
	if (token == 0x0D) {
		uart_write_byte('\r');
		uart_write_byte('\n');
	} else if (token > 0x10) {
		token -= 0x10;
		if (token >= 0x10) {
			token -= 0x10;
		}
		if (token < sizeof(sharp_ce125_option_token_to_ascii)) {
			uart_write_byte(pgm_read_byte(&sharp_ce125_option_token_to_ascii[token]));
		}
	}
}

uint8_t sharp_ce125_option_data_header_count = 0;
uint16_t sharp_ce125_option_data_size = 0;
uint16_t sharp_ce125_option_data_count = 0;

void sharp_ce125_option_received_byte(uint8_t value) {
		
	switch (sharp_ce125_option_receive_mode) {
		case SHARP_CE125_OPTION_CMD_SETCOM:
			sharp_ce125_option_receive_buffer[sharp_ce125_option_receive_buffer_count++] = value;
			if (sharp_ce125_option_receive_buffer_count > 2) {
				if (value == 0x0D) {
					sharp_ce125_option_setcom(sharp_ce125_option_receive_buffer[0], sharp_ce125_option_receive_buffer[1]);
				}
				sharp_ce125_option_acl();
			}
			break;
		case SHARP_CE125_OPTION_CMD_OUTSTAT:
			sharp_ce125_option_receive_buffer[sharp_ce125_option_receive_buffer_count++] = value;
			if (sharp_ce125_option_receive_buffer_count > 1) {
				if (value == 0x0D) {
					sharp_ce125_option_outstat(sharp_ce125_option_receive_buffer[0]);
				}
			}
			break;
		case SHARP_CE125_OPTION_CMD_PRINT:
		case SHARP_CE125_OPTION_CMD_LPRINT:
		case SHARP_CE125_OPTION_CMD_LLIST:
		case SHARP_CE125_OPTION_CMD_CSAVE:
		case SHARP_CE125_OPTION_CMD_PRINTFILE:
			switch (sharp_ce125_option_receive_mode) {
				case SHARP_CE125_OPTION_CMD_PRINT:
				case SHARP_CE125_OPTION_CMD_LPRINT:
					// Output to printer
					sharp_ce125_option_print(value);
					if (value == 0x0D) {
						sharp_ce125_option_acl();
					}
					break;
				case SHARP_CE125_OPTION_CMD_LLIST:
					// Output program listing to printer
					if (value == 0x0D) {
						sharp_ce125_option_acl();
					} else {
						if (value == 0x03) {
							sharp_ce125_option_print(0x0D);
							sharp_ce125_option_llist_column_count = 0;
						} else {
							if (value != 0x11 || sharp_ce125_option_llist_column_count < 24 || sharp_ce125_option_llist_column_count > 27) {
								sharp_ce125_option_print(value);
							}
							if (++sharp_ce125_option_llist_column_count == 48) {
								sharp_ce125_option_llist_column_count = 24;
							}
						}
					}
					break;
				case SHARP_CE125_OPTION_CMD_CSAVE:
					// Send raw program to the serial port
					uart_write_byte(value);
					if (value == 0x0D) {
						sharp_ce125_option_acl();
					}
					break;
				case SHARP_CE125_OPTION_CMD_PRINTFILE:
					switch (sharp_ce125_option_data_header_count) {
						case 0: // MSB of size
							uart_write_byte(value);
							// Was this the end of the transmission?
							if (value == 0x0D && sharp_ce125_option_data_size != 0) {
								sharp_ce125_option_acl();
							} else {
								sharp_ce125_option_data_size = value << 8;
								++sharp_ce125_option_data_header_count;
							}
							break;
						case 1: // LSB of size
							sharp_ce125_option_data_size |= value;
							sharp_ce125_option_data_count = 0;
							++sharp_ce125_option_data_header_count;
							uart_write_byte(value);

							// We should expect at least three more bytes now
							if (sharp_ce125_option_data_size < 3) {
								sharp_ce125_option_data_header_count = 0;
							}
							break;
						case 2: // Row upper bound
						case 3: // Column upper bound
						case 4: // Element size
							++sharp_ce125_option_data_header_count;
						default:
							// Send the data
							uart_write_byte(value);
							++sharp_ce125_option_data_count;

							// Is that the end of the variable?
							if (sharp_ce125_option_data_size == 0xFE00) {
								// 8-byte variables - PRINT#, PRINT# V
								if (((sharp_ce125_option_data_count & 7) == 4) && (value == 0x0F)) {
									// Reset for next variable
									sharp_ce125_option_data_header_count = 0;
								}
							} else {
								// Array variables - PRINT# V(*)
								if (sharp_ce125_option_data_count >= sharp_ce125_option_data_size) {
									// Reset for next variable
									sharp_ce125_option_data_header_count = 0;
								}
							}
							break;
					}
					break;
				default:
					sharp_ce125_option_acl();
					break;
			}
			break;
		default:
			switch (value) {
				case SHARP_CE125_OPTION_CMD_COMSTRING:
					sharp_ce125_option_send_byte(0x0D);
					break;
				case SHARP_CE125_OPTION_CMD_KEYONOFF:
					sharp_ce125_option_send_byte(SHARP_CE125_OPTION_CMD_KEYONOFF);
					sharp_ce125_option_send_byte(0x0D);
					break;
				case SHARP_CE125_OPTION_CMD_SETCOM:
				case SHARP_CE125_OPTION_CMD_OUTSTAT:
					sharp_ce125_option_receive_mode = value;
					break;
				case SHARP_CE125_OPTION_CMD_PRINT:
				case SHARP_CE125_OPTION_CMD_LPRINT:
				case SHARP_CE125_OPTION_CMD_LLIST:
				case SHARP_CE125_OPTION_CMD_CSAVE:
				case SHARP_CE125_OPTION_CMD_PRINTFILE:
					sharp_ce125_option_llist_column_count = 0;
					sharp_ce125_option_data_header_count = 0;
					sharp_ce125_option_data_size = 0;
					sharp_ce125_option_receive_mode = value;
					break;
				case SHARP_CE125_OPTION_CMD_INSTAT:
					if (sharp_ce125_option_send_byte(sharp_ce125_option_instat())) {
						sharp_ce125_option_send_byte(0x0D);
					}
					break;
				case SHARP_CE125_OPTION_CMD_INPUT:
				case SHARP_CE125_OPTION_CMD_INPUTFILE:
				case SHARP_CE125_OPTION_CMD_KEYGET:
				case SHARP_CE125_OPTION_CMD_CLOAD:
					sharp_ce125_option_reset_checksum();
					sharp_ce125_option_data_header_count = 0;
					sharp_ce125_option_data_size = 0;
					sharp_ce125_option_send_mode = value;
					break;
				case SHARP_CE125_OPTION_CMD_ROM:
					{
						// Reset the sharp_ce125_option_checksum
						sharp_ce125_option_reset_checksum();

						// Send the program data
						for (int i = 0; i < sizeof(program); ++i) {
							if (!sharp_ce125_option_send_byte_with_checksum(pgm_read_byte(&program[i]))) goto send_done;
						}

						// Send 8 terminating bytes
						for (int i = 0; i < 8; ++i) {
							if (!sharp_ce125_option_send_byte_with_checksum(0xFF)) goto send_done;
						}
						sharp_ce125_option_send_byte(0x0D); // End of transmission
					send_done:;
					}
					break;
				default:
					sharp_ce125_option_acl();
					break;
			}
			break;
	}
}

void sharp_ce125_option_return_byte(uint8_t value) {
	switch (sharp_ce125_option_send_mode) {
		case SHARP_CE125_OPTION_CMD_INPUT:
		case SHARP_CE125_OPTION_CMD_KEYGET:
			// Send a single key token
			if (value != 0x0D) {
				if (value == '~') {
					value = 0x4D;
				} else if (value < 0x20) {
					value = 0;
				} else {
					value -= 0x20;
					if (value < sizeof(sharp_ce125_option_ascii_to_token)) {
						value = pgm_read_byte(&sharp_ce125_option_ascii_to_token[value]);
					} else {
						value = 0;
					}
				}
			}
			if (value) {
				if (value == 0x0D) {
					value = 0x00;
				}
				if (sharp_ce125_option_send_byte(value)) {
					sharp_ce125_option_send_byte(0x0D);
				}
				sharp_ce125_option_send_mode = 0;
			}
			break;
		case SHARP_CE125_OPTION_CMD_INPUTFILE:
			// Send data with checksum
			if (sharp_ce125_option_data_header_count == 0) {
				sharp_ce125_option_reset_checksum();
				sharp_ce125_option_checksum_count = 3;
				sharp_ce125_option_checksum_reset = 8;
			}
			if (!sharp_ce125_option_send_byte_with_checksum(value)) {
				sharp_ce125_option_acl();
			} else {
				switch (sharp_ce125_option_data_header_count) {
					case 0: // MSB of size
						// Was this the end of the transmission?
						if (value == 0x0D && sharp_ce125_option_data_size != 0) {
							sharp_ce125_option_acl();
						} else {
							sharp_ce125_option_data_size = value << 8;
							++sharp_ce125_option_data_header_count;
						}
						break;
					case 1: // LSB of size
						sharp_ce125_option_data_size |= value;
						sharp_ce125_option_data_count = 0;
						++sharp_ce125_option_data_header_count;
						
						// We should expect at least three more bytes now
						if (sharp_ce125_option_data_size < 3) {
							sharp_ce125_option_data_header_count = 0;
							sharp_ce125_option_acl();
						}
						break;
					case 2: // Row upper bound
					case 3: // Column upper bound
					case 4: // Element size
						++sharp_ce125_option_data_header_count;
					default:
						// We sent the data
						++sharp_ce125_option_data_count;

						// Is that the end of the variable?
						if (sharp_ce125_option_data_size == 0xFE00) {
							// 8-byte variables - INPUT#, INPUT# V
							if (((sharp_ce125_option_data_count & 7) == 4) && (value == 0x0F)) {
								// Reset for next variable
								sharp_ce125_option_data_header_count = 0;
							}
						} else {
							// Array variables - INPUT# V(*)
							if (sharp_ce125_option_data_count >= sharp_ce125_option_data_size) {
								// Reset for next variable
								sharp_ce125_option_data_header_count = 0;
							}
						}
						break;
				}
			}
			break;
		case SHARP_CE125_OPTION_CMD_CLOAD:
			// Send checksummed data
			if (value == 0x0D) {
				// Send terminator
				for (int i = 0; i < 8; ++i) {
					if (!sharp_ce125_option_send_byte_with_checksum(0xFF)) goto send_done;
				}
				sharp_ce125_option_send_byte(0x0D); // End of transmission
			send_done:;
				sharp_ce125_option_send_mode = 0;
			} else {
				// Send single byte
				if (!sharp_ce125_option_send_byte_with_checksum(value)) {
					sharp_ce125_option_send_mode = 0;
				}
			}
			break;
	}
	
}

int main(void) {
	
	sharp_ce125_option_init();
	uart_init();
	
	sei();
	
	uart_write_byte('S');
	uart_write_byte('t');
	uart_write_byte('a');
	uart_write_byte('r');
	uart_write_byte('t');
	uart_write_byte('.');
	uart_write_byte('.');
	uart_write_byte('.');
	_delay_ms(500);
	uart_write_byte('O');
	uart_write_byte('K');
	uart_write_byte('\r');
	uart_write_byte('\n');
	
	uint8_t value;
	
	for (;;) {
		
		if (!(SHARP_CE125_OPTION_PIN & _BV(SHARP_CE125_OPTION_OP2))) {
			// Addressing some other device, not us
			sharp_ce125_option_bits_received = 0;
		} else if (SHARP_CE125_OPTION_PIN & _BV(SHARP_CE125_OPTION_ACL)) {
			// ACL: Clear
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
			wdt_reset();
			sharp_ce125_option_timeout = false;
			while (SHARP_CE125_OPTION_PIN & _BV(SHARP_CE125_OPTION_BUSY)) {
				if (sharp_ce125_option_timeout) {
					sharp_ce125_option_bits_received = -1;
					break;
				}
			}
			
			// Release ACK
			SHARP_CE125_OPTION_PORT &= ~_BV(SHARP_CE125_OPTION_ACK);
			SHARP_CE125_OPTION_DDR &= ~_BV(SHARP_CE125_OPTION_ACK);
			
			// Have we received a full byte?
			if (++sharp_ce125_option_bits_received == 8) {
				sharp_ce125_option_bits_received = 0;
				sharp_ce125_option_received_byte(sharp_ce125_option_byte_in);
			}
			
		} else if (uart_read_byte(&value)) {
			sharp_ce125_option_return_byte(value);
		}
		
	}
	
	
}