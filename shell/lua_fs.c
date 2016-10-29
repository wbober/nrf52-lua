#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "lua.h"

#include "nrf_log.h"
#include "nrf.h"
#include "nrf_soc.h"
#include "softdevice_handler.h"

#include "spiffs_config.h"
#include "spiffs.h"

#define is_aligned(addr) (((uint32_t)addr & 0x03) == 0)

static u8_t spiffs_work_buf[1024];
static u8_t spiffs_fds[32*4];
static spiffs fs;
static u8_t *_cache = NULL;
static u32_t _cache_sz;

static void hexdump(void *p, uint16_t length)
{
    int i;
    int left;

    char *bp = (char *)p;

    while (length > 0) {
        left = length > 16 ? 16 : length;
        for (i = 0; i < left; i++) {
            printf("%02x ", bp[i] & 0xFF);
        }
        for (i = 0; i < 16 - left; i++) {
            printf("   ");
        }
        for (i = 0; i < left; i++) {
            printf("%c", (bp[i] > 32 && bp[i] < 126) ? bp[i] : '.');
        }
        bp += 16;
        length -= left;
        printf("\n");
    }
}


#ifdef SOFTDEVICE_PRESENT
volatile bool flash_op_done;

static void flash_page_erase(uint32_t * page_address)
{
	uint32_t pagenum = (uint32_t)page_address / NRF_FICR->CODEPAGESIZE;

	flash_op_done = false;
	uint32_t err_code = sd_flash_page_erase(pagenum);

	if (err_code == NRF_SUCCESS)
	{
		while (!flash_op_done)
		{
			sd_app_evt_wait();
		}
	}
}

static void flash_word_write(uint32_t * address, uint32_t value)
{
	flash_op_done = false;
	uint32_t err_code = sd_flash_write(address, &value, 1);
	if (err_code == NRF_SUCCESS)
	{
		while (!flash_op_done)
		{
			sd_app_evt_wait();
		}
	}
}

#else
#define NVMC_WAIT_UNTIL_READY() do {} while(NRF_NVMC->READY == NVMC_READY_READY_Busy)

/** @brief Function for erasing a page in flash.
 *
 * @param page_address Address of the first word in the page to be erased.
 */
static void flash_page_erase(uint32_t * page_address)
{
    // Turn on flash erase enable and wait until the NVMC is ready:
    NRF_NVMC->CONFIG = (NVMC_CONFIG_WEN_Een << NVMC_CONFIG_WEN_Pos);
    NVMC_WAIT_UNTIL_READY();

    // Erase page:
    NRF_NVMC->ERASEPAGE = (uint32_t)page_address;
    NVMC_WAIT_UNTIL_READY();

    // Turn off flash erase enable and wait until the NVMC is ready:
    NRF_NVMC->CONFIG = (NVMC_CONFIG_WEN_Ren << NVMC_CONFIG_WEN_Pos);
    NVMC_WAIT_UNTIL_READY();
}


/** @brief Function for filling a page in flash with a value.
 *
 * @param[in] address Address of the first word in the page to be filled.
 * @param[in] value Value to be written to flash.
 */
static void flash_word_write(uint32_t * address, uint32_t value)
{
    // Turn on flash write enable and wait until the NVMC is ready:
    NRF_NVMC->CONFIG = (NVMC_CONFIG_WEN_Wen << NVMC_CONFIG_WEN_Pos);
    NVMC_WAIT_UNTIL_READY();

    *address = value;
    NVMC_WAIT_UNTIL_READY();

    // Turn off flash write enable and wait until the NVMC is ready:
    NRF_NVMC->CONFIG = (NVMC_CONFIG_WEN_Ren << NVMC_CONFIG_WEN_Pos);
    NVMC_WAIT_UNTIL_READY();
}
#endif

static s32_t _read(u32_t addr, u32_t size, u8_t *dst)
{
//	NRF_LOG_DEBUG("_read %x %u %p\r\n", addr, size, dst);
	memcpy(dst, (void *)addr, size);
	return 0;
}

static s32_t _write(u32_t addr, u32_t size, u8_t *data)
{
	uint32_t len;
	uint32_t value;
	uint32_t * aligned_addr;

	NRF_LOG_DEBUG("_write: %x %u %p\r\n", addr, size, data);

	// Check if start address is aligned. If not, then align the address.
	if (!is_aligned(addr))
	{
		len = MIN(size, sizeof(int) - ((uint32_t)addr & 0x03));
		aligned_addr = (uint32_t *)((uint32_t)addr & ~(uint32_t)0x03);
		value = *(uint32_t *)aligned_addr;
		memcpy((uint8_t *)(&value) + (addr - (uint32_t)aligned_addr), data, len);
	}
	else
	{
		len = MIN(size, sizeof(int));
		aligned_addr = (uint32_t *)addr;
		value = *(uint32_t *)aligned_addr;
		memcpy(&value, data, len);
	}

	while (size > 0)
	{
		flash_word_write(aligned_addr, value);

		data += len;
		size -= len;
		aligned_addr += 1;

		len = MIN(size, sizeof(int));

		if (len != sizeof(int))
		{
			value = *(uint32_t *)aligned_addr;
		}

		memcpy(&value, data, len);
	}

    return 0;
}

static s32_t _erase(u32_t addr, u32_t size)
{
	NRF_LOG_DEBUG("_erase: %x %u\r\n", addr, size);
	flash_page_erase((uint32_t *)addr);
    return 0;
}

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
	c.hal_erase_f = _erase;
	c.hal_read_f = _read;
	c.hal_write_f = _write;

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
