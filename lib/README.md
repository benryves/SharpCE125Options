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
| SHARP_SUPPORT_CMD_KEYONOFF  | `KEY ON`/`KEY OFF command              | SHARP_SUPPORT_CMDS_KEYBOARD |
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
