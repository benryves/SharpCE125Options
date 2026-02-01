#include <stdlib.h>
#include <stdbool.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/pgmspace.h>

#include <util/delay.h>

#include "sharp.h"

uint8_t sharp_receive_mode = 0;
uint8_t sharp_send_mode = 0;

uint8_t sharp_receive_buffer[4];
uint8_t sharp_receive_buffer_count = 0;

uint8_t sharp_bits_received = 0;
uint8_t sharp_byte_in = 0;

volatile bool sharp_timeout = false;

ISR(WDT_OVERFLOW_vect) {
	WDTCR |= _BV(WDIE);
	sharp_timeout = true;
}

void sharp_init(void) {
	wdt_enable(WDTO_15MS);
	wdt_reset();
	WDTCR |= _BV(WDIE);
}

void sharp_acl(void) {
	sharp_bits_received = 0;
	sharp_receive_buffer_count = 0;
	if (sharp_receive_mode) {
		sharp_serial_data_end(sharp_receive_mode);
		sharp_receive_mode = 0;
	}
	if (sharp_send_mode) {
		sharp_serial_data_end(sharp_send_mode);
		sharp_send_mode = 0;
	}
}

bool sharp_send_byte(uint8_t value) {
	
	bool sent = false;

	for (uint8_t i = 0; i < 8; ++i) {
		
		_delay_us(500);
		
		if (value & 1) {
			// Drive Din high
			SHARP_PORT |= _BV(SHARP_DIN);
			SHARP_DDR |= _BV(SHARP_DIN);
		} else {
			// Release Din low
			SHARP_DDR &= ~_BV(SHARP_DIN);
			SHARP_PORT &= ~_BV(SHARP_DIN);
		}
		value >>= 1;
		
		// Drive ACK high
		SHARP_PORT |= _BV(SHARP_ACK);
		SHARP_DDR |= _BV(SHARP_ACK);

		// Wait for BUSY to go high
		wdt_reset();
		sharp_timeout = false;
		while (!(SHARP_PIN & _BV(SHARP_BUSY))) {
			if (sharp_timeout) goto timeout;
		}

		// Release ACK low
		SHARP_DDR &= ~_BV(SHARP_ACK);
		SHARP_PORT &= ~_BV(SHARP_ACK);
		
		// Wait for BUSY to go low
		wdt_reset();
		sharp_timeout = false;
		while (SHARP_PIN & _BV(SHARP_BUSY)) {
			if (sharp_timeout) goto timeout;
		}
	}

	sent = true;

timeout:;
	// Ensure ACK is released
	SHARP_DDR &= ~_BV(SHARP_ACK);
	SHARP_PORT &= ~_BV(SHARP_ACK);

	// Release Din
	SHARP_DDR &= ~_BV(SHARP_DIN);
	SHARP_PORT &= ~_BV(SHARP_DIN);
	
	return sent;
}

uint8_t sharp_checksum;
uint8_t sharp_checksum_count;
uint8_t sharp_checksum_reset = 80;
void sharp_reset_checksum(void) {
  sharp_checksum = 0;
  sharp_checksum_count = 0;
  sharp_checksum_reset = 80;
}

bool sharp_send_byte_with_checksum(uint8_t value) {
  
  // Send the byte
  if (!sharp_send_byte(value)) return false;

  // Update the checksum
  uint16_t new_checksum = sharp_checksum;
  new_checksum += value & 0xF;
  if (new_checksum > 0xFF) ++new_checksum;
  new_checksum += value >> 4;
  sharp_checksum = new_checksum;

  // Is it time to send the checksum?
  ++sharp_checksum_count;
  if ((sharp_checksum_count & 7) == 0) {
    if (!sharp_send_byte(sharp_checksum)) return false;
    // Reset the checksum every 80 bytes
    if (sharp_checksum_count == sharp_checksum_reset) {
      sharp_checksum = 0;
      sharp_checksum_count = 0;
    }
  }

  return true;
}

