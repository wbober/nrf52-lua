PROJECT_NAME := lua_shell

BOARD ?= PCA10040

export OUTPUT_FILENAME
MAKEFILE_NAME := $(MAKEFILE_LIST)
MAKEFILE_DIR := $(dir $(MAKEFILE_NAME) ) 
TEMPLATE_PATH = $(NRF5_SDK)/components/toolchain/gcc

MK := mkdir
RM := rm -rf

#echo suspend
ifeq ("$(VERBOSE)","1")
NO_ECHO := 
else
NO_ECHO := @
endif

# Toolchain commands
GNU_PREFIX=arm-none-eabi

# Toolchain commands
CC              := $(GNU_PREFIX)-gcc
AS              := $(GNU_PREFIX)-as
AR              := $(GNU_PREFIX)-ar -r
LD              := $(GNU_PREFIX)-ld
NM              := $(GNU_PREFIX)-nm
OBJDUMP         := $(GNU_PREFIX)-objdump
OBJCOPY         := $(GNU_PREFIX)-objcopy
SIZE            := $(GNU_PREFIX)-size

#function for removing duplicates in a list
remduplicates = $(strip $(if $1,$(firstword $1) $(call remduplicates,$(filter-out $(firstword $1),$1))))

#source common to all targets
C_SOURCE_FILES += \
$(abspath $(NRF5_SDK)/components/drivers_nrf/delay/nrf_delay.c) \
$(abspath $(NRF5_SDK)/components/drivers_nrf/uart/nrf_drv_uart.c) \
$(abspath $(NRF5_SDK)/components/drivers_nrf/common/nrf_drv_common.c) \
$(abspath $(NRF5_SDK)/components/drivers_nrf/gpiote/nrf_drv_gpiote.c) \
$(abspath $(NRF5_SDK)/components/libraries/timer/app_timer.c) \
$(abspath $(NRF5_SDK)/components/libraries/fifo/app_fifo.c) \
$(abspath $(NRF5_SDK)/components/libraries/button/app_button.c) \
$(abspath $(NRF5_SDK)/components/libraries/util/app_error.c) \
$(abspath $(NRF5_SDK)/components/libraries/util/app_error_weak.c) \
$(abspath $(NRF5_SDK)/components/libraries/util/app_util_platform.c) \
$(abspath $(NRF5_SDK)/components/libraries/util/nrf_assert.c) \
$(abspath $(NRF5_SDK)/components/libraries/util/nrf_log.c) \
$(abspath $(NRF5_SDK)/components/libraries/uart/retarget.c) \
$(abspath $(NRF5_SDK)/components/libraries/uart/app_uart_fifo.c) \
$(abspath $(NRF5_SDK)/components/toolchain/system_nrf52.c) \
$(abspath $(NRF5_SDK)/components/ble/common/ble_advdata.c) \
$(abspath $(NRF5_SDK)/components/ble/common/ble_conn_params.c) \
$(abspath $(NRF5_SDK)/components/ble/common/ble_srv_common.c) \
$(abspath $(NRF5_SDK)/components/softdevice/common/softdevice_handler/softdevice_handler.c) \
$(abspath $(NRF5_SDK)/external/segger_rtt/RTT_Syscalls_GCC.c) \
$(abspath $(NRF5_SDK)/external/segger_rtt/SEGGER_RTT.c) \
$(abspath $(NRF5_SDK)/external/segger_rtt/SEGGER_RTT_printf.c) \
$(abspath $(NRF5_SDK)/examples/bsp/bsp.c) \
$(abspath ../umm_malloc/umm_malloc.c) \
$(abspath modules/lbsp.c) \
$(abspath modules/lble.c) \
$(abspath lua_shell.c) \
$(abspath lua_arch.c) \
$(abspath main.c)

#assembly files common to all targets
ASM_SOURCE_FILES  = $(abspath $(NRF5_SDK)/components/toolchain/gcc/gcc_startup_nrf52.s)

#includes common to all targets
INC_PATHS = -I$(abspath config)
ifeq ($(BOARD),PCA10036)
INC_PATHS += -I$(abspath config/pca10036)
else
INC_PATHS += -I$(abspath config/pca10040)
endif

