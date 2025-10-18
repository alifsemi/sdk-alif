/* Copyright (C) 2025 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#ifndef CLIENT_PROFILE_MANAGER_H_
#define CLIENT_PROFILE_MANAGER_H_

/**
 * @brief Start or conitnue client process
 *
 * @param svc_id Service ID
 * @param conidx Connection index
 * @param event Current process event
 *
 * @return 0 on success, error code otherwise
 */
void profile_client_process(uint16_t svc_id, uint8_t conidx, uint8_t event);

#endif /* CENTRAL_ITF_H_ */
