/**
 * @file cache.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include <aipl_cache.h>
#include <zephyr/cache.h>

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
void aipl_cpu_cache_clean(const void *ptr, uint32_t size)
{
	sys_cache_data_flush_range((void *)ptr, size);
}

void aipl_cpu_cache_invalidate(const void *ptr, uint32_t size)
{
	sys_cache_data_invd_range((void *)ptr, size);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
