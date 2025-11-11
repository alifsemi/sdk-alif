/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#ifndef _AURACAST_SCAN_DELEGATOR_H
#define _AURACAST_SCAN_DELEGATOR_H

/**
 * @brief Initialize BLE services for broadcast sink (BASS, scan, sink)
 *
 * @retval 0 on success
 * @retval Negative error code on failure
 */
int auracast_scan_delegator_init(void);

/**
 * @brief Deinitialize BLE services for broadcast sink (BASS, scan, sink)
 */
void auracast_scan_delegator_deinit(void);

/**
 * @brief Start BASS solicitation advertising with a consistent Extended Adv payload.
 *
 * The payload includes Complete Local Name (if it fits) and Appearance.
 *
 * @retval 0 on success
 * @retval Negative error code on failure
 */
int auracast_scan_delegator_start_solicitation(void);

#endif /* _AURACAST_SCAN_DELEGATOR_H */
