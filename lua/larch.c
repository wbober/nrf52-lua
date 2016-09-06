#include "lua.h"
#include "luaconf_arch.h"

#if 0
lua_FileHandle _f_open(const char *fname, const char *mode)
{
	UINT flags;
	WCHAR u_fname[strlen(fname) + 1];

	s2w(fname, u_fname, sizeof(u_fname));

	bool append = false;

	if (strcmp(mode, "r") == 0) {
		flags = FS_READ_ONLY;
	} else if (strcmp(mode, "r+") == 0) {
		flags = FS_READ_WRITE;
	} else if (strcmp(mode, "w") == 0) {
		flags = FS_READ_WRITE | FS_CREATE;
	} else if (strcmp(mode, "w+") == 0) {
		flags = FS_READ_WRITE | FS_CREATE_ALWAYS;
	} else if (strcmp(mode, "a") == 0) {
		flags = FS_READ_WRITE | FS_CREATE;
		append = true;
	} else {
		return LUA_INVALID_FILEHANDLE;
	}

	FS_HANDLE ret = eat_fs_Open((u16*)u_fname, flags);

	if (ret >= 0) {
		if (append) {
			eat_fs_Seek(ret, EAT_FS_FILE_END, 0);
		}
		return ret;
	} else {
		return LUA_INVALID_FILEHANDLE;
	}
}

/**
 * @return 0 on success, 1 otherwise
 */
int _f_close(lua_FileHandle file)
{
	return (eat_fs_Close(file) == EAT_FS_NO_ERROR) ? 0 : 1;
}

int _f_write(const void *ptr, size_t size, size_t count, lua_FileHandle file)
{
	UINT written;
	eat_fs_Write(file, (void *)ptr, size*count, &written);
	return written;
}

/**
 * @return number of bytes read
 */
int _f_read(void *ptr, size_t size, size_t count, lua_FileHandle file)
{
	UINT read = 0;
	eat_fs_Read(file, ptr, size*count, &read);
	return read;
}

l_seeknum _f_seek(lua_FileHandle file, int ofs, int whence)
{
	return eat_fs_Seek(file, ofs, whence);
}

int _f_getc(lua_FileHandle file)
{
	UINT read = 0;
	int c;
	if (eat_fs_Read(file, &c, 1, &read) == EAT_FS_NO_ERROR && read < 1)
		return EOF;
	else
		return c;
}

int _f_ungetc(char c, lua_FileHandle file)
{
	eat_fs_Seek(file, -1, EAT_FS_FILE_CURRENT);
	return c;
}

int _f_flush(lua_FileHandle file)
{
	return !(eat_fs_Commit(file) == EAT_FS_NO_ERROR);
}

l_seeknum _f_tell(lua_FileHandle file)
{
	UINT pos;
	eat_fs_GetFilePosition(file, &pos);
	return (l_seeknum)pos;
}

lua_FileHandle _f_reopen(const char *fname, const char *mode, lua_FileHandle file)
{
	_f_close(file);
	return _f_open(fname, mode);
}

int _f_eof(lua_FileHandle file)
{
	UINT size;
	UINT pos;
	eat_fs_GetFilePosition(file, &pos);
	eat_fs_GetFileSize(file, &size);
	return (size == pos);
}
#endif

int _readline(struct lua_State *L, char *b, size_t len, const char *p)
{
#if 0
	unsigned char c;
	char *bp = b;

	eat_uart_write(LUA_UART, (const unsigned char *)p, strlen(p));

	while (1) {
		// read one character
		while (eat_uart_read(LUA_UART, &c, 1) == 0) {
			process_events(L);
		}

		// handle backspace
		if ((c == 0x7f || c == 0x08) && (bp > b)) {
			eat_uart_write(LUA_UART, "\x08\x20\x08", 3);
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
		eat_uart_write(LUA_UART, &c, 1);

		// exit if new line character received
		if (c == '\n') {
			*bp = 0;
			return 1;
		}
	}
#endif
	return 0;
}
