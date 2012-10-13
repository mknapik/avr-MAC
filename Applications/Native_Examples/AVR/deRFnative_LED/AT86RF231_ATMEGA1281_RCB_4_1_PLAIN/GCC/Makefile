############################################################################################
# Makefile using single source files
############################################################################################
# $Id: Makefile,v 1.1.4.1 2010/09/07 17:39:38 dam Exp $

# Build specific properties
_PLATFORM = RADIO_CONTROLLER_BOARD

# Path variables
## Path to main project directory
MAIN_DIR = ../../..

## General Flags
PROJECT = deRFnative_LED
MCU = atmega1281
_F_CPU = 8000000
TARGET_DIR = .
TARGET = $(TARGET_DIR)/$(PROJECT).elf
CC = avr-gcc

## Options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU)
## Compile options common for all C compilation units.
CFLAGS = $(COMMON)
CFLAGS += -Wall -Werror -g -Wundef -std=c99 -Os
CFLAGS += -DPLATFORM=$(_PLATFORM)
CFLAGS += -DF_CPU=$(_F_CPU)
CFLAGS += -ffunction-sections # switch to remove unused functions -> this compresses the code ('--gc-sections' on Linker flags is needed additionally)
CFLAGS += -MD -MP -MT $(*F).o -MF dep/$(@F).d

## Assembly specific flags
ASMFLAGS = $(COMMON)
ASMFLAGS += $(CFLAGS)
ASMFLAGS += -x assembler-with-cpp -Wa,-g

## Linker flags
LDFLAGS = $(COMMON) -Wl,-Map=$(PROJECT).map
LDFLAGS += -Wl,--gc-sections

## Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom

HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0 --no-change-warnings

## Include directories for application
INCLUDES = -I $(MAIN_DIR)/Inc 
INCLUDES += -I $(MAIN_DIR)/$(PROJECT)/Inc

## Objects that must be built in order to link
OBJECTS = $(TARGET_DIR)/main.o \
	$(TARGET_DIR)/io_access.o

## Objects explicitly added by the user
LINKONLYOBJECTS =

## Build
all: $(TARGET) $(TARGET_DIR)/$(PROJECT).hex $(TARGET_DIR)/$(PROJECT).eep $(TARGET_DIR)/$(PROJECT).lss size

## Compile
$(TARGET_DIR)/main.o: $(MAIN_DIR)/$(PROJECT)/Src/main.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  -o $@ $<
$(TARGET_DIR)/io_access.o: $(MAIN_DIR)/Src/io_access.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
		
## Link
$(TARGET): $(OBJECTS)
	 $(CC) $(LDFLAGS) $(OBJECTS) $(LINKONLYOBJECTS) -o $(TARGET)

%.hex: $(TARGET)
	avr-objcopy -O ihex $(HEX_FLASH_FLAGS)  $< $@

%.eep: $(TARGET)
	avr-objcopy $(HEX_EEPROM_FLAGS) -O ihex $< $@ || exit 0

%.lss: $(TARGET)
	avr-objdump -h -S $< > $@

## avr-size options
IS_WIN32 := $(shell uname -s | sed -n -e 's/^MINGW.*/-C/p' -e 's/^CYGWIN.*/-C/p')
ifdef IS_WIN32
SIZEFLAGS = -C --mcu=${MCU}
else
SIZEFLAGS = -B
endif

size: ${TARGET}
	@echo
	@avr-size $(SIZEFLAGS) ${TARGET}

## Clean target
.PHONY: clean
clean:
	-rm -rf $(TARGET_DIR)/*.o $(TARGET_DIR)/$(PROJECT).elf dep/* $(TARGET_DIR)/$(PROJECT).hex $(TARGET_DIR)/$(PROJECT).eep $(TARGET_DIR)/$(PROJECT).lss $(TARGET_DIR)/$(PROJECT).map

## Other dependencies
-include $(shell mkdir dep 2>/dev/null) $(wildcard dep/*)

