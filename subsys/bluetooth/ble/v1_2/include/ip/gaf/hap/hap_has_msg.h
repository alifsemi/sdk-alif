/**
 ****************************************************************************************
 *
 * @file hap_has_msg.h
 *
 * @brief Hearing Access Profile - Hearing Access Service Server - Message API Definitions
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef HAP_HAS_MSG_H_
#define HAP_HAS_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup HAP_HAS_MSG Message API
 * @ingroup HAP_HAS
 * @brief Description of Message API for Hearing Access Service Server module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "hap_msg.h"        // Message API Definitions
#include "hap_has.h"        // Hearing Access Service Server Definitions

/// @addtogroup HAP_HAS_MSG
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// List of GAF_REQ request code values for Hearing Access Service Server module
enum hap_has_msg_req_code
{
    /// Restore Bond Data (see #hap_has_restore_bond_data_req_t)
    HAP_HAS_RESTORE_BOND_DATA = GAF_CODE(HAP, HAS, 0),
    /// Add Preset (see #hap_has_add_preset_req_t)
    HAP_HAS_ADD_PRESET = GAF_CODE(HAP, HAS, 1),
    /// Remove Preset (see #hap_has_remove_preset_req_t)
    HAP_HAS_REMOVE_PRESET = GAF_CODE(HAP, HAS, 2),
    /// Set Active Preset (see #hap_has_set_active_preset_req_t)
    HAP_HAS_SET_ACTIVE_PRESET = GAF_CODE(HAP, HAS, 3),
    /// Set Features (see #hap_has_set_features_req_t)
    HAP_HAS_SET_FEATURES = GAF_CODE(HAP, HAS, 4),
    /// Update Preset (see #hap_has_update_preset_req_t)
    HAP_HAS_UPDATE_PRESET = GAF_CODE(HAP, HAS, 5),
};

/// List of GAF_IND indication code values for Hearing Access Service Server module
enum hap_has_msg_ind_code
{
    /// Bond Data (see #hap_has_bond_data_ind_t)
    HAP_HAS_BOND_DATA = GAF_CODE(HAP, HAS, 0),
};

/// List of GAF_REQ_IND request indication code values for Hearing Access Service Server module
enum hap_has_msg_req_ind_code
{
    /// Set Active Preset (see #hap_has_set_active_preset_req_ind_t)
    HAP_HAS_SET_ACTIVE_PRESET_RI = GAF_CODE(HAP, HAS, 0),
    /// Set Preset Name (see #hap_has_set_preset_name_req_ind_t)
    HAP_HAS_SET_PRESET_NAME_RI = GAF_CODE(HAP, HAS, 1),
};

/*
 * API MESSAGES
 ****************************************************************************************
 */

/// Structure for #HAP_HAS_RESTORE_BOND_DATA request message
typedef struct hap_has_restore_bond_data_req
{
    /// Request code (shall be set to #HAP_HAS_RESTORE_BOND_DATA)
    uint16_t req_code;
    /// Connection local index
    uint8_t con_lid;
    /// Client configuration bit field for notification (see #hap_has_char_type enumeration)\n
    /// Reject enabling of notification bits if corresponding indication bits are enabled
    uint8_t cli_cfg_ntf_bf;
    /// Client configuration bit field for indication (see #hap_has_char_type enumeration)\n
    /// Reject enabling of indication bits if corresponding notification bits are enabled
    uint8_t cli_cfg_ind_bf;
    /// Event configuration bit field (see #hap_has_char_type enumeration) indicating characteristics\n
    /// for which value has been updated since disconnection and for which a notification (or indication)
    /// must be sent to the Client device
    uint8_t evt_cfg_bf;
    /// Number of changes occurred before reconnection\n
    /// Meaningful only if Hearing Aid Preset Control Point characteristic is supported and
    /// corresponding bits of client and event configuration are enabled
    uint8_t nb_changes;
    /// Information about changes occurred before reconnection\n
    /// Meaningful only if Hearing Aid Preset Control Point characteristic
    /// is supported and corresponding bits of client and event configuration are enabled
    hap_has_change_info_t change_info[__ARRAY_EMPTY];
} hap_has_restore_bond_data_req_t;

/// Structure for #HAP_HAS_ADD_PRESET request message
typedef struct hap_has_add_preset_req
{
    /// Request code (shall be set to #HAP_HAS_ADD_PRESET)
    uint16_t req_code;
    /// Preset local index
    uint8_t preset_lid;
    /// Indicate if Preset name can be written (>= 1) or not (= 0) by a Client device
    uint8_t writable;
    /// Indicate if the Preset is available (>= 1) or not (= 0)
    uint8_t available;
    /// Length of Preset name\n
    /// From #HAP_HAS_PRESET_NAME_LEN_MIN to #HAP_HAS_PRESET_NAME_LEN_MAX
    uint8_t length;
    /// Preset name
    uint8_t name[__ARRAY_EMPTY];
} hap_has_add_preset_req_t;

