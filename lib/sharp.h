#ifndef SHARP_H
#define SHARP_H

#include <stdint.h>
#include <stdbool.h>

#include "sharp_config.h"

#if defined(__AVR_ATtiny4313__) || defined(__AVR_ATtiny2313__)

#define SHARP_OP2  (0)
#define SHARP_ACL  (1)
#define SHARP_BUSY (2)
#define SHARP_DOUT (3)
#define SHARP_DIN  (4)
#define SHARP_ACK  (5)

#define SHARP_PIN  (PIND)
#define SHARP_PORT (PORTD)
#define SHARP_DDR  (DDRD)

#define SHARP_PCINT_PINS (_BV(PCINT11) | _BV(PCINT12) | _BV(PCINT13) | _BV(PCINT14))
#define SHARP_PCINT_vect (PCINT2_vect)
#define SHARP_PCINT_IF (PCIF2)
#define SHARP_PCINT_IE (PCIE2)
#define SHARP_PCINT_MSK (PCMSK2)

#define SHARP_WDT_vect (WDT_OVERFLOW_vect)
#define SHARP_WDT_CR (WDTCR)
#define SHARP_WDT_IE (WDIE)

#elif defined (__AVR_ATtiny84__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny24__)

#define SHARP_OP2  (0)
#define SHARP_ACL  (1)
#define SHARP_BUSY (2)
#define SHARP_DOUT (3)
#define SHARP_DIN  (4)
#define SHARP_ACK  (5)

#define SHARP_PIN  (PINA)
#define SHARP_PORT (PORTA)
#define SHARP_DDR  (DDRA)

#define SHARP_PCINT_PINS (_BV(PCINT0) | _BV(PCINT1) | _BV(PCINT2) | _BV(PCINT3))
#define SHARP_PCINT_vect (PCINT0_vect)
#define SHARP_PCINT_IF (PCIF0)
#define SHARP_PCINT_IE (PCIE0)
#define SHARP_PCINT_MSK (PCMSK0)

#define SHARP_WDT_vect (WDT_vect)
#define SHARP_WDT_CR (WDTCSR)
#define SHARP_WDT_IE (WDIE)

#elif defined(__AVR_ATtiny13__) || defined(__AVR_ATtiny85__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny25__)

#define SHARP_OP2  (0)
#define SHARP_ACL  (5)
#define SHARP_BUSY (1)
#define SHARP_DOUT (2)
#define SHARP_DIN  (3)
#define SHARP_ACK  (4)

#define SHARP_PIN  (PINB)
#define SHARP_PORT (PORTB)
#define SHARP_DDR  (DDRB)

#define SHARP_PCINT_PINS (_BV(PCINT0) | _BV(PCINT1) | _BV(PCINT2))
#define SHARP_PCINT_vect (PCINT0_vect)
#define SHARP_PCINT_IF (PCIF)
#define SHARP_PCINT_IE (PCIE)
#define SHARP_PCINT_MSK (PCMSK)

#define SHARP_WDT_vect (WDT_vect)
#define SHARP_WDT_CR (WDTCR)
#define SHARP_WDT_IE (WDIE)

#endif

#ifdef __AVR_ATtiny13__
#undef SHARP_WDT_IE
#define SHARP_WDT_IE (WDTIE)
#endif

#define SHARP_CMD_COMSTRING (0x2B)
#define SHARP_CMD_KEYONOFF  (0x2E)
#define SHARP_CMD_SETCOM    (0x70)
#define SHARP_CMD_OUTSTAT   (0x71)
#define SHARP_CMD_PRINT     (0x72)
#define SHARP_CMD_LPRINT    (0x73)
#define SHARP_CMD_LLIST     (0x74)
#define SHARP_CMD_CSAVE     (0x75)
#define SHARP_CMD_PRINTFILE (0x76)
#define SHARP_CMD_INSTAT    (0x77)
#define SHARP_CMD_INPUT     (0x78)
#define SHARP_CMD_CLOAD     (0x7A)
#define SHARP_CMD_INPUTFILE (0x7B)
#define SHARP_CMD_ROM       (0x7C)
#define SHARP_CMD_KEYGET    (0x7E)

