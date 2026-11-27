/**
 ****************************************************************************************
 *
 * @file udsc_msg.h
 *
 * @brief Header file - User Data Service Collector/Client Role - Message API.
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef _UDSC_MSG_H_
#define _UDSC_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @addtogroup UDSC
 * @ingroup Profile
 * @brief  User Data Service Profile Collector - Message API
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_task.h"      // Task definitions
#include "uds_common.h"

/*
 * ENUMERATIONS
 ****************************************************************************************
 */


/// Message IDs
/*@TRACE*/
enum udsc_msg_ids
{
    /// Enable the Profile Collector task - at connection
    UDSC_ENABLE_REQ              = MSG_ID(UDSC, 0x00),
    /// Response to Enable the Profile Collector task - at connection
    UDSC_ENABLE_RSP              = MSG_ID(UDSC, 0x01),

    ///*** UDS CHARACTERISTIC READ REQUESTS
    UDSC_RD_CHAR_CMD             = MSG_ID(UDSC, 0x02),

    /// Read the CCC of a DB_CHANGE & USER_CTRL_PT characteristic
    UDSC_RD_CHAR_CCC_CMD         = MSG_ID(UDSC, 0x03),

    ///***  CHARACTERISTIC WRITE COMMAND
    /// Write the CCC of a measurement sensor characteristic
    UDSC_WR_CHAR_CMD             = MSG_ID(UDSC, 0x04),
    /// Write the CCC of a measurement sensor characteristic
    UDSC_WR_CHAR_UTF8_CMD        = MSG_ID(UDSC, 0x05),

    ///*** UDS CHARACTERISTIC READ RESPONSE
    UDSC_RD_CHAR_IND             = MSG_ID(UDSC, 0x06),
    UDSC_RD_CHAR_UTF8_IND        = MSG_ID(UDSC, 0x07),

    /// Read Response the CCC of a DB_CHANGE & USER_CTRL_PT characteristic
    UDSC_RD_CHAR_CCC_IND         = MSG_ID(UDSC, 0x08),

    ///***  CHARACTERISTIC/DESCRIPTOR WRITE REQUESTS
    /// Write the CCC of a measurement sensor characteristic
    UDSC_WR_USER_CTRL_PT_CMD     = MSG_ID(UDSC, 0x09),

    /// Write the CCC of a DB_CHANGE & USER_CTRL_PT characteristic
    UDSC_WR_CHAR_CCC_CMD         = MSG_ID(UDSC, 0x0A),

    /// Characteristic DB_CHG_INC Indication from peer
    UDSC_DB_CHG_INC_IND          = MSG_ID(UDSC, 0x0B),
    /// Characteristic User Control Point Indication from peer
    UDSC_USER_CTRL_PT_IND        = MSG_ID(UDSC, 0x0C),
    /// Complete Event Information
    UDSC_CMP_EVT                 = MSG_ID(UDSC, 0x0D),
};


/// Operation Codes for pending operations in the client.
enum udsc_op_codes
{
    /// Reserved operation code
    UDSC_RESERVED_OP_CODE       = 0,

    /// Read op-codes
    UDSC_READ_CHAR_OP_CODE      = 1,

    /// Read op-codes
    UDSC_READ_CCC_OP_CODE       = 2,

    /// Write op-codes.
    UDSC_WRITE_CHAR_OP_CODE     = 3,
    UDSC_WRITE_CCC_OP_CODE      = 4,

    UDSC_CTNL_PT_OP_CODE        = 5
};


