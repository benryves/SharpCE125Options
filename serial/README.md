# RS-232 serial interface

> [!WARNING]
> The SHARP CE-125's power supply has a reversed polarity, i.e. the power supply pin VP has a _negative_ voltage relative to the ground pin GND.
> To keep the serial interface's circuit simple, it's connected "backwards" to the power supply, with the result that the serial port's GND is not at the same potential as the CE-125's other ground connections.  
> Therefore, care must be taken to ensure that the serial port ground and CE-125 ground are not shorted together, for example by plugging the serial port and cassette interface input socket to the same device at the same time, or by splitting a power supply and powering both the CE-125 and a serial device from the same power supply instead of using separate, isolated power supplies.  
> A low-current fuse **must** be included in the serial interface to protect the CE-125 against damage from such short-circuits, as the rechargeable battery pack can deliver a significant amount of current.

## Specifications

* **Baud rate:** 110, 300 or 1200 as standard via SETCOM; 75, 600, 2400, 4800 and 9600 via custom code.
* **Data bits:** 7 or 8.
* **Parity:** None, even or odd.
* **Stop bits:** 1 or 2.
* **Connector:** DE-9 wired as DTE.

## Command reference table

* **Statements:**
  * SETCOM, OUTSTAT
  * INPUT, INPUT #
  * PRINT, PRINT #
  * LPRINT, LLIST
  * CLOAD, CSAVE
  * KEY
* **Functions:**
  * COM$, INSTAT

|Command|Example     |Function                                                            |
|:------|:-----------|:-------------------------------------------------------------------|
|SETCOM |SETCOM 300,N,8,1,PO|Sets communication parameters and device redirection.        |
|       |SETCOM OFF  |Resets device redirection.                                          |
|OUTSTAT|OUTSTAT 0   |Sets status of RS-232 output signals.                               |
|INPUT  |INPUT "A=";A|Loads the variable with ASCII data input from the RS-232 port.      |
|INPUT #|INPUT #A    |Loads the variable with binary data input from the RS-232 port.     |
|PRINT  |PRINT 123   |Sends data through the RS-232 port in ASCII format.                 |
|PRINT #|PRINT #A    |Sends data through the RS-232 port in binary format.                |
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
  * INPUT #, PRINT #
  * CLOAD, CSAVE

`CR` (13, &0D) is used as a line/data terminator in both situations.

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

|Symbol|Item           |In/Out|Affected commands|
|:----:|:--------------|:----:|:----------------|
|  KI  |Keyboard Input |  In  |INPUT            |
|  DO  |Display Output |  Out |PRINT            |
|  PO  |Printer Output |  Out |LPRINT, LLIST    |
|  CI  |Cassette Input |  In  |CLOAD, INPUT #   |
|  CO  |Cassette Output|  Out |CSAVE, PRINT #   |

Individual communication parameters can be omitted (the comma will remain) in which case the default values will be used (300 baud, No parity, 8 data bits, 1 stop bit).

At least one device must be specified. To disable the serial interface, use SETCOM OFF.

## BASIC programming reference

### SETCOM

Used to set communication parameters such as baud rate and number of data bits as well as which I/O devices should be redirected to or from the serial interface.

> SETCOM 300,N,8,1,CI,CO

* **Baud rate:** 110, 300, 1200.
* **Parity:** N (None), E (Even), O (Odd).
* **Data bits:** 7, 8.
* **Stop bits:** 1, 2.
* **Device:** KI (Keyboard Input), DO (Display Output), PO (Printer Output), CI (Cassette Input), CO (Cassette Output).

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

### INPUT

When the KI (Keyboard Input) device has been redirected, this command reads ASCII text from the serial port into the specified variables. Data is read from the serial port until `CR` is received, and a comma is treated as a data separator.

> 10 OUTSTAT 0  
> 20 SETCOM 300,N,8,1,KI  
> 30 INPUT "IN EXCURSION OF KI",A,B$,C  
> 40 PRINT A;B$;C  
> 50 END

|Key operation|Display           |Serial input data              |
|:------------|:-----------------|:------------------------------|
|RUN `ENTER`  |IN EXCURSION OF KI|1+2, RS-232C `CR`<br>SIN30 `CR`|
|             |3.RS-232C0.5      |                               |

### LPRINT

When the PO (Printer Output) device has been redirected, this command sends ASCII text to the serial port.

> LPRINT "THE ANSWER IS ";A

Numeric values will be formatted according to the specified USING format.

### LLIST

When the PO (Printer Output) device has been redirected, this command lists the current program as ASCII text to the serial port.

> LLIST

LLIST on its own lists the entire program.

> LLIST start,end

LLIST with a start and end parameter lists the program between the lines "start" and "end".

### PRINT

When the DO (Display Output) device has been redirected, this command sends ASCII text to the serial port.

