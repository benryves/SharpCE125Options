# SHARP CE-125 Options Library

This library has been developed to ease development of plug-in options for the SHARP CE-125.

## Pin assignments

The SHARP CE-125's 9-pin option port has the following pin assignments:

|Pin| Name  |
|---|-------|
| 1 | VP    |
| 2 | OP2   |
| 3 | ACL   | 
| 4 | VGG   |
| 5 | BUSY  |
| 6 | D out |
| 7 | D in  |
| 8 | ACK   |
| 9 | GND   |

Pins are numbered 1-9 from left to right when looking at the _front_ of the CE-125.
When looking into the port from the rear, they are numbered 1-9 from right to left.
Bear in mind that the CE-125 operates on a negative voltage supply, so VP (pin 1)
(which supplies power to the plug-in option) has a _negative_ voltage with respect to GND (pin 9).

The library is designed to be used with ATtiny microcontrollers, with the following pin assignments:

### ATtiny4313, ATtiny2313

| CE-125 | ATtiny |
|--------|--------|
| 2 OP2  | PD0    |
| 3 ACL  | PD1    |
| 5 BUSY | PD2    |
| 6 D out| PD3    |
| 7 D in | PD4    |
| 8 ACK  | PD5    |

### ATtiny84, ATtiny44, ATtiny24

| CE-125 | ATtiny |
|--------|--------|
| 2 OP2  | PA0    |
| 3 ACL  | PA1    |
| 5 BUSY | PA2    |
| 6 D out| PA3    |
| 7 D in | PA4    |
| 8 ACK  | PA5    |

### ATtiny13, ATtiny85, ATtiny45, ATtiny25

| CE-125 | ATtiny |
|--------|--------|
| 2 OP2  | PB0    |
| 3 ACL  | PB5    |
| 5 BUSY | PB1    |
| 6 D out| PB2    |
| 7 D in | PB3    |
| 8 ACK  | PB4    |

Here `ACL` can also be connected to `/RESET` (PB5) via an inverting transistor due to a lack of pins.

## Basic library usage

There are three main files that make up the library:

* `<project>/sharp_config.h`: Project-specific configuration that selects which features of the library to use.
* `lib/sharp.h`: Sets up pin definitions, enables or disables functions based on selected features.
* `lib/sharp.c`: The main library code.

Individual commands can be enabled by using `#define SHARP_SUPPORT_CMD_<command>` in your `sharp_config.h`,
for example if you wanted to build a device that responded to `OUTSTAT` you could
`#define SHARP_SUPPORT_CMD_OUTSTAT` to enable that command.

The available commands (all disabled by default) are as follows:

| Command                     | Used by                                | Group definition            |
|-----------------------------|----------------------------------------|-----------------------------|
| SHARP_SUPPORT_CMD_ROM       | `ROM` command.                         | SHARP_SUPPORT_CMDS_ROM      |
| SHARP_SUPPORT_CMD_KEYONOFF  | `KEY ON`/`KEY OFF` command             | SHARP_SUPPORT_CMDS_KEYBOARD |
| SHARP_SUPPORT_CMD_KEYGET    | Keyboard input after `KEY ON` command. | SHARP_SUPPORT_CMDS_KEYBOARD |
| SHARP_SUPPORT_CMD_INSTAT    | `INSTAT` function.                     | SHARP_SUPPORT_CMDS_SERIAL   |
| SHARP_SUPPORT_CMD_OUTSTAT   | `OUTSTAT` command.                     | SHARP_SUPPORT_CMDS_SERIAL   |
| SHARP_SUPPORT_CMD_SETCOM    | `SETCOM` command.                      | SHARP_SUPPORT_CMDS_SERIAL   |
| SHARP_SUPPORT_CMD_COMSTRING | `COM$` function                        | SHARP_SUPPORT_CMDS_SERIAL   |
| SHARP_SUPPORT_CMD_PRINT     | `PRINT` when `DO` is redirected.       | SHARP_SUPPORT_CMDS_SERIAL   |
| SHARP_SUPPORT_CMD_LPRINT    | `LPRINT` when `PO` is redirected.      | SHARP_SUPPORT_CMDS_SERIAL   |
| SHARP_SUPPORT_CMD_LLIST     | `LLIST` when `PO` is redirected.       | SHARP_SUPPORT_CMDS_SERIAL   |
| SHARP_SUPPORT_CMD_CSAVE     | `CSAVE` when `CO` is redirected.       | SHARP_SUPPORT_CMDS_SERIAL   |
| SHARP_SUPPORT_CMD_PRINTFILE | `PRINT #` when `CO` is redirected.     | SHARP_SUPPORT_CMDS_SERIAL   |
| SHARP_SUPPORT_CMD_INPUT     | `INPUT` when `KI` is redirected.       | SHARP_SUPPORT_CMDS_SERIAL   |
| SHARP_SUPPORT_CMD_CLOAD     | `CLOAD` when `CI` is redirected.       | SHARP_SUPPORT_CMDS_SERIAL   |
| SHARP_SUPPORT_CMD_INPUTFILE | `INPUT #` when `CI` is redirected.     | SHARP_SUPPORT_CMDS_SERIAL   |

As the commands can be roughly grouped by device type (ROM program module, keyboard, serial interface)
there are also three group definitions that can be used; for example, if you wanted to develop a serial
interface you could use `#define SHARP_SUPPORT_CMDS_SERIAL` and this will enable all serial interface
commands.

A minimal main program for your device would then look like this:

```
#include <avr/io.h>
#include "sharp.h"

int main(void) {
    
    sharp_init();
    
    sei();
    
    for (;;) {
        if (!sharp_update()) {
            sharp_sleep();
        }
    }
    
}
```

The three functions used are:

