#ifndef UART_H
#define UART_H

#include <stdlib.h>
#include <stdbool.h>

#include <avr/io.h>

#define UART_RXD (5)
#define UART_TXD (4)
#define UART_PIN (PINB)
#define UART_PORT (PORTB)
#define UART_DDR (DDRB)

#define UART_BAUD_RATE (9600)

#define UART_RXD_PCINT (PCINT5)
#define UART_RXD_PCINT_vect (PCINT0_vect)
#define UART_RXD_PCINT_IF (PCIF0)
#define UART_RXD_PCINT_IE (PCIE0)
#define UART_RXD_PCINT_MSK (PCMSK)

#ifdef USI_OVF_vect
#define UART_USI_OVERFLOW_vect (USI_OVF_vect)
#endif
#ifdef USI_OVERFLOW_vect
#define UART_USI_OVERFLOW_vect (USI_OVERFLOW_vect)
#endif

#define UART_TXD_COM0 (COM1B0)
#define UART_TXD_COM1 (COM1B1)

#define UART_TXD_COMP_vect (TIMER1_COMPB_vect)
#define UART_TXD_OCIE (OCIE1B)
#define UART_TXD_OCF (OCF1B)

#if (F_CPU / UART_BAUD_RATE) > 255
	#define UART_TIMER_PRESCALER (8)
	#define UART_TIMER0_CLOCKSEL (2)
	#ifdef CS13
		#define UART_TIMER1_CLOCKSEL (4)
	#else
		#define UART_TIMER1_CLOCKSEL (2)
	#endif
#else
	#define UART_TIMER_PRESCALER (1)
	#define UART_TIMER0_CLOCKSEL (1)
	#define UART_TIMER1_CLOCKSEL (1)
#endif

#define UART_TIMER_CYCLES (F_CPU / UART_BAUD_RATE / UART_TIMER_PRESCALER)
#define UART_TIMER_SETUP_CYCLES (10)

#define UART_IN_BUFFER_SIZE (96)
#define UART_OUT_BUFFER_SIZE (96)

void uart_init(void);

void uart_write_byte(uint8_t value);
bool uart_read_byte(uint8_t *value);

#endif