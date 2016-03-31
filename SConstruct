import os
import sys
from os.path import join, expanduser, isfile

platform = ARGUMENTS.get('platform', 'duet')

target_name       = 'libCore' + platform.title()
scons_variant_dir = os.path.join('build', platform)

sam_arch       = 'sam3xa'
variant_syslib = 'libsam_sam3x8e_gcc_rel.a'

# Generate a directory tree of the directories in the list dir
# excluding any directory (and children) from the list ignore

def list_dirs(dir, ignore):
  list = []
  for d in dir:
    subdirs = [ d + '/' + name for name in os.listdir(d) if os.path.isdir(os.path.join(d, name)) and \
    (name[0] != '.') and (not (d + '/' + name) in ignore) ]
    list += list_dirs(subdirs, ignore)
  return dir + list


##########
#
# Load data from ~/.rrf_arduino_paths.py

tmp_dict = {  }
home = expanduser('~')
if home[-1] != '/':
    home += '/'
    
site_file = home + '.rrf_arduino_paths.py'
if isfile(site_file):
    with open(site_file) as f:
        exec(f.read(), tmp_dict)
    keys_of_interest = [ 'bossac_path', 'coreduet_home', 'gccarm_bin' ]
    for key in keys_of_interest:
        if key in tmp_dict:
            if type(tmp_dict[key]) is str:
                exec(key + '="' + tmp_dict[key] + '"')
            elif type(tmp_dict[key]) is int:
                exec(key + '=' + str(tmp_dict[key]))
            else:
                raise Exception(key + ' in ' + site_file + ' is of an ' + \
                        'unsupported type')

have_bossac = 'bossac_path' in globals()
have_home   = 'coreduet_home' in globals()
have_bin    = 'gccarm_bin' in globals()

if not (have_home and have_bin):
  raise Exception('You must first create the file ' + site_file + \
      ' before building.  See ' + \
      '~/sample_rrf_arduino_paths.py for an example.')

os.environ['COREDUET_HOME'] = coreduet_home
os.environ['GCCARM_BIN'] = gccarm_bin
if have_bossac:
    os.environ['BOSSAC_PATH'] = bossac_path

##########
#
# The source directories we will be building
#
core_dirs = [
    'cores',
    'libraries',
    'system',
    'variants' ]

