/**
 ****************************************************************************************
 *
 * @file gatt_db.h
 *
 * @brief Header file - GATT Attribute Database Native API
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef GATT_DB_H_
#define GATT_DB_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup GATT_DB_API Database Management
 * @ingroup GATT_API
 * @brief Attribute database creation and manipulation
 *
 * @defgroup GATT_DB_DEFINE_API Defines / macros
 * @ingroup GATT_DB_API
 *
 * @defgroup GATT_DB_ENUM_API Enumerations
 * @ingroup GATT_DB_API
 *
 * @defgroup GATT_DB_TYPE_API Types definitions
 * @ingroup GATT_DB_API
 *
 * @defgroup GATT_DB_ADD_SVC_API Add/remove services
 * @ingroup GATT_DB_API
 *
 * @defgroup GATT_DB_SVC_MGMT_API Manipulate services properties
 * @ingroup GATT_DB_API
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "gatt.h"

#if (GATT_CACHING_SUPPORT)
/// @defgroup GATT_DB_HASH_API Database hash
/// @ingroup GATT_DB_API
#endif // (GATT_CACHING_SUPPORT)

/*
 * MACROS
 ****************************************************************************************
 */

/// @addtogroup GATT_DB_DEFINE_API
/// @{

/// Helper to define an attribute option bit
/// @param opt see #gatt_att_info_bf enumeration or see #gatt_att_ext_info_bf enumeration
#define OPT(opt) (GATT_ATT_##opt##_BIT)

/// Helper to set attribute security level on a specific permission
/// @param  lvl_name Security level see #gap_sec_lvl enumeration
/// @param  perm     Permission see #gatt_att_info_bf enumeration (only RP, WP, NIP authorized)
#define SEC_LVL(perm, lvl_name) (((GAP_SEC_##lvl_name) << (GATT_ATT_##perm##_LSB)) & (GATT_ATT_##perm##_MASK))

/// Helper to set attribute security level on a specific permission
/// @param  lvl_val  Security level value
/// @param  perm     Permission see #gatt_att_info_bf enumeration (only RP, WP, NIP authorized)
#define SEC_LVL_VAL(perm, lvl_val) (((lvl_val) << (GATT_ATT_##perm##_LSB)) & (GATT_ATT_##perm##_MASK))

/// Helper to set attribute UUID type
/// @param uuid_type UUID type (16, 32, 128)
#define ATT_UUID(uuid_type) (((GATT_UUID_##uuid_type) << (GATT_ATT_UUID_TYPE_LSB)) & (GATT_ATT_UUID_TYPE_MASK))

/// Helper to set service security level
/// @param  lvl_name Security level see #gap_sec_lvl enumeration
#define SVC_SEC_LVL(lvl_name) (((GAP_SEC_##lvl_name) << (GATT_SVC_SEC_LVL_LSB)) & (GATT_SVC_SEC_LVL_MASK))
/// Helper to set service security level
/// @param  lvl_val Security level value
#define SVC_SEC_LVL_VAL(lvl_val) (((lvl_val) << (GATT_SVC_SEC_LVL_LSB)) & (GATT_SVC_SEC_LVL_MASK))

/// Helper to set service UUID type
/// @param uuid_type UUID type (16, 32, 128)
#define SVC_UUID(uuid_type) (((GATT_UUID_##uuid_type) << (GATT_SVC_UUID_TYPE_LSB)) & (GATT_SVC_UUID_TYPE_MASK))

/// @} GATT_DB_DEFINE_API

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// @addtogroup GATT_DB_ENUM_API
/// @{

