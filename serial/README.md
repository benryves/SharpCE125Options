# RS-232 serial interface

## Specifications

* **Baud rate:** 110, 300 or 1200
* **Data bits:** 7 or 8.
* **Parity:** None, even or odd.
* **Stop bits:** 1 or 2.
* **Connector:** DE-9 wired as DTE.

## Command reference table

* **Statements:**
  * SETCOM, OUTSTAT
  * INPUT, INPUT#
  * PRINT, PRINT#
  * LPRINT, LLIST
  * CLOAD, CSAVE
  * KEY
* **Functions:**
  * COM$, INSTAT

|Command|Example     |Function                                                            |
|:------|:-----------|:-------------------------------------------------------------------|
|SETCOM |SETCOM 300,N,8,1,PO|Sets communication parameters and device redirection.        |
|       |SETCOM OFF  |Resets communication parameters and device redirection.             |
|OUTSTAT|OUTSTAT 0   |Sets status of RS-232 output signals.                               |
|INPUT  |INPUT "A=";A|Loads the variable with ASCII data input from the RS-232 port.      |
|INPUT# |INPUT# A    |Loads the variable with binary data input from the RS-232 port.     |
|PRINT  |PRINT 123   |Sends data through the RS-232 port in ASCII format.                 |
|PRINT# |PRINT# A    |Sends data through the RS-232 port in binary format.                |
|LPRINT |LPRINT 123  |Sends data through the RS-232 port in ASCII format.                 |
|LLIST  |LLIST 10,100|Lists the program through the RS-232 port in ASCII format.          |
|CLOAD  |CLOAD       |Loads a program from the RS-232 port in binary format.              |
|CSAVE  |CSAVE       |Saves a program through the RS-232 port in binary format.           |
|KEY    |KEY ON      |Accepts keyboard input from ASCII data sent through the RS-232 port.|
|       |KEY OFF     |Disables keyboard input through the RS-232 port.                    |
|COM$   |COM$        |Current communication parameters as a character string.             |
|INSTAT |INSTAT      |Retrieves current status of RS-232 input signals.                   |

## Pin assignment

|Pin No.|Item               |Symbol|In/Out|Remarks                          |
|:-----:|:------------------|:----:|:----:|:--------------------------------|
|   1   |Carrier Detect     |  CD  |  In  |ON when a cable isn't connected. |
|   2   |Receive Data       |  RD  |  In  |                                 |
|   3   |Transmit Data      |  TD  |  Out |                                 |
|   4   |Data Terminal Ready|  DTR |  Out |OFF on initial power up.         |
|   5   |Signal Ground      |  SG  |      |                                 |
|   6   |Data Set Ready     |  DSR |  In  |ON when a cable isn't connected. |
|   7   |Request-To-Send    |  RTS |  Out |OFF on initial power up.         |
|   8   |Clear-To-Send      |  CTS |  In  |OFF when a cable isn't connected.|
|   9   |Ring Indicator     |  RI  |  In  |ON when a cable isn't connected. |

The RS-232 interface is wired as DTE (data terminal equipment) which means it can be connected to devices wired as DCE (data communication equipment) such as a modem or a printer with a straight-through cable.
To connect it to another DTE (such as directly to another different computer) you will need to use a null modem cable between the two devices.

## Handshaking

The interface will not transmit data unless CD, DSR and CTS are all ON.
CD and DSR are pulled high to the ON state when a cable is not connected, so will not inhibit communication if they are missing however CTS is pulled low to the OFF state and so must be driven high.

When the interface is first powered up both DTR and RTS are in the OFF state, and this will likely prevent the connected device from sending data to the interface.
DTR and RTS can be switched to the ON state by using the OUTSTAT 0 command.

The interface has a small data receive buffer and will automatically switch RTS to the OFF state when this starts to fill up.
It will then automatically switch RTS back to the ON state once the received data has been transferred from the interface to the computer.

## Character encoding

Data transferred between the computer and the interface always uses the internal character encoding specific to the SHARP Pocket Computer.
These characters are converted to and from ASCII when sent or received over the RS-232 port when used with certain commands:

* **ASCII encoding:**
  * INPUT, PRINT
  * LPRINT, LLIST
  * KEY