uint8_t sharp_data_header_count = 0;
uint16_t sharp_data_size = 0;
uint16_t sharp_data_count = 0;

void sharp_received_byte(uint8_t value) {
		
	switch (sharp_receive_mode) {
		case SHARP_CMD_SETCOM:
			sharp_receive_buffer[sharp_receive_buffer_count++] = value;
			if (sharp_receive_buffer_count > 2) {
				if (value == 0x0D) {
					sharp_serial_setcom(sharp_receive_buffer[0], sharp_receive_buffer[1]);
				}
				sharp_acl();
			}
			break;
		case SHARP_CMD_OUTSTAT:
			sharp_receive_buffer[sharp_receive_buffer_count++] = value;
			if (sharp_receive_buffer_count > 1) {
				if (value == 0x0D) {
					sharp_serial_outstat(sharp_receive_buffer[0]);
				}
			}
			break;
		case SHARP_CMD_PRINT:
		case SHARP_CMD_LPRINT:
		case SHARP_CMD_LLIST:
		case SHARP_CMD_CSAVE:
		case SHARP_CMD_PRINTFILE:
			switch (sharp_receive_mode) {
				case SHARP_CMD_PRINT:
				case SHARP_CMD_LPRINT:
				case SHARP_CMD_LLIST:
					// Output to printer
					sharp_serial_data_out(sharp_receive_mode, value);
					if (value == 0x0D) {
						sharp_acl();
					}
					break;
				case SHARP_CMD_CSAVE:
					// Send raw program to the serial port
					sharp_serial_data_out(sharp_receive_mode, value);
					if (value == 0x0D) {
						sharp_acl();
					}
					break;
				case SHARP_CMD_PRINTFILE:
					switch (sharp_data_header_count) {
						case 0: // MSB of size
							sharp_serial_data_out(sharp_receive_mode, value);
							// Was this the end of the transmission?
							if (value == 0x0D && sharp_data_size != 0) {
								sharp_acl();
							} else {
								sharp_data_size = value << 8;
								++sharp_data_header_count;
							}
							break;
						case 1: // LSB of size
							sharp_data_size |= value;
							sharp_data_count = 0;
							++sharp_data_header_count;
							sharp_serial_data_out(sharp_receive_mode, value);

							// We should expect at least three more bytes now
							if (sharp_data_size < 3) {
								sharp_data_header_count = 0;
							}
							break;
						case 2: // Row upper bound
						case 3: // Column upper bound
						case 4: // Element size
							++sharp_data_header_count;
						default:
							// Send the data
							sharp_serial_data_out(sharp_receive_mode, value);
							++sharp_data_count;

							// Is that the end of the variable?
							if (sharp_data_size == 0xFE00) {
								// 8-byte variables - PRINT#, PRINT# V
								if (((sharp_data_count & 7) == 4) && (value == 0x0F)) {
									// Reset for next variable
									sharp_data_header_count = 0;
								}
							} else {
								// Array variables - PRINT# V(*)
								if (sharp_data_count >= sharp_data_size) {
									// Reset for next variable
									sharp_data_header_count = 0;
								}
							}
							break;
					}
					break;
				default:
					sharp_acl();
					break;
			}
			break;
		default:
			switch (value) {
				case SHARP_CMD_COMSTRING:
					sharp_serial_comstring(&sharp_send_byte);
					sharp_send_byte(0x0D);
					break;
				case SHARP_CMD_KEYONOFF:
					sharp_send_byte(SHARP_CMD_KEYONOFF);
					sharp_send_byte(0x0D);
					break;
				case SHARP_CMD_SETCOM:
				case SHARP_CMD_OUTSTAT:
					sharp_receive_mode = value;
					break;
				case SHARP_CMD_PRINT:
				case SHARP_CMD_LPRINT:
				case SHARP_CMD_LLIST:
				case SHARP_CMD_CSAVE:
				case SHARP_CMD_PRINTFILE:
					sharp_data_header_count = 0;
					sharp_data_size = 0;
					sharp_receive_mode = value;
					sharp_serial_data_begin(sharp_receive_mode);
					break;
				case SHARP_CMD_INSTAT:
					if (sharp_send_byte(sharp_serial_instat())) {
						sharp_send_byte(0x0D);
					}
					break;
				case SHARP_CMD_INPUT:
				case SHARP_CMD_INPUTFILE:
				case SHARP_CMD_CLOAD:
					sharp_reset_checksum();
					sharp_data_header_count = 0;
					sharp_data_size = 0;
					sharp_send_mode = value;
					sharp_serial_data_begin(sharp_receive_mode);
					break;
				case SHARP_CMD_KEYGET:
					sharp_send_mode = value;
					break;
				case SHARP_CMD_ROM:
					{
						// Reset the sharp_checksum
						sharp_reset_checksum();
						
						// Send the ROM program data
						if (sharp_rom(sharp_send_byte_with_checksum)) {
							// Send 8 terminating bytes
							for (uint8_t i = 0; i < 8; ++i) {
								if (!sharp_send_byte_with_checksum(0xFF)) goto send_done;
							}
							sharp_send_byte(0x0D); // End of transmission
						}
					send_done:;
					}
					break;
				default:
					sharp_acl();
					break;
			}
			break;
	}
}

