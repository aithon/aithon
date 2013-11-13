# These top lines are ones which Brian has had to change / found useful for various reasons.

# Point to aithon library folder
AITHON_LIBRARY = ../AithonLibrary

# Define project name here (defines name of output binaries)
PROJECT = IAP

# List all source files here
USER_SRC_FILES = $(wildcard *.c)

# Path to the Aithon board makefile (shouldn't have to change).
BOARD_INC_DIR = ../AithonLibrary/Board



##############################################################################
# Build global options
#

# Compiler options here.
USE_OPT = -O2 -ggdb -fomit-frame-pointer -falign-functions=16

# C specific options here (added to USE_OPT).
USE_COPT =

# C++ specific options here (added to USE_OPT).
USE_CPPOPT = -fno-rtti

# Enable this if you want the linker to remove unused code and data
USE_LINK_GC = yes

# If enabled, this option allows to compile the application in THUMB mode.
USE_THUMB = yes

# Enable this if you want to see the full log while compiling.
USE_VERBOSE_COMPILE = no

#
# Build global options
##############################################################################

##############################################################################
# Architecture or project specific options
#

# Enables the use of FPU on Cortex-M4.
# Enable this if you really want to use the STM FWLib.
USE_FPU = no

#
# Architecture or project specific options
##############################################################################

##############################################################################
# Project, sources and paths
#

# Path to ChibiOS
CHIBIOS = $(AITHON_LIBRARY)/ChibiOS

LDSCRIPT= $(CHIBIOS)/os/ports/GCC/ARMCMx/STM32F4xx/ld/STM32F407xG.ld

# Imported source files and paths
include $(CHIBIOS)/os/hal/platforms/STM32F4xx/platform.mk
include $(CHIBIOS)/os/hal/hal.mk
include $(CHIBIOS)/os/ports/GCC/ARMCMx/STM32F4xx/port.mk
include $(CHIBIOS)/os/kernel/kernel.mk


# C sources that can be compiled in ARM or THUMB mode depending on the global
# setting.
CSRC = $(PORTSRC) \
       $(KERNSRC) \
       $(HALSRC) \
       $(PLATFORMSRC) \
       $(CHIBIOS)/os/various/chprintf.c \
       $(AITHON_LIBRARY)/Board/board.c \
       $(AITHON_LIBRARY)/Aithon.c \
       $(AITHON_LIBRARY)/aithon_private.c \
       $(AITHON_LIBRARY)/usbCDC.c \
       $(AITHON_LIBRARY)/lcd.c \
       $(USER_SRC_FILES)

# C++ sources that can be compiled in ARM or THUMB mode depending on the global
# setting.
CPPSRC =

# C sources to be compiled in ARM mode regardless of the global setting.
# NOTE: Mixing ARM and THUMB mode enables the -mthumb-interwork compiler
#       option that results in lower performance and larger code size.
ACSRC =

# C++ sources to be compiled in ARM mode regardless of the global setting.
# NOTE: Mixing ARM and THUMB mode enables the -mthumb-interwork compiler
#       option that results in lower performance and larger code size.
ACPPSRC =

# C sources to be compiled in THUMB mode regardless of the global setting.
# NOTE: Mixing ARM and THUMB mode enables the -mthumb-interwork compiler
#       option that results in lower performance and larger code size.
TCSRC =

# C sources to be compiled in THUMB mode regardless of the global setting.
# NOTE: Mixing ARM and THUMB mode enables the -mthumb-interwork compiler
#       option that results in lower performance and larger code size.
TCPPSRC =

# List ASM source files here
ASMSRC = $(PORTASM)

INCDIR = $(PORTINC) $(KERNINC) \
         $(HALINC) $(PLATFORMINC) \
         $(AITHON_LIBRARY)/Board/ \
         $(CHIBIOS)/os/various/ \
         $(AITHON_LIBRARY)/ChibiOS \
         $(AITHON_LIBRARY)/

#
# Project, sources and paths
##############################################################################

##############################################################################
# Compiler settings
#

MCU  = cortex-m4

TRGT = arm-none-eabi-
CC   = $(TRGT)gcc
CPPC = $(TRGT)g++
LD   = $(TRGT)gcc
CP   = $(TRGT)objcopy
AS   = $(TRGT)gcc -x assembler-with-cpp
OD   = $(TRGT)objdump
HEX  = $(CP) -O ihex
BIN  = $(CP) -O binary

# ARM-specific options here
AOPT =

# THUMB-specific options here
TOPT = -mthumb -DTHUMB

# Define C warning options here
CWARN = -Wall -Wextra -Wstrict-prototypes

# Define C++ warning options here
CPPWARN = -Wall -Wextra

#
# End of user defines
##############################################################################

ifeq ($(USE_FPU),yes)
  USE_OPT += -mfloat-abi=softfp -mfpu=fpv4-sp-d16 -fsingle-precision-constant
  DDEFS = -DCORTEX_USE_FPU=TRUE
else
  DDEFS = -DCORTEX_USE_FPU=FALSE
endif

include $(CHIBIOS)/os/ports/GCC/ARMCMx/rules.mk