INC_PATHS += -I$(abspath $(NRF5_SDK)/components/device)
INC_PATHS += -I$(abspath $(NRF5_SDK)/components/libraries/uart)
INC_PATHS += -I$(abspath $(NRF5_SDK)/components/libraries/util)
INC_PATHS += -I$(abspath $(NRF5_SDK)/components/libraries/fifo)
INC_PATHS += -I$(abspath $(NRF5_SDK)/components/libraries/timer)
INC_PATHS += -I$(abspath $(NRF5_SDK)/components/libraries/button)
INC_PATHS += -I$(abspath $(NRF5_SDK)/components/drivers_nrf/uart)
INC_PATHS += -I$(abspath $(NRF5_SDK)/components/drivers_nrf/hal)
INC_PATHS += -I$(abspath $(NRF5_SDK)/components/drivers_nrf/common)
INC_PATHS += -I$(abspath $(NRF5_SDK)/components/drivers_nrf/config)
INC_PATHS += -I$(abspath $(NRF5_SDK)/components/drivers_nrf/gpiote)
INC_PATHS += -I$(abspath $(NRF5_SDK)/components/drivers_nrf/delay)
INC_PATHS += -I$(abspath $(NRF5_SDK)/components/ble/common)
INC_PATHS += -I$(abspath $(NRF5_SDK)/components/softdevice/s132/headers)
INC_PATHS += -I$(abspath $(NRF5_SDK)/components/softdevice/common/softdevice_handler)
INC_PATHS += -I$(abspath $(NRF5_SDK)/components/softdevice/s132/headers/nrf52)
INC_PATHS += -I$(abspath $(NRF5_SDK)/components/toolchain)
INC_PATHS += -I$(abspath $(NRF5_SDK)/components/toolchain/gcc)
INC_PATHS += -I$(abspath $(NRF5_SDK)/components/toolchain/CMSIS/Include)
INC_PATHS += -I$(abspath $(NRF5_SDK)/external/segger_rtt)
INC_PATHS += -I$(abspath $(NRF5_SDK)/examples/bsp)
INC_PATHS += -I$(abspath modules)
INC_PATHS += -I$(abspath ../lua)
INC_PATHS += -I$(abspath ../spiffs/src)
INC_PATHS += -I$(abspath ../umm_malloc)


OBJECT_DIRECTORY = _build
LISTING_DIRECTORY = $(OBJECT_DIRECTORY)
OUTPUT_BINARY_DIRECTORY = $(OBJECT_DIRECTORY)

# Sorting removes duplicates
BUILD_DIRECTORIES := $(sort $(OBJECT_DIRECTORY) $(OUTPUT_BINARY_DIRECTORY) $(LISTING_DIRECTORY) )

PAN_IDS := 1 2 3 4 7 8 9 10 11 12 15 16 17 20 23 24 25 26 27 28 29 30 32 33 34 35 36 37 38 39 40 41 \
           42 43 44 46 47 48 49 58 63 64 65

#flags common to all targets
CFLAGS = $(foreach id, $(PAN_IDS),-DNRF52_PAN_$(id))
CFLAGS += -DNRF52
CFLAGS += -DCONFIG_GPIO_AS_PINRESET
CFLAGS += -DBOARD_$(BOARD)
CFLAGS += -DSOFTDEVICE_PRESENT
CFLAGS += -DS132
CFLAGS += -DBLE_STACK_SUPPORT_REQD
CFLAGS += -DSWI_DISABLE0
CFLAGS += -DLUA_FS_SUPPORT
CFLAGS += -DNRF_LOG_USES_RTT=1
CFLAGS += -DLUACONF_ARCH=\"luaconf_arch.h\"
CFLAGS += -mcpu=cortex-m4
CFLAGS += -mthumb -mabi=aapcs --std=gnu99
CFLAGS += -Wall -Og -g3
CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
# keep every function in separate section. This will allow linker to dump unused functions
CFLAGS += -ffunction-sections -fdata-sections -fno-strict-aliasing
CFLAGS += -fno-builtin --short-enums
# keep every function in separate section. This will allow linker to dump unused functions
LDFLAGS += -Xlinker -Map=$(LISTING_DIRECTORY)/$(OUTPUT_FILENAME).map
LDFLAGS += -mthumb -mabi=aapcs -L $(TEMPLATE_PATH) -T$(LINKER_SCRIPT)
LDFLAGS += -mcpu=cortex-m4
LDFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
# let linker to dump unused sections
LDFLAGS += -Wl,--gc-sections
# use newlib in nano version
LDFLAGS += --specs=nano.specs -lc -lnosys

# Assembler flags
ASMFLAGS += $(foreach id, $(PAN_IDS),-DNRF52_PAN_$(id))
ASMFLAGS += -x assembler-with-cpp
ASMFLAGS += -DNRF52
ASMFLAGS += -DSOFTDEVICE_PRESENT
#ASMFLAGS += -DNRF_LOG_USES_UART=1
ASMFLAGS += -DS132
ASMFLAGS += -DCONFIG_GPIO_AS_PINRESET
ASMFLAGS += -DBLE_STACK_SUPPORT_REQD
ASMFLAGS += -DSWI_DISABLE0
ASMFLAGS += -DBOARD_$(BOARD)

