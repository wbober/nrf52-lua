#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "auxmods.h"

#include "umm_malloc.h"

#include "app_uart.h"
#include "app_error.h"
#include "nrf_delay.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf.h"

#ifdef LUA_FS_SUPPORT
void lua_fs_init(void);
#endif

void *lua_arch_alloc(void *ud, void *ptr, size_t osize, size_t nsize)
{
	(void) ud;
	void *ret = NULL;

	if (ptr && nsize == 0) {
		umm_free(ptr);
	} else if (nsize > 0) {
		if (ptr) {
			ret = umm_realloc(ptr, nsize);
		} else {
			ret = umm_malloc(nsize);
		}
	}

	return ret;
}

int lua_arch_write(const char *s, size_t len)
{
	while (len-- > 0) {
		app_uart_put(*s++);
	}

	return 0;
}

int lua_arch_readline(struct lua_State *L, char *b, size_t len, const char *p)
{
	char c;
	char *bp = b;

	lua_arch_write(p, strlen(p));

	while (1) {
		// read one character
		while (app_uart_get((unsigned char *)&c) != NRF_SUCCESS) {
//			sd_app_evt_wait();
		}

		// handle backspace
		if ((c == 0x7f || c == 0x08) && (bp > b)) {
			lua_arch_write("\x08\x20\x08", 3);
			bp -= 1;
			continue;
		}

		// skip '\r' character
		if (c == '\r')
			continue;


		if ((bp - b) >= len - 1) {
			continue;
		}

		// copy and echo character
		*bp++ = c;
		lua_arch_write(&c, 1);

		// exit if new line character received
		if (c == '\n') {
			*bp = 0;
			return 1;
		}
	}
}

void lua_arch_init(void)
{
	NRF_LOG_INIT(NULL);
	umm_init();
	NRF_LOG_DEBUG("lua_arch_init done\r\n");
}

#include "vfs.h"

void lua_arch_openlibs(struct lua_State *L)
{
	luaL_requiref(L, "bsp", luaopen_bsp, 1);
#ifdef SOFTDEVICE_PRESENT
	luaL_requiref(L, "ble", luaopen_ble, 1);
#endif
#ifdef LUA_FS_SUPPORT
	lua_fs_init();
#endif

    if (!vfs_mount("/FLASH", 0)) {
        // Failed to mount -- try reformat
    	NRF_LOG_DEBUG("Formatting file system. Please wait...\n");
        if (!vfs_format()) {
        	NRF_LOG_ERROR( "\n*** ERROR ***: unable to format. FS might be compromised.\n" );
        	NRF_LOG_ERROR( "It is advised to re-flash the NodeMCU image.\n" );
        }
        // Note that fs_format leaves the file system mounted
    }

	luaL_requiref(L, "node", luaopen_node, 1);
	luaL_requiref(L, "file", luaopen_file, 1);

#if 0
	luaL_requiref(L, "tmr", luaopen_tmr, 1);
	luaL_requiref(L, "gpio", luaopen_gpio, 1);
	luaL_requiref(L, "adc", luaopen_adc, 1);
	luaL_requiref(L, "pwm", luaopen_pwm, 1);
	luaL_requiref(L, "uart", luaopen_uart, 1);
	luaL_requiref(L, "cfg", luaopen_cfg, 1);
	luaL_requiref(L, "log", luaopen_log, 1);
#endif
}