> PRINT "RS-232C"

Numeric values will not be formatted according to the specified USING format.

### CSAVE

When the CO (Cassette Output) device has been redirected, this command sends the current program in binary format to the serial port.

> CSAVE

A `CR` terminator follows the program data. Filenames and machine code programs are not supported.

### CLOAD

When the CI (Cassette Input) device has been redirected, this command loads a program in binary format from the serial port.

A `CR` terminator must follow the program data to signify the end of the program. Filenames and machine code programs are not supported.

### PRINT \#

When the CO (Cassette Output) device has been redirected, this command sends variable data in binary format to the serial port.

> PRINT #

When used alone all of the preallocated variables (A-Z) and the array A() variables with subscript numbers greater than 26 are transmitted.

> PRINT #X

When a preallocated variable or array A() variable is specified then that variable and all subsequent variables (including array A() variables with subscript numbers greater than 26) are transmitted.

> PRINT #B(\*)

When a user-dimensioned array variable is specified with an asterisk enclosed in the parentheses then the contents of that specific array variable are transmitted.
You cannot transmit a single element of an array.

> PRINT #X,B(\*),F$(\*)

More than one starting variable can be included in the variable list.
A `CR` terminator is sent after all variables listed in the PRINT # command have been transmitted.

As `CR` may appear within the data itself it's not sufficient to record data from the serial port until a `CR` terminator is seen.
Instead, the first five bytes sent over the serial port can be used to calculate the total length of the data:

#### Pre-allocated variables or array A()

The sequence &FE, &00, &00, &00, &00 is sent over the serial port, followed by a number of 8-byte variables.
After the last 8-byte variable has been sent, a terminator byte of &0F is sent.

#### User-dimensioned array variables

* Two bytes are sent indicating the length of the following data in bytes, most significant byte first.
* One byte is sent containing the upper bound of the array's second dimension (0 for single-dimension arrays).
* One byte is sent containing the upper bound of the array's first dimension.
* One byte is sent containing the size of each array element in bytes (8 for numeric variables, 16 for character strings unless modified with \* when dimensioning the array).

The array's element data then follows.
As the size of the array is explicitly recorded in the above five-byte header, there is no need for a terminator byte as with the pre-allocated variables.

### INPUT \#

When the CI (Cassette Input) device has been redirected, this command receives variable data in binary format from the serial port.

> INPUT #

When used alone the preallocated variables are loaded started from A.

> INPUT #X

When a preallocated variable or array A() variable is specified then variables are loaded starting from the specified variable.

> INPUT #B(*)

When a user-dimensioned array variable is specified with an asterisk enclosed in the parentheses then the contents of that specific array variable is loaded.
You cannot load a single element of an array.

> INPUT #X,B(\*),F$(\*)

More than one starting variable can be included in the variable list.
A `CR` terminator needs to be sent after all variables listed in the INPUT # command to end the transfer.

### KEY

The SHARP Pocket Computer can also accept keyboard input from the RS-232 port.
Unlike the INPUT command this will work more generally; for example, you could use this to load a BASIC program onto the computer that was stored as an ASCII text file.

> KEY ON

After executing the KEY ON command the computer will constantly check for incoming key presses from the RS-232 port as well as its own keyboard.

> KEY OFF

The KEY OFF command tells the computer to stop check for incoming key presses from the RS-232 port.

## Configuration variables

The communication parameters and device redirection state are stored in two variables in the computer's memory.

The bit array for redirected devices can be found at address &C6B7:

|Symbol|Value|Item           |In/Out|Affected commands|
|:----:|:---:|:--------------|:----:|:----------------|
|  KI  | &01 |Keyboard Input |  In  |INPUT            |
|  DO  | &02 |Display Output |  Out |PRINT            |
|  PO  | &04 |Printer Output |  Out |LPRINT, LLIST    |
|  CI  | &08 |Cassette Input |  In  |CLOAD, INPUT #   |
|  CO  | &10 |Cassette Output|  Out |CSAVE, PRINT #   |

This value is checked when any of the affected commands is used to determine whether to redirect the data via the serial port instead of the default device.
This means you can enable or disable device redirection without using SETCOM, just POKE.
For example, to redirect cassette devices CI and CO:

> POKE &C6B7,&08 OR &10

The communication parameters can be found at address &C6B8:

|Parameter       |Value|Bits|
|:---------------|:---:|----:
|Baud rate (300) | &00 | 1~0|
|Baud rate (110) | &01 | 1~0|
|Baud rate (1200)| &02 | 1~0|
|Parity (None)   | &00 | 3~2|
|Parity (Even)   | &04 | 3~2|
|Parity (Odd)    | &08 | 3~2|
|Data bits (8)   | &00 |   4|
|Data bits (7)   | &10 |   4|
|Stop bits (1)   | &00 |   5|
|Stop bits (2)   | &20 |   5|

