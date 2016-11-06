#include "compat.h"
#include "platform.h"
#include "spiffs_config.h"
#include "spiffs.h"

static spiffs fs;
static u8_t spiffs_work_buf[1024];
static u8_t spiffs_fds[32*4];
static u8_t *_cache = NULL;
static u32_t _cache_sz;

bool myspiffs_mount(void) {
	s32_t ret;
	spiffs_config c;
	c.hal_erase_f = spiffs_arch_erase;
	c.hal_read_f = spiffs_arch_read;
	c.hal_write_f = spiffs_arch_write;

	ret = SPIFFS_mount(&fs, &c, spiffs_work_buf, spiffs_fds, sizeof(spiffs_fds), _cache, _cache_sz, NULL);
	NRF_LOG_DEBUG("spiffs mount: %d\r\n", ret);

	return (ret == SPIFFS_OK);
}

void myspiffs_unmount(void) {
  SPIFFS_unmount(&fs);
}

// FS formatting function
// Returns 1 if OK, 0 for error
int myspiffs_format( void )
{
	s32_t ret;

	SPIFFS_unmount(&fs);

	ret = SPIFFS_format(&fs);
	NRF_LOG_DEBUG("spiffs format: %d\r\n", ret);

  return myspiffs_mount();
}

// ***************************************************************************
// vfs API
// ***************************************************************************

#include <stdlib.h>
#include "vfs_int.h"

#define MY_LDRV_ID "FLASH"

// default current drive
static int is_current_drive = TRUE;

// forward declarations
static sint32_t myspiffs_vfs_close( const struct vfs_file *fd );
static sint32_t myspiffs_vfs_read( const struct vfs_file *fd, void *ptr, size_t len );
static sint32_t myspiffs_vfs_write( const struct vfs_file *fd, const void *ptr, size_t len );
static sint32_t myspiffs_vfs_lseek( const struct vfs_file *fd, sint32_t off, int whence );
static sint32_t myspiffs_vfs_eof( const struct vfs_file *fd );
static sint32_t myspiffs_vfs_tell( const struct vfs_file *fd );
static sint32_t myspiffs_vfs_flush( const struct vfs_file *fd );
static sint32_t myspiffs_vfs_ferrno( const struct vfs_file *fd );

static sint32_t  myspiffs_vfs_closedir( const struct vfs_dir *dd );
static vfs_item *myspiffs_vfs_readdir( const struct vfs_dir *dd );

static void       myspiffs_vfs_iclose( const struct vfs_item *di );
static uint32_t   myspiffs_vfs_isize( const struct vfs_item *di );
//static const struct tm *myspiffs_vfs_time( const struct vfs_item *di );
static const char *myspiffs_vfs_name( const struct vfs_item *di );

static vfs_vol  *myspiffs_vfs_mount( const char *name, int num );
static vfs_file *myspiffs_vfs_open( const char *name, const char *mode );
static vfs_dir  *myspiffs_vfs_opendir( const char *name );
static vfs_item *myspiffs_vfs_stat( const char *name );
static sint32_t  myspiffs_vfs_remove( const char *name );
static sint32_t  myspiffs_vfs_rename( const char *oldname, const char *newname );
static sint32_t  myspiffs_vfs_fsinfo( uint32_t *total, uint32_t *used );
static sint32_t  myspiffs_vfs_fscfg( uint32_t *phys_addr, uint32_t *phys_size );
static sint32_t  myspiffs_vfs_format( void );
static sint32_t  myspiffs_vfs_errno( void );
static void      myspiffs_vfs_clearerr( void );

static sint32_t myspiffs_vfs_umount( const struct vfs_vol *vol );

// ---------------------------------------------------------------------------
// function tables
//
static vfs_fs_fns myspiffs_fs_fns = {
  .mount    = myspiffs_vfs_mount,
  .open     = myspiffs_vfs_open,
  .opendir  = myspiffs_vfs_opendir,
  .stat     = myspiffs_vfs_stat,
  .remove   = myspiffs_vfs_remove,
  .rename   = myspiffs_vfs_rename,
  .mkdir    = NULL,
  .fsinfo   = myspiffs_vfs_fsinfo,
  .fscfg    = myspiffs_vfs_fscfg,
  .format   = myspiffs_vfs_format,
  .chdrive  = NULL,
  .chdir    = NULL,
  .ferrno   = myspiffs_vfs_errno,
  .clearerr = myspiffs_vfs_clearerr
};

static vfs_file_fns myspiffs_file_fns = {
  .close     = myspiffs_vfs_close,
  .read      = myspiffs_vfs_read,
  .write     = myspiffs_vfs_write,
  .lseek     = myspiffs_vfs_lseek,
  .eof       = myspiffs_vfs_eof,
  .tell      = myspiffs_vfs_tell,
  .flush     = myspiffs_vfs_flush,
  .size      = NULL,
  .ferrno    = myspiffs_vfs_ferrno
};

