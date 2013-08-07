INTRODUCTION

This directory contains all the firmware code for both the main belt
controller (Haptic Controller repository or Funnel I/O board--Arduino clone) and the vibration modules
(Haptic Tactor repository). The ATtiny48 code was developed under Linux using the
AVR GCC toolchain (avr-gcc, avr-libc, avrdude, and so forth) and may make use
of some constructs specific to that toolchain. 

BUILDING THE DOCUMENTATION

Install Doxygen and Graphviz, then run the builddoc script in this directory.
HTML documentation will appear in doc/funnel for the Funnel I/O side and
doc/tiny for the Haptic Tactor side. This is primarily useful to get call graphs
and caller graphs for all the functions in the code.

BUILDING THE TACTOR CODE

The Tactor is programmed entirely in AVR C Code, including nothing from Arduino.
Install the AVR toolchain and related tools: avr-gcc, avr-libc, avrdude, and
so forth. Then take a look at the build, dump, and avrdude scripts in this
directory to see which commands to use to build the code and program the
ATtiny88. The AVRISPmkII USB in-system programmer was used during development;
the arguments to avrdude (as specified in the avrdude script in this
directory) may need to be modified if a different programmer is to be used.
See the avrdude documentation for details.
ex:
./build -DDEFAULT_TWI_ADDRESS=34

BUILDING THE HAPTIC CONTROLLER I/O CODE

The Controller side is built to utilize the Arduino codebase. 
Due to some strange "preprocessing" done by the Arduino GUI, building the code
for the Controller is a little more involved than building the Haptic Tactor code.
The steps are:
 - Install the Arduino GUI (www.arduino.cc)--the code is known to work on the
   Arduino 1.0.
 - You may need to modify the Arduino preferences file to set the baud rate
   to 9600.  Change this item: serial.burn_rate=9600.  For more details see
   (http://www.arduino.cc/en/Guide/Troubleshooting#upload) or 
   (http://arduino.cc/en/Hacking/Preferences)
 - Create a new Arduino sketch (start the GUI, type something in the sketch,
   save the sketch and make note of the sketch directory, exit the GUI)
 - Symlink (or copy if your OS does not support symlinks) active_command.h,
   debug_main.h, error.*, fuelgauge.*, globals_main.h, magnitude.h, main.cpp,
   menu.h, parse.*, rhythm.h, vibration.h, and wire_err.h from this directory
   into the new sketch directory (the one that contains the .pde file that
   was created by Arduino)
 - Overwrite the .pde file created by Arduino with main.cpp (delete the .pde
   and rename main.cpp to match the .pde name--the .pde filename must match
   the name of the sketch directory)
 - Open the sketch in the Arduino GUI
 - Choose "Arduino Pro or Pro Mini (8 MHz)" from the Tools -> Board menu
 - Choose the serial port the Funnel is connected to from Tools -> Serial Port
 - Press the Upload button to compile and program the Funnel I/O board

Although the Arduino documentation indicates that *.c files in the sketch
directory will be compiled separately into object files and then linked as
necessary, this does not appear to work as advertised in arduino-0015. To work
around this, a few C files shared by the ATtiny48 and the Funnel I/O had to be
#included into main.cpp.


