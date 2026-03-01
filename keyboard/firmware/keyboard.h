#ifndef KEYBOARD_H
#define KEYBOARD_H

#define KEYBOARD_CLOCK (1)
#define KEYBOARD_CLOCK_PORT (PORTB)
#define KEYBOARD_CLOCK_PIN (PINB)
#define KEYBOARD_CLOCK_DDR (DDRB)

#define KEYBOARD_DATA (0)
#define KEYBOARD_DATA_PORT (PORTB)
#define KEYBOARD_DATA_PIN (PINB)
#define KEYBOARD_DATA_DDR (DDRB)

#define KEYBOARD_LINE_LOW(line, port, ddr) ddr |= (uint8_t)_BV(line)
#define KEYBOARD_LINE_HIGH(line, port, ddr) ddr &= (uint8_t)~_BV(line)

#define KEYBOARD_CLOCK_LOW() KEYBOARD_LINE_LOW(KEYBOARD_CLOCK, KEYBOARD_CLOCK_PORT, KEYBOARD_CLOCK_DDR)
#define KEYBOARD_CLOCK_HIGH() KEYBOARD_LINE_HIGH(KEYBOARD_CLOCK, KEYBOARD_CLOCK_PORT, KEYBOARD_CLOCK_DDR)

#define KEYBOARD_DATA_LOW() KEYBOARD_LINE_LOW(KEYBOARD_DATA, KEYBOARD_DATA_PORT, KEYBOARD_DATA_DDR)
#define KEYBOARD_DATA_HIGH() KEYBOARD_LINE_HIGH(KEYBOARD_DATA, KEYBOARD_DATA_PORT, KEYBOARD_DATA_DDR)

#define KEYBOARD_BUFFER_SIZE 16

#if defined(__AVR_ATtiny4313__) || defined(__AVR_ATtiny2313__)

#define KEYBOARD_PCINT_PINS (_BV(PCINT1))
#define KEYBOARD_PCINT_vect (PCINT0_vect)
#define KEYBOARD_PCINT_IE (PCIE0)
#define KEYBOARD_PCINT_MSK (PCMSK)

#elif defined (__AVR_ATtiny84__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny24__)

#define KEYBOARD_PCINT_PINS (_BV(PCINT9))
#define KEYBOARD_PCINT_vect (PCINT1_vect)
#define KEYBOARD_PCINT_IE (PCIE1)
#define KEYBOARD_PCINT_MSK (PCMSK1)

#endif


typedef enum {
	KEYBOARD_TRANSMISSION_DEVICE_TO_HOST, // The device is sending data to the host (normal mode).
	KEYBOARD_TRANSMISSION_HOST_TO_DEVICE, // The host is sending data to the device.
} keyboard_transmission_direction_t;

extern volatile keyboard_transmission_direction_t keyboard_transmission_direction;

typedef enum  {
	KEYBOARD_RECEIVE_SCANCODE, // The host is receiving scancodes (normal mode).
	KEYBOARD_RECEIVE_PAUSE, // The host is receiving the pause byte sequence.
	KEYBOARD_RECEIVE_ACKNOWLEDGEMENT, // The host is waiting for acknowledgement.
	KEYBOARD_RECEIVE_ACKNOWLEDGEMENT_LED, // The host is waiting for acknowledgement before sending LED status.
	KEYBOARD_RECEIVE_ACKNOWLEDGEMENT_TYPEMATIC, // The host is waiting for acknowledgement before sending typematic parameters.
} keyboard_receive_mode_t;

extern volatile keyboard_receive_mode_t keyboard_receive_mode;

typedef enum {
	KEYBOARD_LED_SCROLL_LOCK = 0x01,
	KEYBOARD_LED_NUM_LOCK = 0x02,
	KEYBOARD_LED_CAPS_LOCK = 0x04,
} keyboard_led_status_t;

extern volatile uint8_t keyboard_key_state[16];

void keyboard_clear_receive_buffer(void);

void keyboard_reset(void);

void keyboard_byte_received(uint8_t value);

void keyboard_bit_received(uint8_t bit);

void keyboard_send_byte(uint8_t value);

void keyboard_send_bit(void);

void keyboard_set_led_status(keyboard_led_status_t status);

extern volatile uint8_t keyboard_buffer_count;

char keyboard_buffer_dequeue(void);

void keyboard_buffer_flush(void);

extern volatile uint8_t keyboard_reset_flag;

extern volatile uint8_t keyboard_key_received_flag;

void keyboard_set_typematic_delay(uint8_t delay_cs);
uint8_t keyboard_get_typematic_delay(void);

void keyboard_set_typematic_period(uint8_t period_cs);
uint8_t keyboard_get_typematic_period(void);

#endif