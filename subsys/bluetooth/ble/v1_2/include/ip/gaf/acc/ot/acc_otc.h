/**
 ****************************************************************************************
 *
 * @file acc_otc.h
 *
 * @brief Audio Content Control - Object Transfer Client - Definitions
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef ACC_OTC_H_
#define ACC_OTC_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup ACC_OTC Object Transfer Service Client
 * @ingroup ACC_OT
 * @brief Description of Object Transfer Service Client module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ACC_OTC_ENUM Enumerations
 * @ingroup ACC_OTC
 * @brief Enumerations for Object Transfer Service Client module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ACC_OTC_NATIVE_API Native API
 * @ingroup ACC_OTC
 * @brief Description of Native API for Object Transfer Service Client module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ACC_OTC_CALLBACK Callback Functions
 * @ingroup ACC_OTC_NATIVE_API
 * @brief Description of callback functions for Object Transfer Service Client module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ACC_OTC_FUNCTION Functions
 * @ingroup ACC_OTC_NATIVE_API
 * @brief Description of functions for Object Transfer Service Client module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "acc_ot.h"            // Audio Content Control - Object Tranfer Definitions

#if (GAF_ACC_OTC)

#include "gaf.h"               // GAF Defines
#include "otc.h"               // Object Transfer Client Definitions

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// @addtogroup ACC_OTC_ENUM
/// @{

/// List of ACC_CMD command codes for Object Transfer Client
enum acc_otc_cmd_codes
{
    /// Discover
    ACC_OTC_DISCOVER = GAF_CODE(ACC, OTC, 0),
    /// Get
    ACC_OTC_GET = GAF_CODE(ACC, OTC, 1),
    /// Set Configuration
    ACC_OTC_SET_CFG = GAF_CODE(ACC, OTC, 3),
    /// Set Name
    ACC_OTC_SET_NAME = GAF_CODE(ACC, OTC, 4),
    /// Set Time
    ACC_OTC_SET_TIME = GAF_CODE(ACC, OTC, 5),
    /// Set Properties
    ACC_OTC_SET_PROPERTIES = GAF_CODE(ACC, OTC, 6),
    /// Create Object
    ACC_OTC_OBJECT_CREATE = GAF_CODE(ACC, OTC, 7),
    /// Control Object
    ACC_OTC_OBJECT_CONTROL = GAF_CODE(ACC, OTC, 8),
    /// Manipulate Object
    ACC_OTC_OBJECT_MANIPULATE = GAF_CODE(ACC, OTC, 9),
    /// Exexute Object
    ACC_OTC_OBJECT_EXECUTE = GAF_CODE(ACC, OTC, 10),
    /// List Control
    ACC_OTC_LIST_CONTROL = GAF_CODE(ACC, OTC, 11),
    /// List Goto
    ACC_OTC_LIST_GOTO = GAF_CODE(ACC, OTC, 12),
    /// Filter Set
    ACC_OTC_FILTER_SET = GAF_CODE(ACC, OTC, 13),
    /// Filter Set Time
    ACC_OTC_FILTER_SET_TIME = GAF_CODE(ACC, OTC, 14),
    /// Filter Set Size
    ACC_OTC_FILTER_SET_SIZE = GAF_CODE(ACC, OTC, 15),
    /// Filter Set Name
    ACC_OTC_FILTER_SET_NAME = GAF_CODE(ACC, OTC, 16),
    /// Filter Set Type
    ACC_OTC_FILTER_SET_TYPE = GAF_CODE(ACC, OTC, 17),
    /// Connection Oriented Channel - Connect
    ACC_OTC_COC_CONNECT = GAF_CODE(ACC, OTC, 18),
    /// Connection Oriented Channel - Disconnect
    ACC_OTC_COC_DISCONNECT = GAF_CODE(ACC, OTC, 19),
    /// Connection Oriented Channel - Send
    ACC_OTC_COC_SEND = GAF_CODE(ACC, OTC, 20),
    /// Connection Oriented Channel - Release
    ACC_OTC_COC_RELEASE = GAF_CODE(ACC, OTC, 21),
};

/// @} ACC_OTC_ENUM

/*
 * CALLBACK SET DEFINITION
 ****************************************************************************************
 */

/// @addtogroup ACC_OTC_CALLBACK
/// @{

/// Set of callback functions for Object Transfer Client
typedef otc_cb_t acc_otc_cb_t;

/// @} ACC_OTC_CALLBACK

/*
 * API FUNCTIONS DECLARATION
 ****************************************************************************************
 */

/// @addtogroup ACC_OTC_FUNCTION
/// @{

