#include <stddef.h>
#include "boards.h"
#include "lrodefs.h"
#include "lua.h"
#include "lauxlib.h"

#define LED_NO_TO_PIN(id) ((1 << (LED_START + (id - 1))))

static int bsp_led_on(struct lua_State *L)
{
	int led = luaL_checkinteger(L, 1);
	LEDS_ON(LED_NO_TO_PIN(led));
	return 0;
}

static int bsp_led_off(struct lua_State *L)
{
	int led = luaL_checkinteger(L, 1);
	LEDS_OFF(LED_NO_TO_PIN(led));
	return 0;
}

static int bsp_led_toggle(struct lua_State *L)
{
	int led = luaL_checkinteger(L, 1);
	LEDS_INVERT(LED_NO_TO_PIN(led));
	return 0;
}

//Lua: on_buton(button id, callback)
static int bsp_on_button(struct lua_State *L)
{
	//TODO
	return 0;
}

static const LUA_REG_TYPE bsp_map[] =
{
	{ LSTRKEY( "led_on" ), LFUNCVAL( bsp_led_on ) },
	{ LSTRKEY( "led_off" ), LFUNCVAL( bsp_led_off ) },
	{ LSTRKEY( "led_toggle" ), LFUNCVAL( bsp_led_toggle ) },
	{ LSTRKEY( "on_button" ), LFUNCVAL( bsp_on_button ) },
	{ LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_bsp( lua_State *L )
{
	LEDS_CONFIGURE(LEDS_MASK);
	luaL_newlib(L, bsp_map);
	// Add constants
	return 1;
}
