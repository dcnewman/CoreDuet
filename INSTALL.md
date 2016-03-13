Traditionally, the CoreDuet repo is built using the Eclipse IDE.  If that
is to your liking, then by all means go ahead and do so.

This fork of the CoreDuet sources may also be built using the
[scons](http://scons.org/) command line tool,

1. Should your system lack Python 2.x, download and install it.

2. Download and install [scons](http://scons.org/).  Version 2.3.5 or
   later is required.

3. Install the [Arduino](https://www.arduino.cc/) application.
   Versions 1.5.8beta is known to work.  You will merely be using the
   gcc-arm toolchain installed with/by the Arduino application.
   
4. Run the Arduino application and install the ARM programming tools.
   Look under "Tools > Board > Boards Manager...".  Select the "Arduino
   SAM Boards (32-bits ARM Cortex-M3) by Arduino" and install it.

5. From this github repository, edit the file

         sample_rrf_arduino_paths.py

   as appropriate to indicate the location of your installed Arduino
   application and gcc tools.  Then save this file to your home directory
   as the file

         ~/.rrf_arduino_paths.py

6. From the top-level CoreDuet repository directory, build the
   firmware with the command

         scons
   	     
   Once scons finishes a build, the resulting files will be in the directory

        Release/

   The final build product is the file `libCoreDuet.a`.