/// @verbatim
///   7      6     5     4      3     2    1   0
/// +-----+-----+-----+------+-----+-----+---+---+
/// | RFU | UUID_TYPE | HIDE | DIS | EKS |SEC_LVL|
/// +-----+-----+-----+------+-----+-----+---+---+
/// @endverbatim
/// GATT Service information Bit Field
enum gatt_svc_info_bf
{
    /// Service minimum required security level (see #gap_sec_lvl enumeration).
    GATT_SVC_SEC_LVL_MASK = 0x03,
    GATT_SVC_SEC_LVL_LSB = 0,
    /// If set, access to value with encrypted security requirement also requires a 128-bit encryption key size.
    GATT_SVC_EKS_BIT = 0x04,
    GATT_SVC_EKS_POS = 2,
    /// If set, service is visible but cannot be used by peer device
    GATT_SVC_DIS_BIT = 0x08,
    GATT_SVC_DIS_POS = 3,
    /// Hide the service
    GATT_SVC_HIDE_BIT = 0x10,
    GATT_SVC_HIDE_POS = 4,
    /// Type of service UUID (see #gatt_uuid_type enumeration)
    GATT_SVC_UUID_TYPE_MASK = 0x60,
    GATT_SVC_UUID_TYPE_LSB = 5,
};

/// @verbatim
///    15   14    13  12 11 10  9  8   7    6    5   4   3    2    1    0
/// +-----+-----+---+---+--+--+--+--+-----+----+---+---+----+----+----+---+
/// | UUID_TYPE |  NIP  |  WP |  RP | EXT | WS | I | N | WR | WC | RD | B |
/// +-----+-----+---+---+--+--+--+--+-----+----+---+---+----+----+----+---+
///                                  <--------------- PROP -------------->
/// @endverbatim
/// GATT Attribute information Bit Field
enum gatt_att_info_bf
{
    /// Broadcast descriptor present
    GATT_ATT_B_BIT = 0x0001,
    GATT_ATT_B_POS = 0,
    /// Read Access Mask
    GATT_ATT_RD_BIT = 0x0002,
    GATT_ATT_RD_POS = 1,
    /// Write Command Enabled attribute Mask
    GATT_ATT_WC_BIT = 0x0004,
    GATT_ATT_WC_POS = 2,
    /// Write Request Enabled attribute Mask
    GATT_ATT_WR_BIT = 0x0008,
    GATT_ATT_WR_POS = 3,
    /// Notification Access Mask
    GATT_ATT_N_BIT = 0x0010,
    GATT_ATT_N_POS = 4,
    /// Indication Access Mask
    GATT_ATT_I_BIT = 0x0020,
    GATT_ATT_I_POS = 5,
    /// Write Signed Enabled attribute Mask
    GATT_ATT_WS_BIT = 0x0040,
    GATT_ATT_WS_POS = 6,
    /// Extended properties descriptor present
    GATT_ATT_EXT_BIT = 0x0080,
    GATT_ATT_EXT_POS = 7,
    /// Read security level permission (see #gap_sec_lvl enumeration).
    GATT_ATT_RP_MASK = 0x0300,
    GATT_ATT_RP_LSB = 8,
    /// Write security level permission (see #gap_sec_lvl enumeration).
    GATT_ATT_WP_MASK = 0x0C00,
    GATT_ATT_WP_LSB = 10,
    /// Notify and Indication security level permission (see #gap_sec_lvl enumeration).
    GATT_ATT_NIP_MASK = 0x3000,
    GATT_ATT_NIP_LSB = 12,
    /// Type of attribute UUID (see #gatt_uuid_type enumeration)
    GATT_ATT_UUID_TYPE_MASK = 0xC000,
    GATT_ATT_UUID_TYPE_LSB = 14,

    /// Attribute value property
    GATT_ATT_PROP_MASK = 0x00FF,
    GATT_ATT_PROP_LSB = 0,
};