/**
 ****************************************************************************************
 * @brief Create and configure Object Transfer Client module
 *
 * @param[in] p_cb      Pointer to set of callback functions for communications with
 * upper layers
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t acc_otc_configure(const acc_otc_cb_t* p_cb);

__STATIC_FORCEINLINE uint16_t acc_otc_discover(uint8_t con_lid, uint8_t nb_ots_max, uint8_t svc_type, uint16_t shdl,
                                   uint16_t ehdl)
{
    return (otc_discover(con_lid, nb_ots_max, svc_type, shdl, ehdl));
}

#if (GAF_DBG)
__STATIC_FORCEINLINE uint16_t acc_otc_get(uint8_t con_lid, uint8_t transfer_lid, uint8_t get_type, uint8_t char_type)
{
    return (otc_get(con_lid, transfer_lid, get_type, char_type));
}

__STATIC_FORCEINLINE uint16_t acc_otc_set_cfg(uint8_t con_lid, uint8_t transfer_lid, uint8_t char_type, uint8_t enable)
{
    return (otc_set_cfg(con_lid, transfer_lid, char_type, enable));
}

#endif //(GAF_DBG)
__STATIC_FORCEINLINE uint16_t acc_otc_set_name(uint8_t con_lid, uint8_t transfer_lid, uint8_t name_len, const uint8_t* p_name)
{
    return (otc_set_name(con_lid, transfer_lid, name_len, p_name));
}

__STATIC_FORCEINLINE uint16_t acc_otc_set_time(uint8_t con_lid, uint8_t transfer_lid, uint8_t char_type,
                                   const prf_date_time_t* p_time)
{
    return (otc_set_time(con_lid, transfer_lid, char_type, p_time));
}

__STATIC_FORCEINLINE uint16_t acc_otc_set_properties(uint8_t con_lid, uint8_t transfer_lid, uint32_t properties)
{
    return (otc_set_properties(con_lid, transfer_lid, properties));
}

__STATIC_FORCEINLINE uint16_t acc_otc_object_create(uint8_t con_lid, uint8_t transfer_lid, uint32_t size, uint8_t uuid_type,
                                        const void* p_uuid)
{
    return (otc_object_create(con_lid, transfer_lid, size, uuid_type, p_uuid));
}

__STATIC_FORCEINLINE uint16_t acc_otc_object_control(uint8_t con_lid, uint8_t transfer_lid, uint8_t opcode)
{
    return (otc_object_control(con_lid, transfer_lid, opcode));
}

__STATIC_FORCEINLINE uint16_t acc_otc_object_manipulate(uint8_t con_lid, uint8_t transfer_lid, uint8_t opcode,
                                            uint32_t offset, uint32_t length, uint8_t mode)
{
    return (otc_object_manipulate(con_lid, transfer_lid, opcode, offset, length, mode));
}

__STATIC_FORCEINLINE uint16_t acc_otc_object_execute(uint8_t con_lid, uint8_t transfer_lid, uint16_t param_len,
                                         const uint8_t* p_param)
{
    return (otc_object_execute(con_lid, transfer_lid, param_len, p_param));
}

__STATIC_FORCEINLINE uint16_t acc_otc_list_control(uint8_t con_lid, uint8_t transfer_lid, uint8_t opcode, uint8_t order)
{
    return (otc_list_control(con_lid, transfer_lid, opcode, order));
}

__STATIC_FORCEINLINE uint16_t acc_otc_list_goto(uint8_t con_lid, uint8_t transfer_lid, uint8_t opcode,
                                    const ot_object_id_t* p_object_id)
{
    return (otc_list_goto(con_lid, transfer_lid, opcode, p_object_id));
}

__STATIC_FORCEINLINE uint16_t acc_otc_filter_set(uint8_t con_lid, uint8_t transfer_lid, uint8_t filter_lid, uint8_t filter_val)
{
    return (otc_filter_set(con_lid, transfer_lid, filter_lid, filter_val));
}

__STATIC_FORCEINLINE uint16_t acc_otc_filter_set_time(uint8_t con_lid, uint8_t transfer_lid, uint8_t filter_lid,
                                          uint8_t filter_val, const prf_date_time_t* p_time_start,
                                          const prf_date_time_t* p_time_end)
{
    return (otc_filter_set_time(con_lid, transfer_lid, filter_lid, filter_val, p_time_start, p_time_end));
}

__STATIC_FORCEINLINE uint16_t acc_otc_filter_set_size(uint8_t con_lid, uint8_t transfer_lid, uint8_t filter_lid,
                                          uint8_t filter_val, uint32_t size_min, uint32_t size_max)
{
    return (otc_filter_set_size(con_lid, transfer_lid, filter_lid, filter_val, size_min, size_max));
}

__STATIC_FORCEINLINE uint16_t acc_otc_filter_set_name(uint8_t con_lid, uint8_t transfer_lid, uint8_t filter_lid,
                                          uint8_t filter_val, uint8_t name_len, const uint8_t* p_name)
{
    return (otc_filter_set_name(con_lid, transfer_lid, filter_lid, filter_val, name_len, p_name));
}

__STATIC_FORCEINLINE uint16_t acc_otc_filter_set_type(uint8_t con_lid, uint8_t transfer_lid, uint8_t filter_lid,
                                          uint8_t uuid_type, const void* p_uuid)
{
    return (otc_filter_set_type(con_lid, transfer_lid, filter_lid, uuid_type, p_uuid));
}

__STATIC_FORCEINLINE uint16_t acc_otc_coc_connect(uint8_t con_lid, uint16_t local_max_sdu)
{
    return (otc_coc_connect(con_lid, local_max_sdu));
}

__STATIC_FORCEINLINE uint16_t acc_otc_coc_disconnect(uint8_t con_lid)
{
    return (otc_coc_disconnect(con_lid));
}

__STATIC_FORCEINLINE uint16_t acc_otc_coc_send(uint8_t con_lid, uint16_t length, const uint8_t* p_sdu)
{
    return (otc_coc_send(con_lid, length, p_sdu));
}

__STATIC_FORCEINLINE uint16_t acc_otc_coc_release(uint8_t con_lid)
{
    return (otc_coc_release(con_lid));
}

__STATIC_FORCEINLINE uint16_t acc_otc_restore_bond_data(uint8_t con_lid, uint8_t nb_ots, const otc_ots_info_t* p_ots_info)
{
    return (otc_restore_bond_data(con_lid, nb_ots, p_ots_info));
}

/// @} ACC_OTC_FUNCTION
#endif //(GAF_ACC_OTC)

#endif // ACC_OTC_H_
