/**
 ****************************************************************************************
 *
 * @file hap_hac.h
 *
 * @brief Hearing Access Profile - Hearing Access Service Client - Definitions
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef HAP_HAC_H_
#define HAP_HAC_H_


#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup HAP_HAC Hearing Access Service Client
 * @ingroup HAP
 * @brief Description of Hearing Access Service Client module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup HAP_HAC_ENUM Enumerations
 * @ingroup HAP_HAC
 * @brief Enumerations for Hearing Access Service Client module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup HAP_HAC_STRUCT Structures
 * @ingroup HAP_HAC
 * @brief Structures for Hearing Access Service Client module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup HAP_HAC_NATIVE_API Native API
 * @ingroup HAP_HAC
 * @brief Description of Native API for Hearing Access Service Client module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup HAP_HAC_CALLBACK Callback Functions
 * @ingroup HAP_HAC_NATIVE_API
 * @brief Description of callback functions for Hearing Access Service Client module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup HAP_HAC_FUNCTION Functions
 * @ingroup HAP_HAC_NATIVE_API
 * @brief Description of functions for Hearing Access Service Client module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "hap.h"                // Hearing Access Profile Definitions

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// @addtogroup HAP_HAC_ENUM
/// @{

/// List of command type values for Hearing Access Service Client module
enum hap_hac_cmd_type
{
    /// Hearing Access Service Client - Discover
    HAP_HAC_CMD_TYPE_DISCOVER = 0,
    /// Hearing Access Service Client - Get
    HAP_HAC_CMD_TYPE_GET,
    /// Hearing Access Service Client - Set Configuration
    HAP_HAC_CMD_TYPE_SET_CFG = 3,
    /// Hearing Access Service Client - Set Preset Name
    HAP_HAC_CMD_TYPE_SET_PRESET_NAME,
    /// Hearing Access Service Client - Set Active Preset
    HAP_HAC_CMD_TYPE_SET_ACTIVE_PRESET,
    /// Hearing Access Service Client - Get Presets
    HAP_HAC_CMD_TYPE_GET_PRESETS,
};

/// Set type values
enum hap_hac_set_type
{
    /// Set active Preset by providing its index
    HAP_HAC_SET_TYPE_INDEX = 0,
    /// Set active Preset by choosing next Preset
    HAP_HAC_SET_TYPE_NEXT,
    /// Set active Preset by choosing previous Preset
    HAP_HAC_SET_TYPE_PREVIOUS,

    /// Maximum value
    HAP_HAC_SET_TYPE_MAX,
};

/// Service status bit field meaning
enum hap_hac_has_status_bf
{
    /// Indicate if notification of Hearing Aid Preset Control Point characteristic is supported (=1) or not
    HAP_HAC_HAS_STATUS_PRESET_CP_NTF_SUPP_POS = 0,
    HAP_HAC_HAS_STATUS_PRESET_CP_NTF_SUPP_BIT = CO_BIT(HAP_HAC_HAS_STATUS_PRESET_CP_NTF_SUPP_POS),
};

/// @} HAP_HAC_ENUM

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// @addtogroup HAP_HAC_STRUCT
/// @{

/// Configuration Parameter structure for Hearing Access Service Client
struct hap_hac_cfg_param
{
    /// Preferred MTU
    /// Values for 0 to 63 are equivalent to 64
    uint8_t pref_mtu;
};

/// Hearing Access Service characteristic description structure
typedef struct hap_hac_has_char
{
    /// Characteristic value handle
    uint16_t val_hdl;
    /// Client Characteristic Configuration descriptor handle
    uint16_t desc_hdl;
} hap_hac_has_char_t;

/// Hearing Access Service content description structure
typedef struct hap_hac_has
{
    /// Status bit field (see #hap_hac_has_status_bf enumeration)
    uint8_t status_bf;
    /// Service description
    prf_svc_t svc_info;
    /// Characteristics description
    hap_hac_has_char_t char_info[HAP_HAS_CHAR_TYPE_MAX];
} hap_hac_has_t;

/// @} HAP_HAC_STRUCT

/*
 * CALLBACK FUNCTIONS DEFINITION
 ****************************************************************************************
 */

/// @addtogroup HAP_HAC_CALLBACK
/// @{

/**
 ****************************************************************************************
 * @brief Callback function called when a command has been completed
 *
 * @param[in] cmd_type              Command type (see #hap_hac_cmd_type enumeration)
 * @param[in] status                Status
 * @param[in] con_lid               Connection local index
 * @param[in] type                  Type\n
 *                                      - Set type (see #hap_hac_set_type enumeration)\n
 *                                      - Characteristic type (see #hap_has_char_type enumeration)
 * @param[in] preset_idx            Preset index
 ****************************************************************************************
 */