#default target - first one defined
default: clean nrf52832_xxaa

#building all targets
all: clean
	$(NO_ECHO)$(MAKE) -f $(MAKEFILE_NAME) -C $(MAKEFILE_DIR) -e cleanobj
	$(NO_ECHO)$(MAKE) -f $(MAKEFILE_NAME) -C $(MAKEFILE_DIR) -e nrf52832_xxaa

#target for printing all targets
help:
	@echo following targets are available:
	@echo 	nrf52832_xxaa

C_SOURCE_FILE_NAMES = $(notdir $(C_SOURCE_FILES))
C_PATHS = $(call remduplicates, $(dir $(C_SOURCE_FILES) ) )
C_OBJECTS = $(addprefix $(OBJECT_DIRECTORY)/, $(C_SOURCE_FILE_NAMES:.c=.o) )

ASM_SOURCE_FILE_NAMES = $(notdir $(ASM_SOURCE_FILES))
ASM_PATHS = $(call remduplicates, $(dir $(ASM_SOURCE_FILES) ))
ASM_OBJECTS = $(addprefix $(OBJECT_DIRECTORY)/, $(ASM_SOURCE_FILE_NAMES:.s=.o) )

vpath %.c $(C_PATHS)
vpath %.s $(ASM_PATHS)

LIBS = lua_nrf52_lib.a spiffs_nrf52_lib.a
OBJECTS = $(C_OBJECTS) $(ASM_OBJECTS)

nrf52832_xxaa: OUTPUT_FILENAME := nrf52832_xxaa
nrf52832_xxaa: LINKER_SCRIPT=lua_gcc_nrf52.ld

nrf52832_xxaa: $(BUILD_DIRECTORIES) $(OBJECTS)
	@echo Linking target: $(OUTPUT_FILENAME).out
	$(NO_ECHO)$(CC) $(LDFLAGS) $(OBJECTS) $(LIBS) -lm -o $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out
	$(NO_ECHO)$(MAKE) -f $(MAKEFILE_NAME) -C $(MAKEFILE_DIR) -e finalize

## Create build directories
$(BUILD_DIRECTORIES):
	echo $(MAKEFILE_NAME)
	$(MK) $@

# Create objects from C SRC files
$(OBJECT_DIRECTORY)/%.o: %.c
	@echo Compiling file: $(notdir $<)
	$(NO_ECHO)$(CC) $(CFLAGS) $(INC_PATHS) -c -o $@ $<

# Assemble files
$(OBJECT_DIRECTORY)/%.o: %.s
	@echo Assembly file: $(notdir $<)
	$(NO_ECHO)$(CC) $(ASMFLAGS) $(INC_PATHS) -c -o $@ $<
# Link
$(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out: $(BUILD_DIRECTORIES) $(OBJECTS)
	@echo Linking target: $(OUTPUT_FILENAME).out
	$(NO_ECHO)$(CC) $(LDFLAGS) $(OBJECTS) $(LIBS) -lm -o $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out
## Create binary .bin file from the .out file
$(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).bin: $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out
	@echo Preparing: $(OUTPUT_FILENAME).bin
	$(NO_ECHO)$(OBJCOPY) -O binary $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).bin

## Create binary .hex file from the .out file
$(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).hex: $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out
	@echo Preparing: $(OUTPUT_FILENAME).hex
	$(NO_ECHO)$(OBJCOPY) -O ihex $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).hex

finalize: genbin genhex echosize

genbin:
	@echo Preparing: $(OUTPUT_FILENAME).bin
	$(NO_ECHO)$(OBJCOPY) -O binary $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).bin

## Create binary .hex file from the .out file
genhex: 
	@echo Preparing: $(OUTPUT_FILENAME).hex
	$(NO_ECHO)$(OBJCOPY) -O ihex $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).hex
echosize:
	-@echo ''
	$(NO_ECHO)$(SIZE) $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out
	-@echo ''

clean:
	$(RM) $(BUILD_DIRECTORIES)

cleanobj:
	$(RM) $(BUILD_DIRECTORIES)/*.o
flash: $(MAKECMDGOALS)
	@echo Flashing: $(OUTPUT_BINARY_DIRECTORY)/$<.hex
	nrfjprog --program $(OUTPUT_BINARY_DIRECTORY)/$<.hex -f nrf52  --sectorerase
	nrfjprog --reset

## Flash softdevice
flash_softdevice:
	@echo Flashing: s132_nrf52_2.0.0_softdevice.hex
	nrfjprog --program $(NRF5_SDK)/components/softdevice/s132/hex/s132_nrf52_2.0.0_softdevice.hex -f nrf52 --chiperase
	nrfjprog --reset -f nrf52