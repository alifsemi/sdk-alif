/**
 ****************************************************************************************
 *
 * @file lans_msg.h
 *
 * @brief Header file - Location and Navigation Profile Sensor - Message API.
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef _LANS_MSG_H_
#define _LANS_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @addtogroup LANS
 * @ingroup Profile
 * @brief Location and Navigation Profile - Message API.
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


/// Messages for Location and Navigation Profile Sensor
/*@TRACE*/
enum lans_msg_id
{
    /// Start the Location and Navigation Profile Server profile
    LANS_ENABLE_REQ          = MSG_ID(LANS, 0x00),
    /// Confirm profile
    LANS_ENABLE_RSP          = MSG_ID(LANS, 0x01),

    /// Update Position Quality
    LANS_UPD_POS_Q_REQ       = MSG_ID(LANS, 0x02),
    /// Update Position Quality
    LANS_UPD_POS_Q_RSP       = MSG_ID(LANS, 0x03),

    /// Send a Location and Speed to the peer device (Notification)
    LANS_NTF_LOC_SPEED_CMD   = MSG_ID(LANS, 0x04),

    /// Send a LN Vector to the peer device (Notification)
    LANS_NTF_NAVIGATION_CMD  = MSG_ID(LANS, 0x05),

    /// Send a complete event status to the application
    LANS_CMP_EVT             = MSG_ID(LANS, 0x06),

    /// Indicate that an attribute value has been written
    LANS_CFG_NTFIND_IND      = MSG_ID(LANS, 0x07),

    /// Indicate that Control Point characteristic value has been written
    LANS_LN_CTNL_PT_REQ_RECV_IND  = MSG_ID(LANS, 0x08),
    /// Application response after receiving a LANS_CTNL_PT_REQ_IND message
    LANS_LN_CTNL_PT_RSP_SEND_CMD  = MSG_ID(LANS, 0x09),
};

/// Operation Code used in the profile state machine
enum lans_op_code
{
    /// Reserved Operation Code
    LANS_RESERVED_OP_CODE          = 0x00,
    /// Send Location and Speed Operation Code
    LANS_NTF_LOC_SPEED_OP_CODE     = 0x01,
    /// Send Navigation Operation Code
    LANS_NTF_NAVIGATION_OP_CODE    = 0x02,
    /// Send Control point response
    LANS_CTNL_PT_RSP_SEND_OP_CODE  = 0x03,
};

/*
 * STRUCTURES
 ****************************************************************************************
 */

/// Parameters of the @ref LANS_CREATE_DB_REQ message
struct lans_db_cfg
{
    /**
     * LN Feature Value - Not supposed to be modified during the lifetime of the device
     * This bit field is set in order to decide which features are supported:
     */
    uint32_t ln_feature;
    /**
     * Bit 0 - Enable LN Control Point
     * Bit 1 - Enable Navigation Feature
     */
    uint8_t  prfl_config;
};

/// Parameters of the @ref LANS_ENABLE_REQ message
struct lans_enable_req
{
    /// Connection index
    uint8_t  conidx;
    /**
     * Profile characteristic configuration:
     *   Location and speed Characteristic NTF config -- Bit 0
     *   LN Control Point Characteristic IND config ---- Bit 1
     *   Navigation Characteristic NTF config ---------- Bit 2
     */
    uint16_t prfl_ntf_ind_cfg;
};

/// Parameters of the @ref LANS_ENABLE_RSP message
struct lans_enable_rsp
{
    /// Connection index
    uint8_t  conidx;
    /// status
    uint16_t status;
};

/// Parameters of the @ref LANS_CMP_EVT message
struct lans_cmp_evt
{
    /// Connection index
    uint8_t  conidx;
    /// Operation Code
    uint8_t  operation;
    /// Operation Status
    uint16_t status;
};

/// Parameters of the @ref LANS_CFG_NTFIND_IND message
struct lans_cfg_ntfind_ind
{
    /// Connection index
    uint8_t  conidx;
    /// Characteristic Code (Location and speed, Control Point or navigation)
    uint8_t  char_code;
    /// Char. Client Characteristic Configuration
    uint16_t ntf_cfg;
};

/// Parameters of the @ref LANS_NTF_LOC_SPEED_CMD message
struct lans_ntf_loc_speed_cmd
{
    /// Bit field of connection index on which measurement will be send
    /// will be mask according to client that supports reception
    uint32_t         conidx_bf;
    ///Parameters
    lanp_loc_speed_t parameters;
};

/// Parameters of the @ref LANS_NTF_NAVIGATION_CMD message
struct lans_ntf_navigation_cmd
{
    /// Bit field of connection index on which measurement will be send
    /// will be mask according to client that supports reception
    uint32_t          conidx_bf;
    ///Parameters
    lanp_navigation_t parameters;
};

/// Parameters of the @ref LANS_UPD_POS_Q_REQ message
struct lans_upd_pos_q_req
{
    ///Parameters
    lanp_posq_t parameters;
};


/// Parameters of the @ref LANS_UPD_POS_Q_REQ message
struct lans_upd_pos_q_rsp
{
    /// Status
    uint16_t status;
};


/// Parameters of the @ref LANS_LN_CTNL_PT_REQ_RECV_IND message
struct lans_ln_ctnl_pt_req_recv_ind
{
    /// Connection index
    uint8_t                       conidx;
    /// Operation Code see enum #lanp_ln_ctnl_pt_code
    uint8_t                       op_code;
    /// Control point value request
    union lanp_ln_ctnl_pt_req_val value;
};

/// Parameters of the @ref LANS_LN_CTNL_PT_RSP_SEND_CMD message
struct lans_ln_ctnl_pt_rsp_send_cmd
{
    /// Connection index
    uint8_t                    conidx;
    /// Operation code   (see enum #lanp_ln_ctnl_pt_code)
    uint8_t                    req_op_code;
    /// Response Value   (see enum #lanp_ctnl_pt_resp_val)
    uint8_t                    resp_value;

    /// Control point value response
    union lanp_ctnl_pt_rsp_val value;
};

/// @} LANS

#endif //(_LANS_MSG_H_)
