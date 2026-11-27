/**
 ****************************************************************************************
 *
 * @file wscc_msg.h
 *
 * @brief Header file - Weight SCale Profile Collector/Client Role - Message API.
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef _WSCC_MSG_H_
#define _WSCC_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @addtogroup WSCC
 * @ingroup Profile
 * @brief  Weight SCale ProfileProfile Collector - Message API
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_task.h" // Task definitions
#include "wsc_common.h"

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Message IDs
/*@TRACE*/
enum wscc_msg_ids
{
    /// Enable the Profile Collector task - at connection
    WSCC_ENABLE_REQ       = MSG_ID(WSCC, 0x00),
    /// Response to Enable the Profile Collector task - at connection
    WSCC_ENABLE_RSP       = MSG_ID(WSCC, 0x01),
   /// Read the Weight Scale Feature
    WSCC_RD_FEAT_CMD      = MSG_ID(WSCC, 0x02),
    /// Read the CCC of a measurement sensor characteristic
    WSCC_RD_MEAS_CCC_CMD  = MSG_ID(WSCC, 0x03),
    /// Write the CCC of a measurement sensor characteristic
    WSCC_WR_MEAS_CCC_CMD  = MSG_ID(WSCC, 0x04),
    /// Read the Weight Scale Feature
    WSCC_FEAT_IND         = MSG_ID(WSCC, 0x05),
    /// Read the CCC of a measurement sensor characteristic
    WSCC_MEAS_CCC_IND     = MSG_ID(WSCC, 0x06),
    /// Characteristic Measurement Indication from peer
    WSCC_MEAS_IND         = MSG_ID(WSCC, 0x07),
    /// Complete Event Information
    WSCC_CMP_EVT          = MSG_ID(WSCC, 0x08),
};


/// Weight Scale Service Characteristics
enum wscc_chars
{
    /// Features
    WSCC_CHAR_WSS_FEATURE,
    /// Measurement
    WSCC_CHAR_WSS_MEAS,

    WSCC_CHAR_WSS_MAX,
};

/// Weight Scale Service Characteristic Descriptors
enum wscc_descs
{
    /// Client config
    WSCC_DESC_WSS_MEAS_CCC,

    WSCC_DESC_WSS_MAX,
};

/// Operation Codes for pending operations in the client.
enum wscc_op_codes
{
    /// Read op-codes
    WSCC_READ_FEAT_OP_CODE = 0,
    /// Read op-codes
    WSCC_READ_CCC_OP_CODE = 1,
    /// Write op-codes.
    WSCC_WRITE_CCC_OP_CODE = 2,
};

/*
 * API MESSAGE STRUCTURES
 ****************************************************************************************
 */

/**
 * Structure containing the characteristics handles, value handles and descriptors for
 * the Weight Scale Service
 */
typedef struct wscc_wss_content
{
    /// service info
    prf_svc_t           svc;

    /// Included service info - Body Composition Service..
    prf_incl_svc_t      incl_svc;

    /// Characteristic info:
    ///  - Feature
    ///  - Measurement
    prf_char_t      chars[WSCC_CHAR_WSS_MAX];

    /// Descriptor handles:
    ///  - Client cfg
    prf_desc_t descs[WSCC_DESC_WSS_MAX];
} wscc_wss_content_t;

/// Parameters of the @ref WSCC_ENABLE_REQ message
struct wscc_enable_req
{
    /// Connection index
    uint8_t             conidx;
    /// Connection type
    uint8_t             con_type;
    /// Existing handle values WSCC ES
    wscc_wss_content_t  wss;
};

/// Parameters of the @ref WSCC_ENABLE_RSP message
struct wscc_enable_rsp
{
    /// Connection index
    uint8_t             conidx;
    /// status
    uint16_t            status;
    /// Existing handle values WSCC ES
    wscc_wss_content_t  wss;
};

/// Parameters of the @ref WSCC_RD_FEAT_CMD message
/// Read the Weight Scale Feature
struct wscc_rd_feat_cmd
{
    /// Connection index
    uint8_t conidx;
};

/// Parameters of the @ref WSCC_RD_MEAS_CCC_CMD message
/// Read the CCC of a measurement sensor characteristic
struct wscc_rd_meas_ccc_cmd
{
    /// Connection index
    uint8_t conidx;
};

/// Parameters of the @ref WSCC_WR_MEAS_CCC_CMD message
/// Write the CCC of a measurement sensor characteristic
struct wscc_wr_meas_ccc_cmd
{
    /// Connection index
    uint8_t  conidx;
    /// Client Characteristic Configuration read value
    uint16_t ccc;
};

/// Parameters of the @ref WSCC_FEAT_IND message
/// Read the Weight Scale Feature
struct wscc_feat_ind
{
    /// Connection index
    uint8_t  conidx;
    /// Weight Scale Feature read value
    uint32_t feature;
};

/// Parameters of the @ref WSCC_MEAS_CCC_IND message
/// Read the CCC of a measurement sensor characteristic
struct wscc_meas_ccc_ind
{
    /// Connection index
    uint8_t  conidx;
    /// Client Characteristic Configuration read value
    uint16_t ccc;
};

/// Parameters of the @ref WSCC_MEAS_IND message
/// Characteristic Measurement Indication from peer
struct wscc_meas_ind
{
    /// Connection index
    uint8_t         conidx;
    /// Presence of optional fields and measurement units
    uint8_t         flags;
    /// Weight value
    uint16_t        weight;
    /// Weight Resolution
    uint8_t         wght_resol;
    /// Measurement Units
    uint8_t         meas_u;
    /// Time stamp value
    prf_date_time_t time_stamp;
    /// User ID value
    uint8_t         user_id;
    /// Body Mass Index
    uint16_t        bmi;
    /// Height value
    uint16_t        height;
    /// Height Resolution
    uint8_t         hght_resol;
};

/// Parameters of the @ref WSCC_CMP_EVT message
struct wscc_cmp_evt
{
    /// Connection index
    uint8_t  conidx;
    /// Operation code
    uint8_t  operation;
    /// Status
    uint16_t status;
};

/// @} WSCC

#endif //(_WSCC_MSG_H_)
