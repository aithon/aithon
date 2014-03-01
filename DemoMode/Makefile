# Path to Aithon library
AITHON_LIBRARY = ../AithonLibrary

# User source files
USERFOLDER = ./
USERSRC =$(addprefix $(USERFOLDER),$(wildcard *.c))

# User include paths
USERINC = ./

# Compile this in demo mode
IS_DEMO_MODE = yes
FLASH_ORIG = 0x08010000
FLASH_LEN = 64k

# Include the primary Aithon makefile
include $(AITHON_LIBRARY)/Aithon.mk
