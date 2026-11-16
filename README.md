# Sharp CE-125 Options

This repository describes three "option" devices that plug into the expansion port of the Sharp CE-125 printer and cassette interface:

* [ROM](rom): A simple cartridge that contains a BASIC program that can be loaded onto the pocket computer via the ROM command.
* [Keyboard](keyboard): A PS/2 keyboard adaptor that allows you to type on the pocket computer from an external keyboard.
* [Serial](serial): An RS-232 interface that lets you transfer data between the pocket computer and another RS-232 device.

These all rely on undocumented keywords implemented in the pocket computer's ROM,
and are fully compatible with the PC-1251 and mostly compatible with the PC-1245.

Later pocket computers that removed these undocumented keywords (such as the PC-1246 and PC-1248) are unfortunately not supported.

These devices are intentionally kept simple to try to keep them in line with how I imagined the original devices would have worked,
had they been released.
For example, the RS-232 serial interface's BASIC keywords are very similar to those of the CE-158 RS-232 interface for the PC-1500,
so I tried to maintain as much compatibility with the CE-158's operation as possible.

These devices all use a [common library](lib) and so it should be possible to build a device that hooks into the same BASIC keywords,
but performs some other function; for example, using `INSTAT` and `OUTSTAT` to control 8 general I/O lines instead of making them control
RS-232 handshaking lines, or using the cassette output redirection from `CSAVE` to allow programs to be saved to the ROM module.
