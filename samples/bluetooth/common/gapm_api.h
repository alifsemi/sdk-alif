/* Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#ifndef GAPM_API_H
#define GAPM_API_H
#include "gapm.h"
#include "gapc.h"
#include "gapm_le.h"

enum gapm_connection_event {
	GAPM_API_DEV_CONNECTED,
	GAPM_API_SEC_CONNECTED_KNOWN_DEVICE,
	GAPM_API_DEV_DISCONNECTED,
	GAPM_API_PAIRING_FAIL,
};

#define BT_CONN_STATE_CONNECTED    0x00
#define BT_CONN_STATE_DISCONNECTED 0x01

typedef struct {
	/**
	 * Callback for indicate user when Advertisement is stopped.
	 *
	 * @param[in] metainfo      Metadata information provided by API user
	 * @param[in] actv_idx      Activity local index
	 * @param[in] reason        Activity stop reason
	 */
	void (*stopped)(uint32_t metainfo, uint8_t actv_idx, uint16_t reason);

	/**
	 * Callback executed for periodic ADV to indicate that non periodic advertising is stopped.
	 *
	 * @param[in] metainfo      Metadata information provided by API user
	 * @param[in] actv_idx      Activity local index
	 * @param[in] reason        Activity stop reason
	 */
	void (*ext_adv_stopped)(uint32_t metainfo, uint8_t actv_idx, uint16_t reason);

	/**
	 * Callback for indicate user when advertising activity is created
	 *
	 * @param[in] metainfo   Metadata information provided by API user
	 * @param[in] actv_idx   Activity local index
	 * @param[in] tx_pwr     Selected TX power for advertising activity
	 *
	 */
	void (*created)(uint32_t metainfo, uint8_t actv_idx, int8_t tx_pwr);

} gapm_le_adv_user_cb_t;

typedef struct {
	/**
	 * Callback for indicate user when BLE connection state is updated
	 *
	 * @param[in] event	Connection update type
	 * @param[in] con_idx	Connection local index
	 * @param[in] status	Event status report for GAPM_API_PAIRING_FAIL or
	 * GAPM_API_DEV_DISCONNECTED
	 */
	void (*connection_status_update)(enum gapm_connection_event event, uint8_t con_idx,
					 uint16_t status);
} gapm_user_cb_t;

/**
 * @brief Set GAPM preferred connections params
 *
 * Function initialize GAPM service with given name and configuration.
 *
 * @param preferred_params GAPM configure
 *
 */
void bt_gapm_preferred_connection_paras_set(
	const gapc_le_con_param_nego_with_ce_len_t *preferred_params);

/**
 * @brief Initialize GAPM service
 *
 * Function initialize GAPM service with given name and configuration.
 * Function allocate advertisement and scan response buffer
 *
 * @param p_cfg GAPM configure
 * @param p_cbs GAPM user callbacks
 * @param name String pointer to name
 * @param name_len Name length by strlen(name)
 *
 * @return 0 on success, positive error code otherwise
 */
uint16_t bt_gapm_init(const gapm_config_t *p_cfg, gapm_user_cb_t *p_cbs, const char *name,
		      size_t name_len);

/**
 * @brief GAPM advertisement service create
 *
 * @param addrstype Address type for service
 * @param adv_create_params Advertiment sevice parameters
 * @param user_cb Optional user callback's for Advertisement state monitor
 * @param adv_index pointer where advertisement service id is stored
 *
 * @return 0 on success, positive error code otherwise
 */
uint16_t bt_gapm_le_create_advertisement_service(enum gapm_le_own_addr addrstype,
						gapm_le_adv_create_param_t *adv_create_params,
						gapm_le_adv_user_cb_t *user_cb, uint8_t *adv_index);

/**
 * @brief Take configured Advertisement data to use
 *
 * @param adv_index advertisement service identifier
 *
 * @return 0 on success, positive error code otherwise
 */
uint16_t bt_gapm_advertiment_data_set(uint8_t adv_index);

/**
 * @brief Take configured Scan response buffer data to use
 *
 * @param adv_index advertisement service identifier
 *
 * @return 0 on success, positive error code otherwise
 */
uint16_t bt_gapm_scan_response_set(uint8_t adv_index);

/**
 * @brief Start GAPM advertisement
 *
 * Use only at init phase.
 *
 * @param adv_index advertisement service identifier
 *
 * @return 0 on success, positive error code otherwise
 */
uint16_t bt_gapm_advertisement_start(uint8_t adv_index);

/**
 * @brief Continue GAPM advertisement
 *
 * Use this at disconnect callback for re-start advertisement.
 *
 * @param adv_index advertisement service identifier
 *
 * @return 0 on success, positive error code otherwise
 */
uint16_t bt_gapm_advertisement_continue(uint8_t adv_index);

#endif /* GAPM_API_H */
