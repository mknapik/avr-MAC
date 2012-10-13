############################################################################################
# Makefile for the project Performance_Test Release Using single source files
############################################################################################

# Build specific properties
_TAL_TYPE = ATMEGARF_TAL_1
_PAL_TYPE = ATMEGA128RFA1
_PAL_GENERIC_TYPE = MEGA_RF
_BOARD_TYPE = deRFmega128_22X00_deRFtoRCB_BREAKOUT_BOARD
_HIGHEST_STACK_LAYER = TAL

# Path variables
## Path to main project directory
MAIN_DIR = ../../../../..
APP_DIR = ../..
PATH_TAL = $(MAIN_DIR)/TAL
PATH_TAL_CB = $(MAIN_DIR)/TAL/Src
PATH_PAL = $(MAIN_DIR)/PAL
PATH_TFA = $(MAIN_DIR)/TFA
PATH_RES = $(MAIN_DIR)/Resources
PATH_SIO_SUPPORT = $(MAIN_DIR)/Applications/Helper_Files/SIO_Support

## General Flags
PROJECT = Performance
MCU = atmega128rfa1
TARGET_DIR = .
TARGET = $(TARGET_DIR)/$(PROJECT).elf
CC = avr-gcc

## Options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU)

## Compile options common for all C compilation units.
CFLAGS = $(COMMON)
CFLAGS += -Wall -Werror -g -Wundef -std=c99 -Os 
CFLAGS += -DSIO_HUB -DUART1
CFLAGS += -DDEBUG=0
CFLAGS += -DPROMISCUOUS_MODE
CFLAGS += -DENABLE_TFA
CFLAGS += -DFFD
CFLAGS += -DHIGH_DATA_RATE_SUPPORT
CFLAGS += -DTAL_TYPE=$(_TAL_TYPE)
CFLAGS += -DPAL_GENERIC_TYPE=$(_PAL_GENERIC_TYPE)
CFLAGS += -DPAL_TYPE=$(_PAL_TYPE)
CFLAGS += -DVENDOR_BOARDTYPES=1
CFLAGS += -DBOARD_TYPE=$(_BOARD_TYPE)
CFLAGS += -DHIGHEST_STACK_LAYER=$(_HIGHEST_STACK_LAYER)
CFLAGS += -MD -MP -MT $(*F).o -MF dep/$(@F).d

## Assembly specific flags
ASMFLAGS = $(COMMON)
ASMFLAGS += $(CFLAGS)
ASMFLAGS += -x assembler-with-cpp -Wa,-g

## Linker flags
LDFLAGS = $(COMMON) -Wl,-Map=$(PROJECT).map -Wl,--section-start=.data=0x800200
LDFLAGS += -Wl,-u,vfprintf -lprintf_flt

## Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom

HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0 --no-change-warnings

## Include directories for application
INCLUDES = -I $(APP_DIR)/Inc
## Include directories for SIO support
INCLUDES += -I $(PATH_SIO_SUPPORT)/Inc
## Include directories for general includes
INCLUDES += -I $(MAIN_DIR)/Include
## Include directories for resources
INCLUDES += -I $(MAIN_DIR)/Resources/Buffer_Management/Inc/
INCLUDES += -I $(MAIN_DIR)/Resources/Queue_Management/Inc/
## Include directories for TAL
INCLUDES += -I $(MAIN_DIR)/TAL/Inc/
INCLUDES += -I $(MAIN_DIR)/TAL/$(_TAL_TYPE)/Inc/
## Include directories for PAL
INCLUDES += -I $(MAIN_DIR)/PAL/Inc/
INCLUDES += -I $(MAIN_DIR)/PAL/$(_PAL_GENERIC_TYPE)/Generic/Inc
INCLUDES += -I $(MAIN_DIR)/PAL/$(_PAL_GENERIC_TYPE)/$(_PAL_TYPE)/Inc/
## Include directories for specific boards type
INCLUDES += -I $(MAIN_DIR)/PAL/$(_PAL_GENERIC_TYPE)/$(_PAL_TYPE)/Boards/
INCLUDES += -I $(MAIN_DIR)/PAL/$(_PAL_GENERIC_TYPE)/$(_PAL_TYPE)/Boards/$(_BOARD_TYPE)
## Include directories for TFA
INCLUDES += -I $(MAIN_DIR)/TFA/Inc/

