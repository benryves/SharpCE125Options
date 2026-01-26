#include <stdlib.h>
#include <stdbool.h>

#include <avr/io.h>
#include <avr/interrupt.h>

#include <util/delay.h>

#include "uart.h"

int main(void) {
	
	uart_init();
	
	sei();
	
	for (;;) {
		uint8_t value;
		if (uart_read_byte(&value)) {
			uart_write_byte(value);
		}
	}
	
	
}