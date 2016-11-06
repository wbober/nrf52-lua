#ifndef _PLATFORM_H_
#define _PLATFORM_H_

// Error / status codes
enum
{
  PLATFORM_ERR,
  PLATFORM_OK,
  PLATFORM_UNDERFLOW = -1
};

static inline uint32_t platform_flash_write(const void *from, uint32_t toaddr, uint32_t size)
{
	return 0;
}

static inline uint32_t platform_flash_read( void *to, uint32_t fromaddr, uint32_t size )
{
	return 0;
}

static inline uint32_t platform_flash_get_sector_of_address( uint32_t addr )
{
	return 0;
}

static inline int platform_flash_erase_sector( uint32_t sector_id )
{
	return 0;
}

#endif /* _PLATFORM_H_ */
