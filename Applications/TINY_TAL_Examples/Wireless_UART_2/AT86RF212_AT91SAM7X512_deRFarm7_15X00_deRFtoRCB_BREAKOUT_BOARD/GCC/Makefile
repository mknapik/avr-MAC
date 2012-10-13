############################################################################################
# Makefile for the project Wireless_UART Release Using single source files
############################################################################################

# Build specific properties
_TAL_TYPE = AT86RF212
_PAL_TYPE = AT91SAM7X512
_PAL_GENERIC_TYPE = ARM7
_BOARD_TYPE = deRFarm7_15X00_deRFtoRCB_BREAKOUT_BOARD
_HIGHEST_STACK_LAYER = TINY_TAL

# available memories on target 
MEMORIES = sram flash

## Trace level used
# (can be overriden by adding TRACE_LEVEL=#number to the command-line)
# TRACE_LEVEL_DEBUG      5
# TRACE_LEVEL_INFO       4
# TRACE_LEVEL_WARNING    3
# TRACE_LEVEL_ERROR      2
# TRACE_LEVEL_FATAL      1
# TRACE_LEVEL_NO_TRACE   0
TRACE_LEVEL = 0
DYNTRACE = 0

# verbosity level of compilation, uncomment for verbose make console output
SILENT = true

# Path variables
MAIN_DIR = ../../../../..
APP_DIR = ../..
PATH_TINY_TAL = $(MAIN_DIR)/TINY_TAL
PATH_PAL = $(MAIN_DIR)/PAL
PATH_SIO_SUPPORT = $(MAIN_DIR)/Applications/Helper_Files/SIO_Support
PATH_AT91LIB = $(PATH_PAL)/$(_PAL_GENERIC_TYPE)/generic/at91lib
PATH_STARTUP = $(PATH_PAL)/$(_PAL_GENERIC_TYPE)/$(_PAL_TYPE)/Startup
PATH_LDSCRIPTS = $(PATH_PAL)/$(_PAL_GENERIC_TYPE)/$(_PAL_TYPE)/Linker_scripts

# Project name and output files destinations
PROJECT = Wireless_UART
BIN_DIR = bin
OBJ_DIR = obj
OUTPUT := $(BIN_DIR)/$(PROJECT)


#------------------------------------------------------------------------------
# Toolchain
#------------------------------------------------------------------------------

# Tool suffix when cross-compiling, either 'arm-elf-' or 'arm-none-eabi-'
TOOLCHAIN = arm-none-eabi-
# Compilation tools
CC     = $(TOOLCHAIN)gcc
CP     = $(TOOLCHAIN)objcopy
SIZE   = $(TOOLCHAIN)size
STRIP  = $(TOOLCHAIN)strip

## Options common to compile, link and assembly rules
# General Flags
MCU    = arm7tdmi
COMMON = -mcpu=$(MCU)

# comment out, if debugging / USB interface used
OPTFLAGS = -O0

## Compile options common for all C compilation units.
CFLAGS += $(COMMON) 
CFLAGS += $(INCLUDES)
CFLAGS += -Wall -Werror -g -Wundef -std=c99 $(OPTFLAGS)
CFLAGS += -Wno-parentheses
CFLAGS += -DDEBUG=0 -DNOASSERT
CFLAGS += -DSIO_HUB -DUART0 # 9600, on BOB
CFLAGS += -DTAL_TYPE=$(_TAL_TYPE)
CFLAGS += -DPAL_GENERIC_TYPE=$(_PAL_GENERIC_TYPE)
CFLAGS += -DPAL_TYPE=$(_PAL_TYPE)
CFLAGS += -DVENDOR_BOARDTYPES=1
CFLAGS += -DBOARD_TYPE=$(_BOARD_TYPE)
CFLAGS += -DHIGHEST_STACK_LAYER=$(_HIGHEST_STACK_LAYER)
CFLAGS += -MD -MP -MT $(*F).o -MF $(OBJ_DIR)/$(@F).d
# AT91-specific
CFLAGS += -mthumb-interwork  
CFLAGS += -fomit-frame-pointer
CFLAGS += -Wstrict-prototypes
CFLAGS += -mlong-calls 
CFLAGS += -ffunction-sections
CFLAGS += -DTRACE_LEVEL=$(TRACE_LEVEL)
CFLAGS += -DDYNTRACE=$(DYNTRACE)