void sharp_update(void) {
	
	if (!(SHARP_PIN & _BV(SHARP_OP2))) {
		// Addressing some other device, not us
		sharp_bits_received = 0;
	} else if (SHARP_PIN & _BV(SHARP_ACL)) {
		// ACL: Clear
		sharp_acl();
	} else if (SHARP_PIN & _BV(SHARP_BUSY)) {
		// Receiving a bit of data
		
		// Clock in the bit
		sharp_byte_in >>= 1;
		if (SHARP_PIN & _BV(SHARP_DOUT)) {
			sharp_byte_in |= _BV(7);
		}
		
		// Drive ACK high
		SHARP_DDR |= _BV(SHARP_ACK);
		SHARP_PORT |= _BV(SHARP_ACK);
			
		// Wait for BUSY to go back low
		wdt_reset();
		sharp_timeout = false;
		while (SHARP_PIN & _BV(SHARP_BUSY)) {
			if (sharp_timeout) {
				sharp_bits_received = -1;
				break;
			}
		}
		
		// Release ACK
		SHARP_PORT &= ~_BV(SHARP_ACK);
		SHARP_DDR &= ~_BV(SHARP_ACK);
		
		// Have we received a full byte?
		if (++sharp_bits_received == 8) {
			sharp_bits_received = 0;
			sharp_received_byte(sharp_byte_in);
		}
		
	}
	
	uint8_t value;
	
	switch (sharp_send_mode) {
		case SHARP_CMD_INPUT:
			if (sharp_serial_data_in(sharp_send_mode, &value)) {
				if (sharp_send_byte(value)) {
					sharp_send_byte(0x0D);
				}
				sharp_send_mode = 0;
			}
			break;
		case SHARP_CMD_KEYGET:
			if (sharp_key_in(&value)) {
				if (sharp_send_byte(value)) {
					sharp_send_byte(0x0D);
				}
				sharp_send_mode = 0;
			}
			break;
		case SHARP_CMD_INPUTFILE:
			// Send data with checksum
			if (sharp_serial_data_in(sharp_send_mode, &value)) {
				if (sharp_data_header_count == 0) {
					sharp_reset_checksum();
					sharp_checksum_count = 3;
					sharp_checksum_reset = 8;
				}
				if (!sharp_send_byte_with_checksum(value)) {
					sharp_acl();
				} else {
					switch (sharp_data_header_count) {
						case 0: // MSB of size
							// Was this the end of the transmission?
							if (value == 0x0D && sharp_data_size != 0) {
								sharp_acl();
							} else {
								sharp_data_size = value << 8;
								++sharp_data_header_count;
							}
							break;
						case 1: // LSB of size
							sharp_data_size |= value;
							sharp_data_count = 0;
							++sharp_data_header_count;
							
							// We should expect at least three more bytes now
							if (sharp_data_size < 3) {
								sharp_data_header_count = 0;
								sharp_acl();
							}
							break;
						case 2: // Row upper bound
						case 3: // Column upper bound
						case 4: // Element size
							++sharp_data_header_count;
						default:
							// We sent the data
							++sharp_data_count;

							// Is that the end of the variable?
							if (sharp_data_size == 0xFE00) {
								// 8-byte variables - INPUT#, INPUT# V
								if (((sharp_data_count & 7) == 4) && (value == 0x0F)) {
									// Reset for next variable
									sharp_data_header_count = 0;
								}
							} else {
								// Array variables - INPUT# V(*)
								if (sharp_data_count >= sharp_data_size) {
									// Reset for next variable
									sharp_data_header_count = 0;
								}
							}
							break;
					}
				}
			}
			break;
		case SHARP_CMD_CLOAD:
			// Send checksummed data
			if (sharp_serial_data_in(sharp_send_mode, &value)) {
				if (value == 0x0D) {
					// Send terminator
					for (uint8_t i = 0; i < 8; ++i) {
						if (!sharp_send_byte_with_checksum(0xFF)) goto send_done;
					}
					sharp_send_byte(0x0D); // End of transmission
				send_done:;
					sharp_send_mode = 0;
				} else {
					// Send single byte
					if (!sharp_send_byte_with_checksum(value)) {
						sharp_send_mode = 0;
					}
				}
			}
			break;
	}
	
}

