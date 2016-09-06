#source common to all targets
C_SOURCE_FILES += $(abspath $(wildcard spiffs/src/*.c))

#includes common to all targets
INC_PATHS = -I$(abspath spiffs) -I$(abspath config) \
            -I$(NRF5_SDK)/components/libraries/util \
            -I$(NRF5_SDK)/components/device
            
#todo: fixme
OUTPUT_FILENAME=libspiffs
default: _build/$(OUTPUT_FILENAME).a

include mk/rules.mk