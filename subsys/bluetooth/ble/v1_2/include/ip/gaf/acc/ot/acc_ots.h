/**
 ****************************************************************************************
 *
 * @file acc_ots.h
 *
 * @brief Audio Content Control - Object Transfer Server - Definitions
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef ACC_OTS_H_
#define ACC_OTS_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup ACC_OTS Object Transfer Service Server
 * @ingroup ACC_OT
 * @brief Description of Object Transfer Service Server module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ACC_OTS_ENUM Enumerations
 * @ingroup ACC_OTS
 * @brief Enumerations for Object Transfer Service Server module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ACC_OTS_NATIVE_API Native API
 * @ingroup ACC_OTS
 * @brief Description of Native API for Object Transfer Service Server module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ACC_OTS_CALLBACK Callback Functions
 * @ingroup ACC_OTS_NATIVE_API
 * @brief Description of callback functions for Object Transfer Service Server module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ACC_OTS_FUNCTION Functions
 * @ingroup ACC_OTS_NATIVE_API
 * @brief Description of functions for Object Transfer Service Server module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "acc_ot.h"            // Audio Content Control - Object Transfer Definitions

#if (GAF_ACC_OTS)

#include "gaf.h"               // GAF Defines
#include "ots.h"               // Object Transfer Server Definitions

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// @addtogroup ACC_OTS_ENUM
/// @{

/// List of GAF_CMD command codes for Object Transfer Server
enum acc_ots_cmd_codes
{
    /// Disconnect Channel
    ACC_OTS_COC_DISCONNECT = GAF_CODE(ACC, OTS, 0),
    /// Send on Channel
    ACC_OTS_COC_SEND = GAF_CODE(ACC, OTS, 1),
    /// Release Channel
    ACC_OTS_COC_RELEASE = GAF_CODE(ACC, OTS, 2),
};

/// @} ACC_OTS_ENUM

/*
 * CALLBACK SET DEFINITION
 ****************************************************************************************
 */

/// @addtogroup ACC_OTS_CALLBACK
/// @{

/// Set of callback functions for Object Transfer Server
typedef ots_cb_t acc_ots_cb_t;

/// @} ACC_OTS_CALLBACK

/*
 * API FUNCTIONS DECLARATION
 ****************************************************************************************
 */

/// @addtogroup ACC_OTS_FUNCTION
/// @{

