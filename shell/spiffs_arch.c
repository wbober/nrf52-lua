#include "nrf_log.h"
#include "nrf.h"
#include "nrf_soc.h"
#include "softdevice_handler.h"
#include "spiffs_config.h"

#define is_aligned(addr) (((uint32_t)addr & 0x03) == 0)

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

s32_t spiffs_arch_read(u32_t addr, u32_t size, u8_t *dst)
{
//	NRF_LOG_DEBUG("_read %x %u %p\r\n", addr, size, dst);
	memcpy(dst, (void *)addr, size);
	return 0;
}

s32_t spiffs_arch_write(u32_t addr, u32_t size, u8_t *data)
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

s32_t spiffs_arch_erase(u32_t addr, u32_t size)
{
	NRF_LOG_DEBUG("_erase: %x %u\r\n", addr, size);
	flash_page_erase((uint32_t *)addr);
    return 0;
}
