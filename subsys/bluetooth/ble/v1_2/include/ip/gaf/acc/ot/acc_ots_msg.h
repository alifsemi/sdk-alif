/**
 ****************************************************************************************
 *
 * @file acc_ots_msg.h
 *
 * @brief Audio Content Control - Definition of Kernel Messages (Object Transfer Server)
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef ACC_OTS_MSG_H_
#define ACC_OTS_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup ACC_OTS_MSG Message API
 * @ingroup ACC_OTS
 * @brief Description of Message API for Object Transfer Service Server module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "gaf.h"             // GAF Defines
#include "acc_msg.h"         // Audio Content Control Kernel Messages Definitions
#include "ots_msg.h"         // Object Transfer Server - Message API Definitions

/// @addtogroup ACC_OTS_MSG
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// List of GAF_REQ request codes for Object Transfer Server
enum acc_ots_msg_req_codes
{
    ACC_OTS_CONFIGURE = GAF_CODE(ACC, OTS, 0),
    ACC_OTS_ADD = GAF_CODE(ACC, OTS, 1),
    ACC_OTS_RESTORE_BOND_DATA = GAF_CODE(ACC, OTS, 2),
    ACC_OTS_OBJECT_ADD = GAF_CODE(ACC, OTS, 3),
    ACC_OTS_OBJECT_REMOVE = GAF_CODE(ACC, OTS, 4),
    ACC_OTS_OBJECT_CHANGE = GAF_CODE(ACC, OTS, 5),
    ACC_OTS_OBJECT_CHANGED = GAF_CODE(ACC, OTS, 6),
    ACC_OTS_SET = GAF_CODE(ACC, OTS, 7),
    ACC_OTS_SET_TIME = GAF_CODE(ACC, OTS, 8),
};

/// List of GAF_IND indication codes for Object Transfer Server
enum acc_ots_msg_ind_codes
{
    ACC_OTS_BOND_DATA = GAF_CODE(ACC, OTS, 0),
    ACC_OTS_COC_CONNECTED = GAF_CODE(ACC, OTS, 1),
    ACC_OTS_COC_DISCONNECTED = GAF_CODE(ACC, OTS, 2),
    ACC_OTS_COC_DATA = GAF_CODE(ACC, OTS, 3),
};

/// List of GAF_IND request indication codes for Object Transfer Server
enum acc_ots_msg_req_ind_codes
{
    ACC_OTS_GET_NAME = GAF_CODE(ACC, OTS, 0),
    ACC_OTS_SET_NAME = GAF_CODE(ACC, OTS, 1),
    ACC_OTS_OBJECT_CREATE = GAF_CODE(ACC, OTS, 2),
    ACC_OTS_OBJECT_EXECUTE = GAF_CODE(ACC, OTS, 3),
    ACC_OTS_OBJECT_MANIPULATE = GAF_CODE(ACC, OTS, 4),
    ACC_OTS_OBJECT_CONTROL = GAF_CODE(ACC, OTS, 5),
    ACC_OTS_FILTER_GET = GAF_CODE(ACC, OTS, 6),
    ACC_OTS_LIST_CONTROL = GAF_CODE(ACC, OTS, 7),
    ACC_OTS_LIST_GOTO = GAF_CODE(ACC, OTS, 8),
    ACC_OTS_FILTER_SET = GAF_CODE(ACC, OTS, 9),
    ACC_OTS_FILTER_SET_TIME = GAF_CODE(ACC, OTS, 10),
    ACC_OTS_FILTER_SET_SIZE = GAF_CODE(ACC, OTS, 11),
    ACC_OTS_FILTER_SET_NAME = GAF_CODE(ACC, OTS, 12),
    ACC_OTS_FILTER_SET_TYPE = GAF_CODE(ACC, OTS, 13),
    ACC_OTS_COC_CONNECT = GAF_CODE(ACC, OTS, 14),
};

/*
 * API MESSAGES
 ****************************************************************************************
 */

/// Structure for #ACC_OTS_COC_DISCONNECT command message
typedef ots_coc_disconnect_cmd_t acc_ots_coc_disconnect_cmd_t;

