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
