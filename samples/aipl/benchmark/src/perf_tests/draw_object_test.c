/**
 * @file draw_object_test.c
 */

/*********************
 *      INCLUDES
 *********************/
#include "perf_tests.h"
#include "graphics.h"
#include "aipl_dave2d.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static uint32_t draw_object_wrapper(void *arg);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
benchmark_t create_draw_object_benchmark(void)
{
	return benchmark_create(&draw_object_wrapper);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
static uint32_t draw_object_wrapper(void *arg)
{
	graph_object_t *obj = (graph_object_t *)arg;

	graph_draw_object(obj);

	d2_device *handle = aipl_dave2d_handle();

	d2_endframe(handle);

	return 0;
}
