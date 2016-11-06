#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "lua.h"
#include "spiffs_config.h"
#include "spiffs.h"
#include "nrf_log.h"

#define is_aligned(addr) (((uint32_t)addr & 0x03) == 0)

static u8_t spiffs_work_buf[1024];
static u8_t spiffs_fds[32*4];
static spiffs fs;
static u8_t *_cache = NULL;
static u32_t _cache_sz;

lua_FileHandle _f_open(const char *fname, const char *mode)
{
	spiffs_flags flags;

	NRF_LOG_DEBUG("_f_open: %s %s\r\n", fname, mode);

	if (strcmp(mode, "r") == 0) {
		flags = SPIFFS_O_RDONLY;
	} else if (strcmp(mode, "r+") == 0) {
		flags = SPIFFS_O_RDWR;
	} else if (strcmp(mode, "w") == 0) {
		flags = SPIFFS_O_WRONLY | SPIFFS_O_TRUNC | SPIFFS_O_CREAT;
	} else if (strcmp(mode, "w+") == 0) {
		flags = SPIFFS_O_RDWR | SPIFFS_O_CREAT;
	} else if (strcmp(mode, "a") == 0) {
		flags = SPIFFS_O_RDWR | SPIFFS_O_APPEND;
	} else {
		return LUA_INVALID_FILEHANDLE;
	}

	return SPIFFS_open(&fs, fname, flags, 0);
}

/**
 * @return 0 on success, 1 otherwise
 */
int _f_close(lua_FileHandle file)
{
	return SPIFFS_close(&fs, file);
}

int _f_write(const void *ptr, size_t size, size_t count, lua_FileHandle file)
{
	return SPIFFS_write(&fs, file, (void *)ptr, size*count);
}

/**
 * @return number of bytes read
 */
int _f_read(void *ptr, size_t size, size_t count, lua_FileHandle file)
{
	return SPIFFS_read(&fs, file, ptr, size*count);
}

l_seeknum _f_seek(lua_FileHandle file, int ofs, int whence)
{
	return SPIFFS_lseek(&fs, file, ofs, whence);
}

int _f_getc(lua_FileHandle file)
{
	int c;
	if (SPIFFS_read(&fs, file, &c, 1) != 1)
		return EOF;
	else
		return c;
}

int _f_ungetc(char c, lua_FileHandle file)
{
	SPIFFS_lseek(&fs, file, -1, SPIFFS_SEEK_CUR);
	return c;
}

int _f_flush(lua_FileHandle file)
{
	return SPIFFS_fflush(&fs, file);
}

l_seeknum _f_tell(lua_FileHandle file)
{
	return (l_seeknum)SPIFFS_tell(&fs, file);
}

lua_FileHandle _f_reopen(const char *fname, const char *mode, lua_FileHandle file)
{
	_f_close(file);
	return _f_open(fname, mode);
}

int _f_eof(lua_FileHandle file)
{
	return SPIFFS_eof(&fs, file);
}

void lua_fs_init(void)
{
	s32_t ret;
	spiffs_config c;
	c.hal_erase_f = spiffs_arch_erase;
	c.hal_read_f = spiffs_arch_read;
	c.hal_write_f = spiffs_arch_write;

	ret = SPIFFS_mount(&fs, &c, spiffs_work_buf, spiffs_fds, sizeof(spiffs_fds), _cache, _cache_sz, NULL);
	NRF_LOG_DEBUG("spiffs mount: %d\r\n", ret);

	if (ret == SPIFFS_ERR_NOT_A_FS) {
		SPIFFS_unmount(&fs);

		ret = SPIFFS_format(&fs);
		NRF_LOG_DEBUG("spiffs format: %d\r\n", ret);

		ret = SPIFFS_mount(&fs, &c, spiffs_work_buf, spiffs_fds, sizeof(spiffs_fds), _cache, _cache_sz, NULL);
		NRF_LOG_DEBUG("spiffs mount: %d\r\n", ret);
	}
}
