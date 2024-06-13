import os

# toolchains options
ARCH       = 'loongarch'
CPU        = 'la132'
CROSS_TOOL = 'gcc'

if os.getenv('RTT_ROOT'):
    RTT_ROOT = os.getenv('RTT_ROOT')
else:
    RTT_ROOT = os.path.join(os.getcwd(), '..', '..')

if os.getenv('RTT_CC'):
    CROSS_TOOL = os.getenv('RTT_CC')

# only support GNU GCC compiler
if CROSS_TOOL == 'gcc':
    PLATFORM  = 'gcc'
    EXEC_PATH = '/usr/bin'
else:
    print('Please make sure your toolchains is GNU GCC!')
    exit(0)

if os.getenv('RTT_EXEC_PATH'):
    EXEC_PATH = os.getenv('RTT_EXEC_PATH')

BUILD = 'debug'

if PLATFORM == 'gcc':
    # toolchains
    PREFIX     = 'loongarch32-newlib-elf-'
    CC         = PREFIX + 'gcc'
    CXX        = PREFIX + 'g++'
    AS         = PREFIX + 'gcc'
    AR         = PREFIX + 'ar'
    LINK       = PREFIX + 'gcc'
    TARGET_EXT = 'elf'
    SIZE       = PREFIX + 'size'
    OBJDUMP    = PREFIX + 'objdump'
    OBJCPY     = PREFIX + 'objcopy'

    DEVICE = ''
    CFLAGS = DEVICE + ' -Wall'
    AFLAGS = ' -c' + DEVICE + ' -x assembler-with-cpp -D__ASSEMBLY__'
    LFLAGS = DEVICE + ' -nostartfiles -Wl,--gc-sections,-Map=rtthread.map,-cref,-u,_start -T link.lds'
    CPATH  = ''
    LPATH  = ''

    if BUILD == 'debug':
        CFLAGS += ' -O0 -ggdb'
        AFLAGS += ' -ggdb'
    else:
        CFLAGS += ' -O2 -Os'

    CXXFLAGS = CFLAGS

DUMP_ACTION = OBJDUMP + ' -D -S $TARGET > rtthread.asm\n'
POST_ACTION = OBJCPY + ' -O binary $TARGET rtthread.bin\n' + SIZE + ' $TARGET \n'
