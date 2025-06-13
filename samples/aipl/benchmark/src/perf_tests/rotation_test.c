/**
 * @file rotation_test.c
 */

/*********************
 *      INCLUDES
 *********************/
#include "perf_tests.h"
#include "aipl_rotate.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static uint32_t rotation_wrapper(void *arg);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
benchmark_t create_rotation_benchmark(void)
{
	return benchmark_create(&rotation_wrapper);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
static uint32_t rotation_wrapper(void *arg)
{
	rot_op_arg_t *args = (rot_op_arg_t *)arg;

	return aipl_rotate_img(args->src, args->dst, args->rotation);
}
