#ifndef SHARP_H
#define SHARP_H

#include <stdint.h>
#include <stdbool.h>

#define SHARP_OP2  (0)
#define SHARP_ACL  (1)
#define SHARP_BUSY (2)
#define SHARP_DOUT (3)
#define SHARP_DIN  (4)
#define SHARP_ACK  (5)

#define SHARP_PIN  (PIND)
#define SHARP_PORT (PORTD)
#define SHARP_DDR  (DDRD)

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

extern uint8_t sharp_send_mode;

void sharp_init(void);
void sharp_update(void);

bool sharp_char_to_token(char c, uint8_t *token);
bool sharp_token_to_char(uint8_t token, char *c);

extern void sharp_serial_outstat(uint8_t value);
extern uint8_t sharp_serial_instat(void);
extern void sharp_serial_setcom(uint8_t mode, uint8_t devices);
extern void sharp_serial_comstring(bool (*ret)(uint8_t));

extern void sharp_serial_data_out(uint8_t mode, uint8_t value);
extern bool sharp_serial_data_in(uint8_t mode, uint8_t *value);

extern bool sharp_key_in(uint8_t *value);

extern bool sharp_rom(bool (*ret)(uint8_t));

#endif