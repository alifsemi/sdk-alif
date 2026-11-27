/**
 ****************************************************************************************
 *
 * @file hap_has.h
 *
 * @brief Hearing Access Profile - Hearing Access Service Server - Definitions
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef HAP_HAS_H_
#define HAP_HAS_H_


#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup HAP_HAS Hearing Access Service Server
 * @ingroup HAP
 * @brief Description of Hearing Access Service Server module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup HAP_HAS_DEFINE Definitions
 * @ingroup HAP_HAS
 * @brief Definitions for Hearing Access Service Server module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup HAP_HAS_ENUM Enumerations
 * @ingroup HAP_HAS
 * @brief Enumerations for Hearing Access Service Server module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup HAP_HAS_STRUCT Structures
 * @ingroup HAP_HAS
 * @brief Structures for Hearing Access Service Server module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup HAP_HAS_NATIVE_API Native API
 * @ingroup HAP_HAS
 * @brief Description of Native API for Hearing Access Service Server module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup HAP_HAS_CALLBACK Callback Functions
 * @ingroup HAP_HAS_NATIVE_API
 * @brief Description of callback functions for Hearing Access Service Server module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup HAP_HAS_FUNCTION Functions
 * @ingroup HAP_HAS_NATIVE_API
 * @brief Description of functions for Hearing Access Service Server module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "hap.h"                // Hearing Access Profile Definitions

/*
 * DEFINES
 ****************************************************************************************
 */

/// @addtogroup HAP_HAS_DEFINE
/// @{

/// Maximum number of Preset Records that can be supported
#define HAP_HAS_PRESET_RECORDS_NB_MAX         (32)

/// @} HAP_HAS_DEFINE

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// @addtogroup HAP_HAS_ENUM
/// @{

/// Configuration bit field meaning (see #hap_has_cfg_param structure)
enum hap_has_cfg_bf
{
    /// Indicate if sending of notifications is supported
    /// for Hearing Aid Features characteristic (= 1) or not - Position
    HAP_HAS_CFG_FEATURES_NTF_SUPP_POS = 0,
    /// Indicate if sending of notifications is supported
    /// for Hearing Aid Features characteristic (= 1) or not - Bit
    HAP_HAS_CFG_FEATURES_NTF_SUPP_BIT = CO_BIT(HAP_HAS_CFG_FEATURES_NTF_SUPP_POS),
};

/// @} HAP_HAS_ENUM

/*
 * TYPES DEFINTIONS
 ****************************************************************************************
 */

/// @addtogroup HAP_HAS_STRUCT
/// @{

/// Configuration Parameter structure for Hearing Access Service Server
struct hap_has_cfg_param
{
    /// Configuration bit field (see #hap_has_cfg_bf enumeration)
    uint8_t cfg_bf;
    /// Preferred MTU\n
    /// Values for 0 to 63 are equivalent to 64
    uint8_t pref_mtu;
    /// Required start handle\n
    /// If set to GATT_INVALID_HANDLE, the start handle will be automatically chosen
    uint16_t shdl;
    /// Number of Presets records that may be added\n
    /// Up to 32\n
    /// Hearing Aid Preset Control Point characteristic not supported if set to 0
    uint8_t nb_presets;
    /// Hearing Aid Features bit field value (see #hap_has_features_bf enumeration)\n
    /// #HAP_HAS_FEATURES_WRITABLE_PRESETS_BIT set by the stack
    uint8_t features_bf;
};

/// Change information structure
typedef struct hap_has_change_info
{
    /// Preset local index of the changed Preset record
    uint8_t preset_lid;
    /// Changed ID value (see #hap_has_change_id enumeration)
    uint8_t change_id;
} hap_has_change_info_t;

/// @} HAP_HAS_STRUCT

/*
 * CALLBACK FUNCTIONS DEFINITION
 ****************************************************************************************
 */

/// @addtogroup HAP_HAS_CALLBACK
/// @{

/**
 ****************************************************************************************
 * @brief Callback function called when configuration for sending of notifications has been updated by a peer Client
 * device
 *
 * @param[in] con_lid               Connection local index
 * @param[in] cli_cfg_ntf_bf        Client configuration bit field for notification
 * @param[in] cli_cfg_ind_bf        Client configuration bit field for indication
 ****************************************************************************************
 */
