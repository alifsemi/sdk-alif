/**
 * @file color_correction_test.c
 */

/*********************
 *      INCLUDES
 *********************/
#include "perf_tests.h"
#include "aipl_color_correction.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static uint32_t color_correction_wrapper(void *arg);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
benchmark_t create_color_correction_benchmark(void)
{
	return benchmark_create(&color_correction_wrapper);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
static uint32_t color_correction_wrapper(void *arg)
{
	cc_op_arg_t *args = (cc_op_arg_t *)arg;

	return aipl_color_correction_rgb_img(args->src, args->dst, args->ccm);
}
