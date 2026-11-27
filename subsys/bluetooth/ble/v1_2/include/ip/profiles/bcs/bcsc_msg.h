/**
 ****************************************************************************************
 *
 * @file bcsc_msg.h
 *
 * @brief Header file - Body Composition Service Collector/Client Role - Message API
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 * $ Rev $
 *
 ****************************************************************************************
 */

#ifndef _BCSC_MSG_H_
#define _BCSC_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @addtogroup BCSC
 * @ingroup Profile
 * @brief  Body Composition Service Collector/Client Role - Message API
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_task.h" // Task definitions
#include "prf_types.h"
#include "bcs_common.h"

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Message IDs
/*@TRACE*/
enum bcsc_msg_ids
{
    /// Enable the Profile Collector task - at connection
    BCSC_ENABLE_REQ           = MSG_ID(BCSC, 0x00),
    /// Response to Enable the Profile Collector task - at connection
    BCSC_ENABLE_RSP           = MSG_ID(BCSC, 0x01),
    /// Read the Body Composition Feature
    BCSC_RD_FEAT_CMD          = MSG_ID(BCSC, 0x02),
    /// Read the CCC of a measurement sensor characteristic
    BCSC_RD_MEAS_CCC_CMD      = MSG_ID(BCSC, 0x03),
    /// Write the CCC of a measurement sensor characteristic
    BCSC_WR_MEAS_CCC_CMD      = MSG_ID(BCSC, 0x04),
    /// Read the Body Composition Feature
    BCSC_FEAT_IND             = MSG_ID(BCSC, 0x05),
    /// Read the CCC of a measurement sensor characteristic
    BCSC_MEAS_CCC_IND         = MSG_ID(BCSC, 0x06),
    /// Characteristic Measurement Indication from peer
    BCSC_MEAS_IND             = MSG_ID(BCSC, 0x07),
    /// Complete Event Information
    BCSC_CMP_EVT              = MSG_ID(BCSC, 0x08),
};

/// Body Composition Service Characteristics
enum bcsc_chars
{
    /// Body Composition Feature
    BCSC_CHAR_BCS_FEATURE,
    /// Body Composition Measurement
    BCSC_CHAR_BCS_MEAS,

    BCSC_CHAR_BCS_MAX,
};

/// Body Composition Service Characteristic Descriptors
enum bcsc_descs
{
    /// Client config
    BCSC_DESC_BCS_MEAS_CCC,

    BCSC_DESC_BCS_MAX,
};


/// Operation Codes for pending operations in the client.
enum bcsc_op_codes
{
    /// Read feature operation code
    BCSC_READ_FEAT_OP_CODE = 0,
    /// Read CCC operation code
    BCSC_READ_CCC_OP_CODE  = 1,
    /// Write CCC operation codes
    BCSC_WRITE_CCC_OP_CODE = 2,
};

/*
 * API MESSAGE STRUCTURES
 ****************************************************************************************
 */

/**
 * Structure containing the characteristics handles, value handles and descriptors for
 * the Body Composition Service
 */
typedef struct bcsc_bcs_content
{
    /// service info
    prf_svc_t           svc;

    /// Characteristic info:
    ///  - Feature
    ///  - Measurement
    prf_char_t      chars[BCSC_CHAR_BCS_MAX];

    /// Descriptor handles:
    ///  - Client cfg
    prf_desc_t descs[BCSC_DESC_BCS_MAX];
} bcsc_bcs_content_t;

/// Parameters of the @ref BCSC_ENABLE_REQ message
struct bcsc_enable_req
{
    /// Connection Index
    uint8_t            conidx;
    /// Connection type
    uint8_t            con_type;

    /// Primary or Secondary Service
    uint8_t            svc_type;

    /// If Secondary Service - the Start and End Handle for the service are contained
    /// in the bcs.

    /// Existing handle values of BCS
    bcsc_bcs_content_t bcs;

};

/// Parameters of the @ref BCSC_ENABLE_RSP message
struct bcsc_enable_rsp
{
    /// Connection Index
    uint8_t            conidx;
    /// status
    uint16_t           status;
    ///  handle values of BCS
    bcsc_bcs_content_t bcs;
};

/// Parameters of the @ref BCSC_RD_FEAT_CMD message
/// Read the Body Composition Feature
struct bcsc_rd_feat_cmd
{
    /// Connection Index
    uint8_t  conidx;
};

/// Parameters of the @ref BCSC_RD_MEAS_CCC_CMD message
/// Read the CCC of a measurement sensor characteristic
struct bcsc_rd_meas_ccc_cmd
{
    /// Connection Index
    uint8_t  conidx;
};

/// Parameters of the @ref BCSC_WR_MEAS_CCC_CMD message
/// Write the CCC of a measurement sensor characteristic
struct bcsc_wr_meas_ccc_cmd
{
    /// Connection Index
    uint8_t  conidx;
    /// Client Characteristic Configuration written value
    uint16_t ccc;
};

/// Parameters of the @ref BCSC_FEAT_IND message
/// Read the Body Composition Feature
struct bcsc_feat_ind
{
    /// Connection Index
    uint8_t  conidx;
    /// Body Composition Feature read value
    uint32_t feature;
};

/// Parameters of the @ref BCSC_MEAS_CCC_IND message
/// Read the CCC of a measurement sensor characteristic
struct bcsc_meas_ccc_ind
{
    /// Connection Index
    uint8_t  conidx;
    /// Client Characteristic Configuration read value
    uint16_t ccc;
};

/// Parameters of the @ref BCSC_MEAS_IND message
/// Characteristic Measurement Indication from peer
struct bcsc_meas_ind
{
    /// Connection Index
    uint8_t         conidx;
    
    /// Presence of optional fields and measurement units
    uint16_t        flags;
    /// Body fat percentage value
    uint16_t        body_fat_percent;
    /// Measurement Units
    uint8_t         meas_u;
    /// Time stamp value
    prf_date_time_t time_stamp;
    /// User ID value
    uint8_t         user_id;
    /// Basal Metabolism value
    uint16_t        basal_metab;
    /// Muscle Percentage value
    uint16_t        muscle_percent;
    /// Muscle Mass value
    uint16_t        muscle_mass;
    /// Fat Free Mass value
    uint16_t        fat_free_mass;
    /// Soft Lean Mass value
    uint16_t        soft_lean_mass;
    /// Body Water Mass value
    uint16_t        body_water_mass;
    /// Impedance Value
    uint16_t        impedance;
    /// Weight value
    uint16_t        weight;
    /// Mass Resolution
    uint8_t         mass_resol;
    /// Height value
    uint16_t        height;
    /// Height Resolution
    uint8_t         hght_resol;
};

/// Parameters of the @ref BCSC_CMP_EVT message
struct bcsc_cmp_evt
{
    /// Connection Index
    uint8_t  conidx;
    /// Operation code
    uint8_t  operation;
    /// Status
    uint16_t status;
};

/// @} BCSC

#endif //(_BCSC_MSG_H_)
