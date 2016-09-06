// Module for interfacing with timer

#include <stdbool.h>
#include "lualib.h"
#include "lauxlib.h"
#include "lrodefs.h"

struct alarm {
	int ref;
	bool repeat;
	int interval;
};

static struct alarm alarm[EAT_TIMER_NUM] = {{EAT_TIMER_NUM, 0, 0}};

void alarm_timer_cb(lua_State* L, unsigned int id)
{
	if (alarm[id].ref == LUA_NOREF)
		return;

	lua_rawgeti(L, LUA_REGISTRYINDEX, alarm[id].ref);

	if (alarm[id].repeat == false) {
		luaL_unref(L, LUA_REGISTRYINDEX, alarm[id].ref);
	} else {
		eat_timer_start((EatTimer_enum)id, alarm[id].interval);
	}
	lua_call(L, 0, 0);
}

// Lua: delay(ms)
static int tmr_delay(lua_State* L)
{
	unsigned int interval;
	interval = luaL_checkinteger(L, 1);
	if (interval <= 0)
		return luaL_error(L, "wrong arg range");

	eat_sleep(interval);
	return 0;
}

// Lua: now() , return system timer in us
static int tmr_now(lua_State* L)
{
	unsigned now = 0x7FFFFFFF & eat_get_current_time();
	lua_pushinteger(L, now);
	return 1;
}

// Lua: alarm( id, interval, repeat, function )
static int tmr_alarm(lua_State* L)
{
	s32 interval;
	unsigned repeat = 0;
	int stack = 1;

	unsigned id = luaL_checkinteger(L, stack);
	stack++;

	interval = luaL_checkinteger(L, stack);
	stack++;

	if (interval <= 0)
		return luaL_error(L, "wrong arg range");

	alarm[id].interval = interval;

	if (lua_isnumber(L, stack)) {
		repeat = lua_tointeger(L, stack);
		stack++;
		if (repeat != 1 && repeat != 0)
			return luaL_error(L, "wrong arg type");
		alarm[id].repeat = repeat;
	}

	// luaL_checkanyfunction(L, stack);
	if (lua_type(L, stack) == LUA_TFUNCTION) {
		lua_pushvalue(L, stack);  // copy argument (func) to the top of stack
		if (alarm[id].ref != LUA_NOREF)
			luaL_unref(L, LUA_REGISTRYINDEX, alarm[id].ref);
		alarm[id].ref = luaL_ref(L, LUA_REGISTRYINDEX);
	}

	eat_timer_start((EatTimer_enum)id, interval);
	return 0;
}

// Lua: stop(id)
static int tmr_stop(lua_State* L)
{
	unsigned id = luaL_checkinteger(L, 1);

	eat_timer_stop((EatTimer_enum)id);

	if (alarm[id].ref != LUA_NOREF) {
		luaL_unref(L, LUA_REGISTRYINDEX, alarm[id].ref);
		alarm[id].ref = LUA_NOREF;
	}

	return 0;
}

#if 0
// Lua: wdclr()
static int tmr_wdclr( lua_State* L )
{
//  WRITE_PERI_REG(0x60000914, 0x73);
 // update_key_led();
  return 0;
}
#endif

// Lua: time()
// return rtc time in second, nsec
static int tmr_time( lua_State* L )
{
	struct timespec tp;
	clock_gettime(CLOCK_REALTIME, &tp);
	lua_pushinteger(L, tp.tv_sec);
	lua_pushinteger(L, tp.tv_nsec);
	return 2;
}

// Module function map
#define MIN_OPT_LEVEL 2
#include "lrodefs.h"
const LUA_REG_TYPE tmr_map[] =
{
  { LSTRKEY( "delay" ), LFUNCVAL( tmr_delay ) },
  { LSTRKEY( "now" ), LFUNCVAL( tmr_now ) },
  { LSTRKEY( "alarm" ), LFUNCVAL( tmr_alarm ) },
  { LSTRKEY( "stop" ), LFUNCVAL( tmr_stop ) },
//  { LSTRKEY( "wdclr" ), LFUNCVAL( tmr_wdclr ) },
  { LSTRKEY( "time" ), LFUNCVAL( tmr_time ) },
#if LUA_OPTIMIZE_MEMORY > 0
#endif
  { LNILKEY, LNILVAL }
};


LUALIB_API int luaopen_tmr( lua_State *L )
{
  luaL_newlib(L, tmr_map);
  return 1;
}