typedef void (*hap_has_cb_bond_data)(uint8_t con_lid, uint8_t cli_cfg_ntf_bf, uint8_t cli_cfg_ind_bf);

/**
 ****************************************************************************************
 * @brief Callback function called when a peer Client device requires to update the current Active Preset\n
 * #hap_has_set_active_preset_cfm function shall be called by upper layer
 *
 * @param[in] con_lid               Connection local index
 * @param[in] preset_lid            Preset local index of preset required to become the new active Preset
 * @param[in] relay                 Indicate if Server must inform other member of the Coordinated Set about the
 * new active Preset
 ****************************************************************************************
 */
typedef void (*hap_has_cb_set_active_preset_req)(uint8_t con_lid, uint8_t preset_lid, bool relay);

/**
 ****************************************************************************************
 * @brief Callback function called when a peer Client device has updated name of a Preset\n
 * #hap_has_set_preset_name_cfm function shall be called by upper layer
 *
 * @param[in] con_lid               Connection local index
 * @param[in] preset_lid            Preset local index
 * @param[in] length                Length of Preset name
 * @param[in] p_name                Pointer to Preset name
 ****************************************************************************************
 */
typedef void (*hap_has_cb_set_preset_name_req)(uint8_t con_lid, uint8_t preset_lid,
                                               uint8_t length, const uint8_t* p_name);

/*
 * CALLBACK SET DEFINITION
 ****************************************************************************************
 */

/// Set of callback functions for Hearing Access Service Server module communication with upper layer
struct hap_has_cb
{
    /// Callback function called when configuration for sending of notifications has been updated by a peer Client
    /// device
    hap_has_cb_bond_data cb_bond_data;
    /// Callback function called when a peer Client device requires to update the current active Preset\n
    /// #hap_has_set_active_preset_cfm function shall be called by upper layer
    hap_has_cb_set_active_preset_req cb_set_active_preset_req;
    /// Callback function called when a peer Client device has updated name of a Preset\n
    /// #hap_has_set_preset_name_cfm function shall be called by upper layer
    hap_has_cb_set_preset_name_req cb_set_preset_name_req;
};

/// @} HAP_HAS_CALLBACK

/*
 * API FUNCTIONS DECLARATION
 ****************************************************************************************
 */

#if (GAF_HAP_HAS)
/// @addtogroup HAP_HAS_FUNCTION
/// @{