/// User Data Service Service Characteristics
enum udsc_char_idx
{
    /// Strings: utf8s
    UDSC_CHAR_UDS_FIRST_NAME                                        = 0,
    UDSC_CHAR_UDS_LAST_NAME                                         = 1,
    UDSC_CHAR_UDS_EMAIL_ADDRESS                                     = 2,
    UDSC_CHAR_UDS_LANGUAGE                                          = 3,
    /// Date
    UDSC_CHAR_UDS_DATE_OF_BIRTH                                     = 4,
    UDSC_CHAR_UDS_DATE_OF_THR_ASSESS                                = 5,
    /// uint16
    UDSC_CHAR_UDS_WEIGHT                                            = 6,
    UDSC_CHAR_UDS_HEIGHT                                            = 7,
    UDSC_CHAR_UDS_WAIST_CIRCUMFERENCE                               = 8,
    UDSC_CHAR_UDS_HIP_CIRCUMFERENCE                                 = 9,
    /// uint8
    UDSC_CHAR_UDS_AGE                                               = 10,
    UDSC_CHAR_UDS_GENDER                                            = 11,
    UDSC_CHAR_UDS_VO2_MAX                                           = 12,
    UDSC_CHAR_UDS_MAX_HEART_RATE                                    = 13,
    UDSC_CHAR_UDS_RESTING_HEART_RATE                                = 14,
    UDSC_CHAR_UDS_MAX_RECO_HEART_RATE                               = 15,
    UDSC_CHAR_UDS_AEROBIC_THR                                       = 16,
    UDSC_CHAR_UDS_ANAERO_THR                                        = 17,
    UDSC_CHAR_UDS_SPORT_TYPE_FOR_AERO_ANAREO_THRS                   = 18,
    UDSC_CHAR_UDS_FAT_BURN_HEART_RATE_LOW_LIM                       = 19,
    UDSC_CHAR_UDS_FAT_BURN_HEART_RATE_UP_LIM                        = 20,
    UDSC_CHAR_UDS_AEROBIC_HEART_RATE_LOW_LIM                        = 21,
    UDSC_CHAR_UDS_AEROBIC_HEART_RATE_UP_LIM                         = 22,
    UDSC_CHAR_UDS_ANAERO_HEART_RATE_LOW_LIM                         = 23,
    UDSC_CHAR_UDS_ANAERO_HEART_RATE_UP_LIM                          = 24,
    UDSC_CHAR_UDS_TWO_ZONE_HEART_RATE_LIMITS                        = 25,
    /// set
    UDSC_CHAR_UDS_FIVE_ZONE_HEART_RATE_LIMITS                       = 26,
    UDSC_CHAR_UDS_THREE_ZONE_HEART_RATE_LIMITS                      = 27,

    UDSC_CHAR_UDS_USER_DEFINED_1                                    = 28,
    UDSC_CHAR_UDS_USER_DEFINED_2                                    = 29,
    UDSC_CHAR_UDS_USER_DEFINED_3                                    = 30,
    UDSC_CHAR_UDS_USER_DEFINED_4                                    = 31,

    /// Database Change Increment
    UDSC_CHAR_UDS_DB_CHG_INC                                        = 32,
    /// User Index
    UDSC_CHAR_UDS_USER_INDEX                                        = 33,
    /// User Control Point
    UDSC_CHAR_UDS_USER_CTRL_PT                                      = 34,

    UDSC_CHAR_UDS_MAX,
};

/// User Data Service Service Characteristic Descriptors
enum udsc_desc_idx
{
    /// Database Change Increment Client config
    UDSC_DESC_UDS_DB_CHG_INC_CCC,
    /// User Control Point Client config
    UDSC_DESC_UDS_USER_CTRL_PT_CCC,
//    /// Extended
//    UDSC_DESC_UDS_FIRST_NAME_EXT,
//    UDSC_DESC_UDS_LAST_NAME_EXT,
//    UDSC_DESC_UDS_EMAIL_ADDRESS_EXT,
//    UDSC_DESC_UDS_LANGUAGE_EXT,

    UDSC_DESC_UDS_MAX,
};

/**
 * Structure containing the characteristics handles, value handles and descriptors for
 * the User Data Service Service
 */
typedef struct udsc_uds_content
{
    /// service info
    prf_svc_t           svc;

    /// Characteristic info:
    ///  - Feature
    ///  - Measurement
    prf_char_t      chars[UDSC_CHAR_UDS_MAX];

    /// Descriptor handles:
    ///  - Client cfg
    prf_desc_t descs[UDSC_DESC_UDS_MAX];
} udsc_uds_content_t;

/*
 * API MESSAGE STRUCTURES
 ****************************************************************************************
 */

/// Parameters of the @ref UDSC_ENABLE_REQ message
struct udsc_enable_req
{
    /// Connection index
    uint8_t            conidx;
    /// Connection type
    uint8_t            con_type;
    /// Existing handle values UDSC ES
    udsc_uds_content_t uds;
};

/// Parameters of the @ref UDSC_ENABLE_RSP message
struct udsc_enable_rsp
{
    /// Connection index
    uint8_t            conidx;
    /// status
    uint16_t           status;
    /// Existing handle values UDSC ES
    udsc_uds_content_t uds;
};

///*** UDS CHARACTERISTIC/DESCRIPTOR READ REQUESTS
/// Parameters of the @ref UDSC_RD_CHAR_CMD message
/// Read the User Data Service Feature
struct udsc_rd_char_cmd
{
    /// Connection index
    uint8_t conidx;
    /// Characteristic from @ref enum uds_val_id
    uint8_t char_idx;
};

/// Parameters of the @ref UDSC_RD_CHAR_CCC_CMD message
/// Read the CCC of a measurement sensor characteristic
struct udsc_rd_char_ccc_cmd
{
    /// Connection index
    uint8_t conidx;
    /// Characteristic from @ref enum udsc_char_idx
    /// UDSC_CHAR_UDS_DB_CHG_INC, UDSC_CHAR_UDS_USER_CTRL_PT
    uint8_t char_idx;
};

