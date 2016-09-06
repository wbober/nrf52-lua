BOARD ?= PCA10040
LINKER_SCRIPT = mk/gcc_nrf52.ld
TEMPLATE_PATH = $(NRF5_SDK)/components/toolchain/gcc

# Toolchain commands
GNU_PREFIX=/home/wojtek/toolchains/gcc-arm-none-eabi-5_4-2016q2/bin/arm-none-eabi
#GNU_PREFIX=arm-none-eabi

# Toolchain commands
CC              := $(GNU_PREFIX)-gcc
AS              := $(GNU_PREFIX)-as
AR              := $(GNU_PREFIX)-ar -r
LD              := $(GNU_PREFIX)-ld
NM              := $(GNU_PREFIX)-nm
OBJDUMP         := $(GNU_PREFIX)-objdump
OBJCOPY         := $(GNU_PREFIX)-objcopy
SIZE            := $(GNU_PREFIX)-size


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
ASMFLAGS += -DS132
ASMFLAGS += -DCONFIG_GPIO_AS_PINRESET
ASMFLAGS += -DBLE_STACK_SUPPORT_REQD
ASMFLAGS += -DSWI_DISABLE0
ASMFLAGS += -DBOARD_$(BOARD)