typedef void (*hap_hac_cb_cmp_evt)(uint8_t cmd_type, uint16_t status, uint8_t con_lid, uint8_t type,
                                   uint8_t preset_idx);

/**
 ****************************************************************************************
 * @brief Callback function called when Hearing Access Service has been discovered in a Service device database
 *
 * @param[in] con_lid               Connection local index
 * @param[in] p_has_info            Pointer to Hearing Access Service content description structure
 ****************************************************************************************
 */
typedef void (*hap_hac_cb_bond_data)(uint8_t con_lid, const hap_hac_has_t* p_has_info);

/**
 ****************************************************************************************
 * @brief Callback function called when a service changed indication for the Hearing Access Service has been received
 *
 * @param[in] con_lid               Connection local index
 ****************************************************************************************
 */
typedef void (*hap_hac_cb_svc_changed)(uint8_t con_lid);

/**
 ****************************************************************************************
 * @brief Callback function called when a Preset Record is received
 *
 * @param[in] con_lid               Connection local index
 * @param[in] last                  Indicate if the preset record is the last one in the list (= 1) or not (= 0)
 * @param[in] preset_idx            Preset index
 * @param[in] writable              Indicate if Preset name can be written (= 1) or not (= 0) by a Client device
 * @param[in] available             Indicate if the Preset is available (= 1) or not (= 0)
 * @param[in] length                Length of Preset name\n
 *                                  From #HAP_HAS_PRESET_NAME_LEN_MIN to #HAP_HAS_PRESET_NAME_LEN_MAX
 * @param[in] p_name                Pointer to Preset name
 ****************************************************************************************
 */
typedef void (*hap_hac_cb_preset)(uint8_t con_lid, bool last, uint8_t preset_idx, bool writable,
                                  bool available, uint8_t length, const uint8_t* p_name);

/**
 ****************************************************************************************
 * @brief Callback function called when either Active Preset index or Hearing Aid Features
 * bit field value is received
 *
 * @param[in] con_lid               Connection local index
 * @param[in] char_type             Characteristic type (see #hap_has_char_type enumeration)
 * @param[in] value                 Active Preset index or Features bit field value
 ****************************************************************************************
 */
typedef void (*hap_hac_cb_value)(uint8_t con_lid, uint8_t char_type, uint8_t value);

/**
 ****************************************************************************************
 * @brief Callback function called when:
 * - A new preset record is added
 * - The name of an existing preset record has changed
 * - More than one change happened during disconnection
 *
 * @param[in] con_lid               Connection local index
 * @param[in] last                  Indicate if the preset record is the last one in the list (= 1) or not (= 0)
 * @param[in] prev_preset_idx       Index of the preset record previous to the changed preset record
 * @param[in] preset_idx            Preset index
 * @param[in] writable              Indicate if Preset name can be written (= 1) or not (= 0) by a Client device
 * @param[in] available             Indicate if the Preset is available (= 1) or not (= 0)
 * @param[in] length                Length of Preset name\n
 *                                  From #HAP_HAS_PRESET_NAME_LEN_MIN to #HAP_HAS_PRESET_NAME_LEN_MAX
 * @param[in] p_name                Pointer to Preset name
 ****************************************************************************************
 */
typedef void (*hap_hac_cb_generic_update)(uint8_t con_lid, bool last, uint8_t prev_preset_idx, uint8_t preset_idx,
                                          bool writable, bool available, uint8_t length, const uint8_t* p_name);

/**
 ****************************************************************************************
 * @brief Callback function called when:
 * - A preset record has been deleted
 * - A preset record has become available
 * - A preset record has become unavailable
 *
 * @param[in] con_lid               Connection local index
 * @param[in] change_id             Change ID value (see #hap_has_change_id enumeration)
 * @param[in] last                  Indicate if the preset record is the last one in the list (= 1) or not (= 0)
 * @param[in] preset_idx            Preset index
 ****************************************************************************************
 */
typedef void (*hap_hac_cb_update)(uint8_t con_lid, uint8_t change_id, bool last, uint8_t preset_idx);

/*
 * CALLBACK SET DEFINITION
 ****************************************************************************************
 */

/// Set of callback functions for Hearing Access Service Client module communication with upper layer
struct hap_hac_cb
{
    /// Callback function called when a command has been completed
    hap_hac_cb_cmp_evt cb_cmp_evt;
    /// Callback function called when Hearing Access Service has been discovered in a Service device database
    hap_hac_cb_bond_data cb_bond_data;
    /// Callback function called when a service changed indication for the Hearing Access Service has been received
    hap_hac_cb_svc_changed cb_svc_changed;
    /// Callback function called when a Preset Record is received
    hap_hac_cb_preset cb_preset;
    /// Callback function called when either Active Preset index or Hearing Aid Features bit field value is received
    hap_hac_cb_value cb_value;
    /// Callback function called when a generic update of Preset record(s) has been received
    hap_hac_cb_generic_update cb_generic_update;
    /// Callback function called when an update of Preset record has been received
    hap_hac_cb_update cb_update;
};

