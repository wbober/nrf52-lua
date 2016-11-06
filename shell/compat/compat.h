#ifndef _COMPAT_H_
#define _COMPAT_H_

#include <stdint.h>
#include <string.h>

#include "nrf_log.h"

#ifdef NODE_DEBUG
#define NODE_DBG(...) NRF_LOG_INTERNAL_DEBUG( __VA_ARGS__)
#else
#define NODE_DBG(...)
#endif	/* NODE_DEBUG */

#ifdef NODE_ERROR
#define NODE_ERR(...) NRF_LOG_INTERNAL_ERROR(__VA_ARGS__)
#else
#define NODE_ERR(...)
#endif	/* NODE_ERROR */


typedef long long int64_t;
typedef int8_t  sint8_t;
typedef int16_t sint16_t;
typedef int32_t sint32_t;
typedef int64_t sint64_t;

#define c_memcmp memcmp
#define c_memcpy memcpy
#define c_memset memset

#define c_strcat strcat
#define c_strchr strchr
#define c_strcmp strcmp
#define c_strcpy strcpy
#define c_strlen strlen
#define c_strncmp strncmp
#define c_strncpy strncpy
#define c_strncasecmp c_strncmp

#define c_strstr strstr
#define c_strncat strncat
#define c_strcspn strcspn
#define c_strpbrk strpbrk
#define c_strcoll strcoll
#define c_strrchr strrchr

#include "umm_malloc.h"
#define c_malloc umm_malloc
#define c_free   umm_free

#define TRUE 1
#define FALSE 0

#endif