The communication parameters need to be sent to the serial interface for them to take effect and so POKEing a new value into the computer's memory is not enough to change them.
However, a side-effect of SETCOM OFF is that it clears the device redirection bit array at &C6B7 and then sends this new zero value to the serial interface along with the current communication parameters at &C6B8.
This means you can POKE a custom value to &C6B8, send it to the serial interface with SETCOM OFF, and then enable redirection for the desired devices by POKEing the appropriate value to &C6B7.

This technique has been used to add five additional baud rates to the three currently supported by SETCOM, giving eight in total:

|Parameter       |Value|Bits |
|:---------------|:---:|----:|
|Baud rate (300) | &00 |6,1~0|
|Baud rate (110) | &01 |6,1~0|
|Baud rate (1200)| &02 |6,1~0|
|Baud rate (600) | &03 |6,1~0|
|Baud rate (2400)| &40 |6,1~0|
|Baud rate (75)  | &41 |6,1~0|
|Baud rate (9600)| &42 |6,1~0|
|Baud rate (4800)| &43 |6,1~0|

A sample program that enables one of the additional baud rates is shown below:

> 10 SETCOM,,,2,CI,CO  
> 20 B=&42: GOSUB "BAUD"  
> 30 PRINT COM$  
> 40 END  
> 100 "BAUD": POKE &C6B8,B OR (&BC AND PEEK &C6B8)  
> 110 D= PEEK &C6B7: SETCOM OFF  
> 120 POKE &C6B7,D: RETURN

Line 10 sets the communication parameters using SETCOM.
Line 20 sets B to &42 (the value for 9600 baud from the table above) and calls the BAUD subroutine to change the baud rate.
Line 30 then shows the updated communication parameters on the display, confirming the switch to 9600 baud.

The BAUD subroutine starts on line 100.
It first updates the baud rate bits in &C6B8, then backs up the device redirection bit array from &C6B7.
SETCOM OFF is then used to send the updated communication parameters to the serial interface.
Finally, the old device redirection bit array is restored to &C6B7 to re-enable any redirected devices that were disabled by SETCOM OFF.

## Use with the SHARP PC-1245

The SETCOM command is disabled on the SHARP PC-1245, which means it is not possible to use it to configure the communication parameters or redirect devices to the serial port.

You can still enable device redirection by POKEing the appropriate configuration value into the bit array at address &C6B7.
For example, to redirect cassette devices CI and CO:

> POKE &C6B7,&08 OR &10

It should be possible to call into the ROM to replicate the behaviour of the missing SETCOM, however.
The PC-1251 instruction manual contains an example on how to send characters directly to the printer (page 221, "Printout of special characters").
This does point out that different ROM revisions have different addresses for the used routines, and it's likely that the same applies to the PC-1245.

As such, I cannot present a one-size-fits-all solution, but this routine works on my particular PC-1245:

|Assembly           |Machine code (Hex)|Machine code (Decimal)|
|:------------------|:-----------------|:---------------------|
|LIA 70	            |&02, &70          |2, 112                |
|CALL PCU_WRITE_BYTE|&78, &7D, &6F     |120, 125, 111         |
|LIDP C6B8          |&10, &C6, &B8     |16, 198, 184          |
|LDD	               |&57               |87                    |
|CALL PRINT_QUEUE   |&78, &7B, &91     |120, 123, 145         |
|CAL SETDEV_PTR	    |&F1, &66          |241, 102              |
|LDD	               |&57	              |87                    |
|JP 6C1E           	|&79, &6C, &1E     |121, 108, 30          |

When included in a BASIC program, the routine might look like this:

> 10 DIM Z$(1)*6  
> 20 POKE &C5BE,2,112,120,125,111,16,198,184,87,120,123,145,241,102,87,121,108,30  
> 30 POKE &C6B7,&04,&42  
> 40 CALL &C5BE  
> 50 PRINT COM$

Line 30 sets the redirected device to PO (Printer Output) and the communication parameters to 9600 8N1 so the program should display "9600,N,8,1,PO" on the computer's LCD.

## Quirks and limitations

The LPRINT and PRINT commands still assume the printer is 24 characters wide.
Printing a single numeric value will align it inside a 24 character field; printing two values with a comma between them will align them inside 12 character fields.

Program listings printed with LLIST are also affected by the 24 character limit.
Individual lines are wrapped to avoid breaking a keyword across multiple lines; additional spaces are printed at the end of each wrapped line to advance the cursor to the next line and additional spaces are printed at the start of each wrapped line to intend them.
The serial interface does attempt to undo this hard wrapping but it's far from perfect.

Not all cassette-related commands are redirected: CLOAD M, CSAVE M and MERGE will still access the cassette instead of the serial interface.
