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

#define SHARP_SUPPORT_CMD_COMSTRING (0)
#define SHARP_SUPPORT_CMD_KEYONOFF  (0)
#define SHARP_SUPPORT_CMD_SETCOM    (0)
#define SHARP_SUPPORT_CMD_OUTSTAT   (0)
#define SHARP_SUPPORT_CMD_PRINT     (0)
#define SHARP_SUPPORT_CMD_LPRINT    (0)
#define SHARP_SUPPORT_CMD_LLIST     (0)
#define SHARP_SUPPORT_CMD_CSAVE     (0)
#define SHARP_SUPPORT_CMD_PRINTFILE (0)
#define SHARP_SUPPORT_CMD_INSTAT    (0)
#define SHARP_SUPPORT_CMD_INPUT     (0)
#define SHARP_SUPPORT_CMD_CLOAD     (0)
#define SHARP_SUPPORT_CMD_INPUTFILE (0)
#define SHARP_SUPPORT_CMD_ROM       (0)
#define SHARP_SUPPORT_CMD_KEYGET    (0)

#ifdef SHARP_SUPPORT_CMDS_ROM
#undef SHARP_SUPPORT_CMD_ROM
#define SHARP_SUPPORT_CMD_ROM       (1)
#endif

#ifdef SHARP_SUPPORT_CMDS_KEYBOARD
#undef SHARP_SUPPORT_CMD_KEYONOFF
#undef SHARP_SUPPORT_CMD_KEYGET
#define SHARP_SUPPORT_CMD_KEYONOFF  (1)
#define SHARP_SUPPORT_CMD_KEYGET    (1)
#endif

#ifdef SHARP_SUPPORT_CMDS_SERIAL
#undef SHARP_SUPPORT_CMD_COMSTRING
#undef SHARP_SUPPORT_CMD_SETCOM
#undef SHARP_SUPPORT_CMD_OUTSTAT
#undef SHARP_SUPPORT_CMD_PRINT
#undef SHARP_SUPPORT_CMD_LPRINT
#undef SHARP_SUPPORT_CMD_LLIST
#undef SHARP_SUPPORT_CMD_CSAVE
#undef SHARP_SUPPORT_CMD_PRINTFILE
#undef SHARP_SUPPORT_CMD_INSTAT
#undef SHARP_SUPPORT_CMD_INPUT
#undef SHARP_SUPPORT_CMD_CLOAD
#undef SHARP_SUPPORT_CMD_INPUTFILE
#define SHARP_SUPPORT_CMD_COMSTRING (1)
#define SHARP_SUPPORT_CMD_SETCOM    (1)
#define SHARP_SUPPORT_CMD_OUTSTAT   (1)
#define SHARP_SUPPORT_CMD_PRINT     (1)
#define SHARP_SUPPORT_CMD_LPRINT    (1)
#define SHARP_SUPPORT_CMD_LLIST     (1)
#define SHARP_SUPPORT_CMD_CSAVE     (1)
#define SHARP_SUPPORT_CMD_PRINTFILE (1)
#define SHARP_SUPPORT_CMD_INSTAT    (1)
#define SHARP_SUPPORT_CMD_INPUT     (1)
#define SHARP_SUPPORT_CMD_CLOAD     (1)
#define SHARP_SUPPORT_CMD_INPUTFILE (1)
#endif

void sharp_init(void);
bool sharp_update(void);
void sharp_sleep(void);

bool sharp_char_to_token(char c, uint8_t *token);
bool sharp_token_to_char(uint8_t token, char *c);

#if SHARP_SUPPORT_CMD_OUTSTAT
extern void sharp_serial_outstat(uint8_t value);
#endif
#if SHARP_SUPPORT_CMD_INSTAT
extern uint8_t sharp_serial_instat(void);
#endif
#if SHARP_SUPPORT_CMD_SETCOM
extern void sharp_serial_setcom(uint8_t mode, uint8_t devices);
#endif
#if SHARP_SUPPORT_CMD_COMSTRING
extern void sharp_serial_comstring(bool (*ret)(uint8_t));
#endif

#if SHARP_SUPPORT_CMD_PRINT || SHARP_SUPPORT_CMD_LPRINT || SHARP_SUPPORT_CMD_LLIST || SHARP_SUPPORT_CMD_CSAVE || SHARP_SUPPORT_CMD_PRINTFILE || SHARP_SUPPORT_CMD_INPUT || SHARP_SUPPORT_CMD_CLOAD || SHARP_SUPPORT_CMD_INPUTFILE
extern void sharp_serial_data_begin(uint8_t mode);
extern void sharp_serial_data_out(uint8_t mode, uint8_t value);
extern bool sharp_serial_data_in(uint8_t mode, uint8_t *value);
extern void sharp_serial_data_end(uint8_t mode);
#endif

#if SHARP_SUPPORT_CMD_KEYGET
extern bool sharp_key_in(uint8_t *value);
#endif

#if SHARP_SUPPORT_CMD_ROM
extern bool sharp_rom(bool (*ret)(uint8_t));
#endif

#endif