/// Structure for #ACC_OTS_COC_SEND command message
typedef ots_coc_send_cmd_t acc_ots_coc_send_cmd_t;

/// Structure for #ACC_OTS_COC_RELEASE command message
typedef ots_coc_release_cmd_t acc_ots_coc_release_cmd_t;

/// Structure command complete event
typedef ots_cmp_evt_t acc_ots_cmp_evt_t;

/// Structure for #ACC_OTS_CONFIGURE request message
typedef struct acc_ots_configure_req
{
    /// Request code
    uint16_t req_code;
    /// Number of Object Transfer Service instances
    uint8_t nb_transfers;
} acc_ots_configure_req_t;

/// Structure for #ACC_OTS_ADD request message
typedef ots_add_req_t acc_ots_add_req_t;

/// Structure for #ACC_OTS_RESTORE_BOND_DATA request message
typedef ots_restore_bond_data_req_t acc_ots_restore_bond_data_req_t;

/// Structure for #ACC_OTS_OBJECT_ADD request message
typedef ots_object_add_req_t acc_ots_object_add_req_t;

/// Structure for #ACC_OTS_OBJECT_REMOVE request message
typedef ots_object_remove_req_t acc_ots_object_remove_req_t;

/// Structure for #ACC_OTS_OBJECT_CHANGE request message
typedef ots_object_change_req_t acc_ots_object_change_req_t;

/// Structure for #ACC_OTS_OBJECT_CHANGED request message
typedef ots_object_changed_req_t acc_ots_object_changed_req_t;

/// Structure for #ACC_OTS_SET request message
typedef ots_set_req_t acc_ots_set_req_t;

/// Structure for #ACC_OTS_SET_TIME request message
typedef ots_set_time_req_t acc_ots_set_time_req_t;

/// Structure for response message
typedef ots_rsp_t acc_ots_rsp_t;

/// Structure for response message
typedef ots_add_rsp_t acc_ots_add_rsp_t;

/// Structure for #ACC_OTS_BOND_DATA indication message
typedef ots_bond_data_ind_t acc_ots_bond_data_ind_t;

/// Structure for #ACC_OTS_COC_CONNECTED indication message
typedef ots_coc_connected_ind_t acc_ots_coc_connected_ind_t;

/// Structure for #ACC_OTS_COC_DISCONNECTED indication message
typedef ots_coc_disconnected_ind_t acc_ots_coc_disconnected_ind_t;

/// Structure for #ACC_OTS_COC_DATA indication message
typedef ots_coc_data_ind_t acc_ots_coc_data_ind_t;

/// Structure for #ACC_OTS_GET_NAME request indication message
typedef ots_get_name_req_ind_t acc_ots_get_name_req_ind_t;

/// Structure for #ACC_OTS_SET_NAME request indication message
typedef ots_set_name_req_ind_t acc_ots_set_name_req_ind_t;

/// Structure for #ACC_OTS_OBJECT_CREATE request indication message
typedef ots_object_create_req_ind_t acc_ots_object_create_req_ind_t;

/// Structure for #ACC_OTS_OBJECT_EXECUTE request indication message
typedef ots_object_execute_req_ind_t acc_ots_object_execute_req_ind_t;

/// Structure for #ACC_OTS_OBJECT_MANIPULATE request indication message
typedef ots_object_manipulate_req_ind_t acc_ots_object_manipulate_req_ind_t;

/// Structure for #ACC_OTS_OBJECT_CONTROL request indication message
typedef ots_object_control_req_ind_t acc_ots_object_control_req_ind_t;

/// Structure for #ACC_OTS_FILTER_GET request indication message
typedef ots_filter_get_req_ind_t acc_ots_filter_get_req_ind_t;

/// Structure for #ACC_OTS_LIST_CONTROL request indication message
typedef ots_list_control_req_ind_t acc_ots_list_control_req_ind_t;

/// Structure for #ACC_OTS_LIST_GOTO request indication message
typedef ots_list_goto_req_ind_t acc_ots_list_goto_req_ind_t;

