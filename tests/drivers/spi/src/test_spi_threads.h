/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#ifndef TEST_SPI_THREADS_H_
#define TEST_SPI_THREADS_H_

#include "test_spi_common.h"

/* Spawn controller + target threads, join, and assert err_count == 0 */
void run_spi_test_threads(void (*controller_func)(void *, void *, void *),
			  void (*target_func)(void *, void *, void *));

/* Simple functional SPI functions for DMA compatibility */
int func_target_transmit(const struct device *dev);
int func_controller_receive(const struct device *dev);

#endif /* TEST_SPI_THREADS_H_ */