static vfs_item_fns myspiffs_item_fns = {
  .close     = myspiffs_vfs_iclose,
  .size      = myspiffs_vfs_isize,
  .time      = NULL,
  .name      = myspiffs_vfs_name,
  .is_dir    = NULL,
  .is_rdonly = NULL,
  .is_hidden = NULL,
  .is_sys    = NULL,
  .is_arch   = NULL
};

static vfs_dir_fns myspiffs_dd_fns = {
  .close     = myspiffs_vfs_closedir,
  .readdir   = myspiffs_vfs_readdir
};


// ---------------------------------------------------------------------------
// specific struct extensions
//
struct myvfs_file {
  struct vfs_file vfs_file;
  spiffs_file fh;
};

struct myvfs_dir {
  struct vfs_dir vfs_dir;
  spiffs_DIR d;
};

struct myvfs_stat {
  struct vfs_item vfs_item;
  spiffs_stat s;
};


// ---------------------------------------------------------------------------
// stat functions
//
#define GET_STAT_S(descr) \
  const struct myvfs_stat *mystat = (const struct myvfs_stat *)descr; \
  spiffs_stat *s = (spiffs_stat *)&(mystat->s);

static void myspiffs_vfs_iclose( const struct vfs_item *di ) {
  // free descriptor memory
  c_free( (void *)di );
}

static uint32_t myspiffs_vfs_isize( const struct vfs_item *di ) {
  GET_STAT_S(di);

  return s->size;
}

static const char *myspiffs_vfs_name( const struct vfs_item *di ) {
  GET_STAT_S(di);

  return s->name;
}

// ---------------------------------------------------------------------------
// dir functions
//
#define GET_DIR_D(descr) \
  const struct myvfs_dir *mydd = (const struct myvfs_dir *)descr; \
  spiffs_DIR *d = (spiffs_DIR *)&(mydd->d);

static sint32_t myspiffs_vfs_closedir( const struct vfs_dir *dd ) {
  GET_DIR_D(dd);

  sint32_t res = SPIFFS_closedir( d );

  // free descriptor memory
  c_free( (void *)dd );

  return res;
}

static vfs_item *myspiffs_vfs_readdir( const struct vfs_dir *dd ) {
  GET_DIR_D(dd);
  struct myvfs_stat *stat;
  struct spiffs_dirent dirent;

  if ((stat = c_malloc( sizeof( struct myvfs_stat ) ))) {
    if (SPIFFS_readdir( d, &dirent )) {
      stat->vfs_item.fs_type = VFS_FS_FATFS;
      stat->vfs_item.fns     = &myspiffs_item_fns;
      // copy entries to vfs' directory item
      stat->s.size = dirent.size;
      c_strncpy( (char *)stat->s.name, (char *)dirent.name, SPIFFS_OBJ_NAME_LEN );
      return (vfs_item *)stat;
    } else {
      c_free( stat );
    }
  }

  return NULL;
}


// ---------------------------------------------------------------------------
// file functions
//
#define GET_FILE_FH(descr) \
  const struct myvfs_file *myfd = (const struct myvfs_file *)descr; \
  spiffs_file fh = myfd->fh;

static sint32_t myspiffs_vfs_close( const struct vfs_file *fd ) {
  GET_FILE_FH(fd);

  sint32_t res = SPIFFS_close( &fs, fh );

  // free descriptor memory
  c_free( (void *)fd );

  return res;
}

static sint32_t myspiffs_vfs_read( const struct vfs_file *fd, void *ptr, size_t len ) {
  GET_FILE_FH(fd);

  return SPIFFS_read( &fs, fh, ptr, len );
}

static sint32_t myspiffs_vfs_write( const struct vfs_file *fd, const void *ptr, size_t len ) {
  GET_FILE_FH(fd);

  return SPIFFS_write( &fs, fh, (void *)ptr, len );
}

static sint32_t myspiffs_vfs_lseek( const struct vfs_file *fd, sint32_t off, int whence ) {
  GET_FILE_FH(fd);
  int spiffs_whence;

  switch (whence) {
  default:
  case VFS_SEEK_SET:
    spiffs_whence = SPIFFS_SEEK_SET;
    break;
  case VFS_SEEK_CUR:
    spiffs_whence = SPIFFS_SEEK_CUR;
    break;
  case VFS_SEEK_END:
    spiffs_whence = SPIFFS_SEEK_END;
    break;
  }

  return SPIFFS_lseek( &fs, fh, off, spiffs_whence );
}

static sint32_t myspiffs_vfs_eof( const struct vfs_file *fd ) {
  GET_FILE_FH(fd);

  return SPIFFS_eof( &fs, fh );
}

static sint32_t myspiffs_vfs_tell( const struct vfs_file *fd ) {
  GET_FILE_FH(fd);

  return SPIFFS_tell( &fs, fh );
}

static sint32_t myspiffs_vfs_flush( const struct vfs_file *fd ) {
  GET_FILE_FH(fd);

  return SPIFFS_fflush( &fs, fh ) >= 0 ? VFS_RES_OK : VFS_RES_ERR;
}

static sint32_t myspiffs_vfs_ferrno( const struct vfs_file *fd ) {
  return SPIFFS_errno( &fs );
}


