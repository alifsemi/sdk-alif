/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

#ifndef SPI_PM_SPI_TEST_H_
#define SPI_PM_SPI_TEST_H_

/**
 * @brief Create the SPI DW controller/peripheral loopback threads.
 *
 * Safe to call more than once; threads are only created on the first call.
 * The threads survive S2RAM (retained RAM) and block on their start
 * semaphores between rounds, so they never need to be re-created.
 *
 * @retval 0       Threads are running.
 * @retval -ENODEV Controller or peripheral SPI device is not ready.
 */
int spi_test_init(void);

/**
 * @brief Run one SPI DMA loopback round and wait for it.
 *
 * All PM states deeper than RUNTIME_IDLE are locked for the duration of the
 * round because the SPI/DMA interrupts are not IWIC/EWIC wakeup sources.
 * A done-semaphore timeout leaves those locks held so a late DMA/SPI
 * completion cannot wake from S2RAM or SOFT_OFF. Later rounds then
 * return -EBUSY without starting another transfer. The same happens if
 * one side finishes with an error while the other is still in
 * spi_transceive(): the waiter times out and the rest of the suite
 * skips SPI.
 *
 * @param phase_label Text printed in the log to identify the round.
 *
 * @retval 0          Both directions transferred and the data matched.
 * @retval -EIO       Data mismatch or transceive error.
 * @retval -ETIMEDOUT A thread did not finish the round in time.
 * @retval -EBUSY     A previous round timed out; this round was skipped.
 * @retval -ENODEV    spi_test_init() has not succeeded.
 */
int spi_loopback_run(const char *phase_label);

#endif /* SPI_PM_SPI_TEST_H_ */
