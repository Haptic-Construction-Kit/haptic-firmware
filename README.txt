INTRODUCTION

This directory contains the firmware code for the haptic controller device. The code is built to utilize the Arduino codebase and is designed for 3.3v 8mhz atmel chips, and specifically was built on the FIO series of boards available from Sparkfun. 
https://www.sparkfun.com/products/11520 

 - Open the sketch in the Arduino GUI
 - Choose your board from the Tools -> Board menu
 - Choose the serial port the device is connected to from Tools -> Serial Port
 - Press the Upload button to compile and program the controller board

Although the Arduino documentation indicates that *.c files in the sketch
directory will be compiled separately into object files and then linked as
necessary, this does not appear to work as advertised in arduino-0015. To work
around this, a few C files shared by the ATtiny48 and the Funnel I/O had to be
#included into main.cpp.

Once uploaded, you can talk to the controller by connecting to it with a terminal program like hypterm, putty, screen, etc. (arduino's built in terminal doesnt send compatible new lines so unfortunately its not usable) 

Hitting enter several times will bring you out of command mode and to the text debug menu where you can teach the belt new rhthyms and magnitudes, query to see if any motors were found, and do some basic actuation.

Ideally you'll utilize a driver and host program to communicate with the belt. Find repositories nearby like pyhaptic for python, and Haptic Driver and HaptikosPC for .net

BUILDING THE DOCUMENTATION

Install Doxygen and Graphviz, then run the builddoc script in this directory.
HTML documentation will appear in doc/funnel for the Funnel I/O side and
doc/tiny for the Haptic Tactor side. This is primarily useful to get call graphs
and caller graphs for all the functions in the code.

BUILDING THE TACTOR CODE

As of 2.0 all tactor firmware has been split into its own repository
