#ifndef UART_H
#define UART_H

#include <stdlib.h>
#include <stdbool.h>

#include <avr/io.h>

#define UART_DCD (7)
#define UART_DSR (6)
#define UART_RXD (5)
#define UART_RTS (4)
#define UART_TXD (3)
#define UART_CTS (2)
#define UART_DTR (1)
#define UART_RI  (0)
#define UART_PIN  (PINB)
#define UART_PORT (PORTB)
#define UART_DDR  (DDRB)

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

#define UART_TXD_COM0 (COM1A0)
#define UART_TXD_COM1 (COM1A1)

#define UART_TXD_COMP_vect (TIMER1_COMPA_vect)
#define UART_TXD_OCIE (OCIE1A)
#define UART_TXD_OCF (OCF1A)

#define UART_TIMER_CYCLES (F_CPU / UART_BAUD_RATE / UART_TIMER_PRESCALER)
#define UART_TIMER_SETUP_CYCLES (10)

#define UART_IN_BUFFER_SIZE (32)
#define UART_OUT_BUFFER_SIZE (32)

#define UART_PARITY_NONE (0)
#define UART_PARITY_EVEN (1)
#define UART_PARITY_ODD  (2)

void uart_init(uint16_t baud, uint8_t data_bits, uint8_t parity, uint8_t stop_bits);

void uart_write_byte(uint8_t value);
bool uart_read_byte(uint8_t *value);

void uart_empty_write_buffer(void);
void uart_empty_read_buffer(void);

#endif