/// @} HAP_HAC_CALLBACK

/*
 * API FUNCTIONS DECLARATION
 ****************************************************************************************
 */

#if (GAF_HAP_HAC)
/// @addtogroup HAP_HAC_FUNCTION
/// @{

/**
 ****************************************************************************************
 * @brief Configure use of HAP Client module
 *
 * @param[in] p_cfg_param   Pointer to configuration parameter structure
 * @param[in] p_cb          Pointer to set to callback functions for communication with upper layer
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t hap_hac_configure(const hap_hac_cfg_param_t* p_cfg_param, const hap_hac_cb_t* p_cb);

/**
 ****************************************************************************************
 * @brief Enable Hearing Access Profile block for use of Hearing Access Profile as Client for a connected device with
 * which no bonding has been established during a previous connection\n
 * All readable characteristics are read during the procedure\n
 * Sending of notifications is enabled for each characteristic supporting this feature during the procedure
 *
 * @param[in] con_lid               Connection local index
 * @param[in] ind_ntf               Indicate if sending of indications (= 1) or notifications (= 0) is
 *                                  enabled for Hearing Aid Preset Control Point characteristic
 * @param[in] shdl                  Start handle for the discovery. Set #GATT_INVALID_HDL if not provided
 * @param[in] ehdl                  End handle for the discovery. Set #GATT_INVALID_HDL if not provided
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t hap_hac_discover(uint8_t con_lid, bool ind_ntf, uint16_t shdl, uint16_t ehdl);

#if (GAF_DBG)
/**
 ****************************************************************************************
 * @brief Get value of either the Active Preset Index characteristic,
 * or the Hearing Aid Features characteristic.
 *
 * @param[in] con_lid               Connection local index
 * @param[in] char_type             Characteristic type (see #hap_has_char_type enumeration)
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t hap_hac_get(uint8_t con_lid, uint8_t char_type);

/**
 ****************************************************************************************
 * @brief Set notification configuration for either the Active Preset Index characteristic,
 * or the Hearing Aid Features characteristic, or the Hearing Aid Preset Control Point characteristic.
 *
 * @param[in] con_lid               Connection local index
 * @param[in] char_type             Characteristic type (see #hap_has_char_type enumeration)
 * @param[in] enable                Indicate if sending of notifications must be enabled or not
 * @param[in] ind_ntf               Indicate if sending of indications (= 1) or notifications (= 0) is enabled
 *                                  for Hearing Aid Preset Control Point characteristic\n
 *                                  Meaningful only if characteristic type is
 *                                  Hearing Aid Preset Control Point characteristic
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t hap_hac_set_cfg(uint8_t con_lid, uint8_t char_type, bool enable, bool ind_ntf);
#endif //(GAF_DBG)

/**
 ****************************************************************************************
 * @brief Set name for a Preset exposed by a peer Server device
 *
 * @param[in] con_lid               Connection local index
 * @param[in] preset_idx            Preset index
 * @param[in] length                Length of Preset name
 * @param[in] p_name                Pointer to Preset name
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t hap_hac_set_preset_name(uint8_t con_lid, uint8_t preset_idx, uint8_t length, const uint8_t* p_name);

/**
 ****************************************************************************************
 * @brief Request peer Server device to update current active Preset
 *
 * @param[in] con_lid               Connection local index
 * @param[in] set_type              Set type (see #hap_hac_set_type enumeration)
 * @param[in] relay                 Indicate if Server device must inform other member of the Coordinated Set it
 *                                  belongs to
 * @param[in] preset_idx            Preset index
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t hap_hac_set_active_preset(uint8_t con_lid, uint8_t set_type, bool relay, uint8_t preset_idx);

/**
 ****************************************************************************************
 * @brief Request peer Server device to indicate or notify one specific Preset record or all Preset records.
 *
 * @param[in] con_lid               Connection local index
 * @param[in] preset_idx            Index of first Preset Record to read\n
 *                                  Shall not be 0
 * @param[in] nb_presets            Maximum number of Preset Record to read\n
 *                                  Shall not be 0
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t hap_hac_get_presets(uint8_t con_lid, uint8_t preset_idx, uint8_t nb_presets);

/**
 ****************************************************************************************
 * @brief Enable Hearing Access Profile block for use of the Hearing Access Service as
 * Client for a connected device with which a bonding has been established during a previous connection
 *
 * @param[in] con_lid               Connection local index
 * @param[in] p_has_info            Pointer to Hearing Access Service content description structure
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t hap_hac_restore_bond_data(uint8_t con_lid, const hap_hac_has_t* p_has_info);

/// @} HAP_HAC_FUNCTION
#endif //(GAF_HAP_HAC)

#endif // HAP_HAC_H_