/**
 ****************************************************************************************
 * @brief Create and configure Object Transfer Client module
 *
 * @param[in] nb_transfers      Number of Object Transfer Service instances
 * @param[in] p_cb              Pointer to set of callback functions for communications with
 * upper layers
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t acc_ots_configure(uint8_t nb_transfers, const acc_ots_cb_t* p_cb);

__STATIC_FORCEINLINE uint16_t acc_ots_coc_disconnect(uint8_t con_lid)
{
    return (ots_coc_disconnect(con_lid));
}

__STATIC_FORCEINLINE uint16_t acc_ots_coc_release(uint8_t con_lid)
{
    return (ots_coc_release(con_lid));
}

__STATIC_FORCEINLINE uint16_t acc_ots_coc_send(uint8_t con_lid, uint16_t length, const uint8_t* p_sdu)
{
    return (ots_coc_send(con_lid, length, p_sdu));
}

__STATIC_FORCEINLINE uint16_t acc_ots_add(uint16_t cfg_bf, uint16_t shdl, uint32_t oacp_features, uint32_t olcp_features,
                              uint8_t* p_transfer_lid, uint16_t* p_start_handle)
{
    return (ots_add(cfg_bf, shdl, oacp_features, olcp_features, p_transfer_lid, p_start_handle));
}

__STATIC_FORCEINLINE uint16_t acc_ots_restore_bond_data(uint8_t con_lid, uint8_t transfer_lid, uint8_t cli_cfg_bf,
                                            uint8_t evt_cfg_bf, uint8_t nb_changes,
                                            const ots_changed_info_t *p_changed_info)
{
    return (ots_restore_bond_data(con_lid, transfer_lid, cli_cfg_bf, evt_cfg_bf, nb_changes, p_changed_info));
}

__STATIC_FORCEINLINE uint16_t acc_ots_object_add(const ot_object_id_t * p_object_id, uint32_t current_size,
                                     uint32_t allocated_size, const prf_date_time_t * p_first_created_time,
                                     const prf_date_time_t* p_last_modified_time, uint32_t properties,
                                     uint8_t uuid_type, const void* p_uuid, uint8_t* p_object_lid)
{
    return (ots_object_add(p_object_id, current_size, allocated_size, p_first_created_time, p_last_modified_time,
                           properties, uuid_type, p_uuid, p_object_lid));
}

__STATIC_FORCEINLINE uint16_t acc_ots_object_remove(uint8_t object_lid)
{
    return (ots_object_remove(object_lid));
}

__STATIC_FORCEINLINE uint16_t acc_ots_object_change(uint8_t con_lid, uint8_t transfer_lid, uint8_t object_lid)
{
    return (ots_object_change(con_lid, transfer_lid, object_lid));
}

__STATIC_FORCEINLINE uint16_t acc_ots_object_changed(uint8_t flags, const ot_object_id_t* p_object_id)
{
    return (ots_object_changed(flags, p_object_id));
}

__STATIC_FORCEINLINE uint16_t acc_ots_set(uint8_t object_lid, uint8_t set_type, uint32_t value)
{
    return (ots_set(object_lid, set_type, value));
}

__STATIC_FORCEINLINE uint16_t acc_ots_set_time(uint8_t object_lid, const prf_date_time_t* p_time)
{
    return (ots_set_time(object_lid, p_time));
}

__STATIC_FORCEINLINE void acc_ots_cfm_get_name(bool accept, uint8_t con_lid, uint16_t token, uint8_t name_len,
                                   const uint8_t* p_name)
{
    ots_cfm_get_name(accept, con_lid, token, name_len, p_name);
}

__STATIC_FORCEINLINE void acc_ots_cfm_set_name(bool accept, uint8_t con_lid, uint16_t token)
{
    ots_cfm_set_name(accept, con_lid, token);
}

__STATIC_FORCEINLINE void acc_ots_cfm_object_control(uint8_t result_code, uint8_t con_lid, uint8_t transfer_lid,
                                             uint16_t token, uint32_t checksum)
{
    ots_cfm_object_control(result_code, con_lid, transfer_lid, token, checksum);
}

__STATIC_FORCEINLINE void acc_ots_cfm_object_execute(uint8_t result_code, uint8_t con_lid, uint8_t transfer_lid,
                                             uint16_t token, uint16_t rsp_len, const uint8_t* p_rsp)
{
    ots_cfm_object_execute(result_code, con_lid, transfer_lid, token, rsp_len, p_rsp);
}

__STATIC_FORCEINLINE void acc_ots_cfm_filter_get(bool accept, uint8_t con_lid, uint8_t transfer_lid, uint32_t token,
                                         uint8_t filter_val)
{
    ots_cfm_filter_get(accept, con_lid, transfer_lid, token, filter_val);
}

__STATIC_FORCEINLINE void acc_ots_cfm_filter_get_time(bool accept, uint8_t con_lid, uint8_t transfer_lid,
                                              uint32_t token, uint8_t filter_val,
                                              const prf_date_time_t* p_time_start, const prf_date_time_t* p_time_end)
{
    ots_cfm_filter_get_time(accept, con_lid, transfer_lid, token, filter_val, p_time_start, p_time_end);
}

__STATIC_FORCEINLINE void acc_ots_cfm_filter_get_size(bool accept, uint8_t con_lid, uint8_t transfer_lid, uint32_t token,
                                              uint8_t filter_val, uint32_t size_min, uint32_t size_max)
{
    ots_cfm_filter_get_size(accept, con_lid, transfer_lid, token, filter_val, size_min, size_max);
}

__STATIC_FORCEINLINE void acc_ots_cfm_filter_get_name(bool accept, uint8_t con_lid, uint8_t transfer_lid, uint32_t token,
                                              uint8_t filter_val, uint8_t name_len, const uint8_t* p_name)
{
    ots_cfm_filter_get_name(accept, con_lid, transfer_lid, token, filter_val, name_len, p_name);
}

__STATIC_FORCEINLINE void acc_ots_cfm_filter_get_type(bool accept, uint8_t con_lid, uint8_t transfer_lid,
                                              uint32_t token, uint8_t uuid_type, const void* p_uuid)
{
    ots_cfm_filter_get_type(accept, con_lid, transfer_lid, token, uuid_type, p_uuid);
}

__STATIC_FORCEINLINE void acc_ots_cfm_list_control(uint8_t result_code, uint8_t con_lid, uint8_t transfer_lid,
                                           uint16_t token, uint32_t nb_object)
{
    ots_cfm_list_control(result_code, con_lid, transfer_lid, token, nb_object);
}

__STATIC_FORCEINLINE void acc_ots_cfm_filter_set(bool accept, uint8_t con_lid, uint8_t transfer_lid, uint16_t token)
{
    ots_cfm_filter_set(accept, con_lid, transfer_lid, token);
}

__STATIC_FORCEINLINE void acc_ots_cfm_coc_connect(bool accept, uint8_t con_lid, uint16_t token, uint16_t local_max_sdu)
{
    ots_cfm_coc_connect(accept, con_lid, token, local_max_sdu);
}

/// @} ACC_OTS_FUNCTION
#endif //(GAF_ACC_OTS)

#endif // ACC_OTS_H_
