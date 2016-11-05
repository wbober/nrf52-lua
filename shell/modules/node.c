// Module for interfacing with system
#include <stddef.h>
#include "lrodefs.h"
#include "lua.h"
#include "lauxlib.h"

#include "ldo.h"
#include "lfunc.h"
#include "lmem.h"
#include "lobject.h"
#include "lopcodes.h"
#include "lstring.h"
#include "lundump.h"

#include "nrf_soc.h"

static int node_temp(lua_State *L)
{
	lua_Number result;
	int32_t temp = 25;
	sd_temp_get(&temp);
	result = temp/4;
	lua_pushinteger(L, result);
	return 1;
}

#if 0
static int node_restart(lua_State* L)
{
	return 0;
}

// Lua: sleep(seconds)
static int node_sleep(lua_State *L)
{
	int duration = luaL_checkinteger(L, 1);
	return 0;
}

// Lua: dsleep(seconds)
static int node_deep_sleep(lua_State *L)
{
	int duration = luaL_checkinteger(L, 1);
	return 0;
}

static int node_chipid(lua_State* L)
{
  return 0;
}

// Lua: heap()
static int node_heap(lua_State* L)
{
	uint32_t sz = 0;
	lua_pushinteger(L, sz);
	return 1;
}

static lua_State *gL = NULL;

extern lua_Load gLoad;
extern os_timer_t lua_timer;
extern void dojob(lua_Load *load);
// Lua: input("string")
static int node_input( lua_State* L )
{
  size_t l = 0;
  const char *s = luaL_checklstring(L, 1, &l);
  if (s != NULL && l > 0 && l < LUA_MAXINPUT - 1)
  {
    lua_Load *load = &gLoad;
    if (load->line_position == 0) {
      c_memcpy(load->line, s, l);
      load->line[l + 1] = '\0';
      load->line_position = c_strlen(load->line) + 1;
      load->done = 1;
      NODE_DBG("Get command:\n");
      NODE_DBG(load->line); // buggy here
      NODE_DBG("\nResult(if any):\n");
      os_timer_disarm(&lua_timer);
      os_timer_setfn(&lua_timer, (os_timer_func_t *)dojob, load);
      os_timer_arm(&lua_timer, READLINE_INTERVAL, 0);   // no repeat
    }
  }
  return 0;
}

static int output_redir_ref = LUA_NOREF;
static int serial_debug = 1;
void output_redirect(const char *str) {
  // if(c_strlen(str)>=TX_BUFF_SIZE){
  //   NODE_ERR("output too long.\n");
  //   return;
  // }

  if (output_redir_ref == LUA_NOREF || !gL) {
    uart0_sendStr(str);
    return;
  }

  if (serial_debug != 0) {
    uart0_sendStr(str);
  }

  lua_rawgeti(gL, LUA_REGISTRYINDEX, output_redir_ref);
  lua_pushstring(gL, str);
  lua_call(gL, 1, 0);   // this call back function should never user output.
}

// Lua: output(function(c), debug)
static int node_output( lua_State* L )
{
  gL = L;
  // luaL_checkanyfunction(L, 1);
  if (lua_type(L, 1) == LUA_TFUNCTION || lua_type(L, 1) == LUA_TLIGHTFUNCTION) {
    lua_pushvalue(L, 1);  // copy argument (func) to the top of stack
    if (output_redir_ref != LUA_NOREF)
      luaL_unref(L, LUA_REGISTRYINDEX, output_redir_ref);
    output_redir_ref = luaL_ref(L, LUA_REGISTRYINDEX);
  } else {    // unref the key press function
    if (output_redir_ref != LUA_NOREF)
      luaL_unref(L, LUA_REGISTRYINDEX, output_redir_ref);
    output_redir_ref = LUA_NOREF;
    serial_debug = 1;
    return 0;
  }

  if ( lua_isnumber(L, 2) )
  {
    serial_debug = lua_tointeger(L, 2);
    if (serial_debug != 0)
      serial_debug = 1;
  } else {
    serial_debug = 1; // default to 1
  }

  return 0;
}