/**
 ****************************************************************************************
 * @brief Configure use of HAP Server module
 *
 * @param[in] p_cfg_param   Pointer to configuration parameter structure
 * @param[in] p_cb          Pointer to set to callback functions for communication with upper layer
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t hap_has_configure(const hap_has_cfg_param_t* p_cfg_param, const hap_has_cb_t* p_cb);

/**
 ****************************************************************************************
 * @brief Set bonding information for Hearing Access Service after connection with a Client device with which a bonded
 * relationship had been established during a previous connection.
 *
 * @param[in] con_lid               Connection local index
 * @param[in] cli_cfg_ntf_bf        Client configuration bit field for notification
 *                                  (see #hap_has_char_type enumeration)\n
 *                                  Reject enabling of notification bits if corresponding indication bits are enabled
 * @param[in] cli_cfg_ind_bf        Client configuration bit field for indication
                                    (see #hap_has_char_type enumeration)\n
                                    Reject enabling of indication bits if corresponding notification bits are enabled
 * @param[in] evt_cfg_bf            Event configuration bit field
 *                                  (see #hap_has_char_type enumeration) indicating characteristics
 *                                  for which value has been updated since disconnection
 *                                  and for which a notification (or indication) must be sent to the Client device
 * @param[in] nb_changes            Number of changes occurred before reconnection\n
 *                                  Meaningful only if Hearing Aid Preset Control Point characteristic is supported
 *                                  and corresponding bits of client and event configuration are enabled
 * @param[in] p_change_info         Pointer to information about changes occurred before reconnection\n
 *                                  Meaningful only if Hearing Aid Preset Control Point characteristic
 *                                  is supported and corresponding bits of client and event configuration are enabled
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t hap_has_restore_bond_data(uint8_t con_lid, uint8_t cli_cfg_ntf_bf, uint8_t cli_cfg_ind_bf,
                                   uint8_t evt_cfg_bf, uint8_t nb_changes,
                                   const hap_has_change_info_t* p_change_info);

/**
 ****************************************************************************************
 * @brief Add a Preset record
 *
 * @param[in] preset_lid            Preset local index
 * @param[in] writable              Indicate if Preset name can be written (= 1) or not (= 0) by a Client device
 * @param[in] available             Indicate if the Preset is available (= 1) or not (= 0)
 * @param[in] length                Preset name length\n
 *                                  From #HAP_HAS_PRESET_NAME_LEN_MIN to #HAP_HAS_PRESET_NAME_LEN_MAX
 * @param[in] p_name                Pointer to Preset Name\n
 *                                  Memory containing the name shall be allocated by the upper layer
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t hap_has_add_preset(uint8_t preset_lid, bool writable, bool available, uint8_t length,
                            const uint8_t* p_name);

/**
 ****************************************************************************************
 * @brief Remove a Preset record\n
 * Note that Active Preset cannot be removed
 *
 * @param[in] preset_lid            Preset local index\n
 *                                  #GAF_INVALID_LID indicates there is no active preset
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t hap_has_remove_preset(uint8_t preset_lid);

/**
 ****************************************************************************************
 * @brief Update index of the active Preset.\n
 * Can be used only if Active Preset Index characteristic is supported.
 *
 * @param[in] preset_lid            Preset local index\n
 *                                  #GAF_INVALID_LID indicates 'No active Preset'
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t hap_has_set_active_preset(uint8_t preset_lid);

/**
 ****************************************************************************************
 * @brief Update Hearing Aid Features value.\n
 * Can be used only if sending of notifications is supported for the characteristic.
 *
 * @param[in] features_bf           Hearing Aid Features bit field (see #hap_has_features_bf enumeration)
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t hap_has_set_features(uint8_t features_bf);

/**
 ****************************************************************************************
 * @brief Update availability or name of a Preset record.\n
 * Can be used only if sending of notifications is supported for the characteristic.
 *
 * @param[in] preset_lid            Preset local index
 * @param[in] available             Indicate if the Preset is available (= 1) or not (= 0)
 * @param[in] length                Preset name length\n
 *                                  Valid length from #HAP_HAS_PRESET_NAME_LEN_MIN to #HAP_HAS_PRESET_NAME_LEN_MAX\n
 *                                  0 means Preset name does not change\n
 *                                  Other values will be rejected
 * @param[in] p_name                Pointer to Preset Name\n
 *                                  Memory containing the name shall be allocated by the upper layer
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t hap_has_update_preset(uint8_t preset_lid, bool available, uint8_t length, const uint8_t* p_name);

/**
 ****************************************************************************************
 * @brief Confirmation for see #hap_has_cb_set_active_preset_req callback function
 *
 * @param[in] accept                Indicate if request is accepted or not
 ****************************************************************************************
 */
void hap_has_set_active_preset_cfm(bool accept);

/**
 ****************************************************************************************
 * @brief Confirmation for see #hap_has_cb_set_preset_name_req callback function
 *
 * @param[in] accept                Indicate if request is accepted or not
 * @param[in] length                Preset name length\n
 *                                  From #HAP_HAS_PRESET_NAME_LEN_MIN to #HAP_HAS_PRESET_NAME_LEN_MAX
 * @param[in] p_name                Pointer to Preset Name\n
 *                                  Memory containing the name shall be allocated by the upper layer
 ****************************************************************************************
 */
void hap_has_set_preset_name_cfm(bool accept, uint8_t length, const uint8_t* p_name);

/**
 ****************************************************************************************
 * @brief If use of Server Role for Hearing Access Service has been configured
 *
 * @return True if configured else false
 ****************************************************************************************
 */
bool hap_has_is_configured();

/// @} HAP_HAS_FUNCTION
#endif //(GAF_HAP_HAS)

#endif // HAP_HAS_H_
