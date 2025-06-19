/**
 * @file white_balance_test.c
 */

/*********************
 *      INCLUDES
 *********************/
#include "perf_tests.h"
#include "aipl_white_balance.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static uint32_t white_balance_wrapper(void *arg);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
benchmark_t create_white_balance_benchmark(void)
{
	return benchmark_create(&white_balance_wrapper);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
static uint32_t white_balance_wrapper(void *arg)
{
	wb_op_arg_t *args = (wb_op_arg_t *)arg;

	return aipl_white_balance_rgb_img(args->src, args->dst, args->ar, args->ag, args->ab);
}
