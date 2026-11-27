/**
 ****************************************************************************************
 *
 * @file gaf_msg.h
 *
 * @brief Generic Audio Framework - Definition of Messages
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef GAF_MSG_H_
#define GAF_MSG_H_

#include "rom_build_cfg.h"

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "gaf.h"            // GAF Definitions
#include "gaf_adv.h"        // GAF Advertising Definitions
#include "rwip_task.h"      // RW IP Task Definitions
#include "ke_task.h"        // Kernel Tasks Definitions

/**
 ****************************************************************************************
 * @defgroup GAF_MSG Message API
 * @ingroup GAF
 * @brief Description of GAF Message API
 ****************************************************************************************
 */

/// @addtogroup GAF_MSG
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// List of GAF message IDs
enum gaf_msg_ids
{
    /// GAF Command
    GAF_CMD = MSG_ID(GAF, 0),
    /// GAF Command complete event
    GAF_CMP_EVT = MSG_ID(GAF, 1),
    /// GAF Request
    GAF_REQ = MSG_ID(GAF, 2),
    /// GAF Response
    GAF_RSP = MSG_ID(GAF, 3),
    /// GAF Indication
    GAF_IND = MSG_ID(GAF, 4),
    /// GAF Request Indication
    GAF_REQ_IND = MSG_ID(GAF, 5),
    /// GAF Confirmation
    GAF_CFM = MSG_ID(GAF, 6),
};

/// List of GAF_REQ request codes
enum gaf_msg_req_code
{
    /// Get supported GAF features (see #gaf_get_features_req_t)
    GAF_GET_FEATURES = GAF_CODE(GAF, COMMON, 0),
    /// Configure GAF (see #gaf_configure_req_t)
    GAF_CONFIGURE = GAF_CODE(GAF, COMMON, 1),
};

/// List of GAF_IND indication codes
enum gaf_msg_ind_code
{
    /// Unknown message has been received (see #gaf_unknown_msg_ind_t)
    GAF_UNKNOWN_MSG = GAF_CODE(GAF, COMMON, 0),
};

/// Features bit field meaning - Part 1
enum gaf_msg_features_bf
{
    /// Indicate if Volume Control Service - Server Role is supported
    GAF_FEATURES_BF_ARC_VCS_POS = 0,
    GAF_FEATURES_BF_ARC_VCS_BIT = CO_BIT(GAF_FEATURES_BF_ARC_VCS_POS),
    /// Indicate if Volume Control Service - Client Role is supported
    GAF_FEATURES_BF_ARC_VCC_POS = 1,
    GAF_FEATURES_BF_ARC_VCC_BIT = CO_BIT(GAF_FEATURES_BF_ARC_VCC_POS),
    /// Indicate if Volume Offset Control Service - Server Role is supported
    GAF_FEATURES_BF_ARC_VOCS_POS = 2,
    GAF_FEATURES_BF_ARC_VOCS_BIT = CO_BIT(GAF_FEATURES_BF_ARC_VOCS_POS),
    /// Indicate if Volume Offset Control Service - Client Role is supported
    GAF_FEATURES_BF_ARC_VOCC_POS = 3,
    GAF_FEATURES_BF_ARC_VOCC_BIT = CO_BIT(GAF_FEATURES_BF_ARC_VOCC_POS),
    /// Indicate if Audio Input Control Service - Server Role is supported
    GAF_FEATURES_BF_ARC_AICS_POS = 4,
    GAF_FEATURES_BF_ARC_AICS_BIT = CO_BIT(GAF_FEATURES_BF_ARC_AICS_POS),
    /// Indicate if Audio Input Control Service - Client Role is supported
    GAF_FEATURES_BF_ARC_AICC_POS = 5,
    GAF_FEATURES_BF_ARC_AICC_BIT = CO_BIT(GAF_FEATURES_BF_ARC_AICC_POS),
    /// Indicate if Microphone Control Service - Server Role is supported
    GAF_FEATURES_BF_ARC_MICS_POS = 6,
    GAF_FEATURES_BF_ARC_MICS_BIT = CO_BIT(GAF_FEATURES_BF_ARC_MICS_POS),
    /// Indicate if Microphone Control Service - Client Role is supported
    GAF_FEATURES_BF_ARC_MICC_POS = 7,
    GAF_FEATURES_BF_ARC_MICC_BIT = CO_BIT(GAF_FEATURES_BF_ARC_MICC_POS),

