C_SOURCE_FILES += \
$(abspath umm_malloc/umm_malloc.c) \
$(abspath shell/modules/lbsp.c) \
$(abspath shell/modules/lble.c) \
$(abspath shell/modules/node.c) \
$(abspath shell/lua_shell.c) \
$(abspath shell/lua_arch.c) \
$(abspath shell/lua_fs.c) \
$(abspath shell/main.c)

INC_PATHS += -I$(abspath shell/modules)
INC_PATHS += -I$(abspath lua)
INC_PATHS += -I$(abspath spiffs/src)
INC_PATHS += -I$(abspath umm_malloc)
INC_PATHS += -I$(abspath config)

#fixme: remove includes which aren't needed
INC_PATHS += -I$(abspath $(NRF5_SDK)/components/device)
INC_PATHS += -I$(abspath $(NRF5_SDK)/components/libraries/uart)
INC_PATHS += -I$(abspath $(NRF5_SDK)/components/libraries/util)
INC_PATHS += -I$(abspath $(NRF5_SDK)/components/libraries/fifo)
INC_PATHS += -I$(abspath $(NRF5_SDK)/components/libraries/timer)
INC_PATHS += -I$(abspath $(NRF5_SDK)/components/libraries/button)
INC_PATHS += -I$(abspath $(NRF5_SDK)/components/libraries/log)
INC_PATHS += -I$(abspath $(NRF5_SDK)/components/libraries/log/src)
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
INC_PATHS += -I$(abspath $(NRF5_SDK)/components/toolchain/cmsis/include)
INC_PATHS += -I$(abspath $(NRF5_SDK)/external/segger_rtt)
INC_PATHS += -I$(abspath $(NRF5_SDK)/examples/bsp)

LIBS = _build/liblua.a _build/libspiffs.a _build/libnrf52.a 
OUTPUT_FILENAME=shell

default: _build/shell.hex

include mk/rules.mk