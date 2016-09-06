#ifndef LUA_LFS_H_
#define LUA_LFS_H_

/*
** {======================================================
** File handles for IO library
** =======================================================
*/

/*
** A file handle is a userdata with metatable 'LUA_FILEHANDLE' and
** initial structure 'luaL_Stream' (it may contain other fields
** after that initial structure).
*/

#if !defined(lua_FileHandle)
#define lua_FileHandle           FILE *
#define LUA_FILEHANDLE           "FILE *"
#endif

#if !defined(LUA_INVALID_FILEHANDLE)
#define LUA_INVALID_FILEHANDLE   NULL
#endif

#if !defined(luaL_Stream)
typedef struct luaL_Stream {
  lua_FileHandle f;  /* stream (NULL for incompletely created streams) */
  lua_CFunction closef;  /* to close stream (NULL for closed streams) */
} luaL_Stream;
#endif

/* }====================================================== */


#if !defined(l_getc)		/* { */

#if defined(LUA_USE_POSIX)
#define l_getc(f)		getc_unlocked(f)
#define l_ungetc(c, f)     ungetc(c, f)
#define l_lockfile(f)		flockfile(f)
#define l_unlockfile(f)		funlockfile(f)
#else
#define l_getc(f)		getc(f)
#define l_ungetc(c, f)     ungetc(c, f)
#define l_lockfile(f)		((void)0)
#define l_unlockfile(f)		((void)0)
#endif

#endif				/* } */


/*
** {======================================================
** l_fseek: configuration for longer offsets
** =======================================================
*/

#if !defined(l_fseek)		/* { */

#if defined(LUA_USE_POSIX)	/* { */

#include <sys/types.h>

#define L_SEEK_SET SEEK_SET
#define L_SEEK_CUR SEEK_CUR
#define L_SEEK_END SEEK_END

#define l_fseek(f,o,w)		fseeko(f,o,w)
#define l_ftell(f)		ftello(f)
#define l_seeknum		off_t

#elif defined(LUA_USE_WINDOWS) && !defined(_CRTIMP_TYPEINFO) \
   && defined(_MSC_VER) && (_MSC_VER >= 1400)	/* }{ */

/* Windows (but not DDK) and Visual C++ 2005 or higher */
#define l_fseek(f,o,w)		_fseeki64(f,o,w)
#define l_ftell(f)		_ftelli64(f)
#define l_seeknum		__int64

#else				/* }{ */

/* ISO C definitions */
#define l_fseek(f,o,w)		fseek(f,o,w)
#define l_ftell(f)		ftell(f)
#define l_seeknum		long

#endif				/* } */

#endif				/* } */

#if !defined(l_freopen)
#define l_freopen(name, mode, file) freopen(name, mode, file)
#endif

#if !defined(l_fopen)
#define l_fopen(name, mode) fopen(name, mode)
#endif

#if !defined(l_fclose)
#define l_fclose(file) fclose(file)
#endif

#if !defined(l_fflush)
#define l_fflush(file) fflush(file)
#endif

#if !defined(l_clearerr)
#define l_clearerr(file) clearerr(file)
#endif

#if !defined(l_fwrite)
#define l_fwrite(p,s,c,f) fwrite(p,s,c,f)
#endif

#if !defined(l_fread)
#define l_fread(p,s,c,f) fread(p,s,c,f)
#endif

#if !defined(l_ferror)
#define l_ferror(file) ferror(file)
#endif

#if !defined(l_feof)
#define l_feof(file) feof(lf->f)
#endif

#endif /* LUA_LFS_H_ */