/// @verbatim
///       15     14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
/// +-----------+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
/// | NO_OFFSET |               WRITE_MAX_SIZE               |
/// +-----------+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
/// |                     INC_SVC_HANDLE                     |
/// +-----------+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
/// |                     EXT_PROP_VALUE                     |
/// +-----------+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
/// @endverbatim
///
/// GATT Attribute extended information Bit Field
enum gatt_att_ext_info_bf
{
    /// Maximum value authorized for an attribute write.
    /// Automatically reduce to Maximum Attribute value (GATT_MAX_VALUE) if greater
    GATT_ATT_WRITE_MAX_SIZE_MASK = 0x7FFF,
    GATT_ATT_WRITE_MAX_SIZE_LSB = 0,
    /// 1: Do not authorize peer device to read or write an attribute with an offset != 0
    /// 0: Authorize offset usage
    GATT_ATT_NO_OFFSET_BIT = 0x8000,
    GATT_ATT_NO_OFFSET_POS = 15,
    /// Include Service handle value
    GATT_INC_SVC_HDL_BIT = 0xFFFF,
    GATT_INC_SVC_HDL_POS = 0,
    /// Characteristic Extended Properties value
    GATT_ATT_EXT_PROP_VALUE_MASK = 0xFFFF,
    GATT_ATT_EXT_PROP_VALUE_LSB = 0,
};

/// GATT Service Visibility
enum gatt_svc_visibility
{
    /// Service hidden
    GATT_SVC_HIDE = 0x00,
    /// Service visible
    GATT_SVC_VISIBLE = 0x01,
};

/// @} GATT_DB_ENUM_API

/*
 * STRUCTURES
 ****************************************************************************************
 */

/// @addtogroup GATT_DB_TYPE_API
/// @{

/// Attribute Description structure
/*@TRACE*/
typedef struct
{
    /// Attribute UUID (LSB First)
    uint8_t uuid[GATT_UUID_128_LEN];
    /// Attribute information bit field (see #gatt_att_info_bf enumeration)
    uint16_t info;
    /// Attribute extended information bit field (see #gatt_att_ext_info_bf enumeration)
    /// Note:
    ///   - For Included Services and Characteristic Declarations, this field contains targeted handle.
    ///   - For Characteristic Extended Properties, this field contains 2 byte value
    ///   - For Client Characteristic Configuration and Server Characteristic Configuration, this field is not used.
    uint16_t ext_info;
} gatt_att_desc_t;

/// 16-bit UUID Attribute Description structure
typedef struct
{
    /// Attribute UUID (16-bit UUID - LSB First)
    uint16_t uuid16;
    /// Attribute information bit field (see #gatt_att_info_bf enumeration)
    uint16_t info;
    /// Attribute extended information bit field (see #gatt_att_ext_info_bf enumeration)
    /// Note:
    ///   - For Included Services and Characteristic Declarations, this field contains targeted handle.
    ///   - For Characteristic Extended Properties, this field contains 2 byte value
    ///   - For Client Characteristic Configuration and Server Characteristic Configuration, this field is not used.
    uint16_t ext_info;
} gatt_att16_desc_t;

/// Service Description structure
/*@TRACE*/
typedef struct
{
    /// GATT User Local identifier
    uint8_t user_lid;
    /// Service Information bit field (see #gatt_svc_info_bf enumeration)
    uint8_t info;
    /// Service UUID (LSB first)
    uint8_t uuid[GATT_UUID_128_LEN];
    ///  Attribute Start Handle
    uint16_t start_hdl;
    /// Attribute End Handle
    uint16_t end_hdl;
} gatt_svc_desc_t;

/// @} GATT_DB_TYPE_API

/*
 * DATABASE MANAGEMENT API - CALLBACKS
 ****************************************************************************************
 */

#if (GATT_CACHING_SUPPORT)
/// @addtogroup GATT_DB_HASH_API
/// @{

/// GATT Database Hash callback set
typedef struct
{
    /**
     ****************************************************************************************
     * @brief This function is called when hash value for local attribute database hash has
     *        been computed.
     *
     * @param[in] conidx        Connection index
     * @param[in] user_lid      GATT user local identifier
     * @param[in] metainfo      Metadata information provided by API user (see \glos{METAINFO})
     * @param[in] status        Status of the operation (see #hl_err enumeration)
     * @param[in] p_hash        Pointer to the 128-bit database hash value
     ****************************************************************************************
     */
    void (*cb_db_hash) (uint8_t conidx, uint8_t user_lid, uint16_t metainfo, uint16_t status, const uint8_t* p_hash);
} gatt_db_hash_cb_t;

