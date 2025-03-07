/* Copyright (C) 2025 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#ifndef _UNICAST_SOURCE_H
#define _UNICAST_SOURCE_H

#include <stdint.h>

/**
 * @brief Configure the LE audio unicast source
 *
 * @return 0 on success
 */
int unicast_source_configure(void);

/**
 * @brief Start scanning for LE audio devices
 *
 * @return 0 on success
 */
int unicast_source_scan_start(void);

/**
 * @brief Start the service discovery
 *
 * @return 0 on success
 */
int unicast_source_discover(uint8_t const con_lid);

#endif /* _UNICAST_SOURCE_H */
