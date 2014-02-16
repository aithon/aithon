# sanity checks
ifndef USERSRC
$(error No user source files were specified.)
endif
ifndef AITHON_LIBRARY
$(error Aithon library path was not specified.)
endif


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
USE_FPU = yes


#
# Architecture or project specific options
##############################################################################

##############################################################################
# Project, sources and paths
#

# Path to ChibiOS
CHIBIOS = $(AITHON_LIBRARY)/ChibiOS

# Define project name here (defines name of output binaries)
PROJECT = ch

ifndef BOARD_REV
BOARD_REV = v01
endif

# Directory containing board-specific files
BOARDDIR = $(AITHON_LIBRARY)/Board_$(BOARD_REV)

# use the appropriate linker script based on whether or not we're using an IAP
ifdef IS_BOOTLOADER
   LDSCRIPT = $(BOARDDIR)/AithonBootloader.ld
else
   LDSCRIPT = $(BOARDDIR)/AithonIAP.ld
   USE_COPT += -DUSE_IAP
endif


USE_COPT += -DAITHON_$(BOARD_REV) -DDATE="\"`date`"\"

# Imported source files and paths
include $(CHIBIOS)/os/hal/platforms/STM32F4xx/platform.mk
include $(CHIBIOS)/os/hal/hal.mk
include $(CHIBIOS)/os/ports/GCC/ARMCMx/STM32F4xx/port.mk
include $(CHIBIOS)/os/kernel/kernel.mk
include $(BOARDDIR)/Board.mk
include $(CHIBIOS)/os/various/fatfs_bindings/fatfs.mk


# C sources that can be compiled in ARM or THUMB mode depending on the global
# setting.
CSRC = $(PORTSRC) \
       $(KERNSRC) \
       $(HALSRC) \
       $(PLATFORMSRC) \
       $(BOARDSRC) \
       $(FATFSSRC) \
       $(CHIBIOS)/os/various/chprintf.c \
       $(wildcard $(AITHON_LIBRARY)/*.c) \
       $(USERSRC)

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
         $(BOARDINC) \
         $(FATFSINC) \
         $(CHIBIOS)/os/various/ \
         $(AITHON_LIBRARY)/ChibiOS/ \
         $(AITHON_LIBRARY)/ \
         $(USERINC)

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


# Detect the OS
UNAME := $(shell uname)

program: all
ifeq ($(UNAME), windows32)
ifdef IS_BOOTLOADER
	@$(AITHON_LIBRARY)/Programmer/Windows/ST-LINK_Utility/ST-LINK_CLI.exe -P $(BUILDDIR)/$(PROJECT).bin 0x08000000
else
	@$(AITHON_LIBRARY)/Programmer/Windows/AithonProgrammer.exe program $(BUILDDIR)/$(PROJECT).bin
endif
endif
ifeq ($(UNAME), darwin)
	@$(AITHON_LIBRARY)/Programmer/MacOSX/AithonProgrammer program $(BUILDDIR)/$(PROJECT).bin
endif
ifeq ($(UNAME), Linux)
	@$(AITHON_LIBRARY)/Programmer/Linux/AithonProgrammer program $(BUILDDIR)/$(PROJECT).bin
endif

program_debug: all
ifeq ($(UNAME), windows32)
ifdef IS_BOOTLOADER
	@$(AITHON_LIBRARY)/Programmer/Windows/ST-LINK_Utility/ST-LINK_CLI.exe -P $(BUILDDIR)/$(PROJECT).bin 0x08000000
else
	@$(AITHON_LIBRARY)/Programmer/Windows/AithonProgrammer.exe -d program $(BUILDDIR)/$(PROJECT).bin
endif
endif
ifeq ($(UNAME), darwin)
	@$(AITHON_LIBRARY)/Programmer/MacOSX/AithonProgrammer -d program $(BUILDDIR)/$(PROJECT).bin
endif
ifeq ($(UNAME), Linux)
	@$(AITHON_LIBRARY)/Programmer/Linux/AithonProgrammer -d program $(BUILDDIR)/$(PROJECT).bin
endif
