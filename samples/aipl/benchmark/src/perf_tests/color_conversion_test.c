/**
 * @file color_conversion_test.c
 */

/*********************
 *      INCLUDES
 *********************/
#include "perf_tests.h"
#include "aipl_color_conversion.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static uint32_t color_conversion_wrapper(void *arg);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
benchmark_t create_color_conversion_benchmark(void)
{
	return benchmark_create(&color_conversion_wrapper);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
static uint32_t color_conversion_wrapper(void *arg)
{
	op_arg_t *args = (op_arg_t *)arg;

	return aipl_color_convert_img(args->src, args->dst);
}