// #define SHARP_SUPPORT_CMD_COMSTRING
// #define SHARP_SUPPORT_CMD_KEYONOFF
// #define SHARP_SUPPORT_CMD_SETCOM
// #define SHARP_SUPPORT_CMD_OUTSTAT
// #define SHARP_SUPPORT_CMD_PRINT
// #define SHARP_SUPPORT_CMD_LPRINT
// #define SHARP_SUPPORT_CMD_LLIST
// #define SHARP_SUPPORT_CMD_CSAVE
// #define SHARP_SUPPORT_CMD_PRINTFILE
// #define SHARP_SUPPORT_CMD_INSTAT
// #define SHARP_SUPPORT_CMD_INPUT
// #define SHARP_SUPPORT_CMD_CLOAD
// #define SHARP_SUPPORT_CMD_INPUTFILE
// #define SHARP_SUPPORT_CMD_ROM
// #define SHARP_SUPPORT_CMD_KEYGET

#ifdef SHARP_SUPPORT_CMDS_ROM
#define SHARP_SUPPORT_CMD_ROM
#endif

#ifdef SHARP_SUPPORT_CMDS_KEYBOARD
#define SHARP_SUPPORT_CMD_KEYONOFF
#define SHARP_SUPPORT_CMD_KEYGET
#endif

#ifdef SHARP_SUPPORT_CMDS_SERIAL
#define SHARP_SUPPORT_CMD_COMSTRING
#define SHARP_SUPPORT_CMD_SETCOM
#define SHARP_SUPPORT_CMD_OUTSTAT
#define SHARP_SUPPORT_CMD_PRINT
#define SHARP_SUPPORT_CMD_LPRINT
#define SHARP_SUPPORT_CMD_LLIST
#define SHARP_SUPPORT_CMD_CSAVE
#define SHARP_SUPPORT_CMD_PRINTFILE
#define SHARP_SUPPORT_CMD_INSTAT
#define SHARP_SUPPORT_CMD_INPUT
#define SHARP_SUPPORT_CMD_CLOAD
#define SHARP_SUPPORT_CMD_INPUTFILE
#endif

void sharp_init(void);
bool sharp_update(void);
void sharp_sleep(void);

#ifdef SHARP_UTIL_CHAR_TO_TOKEN
bool sharp_char_to_token(char c, uint8_t *token);
#endif
#ifdef SHARP_UTIL_TOKEN_TO_CHAR
bool sharp_token_to_char(uint8_t token, char *c);
#endif

#ifdef SHARP_SUPPORT_CMD_OUTSTAT
extern void sharp_serial_outstat(uint8_t value);
#endif
#ifdef SHARP_SUPPORT_CMD_INSTAT
extern uint8_t sharp_serial_instat(void);
#endif
#ifdef SHARP_SUPPORT_CMD_SETCOM
extern void sharp_serial_setcom(uint8_t mode, uint8_t devices);
#endif
#ifdef SHARP_SUPPORT_CMD_COMSTRING
extern void sharp_serial_comstring(bool (*ret)(uint8_t));
#endif

#if defined(SHARP_SUPPORT_CMD_PRINT) || defined(SHARP_SUPPORT_CMD_LPRINT) || defined(SHARP_SUPPORT_CMD_LLIST) || defined(SHARP_SUPPORT_CMD_CSAVE) || defined(SHARP_SUPPORT_CMD_PRINTFILE) || defined(SHARP_SUPPORT_CMD_INPUT) || defined(SHARP_SUPPORT_CMD_CLOAD) || defined(SHARP_SUPPORT_CMD_INPUTFILE)
extern void sharp_serial_data_begin(uint8_t mode);
extern void sharp_serial_data_out(uint8_t mode, uint8_t value);
extern bool sharp_serial_data_in(uint8_t mode, uint8_t *value);
extern void sharp_serial_data_end(uint8_t mode);
#endif

#ifdef SHARP_SUPPORT_CMD_KEYGET
extern bool sharp_key_in(uint8_t *value);
#endif

#ifdef SHARP_SUPPORT_CMD_ROM
extern bool sharp_rom(bool (*ret)(uint8_t));
#endif

#endif