############################################################################################
# Makefile for the project Simple_Remote_LED_Control Release Using single source files
############################################################################################
# $Id: Makefile 15381 2009-05-07 12:35:48Z awachtle $

# Command line options
# CHAN = radio channel {11,...,26}
DEFAULT_CHAN=20
CHAN=$(DEFAULT_CHAN)
ifeq ($(DEFAULT_CHAN),$(CHAN))
 PROJEXT=
else
 PROJEXT=_ch$(CHAN)
endif
# select optimization level (-Os for size optimization, -O0 for AvrStudio Debugging)
OPTFLAGS=-O0

# Build specific properties
_TAL_TYPE = ATMEGARF_TAL_1
_PAL_TYPE = ATMEGA128RFA1
_PAL_GENERIC_TYPE = MEGA_RF
_BOARD_TYPE = RCB_6_3_PLAIN
_HIGHEST_STACK_LAYER = PAL
_RADIO_CHANNEL=$(CHAN)

# Path variables
## Path to main project directory
MAIN_DIR = ../../../../..
APP_DIR = ../..
PATH_APP = $(MAIN_DIR)/Applications
PATH_PAL = $(MAIN_DIR)/PAL
PATH_REGISTERS = $(MAIN_DIR)/TAL/$(_TAL_TYPE)
PATH_RES = $(MAIN_DIR)/Resources
PATH_GLOB_INC = $(MAIN_DIR)/Includes

## General Flags
PROJECT = simple_remote_led_control$(PROJEXT)
MCU = atmega128rfa1
TARGET_DIR = .
TARGET = $(TARGET_DIR)/$(PROJECT).elf
CC = avr-gcc

## Options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU)

## Compile options common for all C compilation units.
CFLAGS = $(COMMON)
CFLAGS += -Wall -Werror -g -Wundef -std=c99 $(OPTFLAGS)
CFLAGS += -DDEBUG=0
CFLAGS += -DTAL_TYPE=$(_TAL_TYPE)
CFLAGS += -DPAL_GENERIC_TYPE=$(_PAL_GENERIC_TYPE)
CFLAGS += -DPAL_TYPE=$(_PAL_TYPE)
CFLAGS += -DBOARD_TYPE=$(_BOARD_TYPE)
CFLAGS += -DHIGHEST_STACK_LAYER=$(_HIGHEST_STACK_LAYER)
CFLAGS += -MD -MP -MT $(*F).o -MF dep/$(@F).d
CFLAGS += -DDEFAULT_CHANNEL=$(_RADIO_CHANNEL)

## Assembly specific flags
ASMFLAGS = $(COMMON)
ASMFLAGS += $(CFLAGS)
ASMFLAGS += -x assembler-with-cpp -Wa,-g

## Linker flags
LDFLAGS = $(COMMON) -Wl,-Map=$(PROJECT).map -Wl,--section-start=.data=0x800200

## Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom

HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0 --no-change-warnings

## Include directories for application
INCLUDES = -I $(APP_DIR)/Inc
## Include directories for general includes
INCLUDES += -I $(MAIN_DIR)/Include

## Include directories for PAL
INCLUDES += -I $(MAIN_DIR)/PAL/Inc/
INCLUDES += -I $(MAIN_DIR)/TAL/Inc/
INCLUDES += -I $(MAIN_DIR)/PAL/$(_PAL_GENERIC_TYPE)/Generic/Inc
INCLUDES += -I $(MAIN_DIR)/PAL/$(_PAL_GENERIC_TYPE)/$(_PAL_TYPE)/Inc/
## Include directories for specific boards type
INCLUDES += -I $(MAIN_DIR)/PAL/$(_PAL_GENERIC_TYPE)/$(_PAL_TYPE)/Boards/
INCLUDES += -I $(MAIN_DIR)/PAL/$(_PAL_GENERIC_TYPE)/$(_PAL_TYPE)/Boards/$(_BOARD_TYPE)
INCLUDES += -I$(PATH_REGISTERS)/Inc

## Objects that must be built in order to link
OBJECTS = $(TARGET_DIR)/main$(PROJEXT).o\
	$(TARGET_DIR)/pal_irq.o\
	$(TARGET_DIR)/pal.o\
	$(TARGET_DIR)/pal_mcu_generic.o\
	$(TARGET_DIR)/pal_timer.o\
	$(TARGET_DIR)/pal_board.o\
	$(TARGET_DIR)/pal_utils.o

## Objects explicitly added by the user
LINKONLYOBJECTS =

## Build
all: $(TARGET) $(TARGET_DIR)/$(PROJECT).hex $(TARGET_DIR)/$(PROJECT).eep $(TARGET_DIR)/$(PROJECT).lss size

## Compile
$(TARGET_DIR)/main$(PROJEXT).o: $(APP_DIR)/Src/main.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  -o $@ $<
$(TARGET_DIR)/pal_irq.o: $(PATH_PAL)/$(_PAL_GENERIC_TYPE)/$(_PAL_TYPE)/Src/pal_irq.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/pal.o: $(PATH_PAL)/$(_PAL_GENERIC_TYPE)/Generic/Src/pal.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/pal_mcu_generic.o: $(PATH_PAL)/$(_PAL_GENERIC_TYPE)/$(_PAL_TYPE)/Src/pal_mcu_generic.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/pal_timer.o: $(PATH_PAL)/$(_PAL_GENERIC_TYPE)/$(_PAL_TYPE)/Src/pal_timer.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/pal_board.o: $(PATH_PAL)/$(_PAL_GENERIC_TYPE)/$(_PAL_TYPE)/Boards/$(_BOARD_TYPE)/pal_board.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/pal_utils.o: $(PATH_PAL)/$(_PAL_GENERIC_TYPE)/Generic/Src/pal_utils.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<

##Link
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

