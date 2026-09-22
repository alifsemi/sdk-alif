/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

#ifndef FLASH_TO_PSRAM_H_
#define FLASH_TO_PSRAM_H_

#include <sys/types.h>
#include <stddef.h>

/**
 * @brief Copy a payload from OSPI flash into OSPI HyperRAM (PSRAM) XiP.
 *
 * Copies @p len bytes starting at @p flash_off in the OSPI flash to the PSRAM
 * XiP window at @p psram_off. Flash and PSRAM share one OSPI0 controller, so
 * the copy alternates the bus between them one SRAM-sized chunk at a time --
 * the payload may be arbitrarily large (it need not fit in SRAM).
 *
 * On success the bus is left in PSRAM XiP mode, so the copied data is
 * immediately readable at the PSRAM XiP base address.
 *
 * @param flash_off  Source byte offset within the OSPI flash.
 * @param psram_off  Destination byte offset within the PSRAM XiP window
 *                   (must be 2-byte aligned).
 * @param len        Number of bytes to copy (must be a multiple of 2).
 *
 * @retval 0        on success.
 * @retval -EINVAL  if @p len or @p psram_off is not 2-byte aligned.
 * @retval -ENODEV  if the flash device is not ready.
 * @retval -EIO     if the PSRAM OSPI HAL instance could not be initialised.
 * @retval <0       propagated flash_read() error.
 * @retval -ENOTSUP if the flash/psram aliases are not present.
 */
int flash_to_psram_copy(off_t flash_off, off_t psram_off, size_t len);

/**
 * @brief Verify a payload previously copied into the PSRAM XiP window.
 *
 * Reads back @p len bytes from the PSRAM XiP window at @p psram_off and
 * checks the test.bin layout: 8-byte "ALIFPSRM" magic, offset-encoded 32-bit
 * words, and a trailing little-endian CRC32 over the first (len - 4) bytes.
 * Progress and pass/fail are printed via printk.
 *
 * @param psram_off  Byte offset within the PSRAM XiP window.
 * @param len        Number of bytes to verify (multiple of 4, >= 12).
 *
 * @retval 0        on success (all checks passed).
 * @retval -EINVAL  if @p len is too small or not word-aligned.
 * @retval -EIO     if any check failed.
 * @retval -ENOTSUP if the flash/psram aliases are not present.
 */
int flash_to_psram_verify(off_t psram_off, size_t len);

#endif /* FLASH_TO_PSRAM_H_ */
