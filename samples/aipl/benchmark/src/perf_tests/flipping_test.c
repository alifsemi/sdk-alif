/**
 * @file flipping_test.c
 */

/*********************
 *      INCLUDES
 *********************/
#include "perf_tests.h"
#include "aipl_flip.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static uint32_t flipping_wrapper(void *arg);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
benchmark_t create_flipping_benchmark(void)
{
	return benchmark_create(&flipping_wrapper);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
static uint32_t flipping_wrapper(void *arg)
{
	flip_op_arg_t *args = (flip_op_arg_t *)arg;

	return aipl_flip_img(args->src, args->dst, args->horizontal, args->vertical);
}