* **SHARP Pocket Computer internal encoding:**
  * INPUT#, PRINT#
  * CLOAD, CSAVE

CR (13, &0D) is used as a line/data terminator in both situations.

The SHARP Pocket Computer does support converting its internal character values to and from ASCII using the ASC and CHR$ functions, however there are some differences that are noted here:

|Code (Decimal)|Code (Hex)|ASCII|SHARP|
|:------------:|:--------:|:---:|:---:|
|      39      |    &27   |  '  | INS |
|      91      |    &5B   |  \[ |  √  |
|      92      |    &5C   |  \\ |  ¥  |
|      93      |    &5D   |  \] |  π  |
|      96      |    &60   |  `  |  𝐄  |

## Principle of operation

The serial interface is designed around redirecting certain operations that would normally exchange data with another device (such as the keyboard, screen, printer or cassette) to instead exchange data via the RS-232 port.

The SETCOM command is used to set this up. It first takes a list of communication parameters (such as baud rate and number of data bits) and then a list of devices to redirect.

For example,

> SETCOM 300,N,8,1,CI,CO

sets the communication parameters to 300 baud, no parity, 8 data bits, 1 stop bit and enables redirection of Cassette Input and Cassette Output.
From this point, any operations that would have normally transferred data to or from the tape cassette will instead transfer the data via the serial port using the specified communication parameters.

The five devices that can be redirected are as follows:

|Symbol|Value|Item           |In/Out|Affected commands|
|:----:|:---:|:--------------|:----:|:----------------|
|  KI  | &01 |Keyboard Input |  In  |INPUT            |
|  DO  | &02 |Display Output |  Out |PRINT            |
|  PO  | &04 |Printer Output |  Out |LPRINT, LLIST    |
|  CI  | &08 |Cassette Input |  In  |CLOAD, INPUT#    |
|  CO  | &10 |Cassette Output|  Out |CSAVE, PRINT#    |

Individual communication parameters can be omitted (the comma will remain) in which case the default values will be used (300 baud, No parity, 8 data bits, 1 stop bit).

At least one device must be specified. To disable the serial interface, use SETCOM OFF.

## BASIC programming reference

### SETCOM (statement)

Used to set communication parameters such as baud rate and number of data bits as well as which I/O devices should be redirected to or from the serial interface.

> SETCOM 300,N,8,1,CI,CO

* Baud rate: 110, 300, 1200.
* Parity: N (None), E (Even), O (Odd).
* Data bits: 7, 8.
* Stop bits: 1, 2.
* Device: KI (Keyboard Input), DO (Display Output), PO (Printer Output), CI (Cassette Input), CO (Cassette Output).

Default communication parameters are 300 baud, no parity, 8 data bits and 1 stop bit.

You must always specify at least one device to redirect.

> SETCOM OFF

Disables device redirection.

### COM$

Retrieves a character string that contains the last set of communication parameters and redirected devices.

At power on:

> 300,N,8,1

After executing SETCOM 1200,E,7,1,PO:

> 1200,E,7,1,PO

### OUTSTAT

Used to set the state of the handshaking signal lines (RTS, DTR).

> OUTSTAT 0

The handshaking signal lines are assigned as follows:

|Value              |Item               |Symbol|In/Out|
|:-----------------:|:------------------|:----:|:----:|
|2<sup>5</sup> = &20|Ring Indicator     |  RI  |  In  |
|2<sup>4</sup> = &10|Data Set Ready     |  DSR |  In  |
|2<sup>3</sup> = &08|Carrier Detect     |  CD  |  In  |
|2<sup>2</sup> = &04|Clear-To-Send      |  CTS |  In  |
|2<sup>1</sup> = &02|Request-To-Send    |  RTS |  Out |
|2<sup>0</sup> = &01|Data Terminal Ready|  DTR |  Out |

* 0 bit: ON, high level, V+ at connector.
* 1 bit: OFF, low level, V- at connector.

RTS and DTR are 1 at power on, putting them in the OFF state.

### INSTAT

Used to get the state of the handshaking signal lines. The values returned by this have the same pin assignments as documented above for OUTSTAT.