/// Structure for #ACC_OTS_FILTER_SET request indication message
typedef ots_filter_set_req_ind_t acc_ots_filter_set_req_ind_t;

/// Structure for #ACC_OTS_FILTER_SET_TIME request indication message
typedef ots_filter_set_time_req_ind_t acc_ots_filter_set_time_req_ind_t;

/// Structure for #ACC_OTS_FILTER_SET_SIZE request indication message
typedef ots_filter_set_size_req_ind_t acc_ots_filter_set_size_req_ind_t;

/// Structure for #ACC_OTS_FILTER_SET_NAME request indication message
typedef ots_filter_set_name_req_ind_t acc_ots_filter_set_name_req_ind_t;

/// Structure for #ACC_OTS_FILTER_SET_TYPE request indication message
typedef ots_filter_set_type_req_ind_t acc_ots_filter_set_type_req_ind_t;

/// Structure for #ACC_OTS_COC_CONNECT request indication message
typedef ots_coc_connect_req_ind_t acc_ots_coc_connect_req_ind_t;

/// Structure for #ACC_OTS_GET_NAME confirmation message
typedef ots_get_name_cfm_t acc_ots_get_name_cfm_t;

/// Structure for #ACC_OTS_SET_NAME confirmation message
typedef ots_set_name_cfm_t acc_ots_set_name_cfm_t;

/// Structure for #ACC_OTS_OBJECT_CREATE/MANIPULATE/CONTROL confirmation message
typedef ots_object_control_cfm_t acc_ots_object_control_cfm_t;

/// Structure for #ACC_OTS_OBJECT_EXECUTE confirmation message
typedef ots_object_execute_cfm_t acc_ots_object_execute_cfm_t;

/// Structure for #ACC_OTS_FILTER_GET confirmation message
/// Used for OTP_FILTER_TYPE_NO_FILTER, OTP_FILTER_TYPE_MARKED_OBJECTS
/// filter values
typedef ots_filter_get_empty_cfm_t acc_ots_filter_get_empty_cfm_t;

/// Structure for OTS_FILTER_GET confirmation message
/// Used for OTP_FILTER_TYPE_CREATED_BETW, OTP_FILTER_TYPE_MODIFIED_BETW
/// filter values
typedef ots_filter_get_time_cfm_t acc_ots_filter_get_time_cfm_t;

/// Structure for OTS_FILTER_GET confirmation message
/// Used for OTP_FILTER_TYPE_CURRENT_SIZE_BETW, OTP_FILTER_TYPE_ALLOCATED_SIZE_BETW
/// filter values
typedef ots_filter_get_size_cfm_t acc_ots_filter_get_size_cfm_t;

/// Structure for OTS_FILTER_GET confirmation message
/// Used for OTP_FILTER_TYPE_NAME_STARTS_WITH, OTP_FILTER_TYPE_NAME_ENDS_WITH
/// OTP_FILTER_TYPE_NAME_CONTAINS, OTP_FILTER_TYPE_NAME_IS_EXACTLY
/// filter values
typedef ots_filter_get_name_cfm_t acc_ots_filter_get_name_cfm_t;

/// Structure for OTS_FILTER_GET confirmation message
/// Used for OTP_FILTER_TYPE_OBJECT_TYPE filter value
typedef ots_filter_get_type_cfm_t acc_ots_filter_get_type_cfm_t;

/// Structure for OTS_LIST_CONTROL/GOTO confirmation message
typedef ots_list_control_cfm_t acc_ots_list_control_cfm_t;

/// Structure for OTS_FILTER_SET/SET_NAME/SET_TIME/SET_SIZE/SET_TYPE confirmation message
typedef ots_filter_set_cfm_t acc_ots_filter_set_cfm_t;

/// Structure for OTS_COC_CONNECT confirmation message
typedef ots_coc_connect_cfm_t acc_ots_coc_connect_cfm_t;

/// @} ACC_OTC_MSG

#endif // ACC_OTS_MSG_H_
