/**
 ****************************************************************************************
 *
 * @file lanc_msg.h
 *
 * @brief Header file - Location and Navigation Profile Collector Role  - Message API.
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef _LANC_MSG_H_
#define _LANC_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @addtogroup LANC
 * @ingroup Profile
 * @brief Location and Navigation Profile Collector - Message API
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "lan_common.h"
#include "rwip_task.h" // Task definitions


/*
 * DEFINES
 ****************************************************************************************
 */

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Message IDs
/*@TRACE*/
enum lanc_msg_ids
{
    /// Enable the Location and Navigation Profile Collector task - at connection
    LANC_ENABLE_REQ             = MSG_ID(LANC, 0x00),
    /// Confirm that cfg connection has finished with discovery results, or that normal cnx started
    LANC_ENABLE_RSP             = MSG_ID(LANC, 0x01),

    /// Read the value of an attribute in the peer device database
    LANC_READ_CMD               = MSG_ID(LANC, 0x02),
    /// Configure sending of notification or indication
    LANC_CFG_NTFIND_CMD         = MSG_ID(LANC, 0x03),

    /// Configure the SC Control Point value
    LANC_LN_CTNL_PT_CFG_CMD     = MSG_ID(LANC, 0x04),
    /// Indicate that an control point response has been triggered by peer device
    LANC_LN_CTNL_PT_IND         = MSG_ID(LANC, 0x05),

    /// Indicate that an attribute value has been received either upon notification or read response
    LANC_VALUE_IND              = MSG_ID(LANC, 0x06),

    /// Complete Event Information
    LANC_CMP_EVT                = MSG_ID(LANC, 0x07),
};

/// Operation Codes
enum lanc_op_code
{
    /// Reserved operation code
    LANC_RESERVED_OP_CODE           = 0x00,
    /// Read attribute value Procedure
    LANC_READ_OP_CODE               = 0x01,
    /// Wait for the Write Response after having written a Client Char. Cfg. Descriptor.
    LANC_CFG_NTF_IND_OP_CODE        = 0x02,
    /// Wait for the Write Response after having written the Control Point Char.
    LANC_LN_CTNL_PT_CFG_WR_OP_CODE  = 0x03,
};

/// LANP Service Characteristics
enum lanp_lans_char
{
    /// LN Feature
    LANP_LANS_LN_FEAT_CHAR,
    /// Location and speed
    LANP_LANS_LOC_SPEED_CHAR,
    /// Position quality
    LANP_LANS_POS_Q_CHAR,
    /// LN Control Point
    LANP_LANS_LN_CTNL_PT_CHAR,
    /// Navigation
    LANP_LANS_NAVIG_CHAR,

    LANP_LANS_CHAR_MAX
};

/// Location and Navigation Service Characteristic Descriptors
enum lanc_lns_descs
{
    /// Location and Speed Char. - Client Characteristic Configuration
    LANC_DESC_LOC_SPEED_CL_CFG,
    /// LN Control Point Char. - Client Characteristic Configuration
    LANC_DESC_LN_CTNL_PT_CL_CFG,
    /// Navigation Char. - Client Characteristic Configuration
    LANC_DESC_NAVIGATION_CL_CFG,

    LANC_DESC_MAX,
};


/// Codes for reading/writing a LNS characteristic with one single request
enum lanc_code
{
    /// Read LN Feature
    LANC_RD_LN_FEAT             = 0x00,
    /// Notified Location and Speed
    LANC_NTF_LOC_SPEED          = 0x01,
    /// Read Position quality
    LANC_RD_POS_Q               = 0x02,
    /// Indicated LN Control Point
    LANC_IND_LN_CTNL_PT         = 0x03,
    /// Notified Navigation
    LANC_NTF_NAVIGATION         = 0x04,

    /// Read/Write Location and Speed Client Char. Configuration Descriptor
    LANC_RD_WR_LOC_SPEED_CL_CFG = 0x05,
    /// Read LN Control Point Client Char. Configuration Descriptor
    LANC_RD_WR_LN_CTNL_PT_CFG   = 0x06,
    /// Read/Write Navigation Client Char. Configuration Descriptor
    LANC_RD_WR_NAVIGATION_CFG   = 0x07,
};


/*
 * API MESSAGE STRUCTURES
 ****************************************************************************************
 */
/**
 * Structure containing the characteristics handles, value handles and descriptors for
 * the Location and Navigation Service
 */
typedef struct lanc_lns_content
{
    /// Service info
    prf_svc_t           svc;

    /// Characteristic info:
    ///  - LN Feature
    ///  - Location and Speed
    ///  - Position quality
    ///  - LN Control Point
    ///  - Navigation
    prf_char_t      chars[LANP_LANS_CHAR_MAX];

    /// Descriptor handles:
    ///  - Location and Speed Client Cfg
    ///  - Control Point Client Cfg
    ///  - Navigation Client Cfg
    prf_desc_t descs[LANC_DESC_MAX];
} lanc_lns_content_t;

/// Parameters of the @ref LANC_ENABLE_REQ message
struct lanc_enable_req
{
    /// Connection index
    uint8_t            conidx;
    /// Connection type
    uint8_t            con_type;
    /// Existing handle values LNS
    lanc_lns_content_t lns;
};

/// Parameters of the @ref LANC_ENABLE_RSP message
struct lanc_enable_rsp
{
    /// Connection index
    uint8_t            conidx;
    /// status
    uint16_t           status;
    /// Existing handle values LNS
    lanc_lns_content_t lns;
};

/// Parameters of the @ref LANC_READ_CMD message
struct lanc_read_cmd
{
    /// Connection index
    uint8_t conidx;
    /// Operation Code
    uint8_t operation;
    /// Read code
    uint8_t read_code;
};

/// Parameters of the @ref LANC_VALUE_IND message
struct lanc_value_ind
{
    /// Connection index
    uint8_t               conidx;
    /// Attribute Code
    uint8_t               att_code;
    /// Value
    union lanc_value_tag
    {
        /// LN Feature
        uint32_t          ln_feat;
        /// Location and Speed
        lanp_loc_speed_t  loc_speed;
        /// LAN Position quality
        lanp_posq_t       pos_q;
        /// Navigation
        lanp_navigation_t navigation;
        /// Client Characteristic Configuration Descriptor Value
        uint16_t          ntf_cfg;
    } value;
};

/// Parameters of the @ref LANC_CFG_NTFIND_CMD message
struct lanc_cfg_ntfind_cmd
{
    /// Connection index
    uint8_t  conidx;
    /// Operation Code
    uint8_t operation;
    /// Descriptor code
    uint8_t  desc_code;
    /// Ntf/Ind Configuration
    uint16_t ntfind_cfg;
};

/// Parameters of the @ref LANC_LN_CTNL_PT_CFG_CMD message
struct lanc_ln_ctnl_pt_cfg_cmd
{
    /// Connection index
    uint8_t              conidx;
    /// Operation Code
    uint8_t              operation;
    /// SC Control Point Request
    lan_ln_ctnl_pt_req_t ln_ctnl_pt;
};

/// Parameters of the @ref LANC_LN_CTNL_PT_IND message
struct lanc_ln_ctnl_pt_ind
{
    /// Connection index
    uint8_t               conidx;
    /// SC Control Point Response
    lanp_ln_ctnl_pt_rsp_t rsp;
};

/// Parameters of the @ref LANC_CMP_EVT message
struct lanc_cmp_evt
{
    /// Connection index
    uint8_t  conidx;
    /// Operation code
    uint8_t  operation;
    /// Status
    uint16_t status;
};

/// @} LANC

#endif //(_LANC_MSG_H_)