## Assembly specific flags
ASMFLAGS += $(COMMON) 
ASMFLAGS += $(CFLAGS)
ASMFLAGS += -x assembler-with-cpp -Wa,-g
ASMFLAGS += -g -gdwarf-2
ASMFLAGS += -D__ASSEMBLY__

## Linker flags
LDFLAGS += $(COMMON) -g $(OPTFLAGS) -nostartfiles -Wl,--gc-sections
LDFLAGS += -Wl,--cref


#------------------------------------------------------------------------------
# Include paths, files to build
#------------------------------------------------------------------------------

# Include directories for application
INCLUDES = -I $(APP_DIR)/Inc
# Include directories for SIO support
INCLUDES += -I $(PATH_SIO_SUPPORT)/Inc
# Include directories for general includes
INCLUDES += -I$(MAIN_DIR)/Include/
## Include directories for TAL
INCLUDES += -I$(MAIN_DIR)/TAL/$(_TAL_TYPE)/Inc/
## Include directories for TINY_TAL
INCLUDES += -I$(MAIN_DIR)/TINY_TAL/Inc/
INCLUDES += -I$(MAIN_DIR)/TINY_TAL/$(_TAL_TYPE)/Inc/
# Include directories for PAL
INCLUDES += -I$(MAIN_DIR)/PAL/Inc/
INCLUDES += -I$(MAIN_DIR)/PAL/$(_PAL_GENERIC_TYPE)/Generic/Inc
INCLUDES += -I$(MAIN_DIR)/PAL/$(_PAL_GENERIC_TYPE)/$(_PAL_TYPE)/Boards/
# Include directories for specific boards types
INCLUDES += -I$(MAIN_DIR)/PAL/$(_PAL_GENERIC_TYPE)/$(_PAL_TYPE)/Boards/$(_BOARD_TYPE)
INCLUDES += -I$(MAIN_DIR)/PAL/$(_PAL_GENERIC_TYPE)/$(_PAL_TYPE)/Inc
# AT91Lib 
INCLUDES += -I$(PATH_AT91LIB)/peripherals 
INCLUDES += -I$(PATH_AT91LIB)/components
INCLUDES += -I$(PATH_AT91LIB)/utility
INCLUDES += -I$(PATH_AT91LIB)
# USB descriptors 
INCLUDES += -I ./../descriptors

## paths to search for source files
VPATH += $(APP_DIR)/Src
VPATH += $(PATH_SIO_SUPPORT)/Src
VPATH += $(PATH_PAL)/$(_PAL_GENERIC_TYPE)/Generic/Src
VPATH += $(PATH_PAL)/$(_PAL_GENERIC_TYPE)/$(_PAL_TYPE)/Src
VPATH += $(PATH_PAL)/$(_PAL_GENERIC_TYPE)/$(_PAL_TYPE)/Boards/$(_BOARD_TYPE)
VPATH += $(PATH_TINY_TAL)/$(_TAL_TYPE)/Src
VPATH += $(PATH_TINY_TAL)/Src
VPATH += $(PATH_AT91LIB)/utility
VPATH += $(PATH_AT91LIB)/peripherals
VPATH += $(PATH_AT91LIB)/peripherals/aic
VPATH += $(PATH_AT91LIB)/peripherals/dbgu
VPATH += $(PATH_AT91LIB)/peripherals/usart
VPATH += $(PATH_AT91LIB)/peripherals/pio
VPATH += $(PATH_STARTUP)