* `sharp_init`: performs hardware and library initialisation.
* `sharp_update`: checks to see if there is anything to do.
  If there is, this function will call the command-specific function in your program, then return `true`.
  If not, it will return `false`.
* `sharp_sleep`: switches to a low power state to avoid battery drain.

## Command functions

When the library receives a command request from the connected computer,
it will call a request-specific function that you need to implement to send an appropriate response back to the computer.

These functions are documented below; for a more detailed explanation of how they should be used, please see the example
projects included in this repository.

### bool sharp_rom(bool (*ret)(uint8_t))

Loads a program from the ROM program module.
This command should send the entire program (in its raw tokenised BASIC form);
the function pointer `ret` is the function you should call to send each byte.

If `ret` returns `false` it indicates a communication error, so you should also stop sending the program and return `false`.
If you get to the end of the program with no issues, you should return `true`.

```
const PROGMEM uint8_t rom_data[] = {
    /* program data bytes */
};

bool sharp_rom(bool (*ret)(uint8_t)) {
    for (size_t i = 0; i < sizeof(rom_data); ++i) {
        if (!ret(pgm_read_byte(rom_data + i))) return false;
    }
    return true;
}
```

### bool sharp_key_in(uint8_t *value)

This function is called when the computer is waiting for a key from the keyboard after the `KEY ON` command has been executed.

If a key is not currently available, return `false` from this function.
If a key is available, store its key token in `value` and return `true`.

### uint8_t sharp_serial_instat(void)

This function is called in response to `<variable>=INSTAT`.
You should return an 8-bit value corresponding to the status of your serial port.

### void sharp_serial_outstat(uint8_t value)

This function is called in response to `OUTSTAT <value>`.
You should update the status of your serial port based on the supplied 8-bit value.

### void sharp_serial_setcom(uint8_t mode, uint8_t devices)

This function is called in response to `OUTSTAT`,
where `mode` contains the communication parameters and `devices` contains the list of redirected variables.

The `mode` byte is in the format `%00SDPPBB`, where:

* `S` = Stop bits: `0` = 1 stop bit, `1` = 2 stop bits.
* `D` = Data bits: `0` = 8 data bits, `1` = 7 data bits.
* `PP` = Parity:
    * `00` = No parity,
    * `01` = Even parity,
    * `10` = Odd parity.
* `BB` = Baud rate:
    * `00` = 300,
    * `01` = 110,
    * `10` = 1200.

The `devices` byte is made up of any combination of the following bits:

|Symbol|Value|Item           |
|:-----|:----|:--------------|
| KI   | &01 |Keyboard Input |
| DO   | &02 |Display Output |
| PO   | &04 |Printer Output |
| CI   | &08 |Cassette Input |
| CO   | &10 |Cassette Output|

### void sharp_serial_comstring(bool (*ret)(uint8_t))

This function is called in response to `<variable$>=COM$`.

This should return a tokenised string corresponding to the currently-enabled communication parameters;
the function pointer `ret` is the function you should call to send each byte.

If `ret` returns `false` it indicates a communication error, so you should also stop sending the string and return.

### void sharp_serial_data_begin(uint8_t mode)

This function is called at the start of any serial data transfer.
`mode` corresponds to which particular serial operation is about to occur:

* Input:
    * `SHARP_CMD_INPUT`: `INPUT` when `KI` is redirected.
    * `SHARP_CMD_INPUTFILE`: `INPUT #` when `CI` is redirected.
    * `SHARP_CMD_CLOAD`: `CLOAD` when `CI` is redirected.
* Output:
    * `SHARP_CMD_PRINT`: `PRINT` when `DO` is redirected.
    * `SHARP_CMD_PRINTFILE`: `PRINT #` when `CO` is redirected.
    * `SHARP_CMD_CSAVE`: `CSAVE` when `CO` is redirected.
    * `SHARP_CMD_LPRINT`: `LPRINT` when `PO` is redirected.
    * `SHARP_CMD_LLIST`: `LLIST` when `PO` is redirected.

In addition, if any of the above input commands are supported _and_ `SHARP_CMD_KEYGET` is also enabled,
`sharp_serial_data_begin(SHARP_CMD_KEYGET)` will also be called before keyboard input is handled.

### bool sharp_serial_data_in(uint8_t mode, uint8_t *value)

This function is used to retrieve data from the serial port and transfer it back to the computer.
`mode` is the input mode (the same value as described in `sharp_serial_data_begin`).

If data is not currently available, return `false` from this function.
If data is available, store the incoming byte in `value` and return `true`.

Note that data is a token in the computer's native encoding.
In the case of binary data transfers (e.g. `SHARP_CMD_CLOAD`, `SHARP_CMD_INPUTFILE`) you will probably want
to transfer the data verbatim; for text transfers (e.g. `SHARP_CMD_INPUT`) you will probably
want to convert from ASCII (the `sharp_char_to_token` utility function is provided for this purpose).

### void sharp_serial_data_out(uint8_t mode, uint8_t value)

This function is used to transfer data from the computer out of the serial port.
`mode` is the output mode (the same value as described in `sharp_serial_data_begin`).

The byte to transfer is in `value`; note that this is a tokenised byte in the computer's native encoding.
In the case of binary data transfers (e.g. `SHARP_CMD_CSAVE`, `SHARP_CMD_PRINTFILE`) you will probably want
to transfer the data verbatim; for text transfers (e.g. `SHARP_CMD_LPRINT`, `SHARP_CMD_LLIST`) you will probably
want to convert to ASCII (the `sharp_token_to_char` utility function is provided for this purpose).

### void sharp_serial_data_end(uint8_t mode)

This function is called when the computer resets the device following a serial data transfer,
with `mode` being the same value described in `sharp_serial_data_begin`.
