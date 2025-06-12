/**
 * @file video_alloc.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include <aipl_video_alloc.h>
#include <dave_d0lib.h>

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
void *aipl_video_alloc(uint32_t size)
{
	return d0_allocvidmem(size);
}

void aipl_video_free(void *ptr)
{
	d0_freevidmem(ptr);
}
/**********************
 *   STATIC FUNCTIONS
 **********************/