## Objects that must be built in order to link
OBJECTS = $(TARGET_DIR)/main.o\
	$(TARGET_DIR)/sio_handler.o\
	$(TARGET_DIR)/pal_uart.o\
	$(TARGET_DIR)/pal_sio_hub.o\
	$(TARGET_DIR)/pal_irq.o\
	$(TARGET_DIR)/pal.o\
	$(TARGET_DIR)/pal_mcu_generic.o\
	$(TARGET_DIR)/pal_timer.o\
	$(TARGET_DIR)/pal_board.o\
	$(TARGET_DIR)/pal_utils.o\
	$(TARGET_DIR)/bmm.o\
	$(TARGET_DIR)/qmm.o\
	$(TARGET_DIR)/tal.o\
	$(TARGET_DIR)/tal_rx.o\
	$(TARGET_DIR)/tal_tx.o\
	$(TARGET_DIR)/tal_ed.o\
	$(TARGET_DIR)/tal_slotted_csma.o\
	$(TARGET_DIR)/tal_pib.o\
	$(TARGET_DIR)/tal_init.o\
	$(TARGET_DIR)/tal_irq_handler.o\
	$(TARGET_DIR)/tal_pwr_mgmt.o\
	$(TARGET_DIR)/tal_rx_enable.o \
	$(TARGET_DIR)/tfa.o

## Objects explicitly added by the user
LINKONLYOBJECTS =

## Build
all: $(TARGET) $(TARGET_DIR)/$(PROJECT).hex $(TARGET_DIR)/$(PROJECT).eep $(TARGET_DIR)/$(PROJECT).lss size

## Compile
$(TARGET_DIR)/main.o: $(APP_DIR)/Src/main.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  -o $@ $<
$(TARGET_DIR)/sio_handler.o: $(PATH_SIO_SUPPORT)/Src/sio_handler.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  -o $@ $<
$(TARGET_DIR)/pal_uart.o: $(PATH_PAL)/$(_PAL_GENERIC_TYPE)/Generic/Src/pal_uart.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/pal_sio_hub.o: $(PATH_PAL)/$(_PAL_GENERIC_TYPE)/$(_PAL_TYPE)/Src/pal_sio_hub.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
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
$(TARGET_DIR)/bmm.o: $(PATH_RES)/Buffer_Management/Src/bmm.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/qmm.o: $(PATH_RES)/Queue_Management/Src/qmm.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/tal.o: $(PATH_TAL)/$(_TAL_TYPE)/Src/tal.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/tal_rx.o: $(PATH_TAL)/$(_TAL_TYPE)/Src/tal_rx.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/tal_tx.o: $(PATH_TAL)/$(_TAL_TYPE)/Src/tal_tx.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/tal_ed.o: $(PATH_TAL)/$(_TAL_TYPE)/Src/tal_ed.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/tal_slotted_csma.o: $(PATH_TAL)/$(_TAL_TYPE)/Src/tal_slotted_csma.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/tal_pib.o: $(PATH_TAL)/$(_TAL_TYPE)/Src/tal_pib.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/tal_init.o: $(PATH_TAL)/$(_TAL_TYPE)/Src/tal_init.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/tal_irq_handler.o: $(PATH_TAL)/$(_TAL_TYPE)/Src/tal_irq_handler.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/tal_pwr_mgmt.o: $(PATH_TAL)/$(_TAL_TYPE)/Src/tal_pwr_mgmt.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/tal_rx_enable.o: $(PATH_TAL)/$(_TAL_TYPE)/Src/tal_rx_enable.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/tal_ed_end_cb.o: $(PATH_TAL_CB)/tal_ed_end_cb.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/tfa.o: $(PATH_TFA)/$(_TAL_TYPE)/Src/tfa.c
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

