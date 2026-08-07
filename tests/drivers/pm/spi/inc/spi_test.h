/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

#ifndef __SPI_PM_TEST_INC__
#define __SPI_PM_TEST_INC__

#include <stdbool.h>

int spi_pm_thread_init(void);
int spi_pm_thread_start(void);
int spi_pm_thread_suspend(void);
int spi_pm_thread_resume(void);
int spi_pm_thread_stop(void);

bool spi_pm_thread_is_started(void);
bool spi_pm_thread_is_suspended(void);

#endif /* __SPI_PM_TEST_INC__ */


