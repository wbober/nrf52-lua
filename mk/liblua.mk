#source common to all targets
C_SOURCE_FILES += $(abspath $(filter-out lua/liolib.c lua/lua.c, $(wildcard lua/*.c))) \

#includes common to all targets
INC_PATHS += -I$(abspath config) -I$(abspath .) -I$(abspath spiffs/src)

#todo: fixme
OUTPUT_FILENAME=liblua
default: _build/$(OUTPUT_FILENAME).a

include mk/rules.mk