## Objects built from C source files
# main application
C_OBJECTS += main.o
# stack sources
C_OBJECTS += sio_handler.o
C_OBJECTS += pal_sio_hub.o
C_OBJECTS += pal_uart.o
C_OBJECTS += pal.o
C_OBJECTS += pal_timer.o
C_OBJECTS += pal_utils.o
C_OBJECTS += pal_trx_access.o
C_OBJECTS += pal_irq.o
C_OBJECTS += pal_board.o
C_OBJECTS += tiny_tal.o
C_OBJECTS += tiny_tal_rx.o
C_OBJECTS += tiny_tal_tx.o
C_OBJECTS += tiny_tal_pib.o
C_OBJECTS += tiny_tal_init.o
C_OBJECTS += tiny_tal_irq_handler.o
C_OBJECTS += tiny_tal_pwr_mgmt.o
C_OBJECTS += tiny_tal_rx_enable.o
# AT91Lib sources
C_OBJECTS += led.o
C_OBJECTS += string.o
C_OBJECTS += stdio.o
C_OBJECTS += aic.o
C_OBJECTS += dbgu.o
C_OBJECTS += usart.o
C_OBJECTS += pio.o
C_OBJECTS += pio_it.o
C_OBJECTS += board_memories.o
C_OBJECTS += board_lowlevel.o

## Objects built from Assembly source files
ASM_OBJECTS = board_cstartup.o


# -----------------------------------------------------------------------------
# make targets and -rules
# -----------------------------------------------------------------------------

all: $(BIN_DIR) $(OBJ_DIR) $(MEMORIES)

$(BIN_DIR) $(OBJ_DIR):
	mkdir $@

define RULES

C_OBJECTS_$(1) = $(addprefix $(OBJ_DIR)/$(1)_, $(C_OBJECTS))
ASM_OBJECTS_$(1) = $(addprefix $(OBJ_DIR)/$(1)_, $(ASM_OBJECTS))

$(1): $$(ASM_OBJECTS_$(1)) $$(C_OBJECTS_$(1))
	@echo --------------------------------------------------------------------
	@echo linking
	@echo --------------------------------------------------------------------
	$(CC) $(LDFLAGS) -T$(PATH_LDSCRIPTS)/$$@.lds -Wl,-Map=$(OUTPUT)-$$@.map \
					-o $(OUTPUT)-$$@.elf $$^
	@echo --------------------------------------------------------------------
	@echo creating BIN file
	@echo --------------------------------------------------------------------
	$(CP) -O binary $(OUTPUT)-$$@.elf $(OUTPUT)-$$@.bin
	@echo --------------------------------------------------------------------
	@echo determine file size
	@echo --------------------------------------------------------------------
	@-$(SIZE) $$^ $(OUTPUT)-$$@.elf

$$(C_OBJECTS_$(1)): $(OBJ_DIR)/$(1)_%.o: %.c Makefile $(OBJ_DIR) $(BIN_DIR)
ifeq ($(SILENT),true)
	@echo compiling $$@
	@-$(CC) $(CFLAGS) -D$(1) -c -o $$@ $$<
else
	$(CC) $(CFLAGS) -D$(1) -c -o $$@ $$<	
endif

$$(ASM_OBJECTS_$(1)): $(OBJ_DIR)/$(1)_%.o: %.S Makefile $(OBJ_DIR) $(BIN_DIR)
ifeq ($(SILENT),true)
	@echo compiling $$@
	@-$(CC) $(ASMFLAGS) -D$(1) -c -o $$@ $$<
else
	$(CC) $(ASMFLAGS) -D$(1) -c -o $$@ $$<
endif	

endef

$(foreach MEMORY, $(MEMORIES), $(eval $(call RULES,$(MEMORY))))

clean:
	-rm -f $(OBJ_DIR)/*.o $(OBJ_DIR)/*.lst $(OBJ_DIR)/.d -r $(OBJ_DIR)/.dep
	-rm -f $(BIN_DIR)/*.bin $(BIN_DIR)/*.elf $(BIN_DIR)/*.map
	

# Include the dependency files, should be the last of the makefile
-include $(shell mkdir $(OBJ_DIR)/.dep 2>/dev/null) $(wildcard  $(OBJ_DIR)/.dep/*)

# *** EOF ***
