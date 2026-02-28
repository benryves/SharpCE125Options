#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <avr/pgmspace.h>
#include <avr/io.h>
#include <util/delay.h>

#include "keyboard.h"
#include "keyboard_layout.h"

char keyboard_buffer[KEYBOARD_BUFFER_SIZE];

volatile uint8_t keyboard_buffer_count = 0;

volatile uint8_t keyboard_buffer_read_ptr = 0;
volatile uint8_t keyboard_buffer_write_ptr = 0;

volatile uint8_t keyboard_reset_flag = 0;
volatile uint8_t keyboard_key_received_flag = 0;

char keyboard_buffer_dequeue(void) {
	if (keyboard_buffer_count == 0) return 0;
	char value = keyboard_buffer[keyboard_buffer_read_ptr++];
	keyboard_buffer_read_ptr %= KEYBOARD_BUFFER_SIZE;
	if (--keyboard_buffer_count == 0) {
		keyboard_key_received_flag = 0;
	}
	return value;
}

void keyboard_buffer_flush(void) {
	keyboard_key_received_flag = 0;
	while (keyboard_buffer_count > 0) {
		keyboard_buffer_dequeue();
	}
}

void keyboard_buffer_enqueue(char value) {
	if (keyboard_buffer_count < KEYBOARD_BUFFER_SIZE) {
		keyboard_buffer[keyboard_buffer_write_ptr++] = value;
		keyboard_buffer_write_ptr %= KEYBOARD_BUFFER_SIZE;
		++keyboard_buffer_count;
		keyboard_key_received_flag = 1;
	}
}

volatile keyboard_transmission_direction_t keyboard_transmission_direction;
volatile keyboard_receive_mode_t keyboard_receive_mode;
volatile uint8_t keyboard_pause_sequence_count;

uint16_t incoming_data;
uint8_t incoming_bit_count;
uint8_t incoming_is_break;
uint8_t incoming_is_extended;

uint8_t outgoing_data;
uint8_t outgoing_bit_count;
uint8_t outgoing_parity;

volatile keyboard_led_status_t keyboard_led_status;
volatile uint8_t keyboard_modifier;
volatile uint8_t keyboard_modifier_held;

volatile uint8_t keyboard_key_state[16];

volatile uint8_t keyboard_typematic_timing = 0b0101011;
uint8_t keyboard_typematic_delay = 50;
uint8_t keyboard_typematic_period = 9;
bool keyboard_typematic_enabled = true;

void keyboard_clear_receive_buffer(void) {
	incoming_data = 0;
	incoming_bit_count = 0;
}

void keyboard_reset(void) {
	keyboard_transmission_direction = KEYBOARD_TRANSMISSION_DEVICE_TO_HOST;
	keyboard_receive_mode = KEYBOARD_RECEIVE_SCANCODE;
	keyboard_modifier = 0;
	KEYBOARD_CLOCK_PORT &= (uint8_t)~_BV(KEYBOARD_CLOCK);
	KEYBOARD_DATA_PORT &= (uint8_t)~_BV(KEYBOARD_DATA);
	KEYBOARD_CLOCK_HIGH();
	KEYBOARD_DATA_HIGH();
	keyboard_clear_receive_buffer();
	keyboard_enable_clock_interrupt();
	keyboard_reset_flag = 0;
	memset((uint8_t*)keyboard_key_state, 0, 16);
}