static int fs_mode2flag(const char *mode){
  if(c_strlen(mode)==1){
  	if(c_strcmp(mode,"w")==0)
  	  return SPIFFS_WRONLY|SPIFFS_CREAT|SPIFFS_TRUNC;
  	else if(c_strcmp(mode, "r")==0)
  	  return SPIFFS_RDONLY;
  	else if(c_strcmp(mode, "a")==0)
  	  return SPIFFS_WRONLY|SPIFFS_CREAT|SPIFFS_APPEND;
  	else
  	  return SPIFFS_RDONLY;
  } else if (c_strlen(mode)==2){
  	if(c_strcmp(mode,"r+")==0)
  	  return SPIFFS_RDWR;
  	else if(c_strcmp(mode, "w+")==0)
  	  return SPIFFS_RDWR|SPIFFS_CREAT|SPIFFS_TRUNC;
  	else if(c_strcmp(mode, "a+")==0)
  	  return SPIFFS_RDWR|SPIFFS_CREAT|SPIFFS_APPEND;
  	else
  	  return SPIFFS_RDONLY;
  } else {
  	return SPIFFS_RDONLY;
  }
}

// ---------------------------------------------------------------------------
// filesystem functions
//
static vfs_file *myspiffs_vfs_open( const char *name, const char *mode ) {
  struct myvfs_file *fd;
  int flags = fs_mode2flag( mode );

  if ((fd = (struct myvfs_file *)c_malloc( sizeof( struct myvfs_file ) ))) {
    if (0 < (fd->fh = SPIFFS_open( &fs, name, flags, 0 ))) {
      fd->vfs_file.fs_type = VFS_FS_SPIFFS;
      fd->vfs_file.fns     = &myspiffs_file_fns;
      return (vfs_file *)fd;
    } else {
      c_free( fd );
    }
  }

  return NULL;
}

static vfs_dir *myspiffs_vfs_opendir( const char *name ){
  struct myvfs_dir *dd;

  if ((dd = (struct myvfs_dir *)c_malloc( sizeof( struct myvfs_dir ) ))) {
    if (SPIFFS_opendir( &fs, name, &(dd->d) )) {
      dd->vfs_dir.fs_type = VFS_FS_SPIFFS;
      dd->vfs_dir.fns     = &myspiffs_dd_fns;
      return (vfs_dir *)dd;
    } else {
      c_free( dd );
    }
  }

  return NULL;
}

static vfs_item *myspiffs_vfs_stat( const char *name ) {
  struct myvfs_stat *s;

  if ((s = (struct myvfs_stat *)c_malloc( sizeof( struct myvfs_stat ) ))) {
    if (0 <= SPIFFS_stat( &fs, name, &(s->s) )) {
      s->vfs_item.fs_type = VFS_FS_SPIFFS;
      s->vfs_item.fns     = &myspiffs_item_fns;
      return (vfs_item *)s;
    } else {
      c_free( s );
    }
  }

  return NULL;
}

static sint32_t myspiffs_vfs_remove( const char *name ) {
  return SPIFFS_remove( &fs, name );
}

static sint32_t myspiffs_vfs_rename( const char *oldname, const char *newname ) {
  return SPIFFS_rename( &fs, oldname, newname );
}

static sint32_t myspiffs_vfs_fsinfo( uint32_t *total, uint32_t *used ) {
  return SPIFFS_info( &fs, total, used );
}

static sint32_t myspiffs_vfs_fscfg( uint32_t *phys_addr, uint32_t *phys_size ) {
  *phys_addr = SPIFFS_CFG_PHYS_ADDR(fs);
  *phys_size = SPIFFS_CFG_PHYS_SZ(fs);
  return VFS_RES_OK;
}

static vfs_vol  *myspiffs_vfs_mount( const char *name, int num ) {
  // volume descriptor not supported, just return TRUE / FALSE
  return myspiffs_mount() ? (vfs_vol *)1 : NULL;
}

static sint32_t myspiffs_vfs_format( void ) {
  return myspiffs_format();
}

static sint32_t myspiffs_vfs_errno( void ) {
  return SPIFFS_errno( &fs );
}

static void myspiffs_vfs_clearerr( void ) {
  SPIFFS_clearerr( &fs );
}


// ---------------------------------------------------------------------------
// VFS interface functions
//
vfs_fs_fns *myspiffs_realm( const char *inname, char **outname, int set_current_drive ) {
  if (inname[0] == '/') {
    size_t idstr_len = c_strlen( MY_LDRV_ID );
    // logical drive is specified, check if it's our id
    if (0 == c_strncmp( &(inname[1]), MY_LDRV_ID, idstr_len )) {
      *outname = (char *)&(inname[1 + idstr_len]);
      if (*outname[0] == '/') {
        // skip leading /
        (*outname)++;
      }

      if (set_current_drive) is_current_drive = TRUE;
      return &myspiffs_fs_fns;
    }
  } else {
    // no logical drive in patchspec, are we current drive?
    if (is_current_drive) {
      *outname = (char *)inname;
      return &myspiffs_fs_fns;
    }
  }

  if (set_current_drive) is_current_drive = FALSE;
  return NULL;
}
