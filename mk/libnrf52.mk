BOARD ?= PCA10040

#assembly files common to all targets
ASM_SOURCE_FILES = $(abspath $(NRF5_SDK)/components/toolchain/gcc/gcc_startup_nrf52.S)

C_SOURCE_FILES += \
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
$(abspath $(NRF5_SDK)/components/libraries/log/src/nrf_log_backend_serial.c) \
$(abspath $(NRF5_SDK)/components/libraries/log/src/nrf_log_frontend.c) \
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
$(abspath $(NRF5_SDK)/examples/bsp/bsp.c)

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

#todo: fixme
OUTPUT_FILENAME=libnrf52
default: _build/$(OUTPUT_FILENAME).a

include mk/rules.mk