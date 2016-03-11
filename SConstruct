import os
import sys
from os.path import join, expanduser, isfile

sys.path.append('./scons_tools')

target_name   = 'libCoreDuet'
arduino_board = 'arduino_due_x'
sam_arch      = 'sam3xa'

# Generate a directory tree of the directories in the list dir
# excluding any directory (and children) from the list ignore

def list_dirs(dir, ignore):
  list = []
  for d in dir:
    subdirs = [ d + '/' + name for name in os.listdir(d) if os.path.isdir(os.path.join(d, name)) and \
                    (name[0] != '.') and (not (d + '/' + name) in ignore) ]
    list += list_dirs(subdirs, ignore)
  return dir + list


# Load data from ~/.rrf_arduino_paths.py

tmp_dict = {  }
home = expanduser('~')
if home[-1] != '/':
    home += '/'
site_file = home + '.rrf_arduino_paths.py'

if isfile(site_file):
    with open(site_file) as f:
        exec(f.read(), tmp_dict)
	keys_of_interest = [ 'arduino_version', 'arduino_arch',
                         'arduino_home', 'arduino_tools',
						 'bossac_path' ]
        for key in keys_of_interest:
            if key in tmp_dict:
                if type(tmp_dict[key]) is str:
                    exec(key + '="' + tmp_dict[key] + '"')
                elif type(tmp_dict[key]) is int:
                    exec(key + '=' + str(tmp_dict[key]))
                else:
                    raise Exception(key + ' in ' + site_file + ' is of an ' + \
                                    'unsupported type')

have_version = 'arduino_version' in globals()
have_arch    = 'arduino_arch' in globals()
have_home    = 'arduino_home' in globals()
have_tools   = 'arduino_tools' in globals()

if not (have_version and have_arch and have_tools):
    raise Exception('You must first create the file ' + site_file + \
                    ' before building.  See ' + \
                    '~/sample_rrf_arduino_paths.py for an example.')


# We're only interested in SAM

arduino_arch = arduino_arch.lower()
if not (arduino_arch == 'sam'):
    raise Exception('This collection of source files is SAM architecture only')

if type(arduino_version) is str:
    arduino_version = int(arduino_version)

if (arduino_version >= 160) and (not have_tools):
    raise Exception('For SAM/ARM and Arduino 1.6 or later, arduino_tools ' + \
                    'must be set in ' + site_file)

os.environ['ARDUINO_VERSION'] = str(arduino_version)
os.environ['ARDUINO_HOME'] = arduino_home
os.environ['ARDUINO_ARCH'] = arduino_arch
if have_tools:
    os.environ['ARDUINO_TOOLS'] = arduino_tools

# Flags common to both compilers
flags = [ '-Dprintf=iprintf' ]

# C & C++ compiler flags
cflags = [ ] + flags
cxxflags = [ ] + flags

# Include file directories
include_paths = [
    'cores/arduino',
    'system/CMSIS/CMSIS/Include',
    'variants/duet',
    'system/CMSIS/Device/ATMEL',
    join('system/CMSIS/Device/ATMEL', sam_arch, 'include'),
    'system/libsam',
    'system/libsam/include' ]

# Initialize an environment
VariantDir('Release', './', 'duplicate=0')
env=Environment( tools = ['default', 'ar', 'g++', 'gcc', 'arduino'],
                 toolpath = ['./scons_tools'],
                 CPPPATH = include_paths,
                 CCFLAGS = '',
                 LIBPATH = ['.'] )

drop_list = [
   ('-ffunction-sections', 0),
   ('-fdata-sections', 0),
   ('-nostdlib', 0),
   ('-fno-threadsafe-statics', 0),
   ('--param', 1),
   ('-DARDUINO-158', 0),
   ('-DARDUINO_SAM_DUE', 0),
   ('-DARDUINO_ARCH_SAM', 0),
   ('\'-DUSB_Product="Arduino Due"\'', 0),
   ('\'-DUSB_MANUFACTURER="Unknown"\'', 0),
   ('-g', 0),
   ('-o', 1),   # Remove -o and single argument following it
   ('-w', 0),   # Yeah, Arduino suppresses all warnings!  They should fix their own!
   ('-Os', 0) ] # Optimize for size?  On ARM?  Really?

options = {
    'symlinks' : False,
    'cc_flags_drop_list'  : drop_list,
    'cxx_flags_drop_list' : drop_list }

# Now set up all the compiler paths and switches
env.ConfigureBoard(arduino_version, arduino_arch, arduino_board, options)

# Add in our compiler flags
env.Append( CXXFLAGS = cxxflags, CFLAGS = cflags )

# Source directories to ignore
ignore_dirs = [
    'libraries/SPI/examples',
    'system/CMSIS/Device/ATMEL/sam3xa/source/as_gcc',
    'system/CMSIS/Device/ATMEL/sam3xa/source/gcc_atmel',
    'system/CMSIS/Device/ATMEL/sam3xa/source/gcc_arm',
    'system/CMSIS/Device/ATMEL/sam3xa/source/iar',
    'system/libsam/build_iar',
    'system/CMSIS/CMSIS/Lib/ARM',
    'system/CMSIS/Device/ARM/ARMCM3/Source/Templates',
    'system/CMSIS/CMSIS/DSP_Lib',
    'system/CMSIS/Device/ATMEL/sam4s',
    'system/CMSIS/Device/ATMEL/sam3u',
    'system/CMSIS/Device/ATMEL/sam3sd8',
    'system/CMSIS/Device/ATMEL/sam3s',
    'system/CMSIS/Device/ATMEL/sam3n',
    'system/CMSIS/Device/ARM/ARMCM4',
    'system/CMSIS/Device/ARM/ARMCM0' ]

# Generate the list of source directories to consider
src_dirs = list_dirs(['cores',
                      'libraries',
                      'system',
                      'variants'], ignore_dirs)

# Generate the list of source files to compile
srcs = []
for dir in src_dirs:
    srcs += \
        env.Glob(join('Release', dir, '*.c')) + \
        env.Glob(join('Release', dir, '*.cpp')) + \
        env.Glob(join('Release', dir, '*.S'))

# Now generate the target library
objs = env.Object(srcs)
env.Library('Release/' + target_name, objs)