const char token_to_ascii_table[] PROGMEM = {
  '\0', ' ', '"', '?', '!', '#', '%', '\\', '$', ']', '[', ',', ';', ':', '@', '&',
  '(', ')', '>', '<', '=', '+', '-', '*', '/', '^', '\0', '\0', '\0', '\0', '\0', '\0',
  '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '.', '`', '\0', '~', '_', ' ',
  ' ', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
  'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'
};

const char ascii_to_token_table[] PROGMEM = {
  /*  !"#$%&'()*+,-./ */ 0x11, 0x14, 0x12, 0x15, 0x18, 0x16, 0x1E, 0x10, 0x30, 0x31, 0x37, 0x35, 0x1B, 0x36, 0x4A, 0x38,
  /* 0123456789:;<=>? */ 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x1D, 0x1C, 0x33, 0x34, 0x32, 0x13,
  /* @ABCDEFGHIJKLMNO */ 0x1E, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
  /* PQRSTUVWXYZ[\]^_ */ 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x1A, 0x17, 0x19, 0x39, 0x4E,
  /* ` */                0x4B,
};

bool sharp_token_to_char(uint8_t token, char *c) {
	if (token == 0x00 || token == 0x03 || token == 0x0D) {
		*c = '\r';
		return true;
	} else if (token > 0x10) {
		token -= 0x10;
		if (token >= 0x10) {
			token -= 0x10;
		}
		if (token < sizeof(token_to_ascii_table)) {
			*c = pgm_read_byte(&token_to_ascii_table[token]);
			return true;
		}
	}
	return false;
}

bool sharp_char_to_token(char c, uint8_t *token) {
	if (c == '\r') {
		*token = 0x00;
		return true;
	} else if (c == '~') {
		*token = 0x4D;
		return true;
	} else if (c < 0x20) {
		return false;
	} else {
		c -= 0x20;
		if (c >= 0 && c < sizeof(ascii_to_token_table)) {
			*token = pgm_read_byte(&ascii_to_token_table[(uint8_t)c]);
			return true;
		}
	}
	return false;
}	