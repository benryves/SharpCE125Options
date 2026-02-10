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