static int writer(lua_State* L, const void* p, size_t size, void* u)
{
  UNUSED(L);
  int file_fd = *( (int *)u );
  if ((FS_OPEN_OK - 1) == file_fd)
    return 1;
  NODE_DBG("get fd:%d,size:%d\n", file_fd, size);

  if (size != 0 && (size != fs_write(file_fd, (const char *)p, size)) )
    return 1;
  NODE_DBG("write fd:%d,size:%d\n", file_fd, size);
  return 0;
}

#define toproto(L,i) (clvalue(L->top+(i))->l.p)
// Lua: compile(filename) -- compile lua file into lua bytecode, and save to .lc
static int node_compile( lua_State* L )
{
  Proto* f;
  int file_fd = FS_OPEN_OK - 1;
  size_t len;
  const char *fname = luaL_checklstring( L, 1, &len );
  if ( len > FS_NAME_MAX_LENGTH )
    return luaL_error(L, "filename too long");

  char output[FS_NAME_MAX_LENGTH];
  c_strcpy(output, fname);
  // check here that filename end with ".lua".
  if (len < 4 || (c_strcmp( output + len - 4, ".lua") != 0) )
    return luaL_error(L, "not a .lua file");

  output[c_strlen(output) - 2] = 'c';
  output[c_strlen(output) - 1] = '\0';
  NODE_DBG(output);
  NODE_DBG("\n");
  if (luaL_loadfsfile(L, fname) != 0) {
    return luaL_error(L, lua_tostring(L, -1));
  }

  f = toproto(L, -1);

  int stripping = 1;      /* strip debug information? */

  file_fd = fs_open(output, fs_mode2flag("w+"));
  if (file_fd < FS_OPEN_OK)
  {
    return luaL_error(L, "cannot open/write to file");
  }

  lua_lock(L);
  int result = luaU_dump(L, f, writer, &file_fd, stripping);
  lua_unlock(L);

  fs_flush(file_fd);
  fs_close(file_fd);
  file_fd = FS_OPEN_OK - 1;

  if (result == LUA_ERR_CC_INTOVERFLOW) {
    return luaL_error(L, "value too big or small for target integer type");
  }
  if (result == LUA_ERR_CC_NOTINTEGER) {
    return luaL_error(L, "target lua_Number is integral but fractional value found");
  }

  return 0;
}
#endif

const LUA_REG_TYPE node_map[] =
{
  { LSTRKEY( "temp" ), LFUNCVAL( node_temp ) },
#if 0
  { LSTRKEY( "chipid" ), LFUNCVAL( node_chipid ) },
  { LSTRKEY( "restart" ), LFUNCVAL( node_restart ) },
  { LSTRKEY( "sleep" ), LFUNCVAL( node_sleep ) },
  { LSTRKEY( "dsleep" ), LFUNCVAL( node_deep_sleep ) },
  { LSTRKEY( "power_down" ), LFUNCVAL( node_power_down ) },
  { LSTRKEY( "heap" ), LFUNCVAL( node_heap ) },
  { LSTRKEY( "input" ), LFUNCVAL( node_input ) },
  { LSTRKEY( "output" ), LFUNCVAL( node_output ) },
  { LSTRKEY( "compile" ), LFUNCVAL( node_compile) },
  { LSTRKEY( "dsleepsetoption" ), LFUNCVAL( node_deepsleep_setoption) },
#endif
  { LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_node( lua_State *L )
{
#if LUA_OPTIMIZE_MEMORY > 0
  return 0;
#else // #if LUA_OPTIMIZE_MEMORY > 0
  luaL_newlib(L, node_map);
  // Add constants
  return 1;
#endif // #if LUA_OPTIMIZE_MEMORY > 0
}