/// Structure for #HAP_HAS_REMOVE_PRESET request message
typedef struct hap_has_remove_preset_req
{
    /// Request code (shall be set to #HAP_HAS_REMOVE_PRESET)
    uint16_t req_code;
    /// Preset local index\n
    /// Note that active Preset cannot be removed
    uint8_t preset_lid;
} hap_has_remove_preset_req_t;

/// Structure for #HAP_HAS_SET_ACTIVE_PRESET request message
typedef struct hap_has_set_active_preset_req
{
    /// Request code (shall be set to #HAP_HAS_SET_ACTIVE_PRESET)
    uint16_t req_code;
    /// Preset local index\n
    /// #GAF_INVALID_LID indicates 'No active Preset'
    uint8_t preset_lid;
} hap_has_set_active_preset_req_t;

/// Structure for #HAP_HAS_SET_FEATURES request message
typedef struct hap_has_set_features_req
{
    /// Request code (shall be set to #HAP_HAS_SET_FEATURES)
    uint16_t req_code;
    /// Hearing Aid Features bit field (see #hap_has_features_bf enumeration)
    uint8_t features_bf;
} hap_has_set_features_req_t;

/// Structure for #HAP_HAS_UPDATE_PRESET request message
typedef struct hap_has_update_preset_req
{
    /// Request code (shall be set to #HAP_HAS_UPDATE_PRESET)
    uint16_t req_code;
    /// Preset local index
    uint8_t preset_lid;
    /// Indicate if the Preset is available (>= 1) or not (= 0)
    uint8_t available;
    /// Length of Preset name\n
    /// Valid length from #HAP_HAS_PRESET_NAME_LEN_MIN to #HAP_HAS_PRESET_NAME_LEN_MAX\n
    /// 0 means Preset name does not change\n
    /// Other values will be rejected
    uint8_t length;
    /// Preset name
    uint8_t name[__ARRAY_EMPTY];
} hap_has_update_preset_req_t;

/// Structure of response message for Hearing Access Service Server module
typedef struct hap_has_rsp
{
    /// Request code (see #hap_has_msg_req_code enumeration)
    uint16_t req_code;
    /// Status
    uint16_t status;
    /// Union
    union
    {
        /// Local index
        uint8_t lid;
        /// Connection local index
        uint8_t con_lid;
        /// Preset local index
        uint8_t preset_lid;
    } lid;
} hap_has_rsp_t;

/// Structure for #HAP_HAS_BOND_DATA indication message
typedef struct hap_has_bond_data_ind
{
    /// Indication code (set to #HAP_HAS_BOND_DATA)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Client configuration bit field for notification
    /// (see #hap_has_char_type enumeration)
    uint8_t cli_cfg_ntf_bf;
    /// Client configuration bit field for indication
    /// (see #hap_has_char_type enumeration)
    uint8_t cli_cfg_ind_bf;
} hap_has_bond_data_ind_t;

/// Structure for #HAP_HAS_SET_ACTIVE_PRESET_RI request indication message
typedef struct hap_has_set_active_preset_req_ind
{
    /// Request Indication code (set to #HAP_HAS_SET_ACTIVE_PRESET_RI)
    uint16_t req_ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Preset local index
    uint8_t preset_lid;
    /// Indicate if Server must inform other member of the Coordinated Set about the new active preset (>= 1) or not
    uint8_t relay;
} hap_has_set_active_preset_req_ind_t;

/// Structure for #HAP_HAS_SET_PRESET_NAME_RI request indication message
typedef struct hap_has_set_preset_name_req_ind
{
    /// Request Indication code (set to #HAP_HAS_SET_PRESET_NAME_RI)
    uint16_t req_ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Preset local index
    uint8_t preset_lid;
    /// Length of Preset name\n
    /// From #HAP_HAS_PRESET_NAME_LEN_MIN to #HAP_HAS_PRESET_NAME_LEN_MAX
    uint8_t length;
    /// Preset name
    uint8_t name[__ARRAY_EMPTY];
} hap_has_set_preset_name_req_ind_t;

/// Structure for #HAP_HAS_SET_ACTIVE_PRESET_RI confirmation message
typedef struct hap_has_set_active_preset_cfm
{
    /// Request Indication code (shall be set to #HAP_HAS_SET_ACTIVE_PRESET_RI)
    uint16_t req_ind_code;
    /// Status
    uint16_t status;
} hap_has_set_active_preset_cfm_t;

/// Structure for #HAP_HAS_SET_PRESET_NAME_RI confirmation message
typedef struct hap_has_set_preset_name_cfm
{
    /// Request Indication code (shall be set to #HAP_HAS_SET_PRESET_NAME_RI)
    uint16_t req_ind_code;
    /// Status
    uint16_t status;
    /// Length of Preset name\n
    /// From #HAP_HAS_PRESET_NAME_LEN_MIN to #HAP_HAS_PRESET_NAME_LEN_MAX
    uint8_t length;
    /// Preset name
    uint8_t name[__ARRAY_EMPTY];
} hap_has_set_preset_name_cfm_t;

/// @} HAP_HAS_MSG

#endif // HAP_HAS_MSG_H_
