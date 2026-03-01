# PS/2 keyboard interface

This interface allows you to type text into your SHARP Pocket Computer using an external PS/2 keyboard.

## Command reference table

* **Statements:**
  * KEY

|Command|Example     |Function                                                            |
|:------|:-----------|:-------------------------------------------------------------------|
|KEY    |KEY ON      |Accepts keyboard input from the keyboard connected to the PS/2 port.|
|       |KEY OFF     |Disables keyboard input through the PS/2 port.                      |

## Special keys

Certain special keys are present on the SHARP Pocket Computer keyboard that are not commonly found on PS/2 keyboards.
These keys have generally been remapped to equivalents based on the ASCII code assignments used by the Pocket Computer:
for example, ASC"√" yields 91 on the SHARP Pocket Computer which normally corresponds to the [ character in ASCII,
and so pressing [ on the PS/2 keyboard produces a √ symbol on the Pocket Computer.

|SHARP keyboard|PS/2 keyboard|
|:------------:|:-----------:|
|       √      |     \[      |
|       ¥      |     \\      |
|       π      |     \]      |
|       𝐄      |      `      |
|      CL      |     Esc     |
|      CA      |  Shift+Esc  |
|     BRK      | Pause/Break |
|     P↔NP     | Print Screen|

## Shortcuts

The SHARP Pocket Computer has two sets of keyboard shortcuts:
labelled programs that can be run by pressing DEF and then a key,
or a string that can be entered by pressing SHIFT and then a key.

These shortcuts are accessible when using the PS/2 keyboard,
using Ctrl+key instead of DEF then key for labelled programs and
Shift+key instead of SHIFT then key for reserve mode strings.

Normally these shortcuts are only avaiable for the alphabetic keys in the bottom two rows of the keyboard along with SPC and  =,
however when using a PS/2 keyboard many more keys can be redefined in this way.

The Q and T keys cannot be used as shortcuts.

### Labelled programs (DEF)

Label the start of a program's line in memory with a single-character shortcut key in quotes, for example:

```
10:"A":PRINT "FIRST"
20:END
80:"B":PRINT "SECOND"
90:END
```

Pressing Ctrl+A will run the program starting from line 10 and pressing Ctrl+B will run the program starting from line 80.

The function keys F1 to F9 and F10 can be also used to jump to the labelled programs "1" to "9" and "0" without pressing the Ctrl key.

### Reserve mode (SHIFT)

Reserve mode strings can be entered and defined for the alphabetic keys A-Z with the exception of Q and T;
in addition, reserve mode strings can be assigned to the = key and the SPC key.

For Pocket Computers that don't have a user-defineable reserve mode (such as the PC-1245)
these keys will instead type the shortcut printed above the key.

As the = key is normally used to type a + sign when the Shift key is held,
Shift+Enter is used to enter the reserve mode string assigned to =.
