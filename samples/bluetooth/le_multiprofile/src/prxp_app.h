/* Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#ifndef PRXP_APP_H
#define PRXP_APP_H

/**
 * @brief Configure and register the Proximity Profile server (LLSS only).
 *
 * Registers the Link Loss Service (LLSS) with the BLE stack using
 * prf_add_profile(). IASS and TPSS are excluded to stay within the
 * profile task limit.
 */
void prxp_server_configure(void);

/**
 * @brief Process Immediate Alert Service (IAS) alert level.
 *
 * Should be called periodically while connected. Logs a warning if
 * a mild or high alert level is active.
 */
void ias_process(void);

/**
 * @brief Handle disconnection event for the Proximity Profile.
 *
 * Triggers a link loss alert via ll_notify() if the disconnection
 * was not initiated by the remote user. Resets the IAS alert level.
 *
 * @param reason  HCI disconnection reason code.
 */
void prxp_disc_notify(uint16_t reason);

/**
 * @brief Append PRXP advertising parameters to an existing param struct.
 *
 * Sets the TX power parameter for advertising and returns the
 * updated advertising creation parameter struct.
 *
 * @param adv_append_params  Pointer to the base advertising parameter struct.
 * @return Updated gapm_le_adv_create_param_t with PRXP parameters merged in.
 */
gapm_le_adv_create_param_t append_adv_param(gapm_le_adv_create_param_t *adv_append_params);

#endif /* PRXP_APP_H */
