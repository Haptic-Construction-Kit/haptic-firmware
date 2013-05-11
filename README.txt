INTRODUCTION

This directory contains all the firmware code for both the main belt
controller (Funnel I/O board--Arduino clone) and the vibration modules
(Atmega ATtiny48/88). The ATtiny48 code was developed under Linux using the
AVR GCC toolchain (avr-gcc, avr-libc, avrdude, and so forth) and may make use
of some constructs specific to that toolchain. The Funnel I/O code was
developed under Linux using the standard Arduino GUI (version 0015), and also
verified to build successfully on both Windows and Macintosh machines--but
the Macintosh version of Arduino-0015 apparently has a bug with its pre-
compiled twi.o, so extra steps must be taken before the code will operate
correctly.

The code was originally developed on an Ubuntu 8.10 machine with the AVR
toolchain installed from the standard repositories. The standard avrdude
provided by Ubuntu was a somewhat old version that did not support the
ATtiny48/88, so avrdude.conf was taken from the latest avrdude source and
modified slightly to work with the Ubuntu avrdude version. This should not be
necessary if a recent version of avrdude is used.

BUILDING THE DOCUMENTATION

Install Doxygen and Graphviz, then run the builddoc script in this directory.
HTML documentation will appear in doc/funnel for the Funnel I/O side and
doc/tiny for the ATtiny48 side. This is primarily useful to get call graphs
and caller graphs for all the functions in the code.

BUILDING THE ATTINY48 CODE

Install the AVR toolchain and related tools: avr-gcc, avr-libc, avrdude, and
so forth. Then take a look at the build, dump, and avrdude scripts in this
directory to see which commands to use to build the code and program the
ATtiny48. The AVRISPmkII USB in-system programmer was used during development;
the arguments to avrdude (as specified in the avrdude script in this
directory) may need to be modified if a different programmer is to be used.
See the avrdude documentation for details.

BUILDING THE FUNNEL I/O CODE

Due to some strange "preprocessing" done by the Arduino GUI, building the code
for the Funnel I/O is a little more involved than building the ATtiny48 code.
The steps are:
 - Install the Arduino GUI (www.arduino.cc)--the code is known to work on the
   arduino-0015 version
 - You may need to modify the Arduino preferences file to set the baud rate
   to 9600.  Change this item: serial.burn_rate=9600.  For more details see
   (http://www.arduino.cc/en/Guide/Troubleshooting#upload) or 
   (http://arduino.cc/en/Hacking/Preferences)
 - Create a new Arduino sketch (start the GUI, type something in the sketch,
   save the sketch and make note of the sketch directory, exit the GUI)
 - Symlink (or copy if your OS does not support symlinks) active_command.h,
   debug_main.*, error.*, fuelgauge.*, globals_main.h, magnitude.h, main.cpp,
   menu.h, parse.*, rhythm.h, vibration.h, and wire_err.h from this directory
   into the new sketch directory (the one that contains the .pde file that
   was created by Arduino)
 - Overwrite the .pde file created by Arduino with main.cpp (delete the .pde
   and rename main.cpp to match the .pde name--the .pde filename must match
   the name of the sketch directory)
 - If you are on a Mac, delete the precompiled twi.o from the Wire library (in
   the <arduino install dir>/hardware/libraries/Wire/utility/ directory)
 - Open the sketch in the Arduino GUI
 - Choose "Arduino Pro or Pro Mini (8 MHz)" from the Tools -> Board menu
 - Choose the serial port the Funnel is connected to from Tools -> Serial Port
 - Press the Verify button to compile the code, or
 - Make sure to remove any serial devices on the Funnel (such as a 
   bluetooth module).
 - Press the Upload button to compile and program the Funnel I/O board

Deleting the twi.o file simply forces Arduino to recompile the TWI code used
by its Wire library. This is not necessary on Windows or Linux, but in the Mac
version this file seems to have been miscompiled--if it is not deleted, then
the sketch will compile and upload successfully, but the Funnel will lock up
at startup when the code calls a Wire function to communicate over TWI. It is
only necessary to delete twi.o once after a fresh install of Arduino (it need
not be deleted at every recompilation).

Although the Arduino documentation indicates that *.c files in the sketch
directory will be compiled separately into object files and then linked as
necessary, this does not appear to work as advertised in arduino-0015. To work
around this, a few C files shared by the ATtiny48 and the Funnel I/O had to be
#included into main.cpp.