    /// Indicate if Media Control Service - Server Role is supported
    GAF_FEATURES_BF_ACC_MCS_POS = 8,
    GAF_FEATURES_BF_ACC_MCS_BIT = CO_BIT(GAF_FEATURES_BF_ACC_MCS_POS),
    /// Indicate if Media Control Service - Client Role is supported
    GAF_FEATURES_BF_ACC_MCC_POS = 9,
    GAF_FEATURES_BF_ACC_MCC_BIT = CO_BIT(GAF_FEATURES_BF_ACC_MCC_POS),
    /// Indicate if Telephone Bearer Service - Server Role is supported
    GAF_FEATURES_BF_ACC_TBS_POS = 10,
    GAF_FEATURES_BF_ACC_TBS_BIT = CO_BIT(GAF_FEATURES_BF_ACC_TBS_POS),
    /// Indicate if Telephone Bearer Service - Client Role is supported
    GAF_FEATURES_BF_ACC_TBC_POS = 11,
    GAF_FEATURES_BF_ACC_TBC_BIT = CO_BIT(GAF_FEATURES_BF_ACC_TBC_POS),
    /// Indicate if Object Transfer Service - Server Role is supported
    GAF_FEATURES_BF_ACC_OTS_POS = 12,
    GAF_FEATURES_BF_ACC_OTS_BIT = CO_BIT(GAF_FEATURES_BF_ACC_OTS_POS),
    /// Indicate if Object Transfer Service Service - Client Role is supported
    GAF_FEATURES_BF_ACC_OTC_POS = 13,
    GAF_FEATURES_BF_ACC_OTC_BIT = CO_BIT(GAF_FEATURES_BF_ACC_OTC_POS),

    /// Indicate if Coordinated Set Identification Service - Set Member Role is supported
    GAF_FEATURES_BF_ATC_CSISM_POS = 14,
    GAF_FEATURES_BF_ATC_CSISM_BIT = CO_BIT(GAF_FEATURES_BF_ATC_CSISM_POS),
    /// Indicate if Coordinated Set Identification Service - Set Coordinated Role is supported
    GAF_FEATURES_BF_ATC_CSISC_POS = 15,
    GAF_FEATURES_BF_ATC_CSISC_BIT = CO_BIT(GAF_FEATURES_BF_ATC_CSISC_POS),

