# Edit this file to reflect your Arduino build environment/paths
# Then save this file as
#
#   ~/.rrf_arduino_paths.py

# For 1.5.x, we only need to know the path to the top-level Arduino
# directory, arduino_home, and the Arduino architecture of avr or sam,
# arduino_arch.  For 1.5.x,  we can determine the Arduino version as
# well with just the top-level Arduino directory, but for 1.6.x it's
# better to be told it upfront to reduce trial-and-error guessing.
#
# For 1.6.x, arduino_home depends upon whether the target Architecture
# installed with Arduino 1.6.x or if it was downloaded separately via
# Tools > Board > Boards Manager....  Also the location of the gcc
# tool chain must be specified for 1.6.x.  While it is possible to
# determine that path, for separately downloaded boards 1.6.x requires
# parsing of JSON files as well as .txt files.  To avoid that complexity
# a human assistant must provide that information.

# Absolute path to your local copy of the CoreDuet repo
coreduet_path = '/Users/dnewman/git/CoreDuet'

# 1.5.8 SAM
#arduino_version = 158
#arduino_arch  = 'sam'
#arduino_home  = '/Applications/Arduino-1.5.8.app/Contents/Resources/Java'
#arduino_tools = '/Applications/Arduino-1.5.8.app/Contents/Resources/Java/hardware/tools/gcc-arm-none-eabi-4.8.3-2014q1'

# 1.6.4 SAM
arduino_version = 164
arduino_arch  = 'sam'
arduino_home  = '/Users/dnewman/Library/Arduino15/packages/arduino'
arduino_tools = '/Users/dnewman/Library/Arduino15/packages/arduino/tools/arm-none-eabi-gcc/4.8.3-2014q1'