///***  CHARACTERISTIC/DESCRIPTOR WRITE REQUESTS
/// Parameters of the @ref UDSC_WR_CHAR_CCC_CMD message
/// Write the CCC to a selected characteristic
struct udsc_wr_char_ccc_cmd
{
    /// Connection index
    uint8_t  conidx;
    /// Characteristic from @ref enum udsc_char_idx
    /// UDSC_CHAR_UDS_DB_CHG_INC, UDSC_CHAR_UDS_USER_CTRL_PT
    uint8_t  char_idx;
    /// CCC value
    uint16_t ccc;
};

///*** UDS CHARACTERISTIC/DESCRIPTOR READ RESPONSE
/// Parameters of the @ref UDSC_RD_CHAR_IND message
/// Read the User Data Service Characteristic
struct udsc_rd_char_ind
{
    /// Connection index
    uint8_t        conidx;
    /// Characteristic from @ref enum udsc_char_idx
    uint8_t        char_idx;
    /// Characteristic indication value
    union uds_value value;
};

/// Parameters of the @ref UDSC_WR_CHAR_UTF8_CMD message
/// Write to User Data Service UTF8 Characteristic
struct udsc_wr_char_utf8_cmd
{
    /// Connection index
    uint8_t      conidx;
    /// Characteristic from @ref enum udsc_char_idx
    uint8_t      char_idx;
    /// UTF Name
    prf_utf_8_t  utf_name;
};

/// Parameters of the @ref UDSC_WR_CHAR_CMD message
/// Write to User Data Service Characteristic
struct udsc_wr_char_cmd
{
    /// Connection index
    uint8_t            conidx;
    /// Characteristic from @ref enum udsc_char_idx
    uint8_t            char_idx;
    /// Characteristic value
    union uds_value value;
};

/// Parameters of the @ref UDSC_RD_CHAR_UTF8_IND message
/// Read the User Data Service UTF8 Characteristic
struct udsc_rd_char_utf8_ind
{
    /// Connection index
    uint8_t     conidx;
    /// Characteristic from @ref enum udsc_char_idx
    uint8_t     char_idx;
    /// UTF Name
    prf_utf_8_t utf_name;
};

/// Parameters of the @ref UDSC_RD_CHAR_CCC_IND message
/// Read the CCC of a measurement sensor characteristic
struct udsc_rd_char_ccc_ind
{
    /// Connection index
    uint8_t  conidx;
    /// Characteristic from @ref enum udsc_char_idx
    /// UDSC_CHAR_UDS_DB_CHG_INC, UDSC_CHAR_UDS_USER_CTRL_PT
    uint8_t  char_idx;
    /// Indication Configuration
    uint16_t ccc;
};

/// Parameters of the @ref UDSC_DB_CHG_INC_IND message
/// Characteristic Measurement Indication from peer
struct udsc_db_chg_inc_ind
{
    /// Connection index
    uint8_t  conidx;
    /// Database change increment value
    uint32_t value;
};

/// Parameters of the @ref UDSC_USER_CTRL_PT_IND message
/// User Data Service User Control Point Characteristic Indication
struct udsc_user_ctrl_pt_ind
{
    /// Connection index
    uint8_t  conidx;
    /// response op-code =0x20
    uint8_t  resp_code;
    /// requested op-code
    uint8_t  req_op_code;
    /// Response value
    uint8_t  resp_value;
    /// User ID used for register new user command
    uint8_t  user_id;
    /// Parameter length
    uint16_t length;
    /// Parameter value - for the future extension  to send additional commands
    uint8_t  parameter[__ARRAY_EMPTY];
};

/// Parameters of the @ref UDSC_WR_USER_CTRL_PT_CMD message
/// Write to User Data Service User Control Point Characteristic
struct udsc_wr_user_ctrl_pt_cmd
{
    /// Connection index
    uint8_t  conidx;
    /// User Control Point Op-codes see enum #uds_ctrl_pt_op_code
    uint8_t  op_code;
    /// User Index
    uint8_t  user_id;
    /// used for sending consent command
    uint16_t consent;
    /// Parameter length
    uint16_t length;
    /// Parameter value - for the future extension  to send additional commands
    uint8_t  parameter[__ARRAY_EMPTY];
};

/// Parameters of the @ref UDSC_CMP_EVT message
struct udsc_cmp_evt
{
    /// Connection index
    uint8_t  conidx;
    /// Operation code
    uint8_t  operation;
    /// Status
    uint16_t status;
};

/// @} UDSC

#endif //(_UDSC_MSG_H_)
