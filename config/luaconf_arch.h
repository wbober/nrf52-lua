#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define xstr(s) str(s)
#define str(s) #s

#define LUA_ARCH_VERSION "nRF52"

#ifdef LUA_FS_SUPPORT
#include "spiffs.h"

#define l_seeknum int

#define L_SEEK_CUR SPIFFS_SEEK_CUR
#define L_SEEK_SET SPIFFS_SEEK_SET
#define L_SEEK_END SPIFFS_SEEK_END

#define lua_FileHandle          spiffs_file
#define LUA_FILEHANDLE          xstr(lua_FileHandle)
#define LUA_INVALID_FILEHANDLE  -1

lua_FileHandle _f_open(const char *fname, const char *mode);
int _f_close(lua_FileHandle file);
int _f_write(const void *ptr, size_t size, size_t count, lua_FileHandle file);
int _f_read(void *ptr, size_t size, size_t count, lua_FileHandle file);
l_seeknum _f_seek(lua_FileHandle file, int ofs, int whence);
int _f_getc(lua_FileHandle file);
int _f_ungetc(char c, lua_FileHandle file);
int _f_flush(lua_FileHandle file);
l_seeknum _f_tell(lua_FileHandle file);
lua_FileHandle _f_reopen(const char *fname, const char *mode, lua_FileHandle file);
int _f_eof(lua_FileHandle file);

#define l_freopen(name, mode, file) _f_reopen(name, mode, file)
#define l_fopen(name, mode) _f_open(name, mode)
#define l_fclose(f) _f_close(f)
#define l_getc(f) _f_getc(f)
#define l_ungetc(c, f) _f_ungetc(c, f)
#define l_fseek(f,o,w) _f_seek(f,o,w)
#define l_ftell(f) _f_tell(f)
#define l_fwrite(p,s,c,f) _f_write(p,s,c,f)
#define l_fread(p,s,c,f) _f_read(p,s,c,f)
#define l_fflush(f) _f_flush(f)
#define l_clearerr(f) do {} while(0)
#define l_ferror(f) (0)
#define l_lockfile(f) ((void)0)
#define l_unlockfile(f) ((void)0)
#define l_feof(f) _f_eof(f)
#endif

/* print a string */
#define lua_writestring(s,l)      lua_arch_write(s, l)

/* print a newline and flush the output */
#define lua_writeline()           lua_arch_write("\n", 1)

/* print an error message */
#define lua_writestringerror(s,p) do {} while(0)

#define lua_readline(L,b,s,p) lua_arch_readline(L,b,s,p)
#define lua_saveline(L,idx)	{ (void)L; (void)idx; }
#define lua_freeline(L,b)	{ (void)L; (void)b; }

// FIXME: get proper seed
#define luai_makeseed()  cast(unsigned int, 1)

#define lua_lock(L)      ((void) 0)
#define lua_unlock(L)    ((void) 0)

struct lua_State;

extern struct lua_State * lua_getstate(void);
extern int lua_arch_readline(struct lua_State *L, char *b, size_t len, const char *p);
extern int lua_arch_write(const char *s, size_t len);
extern void *lua_arch_alloc(void *ud, void *ptr, size_t osize, size_t nsize);
extern void lua_arch_openlibs(struct lua_State *L);
extern void lua_arch_init(void);