    /// Indicate if Basic Audio Profile - Capabilities Server Role is supported
    GAF_FEATURES_BF_BAP_CAPA_SRV_POS = 16,
    GAF_FEATURES_BF_BAP_CAPA_SRV_BIT = CO_BIT(GAF_FEATURES_BF_BAP_CAPA_SRV_POS),
    /// Indicate if Basic Audio Profile - Capabilities Client Role is supported
    GAF_FEATURES_BF_BAP_CAPA_CLI_POS = 17,
    GAF_FEATURES_BF_BAP_CAPA_CLI_BIT = CO_BIT(GAF_FEATURES_BF_BAP_CAPA_CLI_POS),
    /// Indicate if Basic Audio Profile - Unicast Server Role is supported
    GAF_FEATURES_BF_BAP_UC_SRV_POS = 18,
    GAF_FEATURES_BF_BAP_UC_SRV_BIT = CO_BIT(GAF_FEATURES_BF_BAP_UC_SRV_POS),
    /// Indicate if Basic Audio Profile - Unicast Client Role is supported
    GAF_FEATURES_BF_BAP_UC_CLI_POS = 19,
    GAF_FEATURES_BF_BAP_UC_CLI_BIT = CO_BIT(GAF_FEATURES_BF_BAP_UC_CLI_POS),
    /// Indicate if Basic Audio Profile - Broadcast Source Role is supported
    GAF_FEATURES_BF_BAP_BC_SRC_POS = 20,
    GAF_FEATURES_BF_BAP_BC_SRC_BIT = CO_BIT(GAF_FEATURES_BF_BAP_BC_SRC_POS),
    /// Indicate if Basic Audio Profile - Broadcast Sink Role is supported
    GAF_FEATURES_BF_BAP_BC_SINK_POS = 21,
    GAF_FEATURES_BF_BAP_BC_SINK_BIT = CO_BIT(GAF_FEATURES_BF_BAP_BC_SINK_POS),
    /// Indicate if Basic Audio Profile - Broadcast Scan Role is supported
    GAF_FEATURES_BF_BAP_BC_SCAN_POS = 22,
    GAF_FEATURES_BF_BAP_BC_SCAN_BIT = CO_BIT(GAF_FEATURES_BF_BAP_BC_SCAN_POS),
    /// Indicate if Basic Audio Profile - Broadcast Assistant Role is supported
    GAF_FEATURES_BF_BAP_BC_ASSIST_POS = 23,
    GAF_FEATURES_BF_BAP_BC_ASSIST_BIT = CO_BIT(GAF_FEATURES_BF_BAP_BC_ASSIST_POS),
    /// Indicate if Basic Audio Profile - Broadcast Delegator Role is supported
    GAF_FEATURES_BF_BAP_BC_DELEG_POS = 24,
    GAF_FEATURES_BF_BAP_BC_DELEG_BIT = CO_BIT(GAF_FEATURES_BF_BAP_BC_DELEG_POS),

    /// Indicate if Common Audio Service - Server Role is supported
    GAF_FEATURES_BF_CAP_CAS_POS = 25,
    GAF_FEATURES_BF_CAP_CAS_BIT = CO_BIT(GAF_FEATURES_BF_CAP_CAS_POS),
    /// Indicate if Common Audio Service - Client Role is supported
    GAF_FEATURES_BF_CAP_CAC_POS = 26,
    GAF_FEATURES_BF_CAP_CAC_BIT = CO_BIT(GAF_FEATURES_BF_CAP_CAC_POS),

    /// Indicate if Hearing Access Service - Server Role is supported
    GAF_FEATURES_BF_HAP_HAS_POS = 27,
    GAF_FEATURES_BF_HAP_HAS_BIT = CO_BIT(GAF_FEATURES_BF_HAP_HAS_POS),
    /// Indicate if Hearing Access Service - Client Role is supported
    GAF_FEATURES_BF_HAP_HAC_POS = 28,
    GAF_FEATURES_BF_HAP_HAC_BIT = CO_BIT(GAF_FEATURES_BF_HAP_HAC_POS),

    /// Indicate if Telephony and Media Service - Server Role is supported
    GAF_FEATURES_BF_TMAP_TMAS_POS = 29,
    GAF_FEATURES_BF_TMAP_TMAS_BIT = CO_BIT(GAF_FEATURES_BF_TMAP_TMAS_POS),
    /// Indicate if Telephony and Media Service - Client Role is supported
    GAF_FEATURES_BF_TMAP_TMAC_POS = 30,
    GAF_FEATURES_BF_TMAP_TMAC_BIT = CO_BIT(GAF_FEATURES_BF_TMAP_TMAC_POS),
};

/// Features bit field meaning - Part 2
enum gaf_msg_features_bf_2
{
    /// Indicate if GAF Advertiser module is supported
    GAF_FEATURES_BF_GAF_ADV_POS = 0,
    GAF_FEATURES_BF_GAF_ADV_BIT = CO_BIT(GAF_FEATURES_BF_GAF_ADV_POS),
    /// Indicate if GAF Scanner module is supported
    GAF_FEATURES_BF_GAF_SCAN_POS = 1,
    GAF_FEATURES_BF_GAF_SCAN_BIT = CO_BIT(GAF_FEATURES_BF_GAF_SCAN_POS),
    /// Indicate if GAF Client module is supported
    GAF_FEATURES_BF_GAF_CLI_POS = 2,
    GAF_FEATURES_BF_GAF_CLI_BIT = CO_BIT(GAF_FEATURES_BF_GAF_CLI_POS),