void keyboard_byte_received(uint8_t value) {
	switch (keyboard_receive_mode) {
		case KEYBOARD_RECEIVE_SCANCODE:
			if (value == 0xE0) {
				incoming_is_extended = !0;
			} else if (value == 0xF0) {
				incoming_is_break = !0;
			} else {
				// Hold the clock line low to inhibit communications.
				KEYBOARD_CLOCK_LOW();
				// Index into the scancode table slightly differently when handling extended scancodes.
				uint8_t scancode_offset = incoming_is_extended ? KEYBOARD_SCANCODE_COUNT : 0;
				uint8_t scancode_count = incoming_is_extended ? KEYBOARD_EXTENDED_SCANCODE_COUNT : KEYBOARD_SCANCODE_COUNT;
				// Is it the pause key?
				if (!incoming_is_extended && value == 0xE1) {
					keyboard_pause_sequence_count = 7; // We have seven further bytes to receive.
					keyboard_receive_mode = KEYBOARD_RECEIVE_PAUSE;
				}
				// Search for a matching scancode.
				while (scancode_count) {
					if (pgm_read_byte(keyboard_scancodes + scancode_offset) == value) {
						break;
					} else {
						++scancode_offset;
						--scancode_count;
					}
				}
				if (scancode_count) {
					// We've identified a scancode!
					uint16_t scancode_info = pgm_read_word(keyboard_data_offsets + scancode_offset);
					uint16_t data_offset = scancode_info & 0x0FFF;
					// Read the INKEY value.
					uint8_t inkey_primary = pgm_read_byte(keyboard_data + data_offset);
					uint8_t inkey_secondary = 0;
					++data_offset;
					if (inkey_primary & 0x80) {
						inkey_primary &= 0x7F;
						inkey_secondary = pgm_read_byte(keyboard_data + data_offset);
						++data_offset;
					}
					
					// Set the keyboard state.
					bool pre_pressed = false;
					if (inkey_primary) {
						pre_pressed |= (keyboard_key_state[inkey_primary / 8] & _BV(inkey_primary & 7));
						if (incoming_is_break) {
							keyboard_key_state[inkey_primary / 8] &= (uint8_t)~_BV(inkey_primary & 7);
						} else {
							keyboard_key_state[inkey_primary / 8] |= _BV(inkey_primary & 7);
						}
					}
					if (inkey_secondary) {
						pre_pressed |= (keyboard_key_state[inkey_secondary / 8] & _BV(inkey_secondary & 7));
						if (incoming_is_break) {
							keyboard_key_state[inkey_secondary / 8] &= (uint8_t)~_BV(inkey_secondary & 7);
						} else {
							keyboard_key_state[inkey_secondary / 8] |= _BV(inkey_secondary & 7);
						}
					}
					
					if (scancode_info & 0x8000) {
						// It's a modifier.
						uint8_t modifier_value = pgm_read_byte(keyboard_data + data_offset + 0);
						if (scancode_info & 0x4000) {
							// It's a toggling modifier.
							if (!incoming_is_break) {
								if ((modifier_value & keyboard_modifier_held) == 0) {
									keyboard_modifier ^= modifier_value;
									// data + 1 = LEDs that are toggled.
									keyboard_set_led_status(keyboard_led_status ^ pgm_read_byte(keyboard_data + data_offset + 1));
									keyboard_modifier_held |= modifier_value;
								}
							} else {
								keyboard_modifier_held &= (uint8_t)~modifier_value;
							}
						} else {
							// It's a temporary modifier.
							if (!incoming_is_break) {
								keyboard_modifier |= modifier_value;
							} else {
								keyboard_modifier &= (uint8_t)~modifier_value;
							}
						}
					} else {
						// It's not a modifier.
						if (!incoming_is_break) {
							// Store the received key.
							uint8_t key = 0xFF;
							// data + 0 = modifier bits that can affect this key.
							uint8_t legal_modifiers = pgm_read_byte(keyboard_data + data_offset + 0);
							if (legal_modifiers) {
								// Search for the mask table index.
								uint8_t mask_table_index;
								for (mask_table_index = 0; mask_table_index < KEYBOARD_MASK_COMBINATION_COUNT; ++mask_table_index) {
									if (pgm_read_byte(keyboard_masks + mask_table_index) == legal_modifiers) {
										break;
									}
								}
								if (mask_table_index < KEYBOARD_MASK_COMBINATION_COUNT) {
									// Mask off the current modifiers against the legal ones.
									uint8_t active_modifiers = legal_modifiers & keyboard_modifier;
									// Which index is the selected mask?
									uint8_t mask_index_offset = pgm_read_byte(keyboard_masks_offset + mask_table_index);
									// Search for the index from the mask table.
									uint8_t mask_index;
									for (mask_index = 1; pgm_read_byte(keyboard_masks_table + mask_index_offset) != active_modifiers; ++mask_index, ++mask_index_offset);
									// Read the key data for the mask.
									data_offset += mask_index;
									key = pgm_read_byte(keyboard_data + data_offset);
								}
							} else {
								// No modifiers.
								++data_offset;
								key = pgm_read_byte(keyboard_data + data_offset);
							}
							// If it's a usable key (0xFF) enqueue it.
							if (key != 0xFF && (!pre_pressed || keyboard_typematic_enabled)) {
								// Is it a printable key?
								if (pgm_read_byte(keyboard_unprintable_data + data_offset / 8) & (0x80 >> (data_offset & 7))) {
									// Non-printable key
								} else {
									keyboard_buffer_enqueue(key);
									if (key == 0x7F && ((keyboard_modifier & 0b1100) == 0b1100)) {
										keyboard_reset_flag = !0;
									}
								}
							}
						}
					}
				}
				incoming_is_extended = 0;
				incoming_is_break = 0;
				// Release the clock line.
				KEYBOARD_CLOCK_HIGH();
			}
			break;
		case KEYBOARD_RECEIVE_PAUSE:
			if (--keyboard_pause_sequence_count == 0) {
				keyboard_receive_mode = KEYBOARD_RECEIVE_SCANCODE;
			}
			break;
		case KEYBOARD_RECEIVE_ACKNOWLEDGEMENT:
			keyboard_receive_mode = KEYBOARD_RECEIVE_SCANCODE;
			break;
		case KEYBOARD_RECEIVE_ACKNOWLEDGEMENT_LED:
			keyboard_receive_mode = KEYBOARD_RECEIVE_ACKNOWLEDGEMENT;
			keyboard_send_byte(keyboard_led_status);
			break;
		case KEYBOARD_RECEIVE_ACKNOWLEDGEMENT_TYPEMATIC:
			keyboard_receive_mode = KEYBOARD_RECEIVE_ACKNOWLEDGEMENT;
			keyboard_send_byte(keyboard_typematic_timing);
			break;
	}
	
}