# Source directories to ignore
ignore_dirs = [
	'libraries/SPI',
	'libraries/HID',
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

##########
#
# Include file directories
#
include_paths = [
    'cores/arduino',
    'system/CMSIS/CMSIS/Include',
    'variants/duet',
    'system/CMSIS/Device/ATMEL',
    join('system/CMSIS/Device/ATMEL', sam_arch, 'include'),
    'system/libsam',
    'system/libsam/include' ]

##########
#
# Build a scons environment

VariantDir(scons_variant_dir, './', 'duplicate=0')

# Make the clean command remove the variant sources
clean_dirs = [ join(scons_variant_dir, i) for i in core_dirs ]
Clean('.', clean_dirs)

env=Environment( tools = ['default', 'ar', 'g++', 'gcc'],
   CPPPATH = include_paths,
   LIBPATH = ['.'] )

# Compiler flags shared by C and C++
env.Replace( CCFLAGS = [
    '-c',
    '-Dprintf=iprintf',
    '-D__SAM3X8E__',
    '-DF_CPU=84000000L',
    '-DARDUINO=158',
    '-DUSB_VID=0x2341',
    '-DUSB_PID=0x003e',
    '-DUSBCON',
    '-DUSB_MANUFACTURER="Unknown"',
    '-DUSB_PRODUCT=\\"Arduino Due\\"' ] )

if platform == 'radds':
    env.Append( CCFLAGS = [
        '-DSD_MMC_SPI_MODE',
        '-DSPI_PIN=77',
        '-DSPI_CHAN=0',
        '-DSD_SS=4',
        '-DSD_DETECT_PIN=14',
        '-DSD_DETECT_VAL=0',
        '-DSD_DETECT_PIO_ID=ID_PIOD',
        '-DUSE_SAM3X_DMAX',
        '-DDMA_TIMEOUT_COMPUTE' ] )

# C compiler flags
env.Replace( CFLAGS = [
    '-ffunction-sections',
    '-fdata-sections',
    '-nostdlib',
    '--param', 'max-inline-insns-single=500',
    '-mcpu=cortex-m3',
    '-O2',
    '-Wall',
    '-std=gnu99',
    '-mthumb' ] )

# C++ flags
env.Replace( CXXFLAGS = [
    '-ffunction-sections',
    '-fdata-sections',
    '-nostdlib',
    '-fno-threadsafe-statics',
    '--param', 'max-inline-insns-single=500',
    '-fno-rtti',
    '-fno-exceptions',
    '-Dprintf=iprintf',
    '-mcpu=cortex-m3',
    '-O2',
    '-Wall',
    '-std=gnu++11',
    '-mthumb' ] )

env.SetDefault( COREDUET_HOME = coreduet_home )
env.SetDefault( GCCARM_BIN = gccarm_bin )
env.SetDefault( VARIANT_PATH = "$COREDUET_HOME/variants/duet" )
env.SetDefault( VARIANT_SYSLIB = variant_syslib )
if have_bossac:
  env.SetDefault( BOSSAC_PATH = bossac_path )

env.Replace( RANLIB = "$GCCARM_BIN/arm-none-eabi-ranlib" )
env.Replace( CC = "$GCCARM_BIN/arm-none-eabi-gcc" )
env.Replace( CXX = "$GCCARM_BIN/arm-none-eabi-g++" )
env.Replace( AR = "$GCCARM_BIN/arm-none-eabi-ar" )
env.Replace( ARFLAGS = "rcs" )
env.Replace( ASFLAGS = "-c -g -x assembler-with-cpp" )
env.Replace( SIZE = "$GCCARM_BIN/arm-none-eabi-size" )
env.Replace( OBJCOPY = "$GCCARM_BIN/arm-none-eabi-objcopy" )
env.Replace( ELF = "$GCCARM_BIN/arm-none-eabi-gcc" )
env.Replace( LD = "$GCCARM_BIN/arm-none-eabi-gcc" )

env.Append( BUILDERS = { 'Elf' : Builder(action='"$GCCARM_BIN/arm-none-eabi-gcc" -Os -Wl,--gc-sections -mcpu=cortex-m3 "-T$COREDUET_HOME/variants/duet/linker_scripts/gcc/flash.ld" "-Wl,-Map,CoreDuet.map"  -o $TARGET $_LIBDIRFLAGS -mthumb -Wl,--cref -Wl,--check-sections -Wl,--gc-sections -Wl,--entry=Reset_Handler -Wl,--unresolved-symbols=report-all -Wl,--warn-common -Wl,--warn-section-align -Wl,--warn-unresolved-symbols -Wl,--start-group $COREDUET_HOME/Release/cores/arduino/syscalls_sam3.o $_LIBFLAGS $SOURCES "$VARIANT_PATH/$VARIANT_SYSLIB"  -Wl,--end-group -lm -gcc') } )

env.Append( BUILDERS = { 'Hex' : Builder(action='"$GCCARM_BIN/arm-none-eabi-objcopy" -O binary  $SOURCES $TARGET', suffix='.hex', src_suffix='.elf') } )

if have_bossac:
  env.Replace( UPLOAD = '"$BOSSAC_PATH" --port=$PORT -U $NATIVE -e -w -v -b $SOURCES -R' )

# Generate the list of source directories to consider
src_dirs = list_dirs(core_dirs, ignore_dirs)

# Generate the list of source files to compile
srcs = []
for dir in src_dirs:
    srcs += \
    env.Glob(join(scons_variant_dir, dir, '*.c')) + \
    env.Glob(join(scons_variant_dir, dir, '*.cpp')) + \
    env.Glob(join(scons_variant_dir, dir, '*.S'))

env.Depends(srcs, join(scons_variant_dir, 'libraries', 'Storage', 'sd_mmc_mem.h'))

# Now generate the target library
objs = env.Object(srcs)
lib = env.Library(join(scons_variant_dir, target_name), objs)

env.Install('./Release', lib)
env.Alias('install', './Release')
