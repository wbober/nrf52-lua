#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

// #define FLASH_512K
// #define FLASH_1M
// #define FLASH_2M
// #define FLASH_4M
// #define FLASH_8M
// #define FLASH_16M
#define FLASH_AUTOSIZE
#define FLASH_SAFE_API

// This adds the asserts in LUA. It also adds some useful extras to the
// node module. This is all silent in normal operation and so can be enabled
// without any harm (except for the code size increase and slight slowdown)
//#define DEVELOPMENT_TOOLS

#ifdef DEVELOPMENT_TOOLS
extern void luaL_assertfail(const char *file, int line, const char *message);
#define lua_assert(x)    ((x) ? (void) 0 : luaL_assertfail(__FILE__, __LINE__, #x))
#endif

// This enables lots of debug output and changes the serial bit rate. This
// is normally only used by hardcore developers
// #define DEVELOP_VERSION
#ifdef DEVELOP_VERSION
#define NODE_DEBUG
#define COAP_DEBUG
#endif /* DEVELOP_VERSION */

#define BIT_RATE_DEFAULT BIT_RATE_115200

// This enables automatic baud rate detection at startup
#define BIT_RATE_AUTOBAUD

#define NODE_ERROR

#define GPIO_INTERRUPT_ENABLE
#define GPIO_INTERRUPT_HOOK_ENABLE
// #define GPIO_SAFE_NO_INTR_ENABLE

#define ICACHE_STORE_TYPEDEF_ATTR __attribute__((aligned(4),packed))
#define ICACHE_STORE_ATTR __attribute__((aligned(4)))
#define ICACHE_RAM_ATTR __attribute__((section(".iram0.text")))
#ifdef  GPIO_SAFE_NO_INTR_ENABLE
#define NO_INTR_CODE ICACHE_RAM_ATTR __attribute__ ((noinline))
#else
#define NO_INTR_CODE inline
#endif

// SSL buffer size used only for espconn-layer secure connections.
// See https://github.com/nodemcu/nodemcu-firmware/issues/1457 for conversation details.
#define SSL_BUFFER_SIZE 5120

//#define CLIENT_SSL_ENABLE
//#define MD2_ENABLE
#define SHA2_ENABLE

#define BUILD_SPIFFS
#define SPIFFS_CACHE 1

//#define BUILD_FATFS

// maximum length of a filename
#define FS_OBJ_NAME_LEN 31

#define READLINE_INTERVAL 80
#define LUA_TASK_PRIO USER_TASK_PRIO_0
#define LUA_PROCESS_LINE_SIG 2
#define LUA_OPTIMIZE_DEBUG      2

#define STRBUF_DEFAULT_INCREMENT 32

#endif	/* __USER_CONFIG_H__ */
