/**
 * @file resize_test.c
 */

/*********************
 *      INCLUDES
 *********************/
#include "perf_tests.h"
#include "aipl_resize.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static uint32_t resize_wrapper(void *arg);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
benchmark_t create_resize_benchmark(void)
{
	return benchmark_create(&resize_wrapper);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
static uint32_t resize_wrapper(void *arg)
{
	op_arg_t *args = (op_arg_t *)arg;

	return aipl_resize_img(args->src, args->dst, true);
}