/// @} GATT_DB_HASH_API
#endif // (GATT_CACHING_SUPPORT)

/*
 * DATABASE MANAGEMENT API - FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

/// @addtogroup GATT_DB_ADD_SVC_API
/// @{

/**
 ****************************************************************************************
 * @brief Function used to add a service into local attribute database.
 *
 * Service and attributes UUIDs in service must be 16-bit
 *
 * If start handle is set to zero (invalid attribute handle), GATT looks for a
 * free handle block matching with number of attributes to reserve.
 * Else, according to start handle, GATT checks if attributes to reserve are
 * not overlapping part of existing database.
 *
 * An added service is automatically visible for peer device.
 *
 * @note First attribute in attribute array must be a Primary or a Secondary service
 *
 * @param[in]     user_lid     GATT User Local identifier
 * @param[in]     info         Service Information bit field (see #gatt_svc_info_bf enumeration)
 * @param[in]     uuid16       Service UUID (16-bit UUID - LSB First)
 * @param[in]     nb_att       Number of attribute(s) in service
 * @param[in]     p_att_mask   Pointer to mask of attribute to insert in database:
 *                               - If NULL insert all attributes
 *                               - If bit set to 1: attribute inserted
 *                               - If bit set to 0: attribute not inserted
 * @param[in]     p_atts       Pointer to List of attribute (with 16-bit uuid) description present in service.
 * @param[in]     nb_att_rsvd  Number of attribute(s) reserved for the service (if not 0, shall be equals or greater
 *                             than nb_att)
 *                             Prevent any services to be inserted between start_hdl and (start_hdl + nb_att_rsvd - 1)
 * @param[in,out] p_start_hdl  Pointer to Service Start Handle (0 = chosen by GATT module)
 *                             Pointer updated with service start handle associated to
 *                             created service.
 *
 * @return Status of the function execution (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t gatt_db_svc16_add(uint8_t user_lid, uint8_t info, uint16_t uuid16, uint8_t nb_att, const uint8_t* p_att_mask,
                           const gatt_att16_desc_t* p_atts, uint8_t nb_att_rsvd, uint16_t* p_start_hdl);

/**
 ****************************************************************************************
 * @brief Function used to add a service into local attribute database.
 *
 * If start handle is set to zero (invalid attribute handle), GATT looks for a
 * free handle block matching with number of attributes to reserve.
 * Else, according to start handle, GATT checks if attributes to reserve are
 * not overlapping part of existing database.
 *
 * An added service is automatically visible for peer device.
 *
 * @note First attribute in attribute array must be a Primary or a Secondary service
 *
 * @param[in]     user_lid     GATT User Local identifier
 * @param[in]     info         Service Information bit field (see #gatt_svc_info_bf enumeration)
 * @param[in]     p_uuid       Pointer to service UUID (LSB first)
 * @param[in]     nb_att       Number of attribute(s) in service
 * @param[in]     p_att_mask   Pointer to mask of attribute to insert in database:
 *                               - If NULL insert all attributes
 *                               - If bit set to 1: attribute inserted
 *                               - If bit set to 0: attribute not inserted
 * @param[in]     p_atts       Pointer to List of attribute description present in service.
 * @param[in]     nb_att_rsvd  Number of attribute(s) reserved for the service (shall be equals or greater nb_att)
 *                             Prevent any services to be inserted between start_hdl and (start_hdl + nb_att_rsvd -1)
 * @param[in,out] p_start_hdl  Pointer to Service Start Handle (0 = chosen by GATT module)
 *                             Pointer updated with service start handle associated to
 *                             created service.
 *
 * @return Status of the function execution (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t gatt_db_svc_add(uint8_t user_lid, uint8_t info, const uint8_t* p_uuid, uint8_t nb_att,
                         const uint8_t* p_att_mask, const gatt_att_desc_t* p_atts, uint8_t nb_att_rsvd,
                         uint16_t* p_start_hdl);

/**
 ****************************************************************************************
 * @brief Function used to remove a service from local attribute database.
 *
 * Only GATT user responsible of service can remove it.
 *
 * @param[in]  user_lid     GATT User Local identifier
 * @param[in]  start_hdl    Service Start Handle
 *
 * @return Status of the function execution (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t gatt_db_svc_remove(uint8_t user_lid, uint16_t start_hdl);

/**
 ****************************************************************************************
 * @brief Function use to verify if several services can be allocated on a contiguous
 *        handle range.
 *
 * @param[in]     user_lid     GATT User Local identifier
 * @param[in]     nb_att       Number of attribute(s) to reserve
 * @param[in,out] p_start_hdl  Pointer to Service Start Handle (0 = chosen by GATT module)
 *                             Pointer updated with service start handle associated to
 *                             first attribute range available.
 *
 * @return Status of the function execution (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t gatt_db_handle_range_reserve(uint8_t user_lid, uint8_t nb_att, uint16_t* p_start_hdl);

/// @} GATT_DB_ADD_SVC_API

/// @addtogroup GATT_DB_SVC_MGMT_API
/// @{

/**
 ****************************************************************************************
 * @brief Function used to control visibility and usage authorization of a local service.
 *        A hidden service is present in database but cannot be discovered or manipulated
 *        by a peer device.
 *        A disabled service can be discovered by a peer device but it is not authorized to
 *        use it.
 *
 * Only GATT user responsible of service can update its properties
 *
 * @param[in]  user_lid     GATT User Local identifier
 * @param[in]  start_hdl    Service Start Handle
 * @param[in]  enable       True: Authorize usage of the service
 *                          False: reject usage of the service
 * @param[in]  visible      Service visibility (see #gatt_svc_visibility enumeration)
 *
 * @return Status of the function execution (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t gatt_db_svc_ctrl(uint8_t user_lid, uint16_t start_hdl, uint8_t enable, uint8_t visible);

/**
 ****************************************************************************************
 * @brief Function used to retrieve information of an attribute.
 *
 * @param[in]  user_lid     GATT User Local identifier
 * @param[in]  hdl          Attribute Handle
 * @param[out] p_info       Attribute information bit field
 *                          (see #gatt_att_info_bf enumeration)
 *
 * @return Status of the function execution (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t gatt_db_att_info_get(uint8_t user_lid, uint16_t hdl, uint16_t* p_info);

/**
 ****************************************************************************************
 * @brief Function used to set information of an attribute.
 *
 * @param[in]  user_lid     GATT User Local identifier
 * @param[in]  hdl          Attribute Handle
 * @param[in]  info         Attribute information bit field (see #gatt_att_info_bf enumeration)
 *
 * @return Status of the function execution (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t gatt_db_att_info_set(uint8_t user_lid, uint16_t hdl, uint16_t info);
/// @} GATT_DB_SVC_MGMT_API

#if (GATT_CACHING_SUPPORT)
/// @addtogroup GATT_DB_HASH_API
/// @{

/**
 ****************************************************************************************
 * @brief Function used to retrieve (or compute) the local database hash value.
 *
 * @param[in]  conidx   Connection index (not used but returned as requested)
 * @param[in]  user_lid GATT User Local identifier (must be a server user)
 * @param[in]  metainfo Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in]  p_cb     Callback where database hash is returned
 *
 * @return Status of the function execution (see #hl_err enumeration)
 *         Consider status only if an error occurs; else wait for execution completion
 ****************************************************************************************
 */
uint16_t gatt_db_hash_get(uint8_t conidx, uint8_t user_lid, uint16_t metainfo, const gatt_db_hash_cb_t* p_cb);

/// @} GATT_DB_HASH_API
#endif // (GATT_CACHING_SUPPORT)

#endif /* GATT_DB_H_ */