void keyboard_bit_received(uint8_t bit) {
	++incoming_bit_count;
	incoming_data >>= 1;
	if (bit) incoming_data |= _BV(11);
	if (incoming_bit_count == 11) {
		uint8_t data = (incoming_data >> 2);
		keyboard_clear_receive_buffer();
		keyboard_byte_received(data);
	}
}

void keyboard_send_byte(uint8_t value) {

	// If the host is currently writing to the device, wait for it to finish first.
	while (keyboard_transmission_direction == KEYBOARD_TRANSMISSION_HOST_TO_DEVICE);

	// Initialise the outgoing variables for transmission.
	outgoing_data = value;
	outgoing_bit_count = 0;
	outgoing_parity = 1;

	// We don't want to receive interrupt events when we're performing the request-to-send operation.
	keyboard_disable_clock_interrupt();

	// Hold clock low for at least 100us.
	KEYBOARD_CLOCK_LOW();
	_delay_us(100);
	
	// Hold data low.
	KEYBOARD_DATA_LOW();
	
	// Release clock.
	KEYBOARD_CLOCK_HIGH();
	
	// We're sending data.
	keyboard_transmission_direction = KEYBOARD_TRANSMISSION_HOST_TO_DEVICE;
	
	// Re-enable interrupts now that we're sending data.
	keyboard_enable_clock_interrupt();
}

void keyboard_send_bit(void) {
	if (!(KEYBOARD_CLOCK_PIN & _BV(KEYBOARD_CLOCK))) {
		// Clock is low; set data line.
		if (outgoing_bit_count < 8) {
			if (outgoing_data & _BV(outgoing_bit_count)) {
				KEYBOARD_DATA_HIGH();
				++outgoing_parity;
			} else {
				KEYBOARD_DATA_LOW();
			}
		} else if (outgoing_bit_count == 8) {
			// Output parity.
			if (outgoing_parity & 1) {
				KEYBOARD_DATA_HIGH();
			} else {
				KEYBOARD_DATA_LOW();
			}
		} else if (outgoing_bit_count == 9) {
			// We're nearly done.
			KEYBOARD_DATA_HIGH();
		} else if (outgoing_bit_count == 10) {
			// Done!
			keyboard_transmission_direction = KEYBOARD_TRANSMISSION_DEVICE_TO_HOST;
		}
		// Advance to next bit.
		++outgoing_bit_count;
	}
}

void keyboard_set_led_status(keyboard_led_status_t status) {
	while (keyboard_receive_mode != KEYBOARD_RECEIVE_SCANCODE);
	keyboard_led_status = status;
	keyboard_receive_mode = KEYBOARD_RECEIVE_ACKNOWLEDGEMENT_LED;
	keyboard_send_byte(0xED);
}

void keyboard_set_typematic_timing(uint8_t timing) {
	while (keyboard_receive_mode != KEYBOARD_RECEIVE_SCANCODE);
	keyboard_typematic_timing = timing;
	keyboard_receive_mode = KEYBOARD_RECEIVE_ACKNOWLEDGEMENT_TYPEMATIC;
	keyboard_send_byte(0xF3);
}

void keyboard_set_typematic_delay(uint8_t delay_cs) {
	if (delay_cs == 0) {
		keyboard_typematic_enabled = false;
	} else {
		int8_t delay_qs = (delay_cs - 12) / 25;
		if (delay_qs < 0) delay_qs = 0;
		if (delay_qs > 3) delay_qs = 3;
		keyboard_set_typematic_timing((delay_qs << 5) | (keyboard_typematic_timing & 0b11111));
		keyboard_typematic_delay = delay_cs;
		keyboard_typematic_enabled = true;
	}
}

uint8_t keyboard_get_typematic_delay(void) {
	return keyboard_typematic_delay;
}

const uint8_t keyboard_typematic_rates[32] PROGMEM = {
	 2,  3,  4,  5,  5,  5,  6,  6,
	 7,  8,  8,  9, 10, 11, 12, 13,
	14, 15, 17, 18, 20, 22, 23, 25,
	27, 30, 33, 37, 40, 43, 48, 50,
};

void keyboard_set_typematic_period(uint8_t period_cs) {
	
	if (period_cs == 0) {
		keyboard_set_typematic_timing(keyboard_typematic_timing = 0b0101011);
		keyboard_typematic_delay = 50;
		keyboard_typematic_period = 9;
		keyboard_typematic_enabled = true;
	} else {
		uint8_t rate_cps = 0;
		for (; rate_cps < 31; ++rate_cps) {
			if (pgm_read_byte(keyboard_typematic_rates + rate_cps) > period_cs) {
				break;
			}
		}
		keyboard_set_typematic_timing((keyboard_typematic_timing & 0b1100000) | rate_cps);
		keyboard_typematic_period = period_cs;
	}
}

uint8_t keyboard_get_typematic_period(void) {
	return keyboard_typematic_period;
}