    /// Indicate if Gaming Service - Server Role is supported
    GAF_FEATURES_BF_GMAP_GMAS_POS = 3,
    GAF_FEATURES_BF_GMAP_GMAS_BIT = CO_BIT(GAF_FEATURES_BF_GMAP_GMAS_POS),
    /// Indicate if Gaming Service - Client Role is supported
    GAF_FEATURES_BF_GMAP_GMAC_POS = 4,
    GAF_FEATURES_BF_GMAP_GMAC_BIT = CO_BIT(GAF_FEATURES_BF_GMAP_GMAC_POS),
};

/*
 * TYPES DEFINITIONS
 ****************************************************************************************
 */

/// Basic structure for GAF_CMD message
typedef struct gaf_cmd
{
    /// Command code
    uint16_t cmd_code;
} gaf_cmd_t;

/// Basic structure for GAF_CMP_EVT message
typedef struct gaf_cmp_evt
{
    /// Command code
    uint16_t cmd_code;
    /// Status
    uint16_t status;
} gaf_cmp_evt_t;

/// Basic structure for GAF_REQ message
typedef struct gaf_req
{
    /// Request code
    uint16_t req_code;
} gaf_req_t;

/// Basic structure for GAF_RSP message
typedef struct gaf_rsp
{
    /// Request code
    uint16_t req_code;
    /// Status
    uint16_t status;
} gaf_rsp_t;

/// Basic structure for GAF_IND message
typedef struct gaf_ind
{
    /// Indication code
    uint16_t ind_code;
} gaf_ind_t;

/// Basic structure for GAF_REQ_IND message
typedef struct gaf_req_ind
{
    /// Request Indication code
    uint16_t req_ind_code;
} gaf_req_ind_t;

/// Basic structure for GAF_CFM message
typedef struct gaf_cfm
{
    /// Request Indication code
    uint16_t req_ind_code;
    /// Status
    uint16_t status;
} gaf_cfm_t;

/// Structure for GAF_GET_FEATURES request message
typedef struct gaf_get_features_req
{
    /// Request code (see #gaf_msg_req_code enumeration)
    ///  - GAF_GET_FEATURES
    uint16_t req_code;
} gaf_get_features_req_t;

/// Structure for GAF_GET_FEATURES response message
typedef struct gaf_get_features_rsp
{
    /// Request code (see #gaf_msg_req_code enumeration)
    ///  - GAF_GET_FEATURES
    uint16_t req_code;
    /// Status
    uint16_t status;
    /// Features bit field - Part 1 (see #gaf_msg_features_bf enumeration)
    uint32_t features_bf;
    /// Features bit field - Part 2 (see #gaf_msg_features_bf_2 enumeration)
    uint32_t features_bf_2;
} gaf_get_features_rsp_t;

/// Structure for GAF_CONFIGURE request message
typedef struct gaf_configure_req
{
    /// Request code (see #gaf_msg_req_code enumeration)
    ///  - GAF_CONFIGURE
    uint16_t req_code;
    /// Configuration bit field (see #gaf_cfg_bf enumeration)
    uint8_t cfg_bf;
    /// Advertiser module configuration\n
    /// Meaningful only if GAF_CFG_ADV_SUPP_BIT set in cfg_bf
    gaf_adv_cfg_t adv_cfg;
} gaf_configure_req_t;

/// Structure for GAF_UNKNOWN_MSG indication message
typedef struct gaf_unknown_msg_ind
{
    /// Indication code (see #gaf_msg_ind_code enumeration)
    uint16_t ind_code;
    /// Message ID
    ke_msg_id_t msg_id;
} gaf_unknown_msg_ind_t;

/// @} GAF_MSG

#endif // GAF_MSG_H_
