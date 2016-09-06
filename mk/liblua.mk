#source common to all targets
C_SOURCE_FILES += $(abspath $(filter-out lua/lua.c, $(wildcard lua/*.c))) \

#includes common to all targets
INC_PATHS += -I$(abspath config) -I$(abspath .) -I$(abspath spiffs/src)

#flags common to all targets
CFLAGS += -DLUACONF_ARCH=\"luaconf_arch.h\" -DLUA_FS_SUPPORT

#todo: fixme
OUTPUT_FILENAME=liblua
default: _build/$(OUTPUT_FILENAME).a

include mk/rules.mk