# Path to Aithon library
AITHON_LIBRARY = ../AithonLibrary

# User source files
USERSRC = $(wildcard *.c)

# User include paths
USERINC = ./

# This is the bootloader so goes at the start of the FLASH
IS_BOOTLOADER = yes
FLASH_ORIG = 0x08000000
FLASH_LEN = 32k

# Include the primary Aithon makefile
include $(AITHON_LIBRARY)/Aithon.mk