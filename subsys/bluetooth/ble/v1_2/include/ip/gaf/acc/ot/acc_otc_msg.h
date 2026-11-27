/**
 ****************************************************************************************
 *
 * @file acc_otc_msg.h
 *
 * @brief Audio Content Control - Definition of Kernel Messages (Object Transfer Client)
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef ACC_OTC_MSG_H_
#define ACC_OTC_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup ACC_OTC_MSG Message API
 * @ingroup ACC_OTC
 * @brief Description of Message API for Object Transfer Service Client module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "acc_msg.h"         // Audio Content Control Kernel Messages Definitions
#include "otc_msg.h"         // Object Transfer Client - Message API Definitions
#include "gaf.h"             // GAF API

/// @addtogroup ACC_OTC_MSG
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// List of GAF_REQ request codes for Object Transfer Client
enum acc_otc_msg_req_codes
{
    ACC_OTC_CONFIGURE = GAF_CODE(ACC, OTC, 0),
    ACC_OTC_RESTORE_BOND_DATA = GAF_CODE(ACC, OTC, 1),
};

/// List of GAF_IND indication codes for Object Transfer Client
enum acc_otc_msg_ind_codes
{
    ACC_OTC_BOND_DATA = GAF_CODE(ACC, OTC, 0),
    ACC_OTC_EXECUTE_RSP = GAF_CODE(ACC, OTC, 1),
    ACC_OTC_VALUE = GAF_CODE(ACC, OTC, 2),
    ACC_OTC_TIME = GAF_CODE(ACC, OTC, 3),
    ACC_OTC_OBJECT_ID = GAF_CODE(ACC, OTC, 4),
    ACC_OTC_TYPE = GAF_CODE(ACC, OTC, 5),
    ACC_OTC_NAME = GAF_CODE(ACC, OTC, 6),
    ACC_OTC_FILTER = GAF_CODE(ACC, OTC, 7),
    ACC_OTC_FILTER_TIME = GAF_CODE(ACC, OTC, 8),
    ACC_OTC_FILTER_SIZE = GAF_CODE(ACC, OTC, 9),
    ACC_OTC_FILTER_NAME = GAF_CODE(ACC, OTC, 10),
    ACC_OTC_FILTER_TYPE = GAF_CODE(ACC, OTC, 11),
    ACC_OTC_CHANGED = GAF_CODE(ACC, OTC, 13),
    ACC_OTC_COC_CONNECTED = GAF_CODE(ACC, OTC, 14),
    ACC_OTC_COC_DISCONNECTED = GAF_CODE(ACC, OTC, 15),
    ACC_OTC_COC_DATA = GAF_CODE(ACC, OTC, 16),
    ACC_OTC_SVC_CHANGED = GAF_CODE(ACC, OTC, 17),
};

/*
 * API MESSAGES
 ****************************************************************************************
 */

/// Structure for #ACC_OTC_DISCOVER command message
typedef otc_discover_cmd_t acc_otc_discover_cmd_t;

/// Structure for #ACC_OTC_GET command message
typedef otc_get_cmd_t acc_otc_get_cmd_t;

/// Structure for #ACC_OTC_SET_NAME command message
typedef otc_set_name_cmd_t acc_otc_set_name_cmd_t;

/// Structure for #ACC_OTC_SET_TIME command message
typedef otc_set_time_cmd_t acc_otc_set_time_cmd_t;

/// Structure for #ACC_OTC_SET_PROPERTIES command message
typedef otc_set_properties_cmd_t acc_otc_set_properties_cmd_t;

/// Structure for #ACC_OTC_SET_CFG command message
typedef otc_set_cfg_cmd_t acc_otc_set_cfg_cmd_t;

/// Structure for #ACC_OTC_OBJECT_CREATE command message
typedef otc_object_create_cmd_t acc_otc_object_create_cmd_t;

/// Structure for #ACC_OTC_OBJECT_CONTROL command message
typedef otc_object_control_cmd_t acc_otc_object_control_cmd_t;

/// Structure for #ACC_OTC_OBJECT_MANIPULATE command message
typedef otc_object_manipulate_cmd_t acc_otc_object_manipulate_cmd_t;

/// Structure for #ACC_OTC_OBJECT_EXECUTE command message
typedef otc_object_execute_cmd_t acc_otc_object_execute_cmd_t;

/// Structure for #ACC_OTC_LIST_CONTROL command message
typedef otc_list_control_cmd_t acc_otc_list_control_cmd_t;

/// Structure for #ACC_OTC_LIST_GOTO command message
typedef otc_list_goto_cmd_t acc_otc_list_goto_cmd_t;

/// Structure for #ACC_OTC_FILTER_SET command message
typedef otc_filter_set_cmd_t acc_otc_filter_set_cmd_t;

/// Structure for #ACC_OTC_FILTER_SET_TIME command message
typedef otc_filter_set_time_cmd_t acc_otc_filter_set_time_cmd_t;

/// Structure for #ACC_OTC_FILTER_SET_SIZE command message
typedef otc_filter_set_size_cmd_t acc_otc_filter_set_size_cmd_t;

/// Structure for #ACC_OTC_FILTER_SET_NAME command message
typedef otc_filter_set_name_cmd_t acc_otc_filter_set_name_cmd_t;

/// Structure for #ACC_OTC_FILTER_SET_TYPE command message
typedef otc_filter_set_type_cmd_t acc_otc_filter_set_type_cmd_t;

/// Structure for #ACC_OTC_COC_CONNECT command message
typedef otc_coc_connect_cmd_t acc_otc_coc_connect_cmd_t;

/// Structure for #ACC_OTC_COC_DISCONNECT command message
typedef otc_coc_disconnect_cmd_t acc_otc_coc_disconnect_cmd_t;

/// Structure for #ACC_OTC_COC_SEND command message
typedef otc_coc_send_cmd_t acc_otc_coc_send_cmd_t;

/// Structure for #ACC_OTC_RELEASE_SEND command message
typedef otc_coc_release_cmd_t acc_otc_coc_release_cmd_t;

/// Structure command complete event
typedef otc_cmp_evt_t acc_otc_cmp_evt_t;

/// Structure for #ACC_OTC_CONFIGURE request message
typedef struct acc_otc_configure_req
{
    /// Request code
    uint16_t req_code;
} acc_otc_configure_req_t;

/// Structure for #ACC_OTC_RESTORE_BOND_DATA request message
typedef otc_restore_bond_data_req_t acc_otc_restore_bond_data_req_t;

/// Structure for response message
typedef otc_rsp_t acc_otc_rsp_t;

/// Structure for #ACC_OTC_BOND_DATA indication message
typedef otc_bond_data_ind_t acc_otc_bond_data_ind_t;

/// Structure for #ACC_OTC_EXECUTE_RSP indication message
typedef otc_execute_rsp_ind_t acc_otc_execute_rsp_ind_t;

/// Structure for #ACC_OTC_VALUE indication message
typedef otc_value_ind_t acc_otc_value_ind_t;

/// Structure for #ACC_OTC_TIME indication message
typedef otc_time_ind_t acc_otc_time_ind_t;

/// Structure for #ACC_OTC_OBJECT_ID indication message
typedef otc_object_id_ind_t acc_otc_object_id_ind_t;

/// Structure for #ACC_OTC_TYPE indication message
typedef otc_type_ind_t acc_otc_type_ind_t;

/// Structure for #ACC_OTC_NAME indication message
typedef otc_name_ind_t acc_otc_name_ind_t;

/// Structure for #ACC_OTC_FILTER indication message
typedef otc_filter_ind_t acc_otc_filter_ind_t;

/// Structure for #ACC_OTC_FILTER_TIME indication message
typedef otc_filter_time_ind_t acc_otc_filter_time_ind_t;

/// Structure for #ACC_OTC_FILTER_SIZE indication message
typedef otc_filter_size_ind_t acc_otc_filter_size_ind_t;

/// Structure for #ACC_OTC_FILTER_NAME indication message
typedef otc_filter_name_ind_t acc_otc_filter_name_ind_t;

/// Structure for #ACC_OTC_FILTER_TYPE indication message
typedef otc_filter_type_ind_t acc_otc_filter_type_ind_t;

/// Structure for #ACC_OTC_CHANGED indication message
typedef otc_changed_ind_t acc_otc_changed_ind_t;

/// Structure for #ACC_OTC_COC_CONNECTED indication message
typedef otc_coc_connected_ind_t acc_otc_coc_connected_ind_t;

/// Structure for #ACC_OTC_COC_DISCONNECTED indication message
typedef otc_coc_disconnected_ind_t acc_otc_coc_disconnected_ind_t;

/// Structure for #ACC_OTC_COC_DATA indication message
typedef otc_coc_data_ind_t acc_otc_coc_data_ind_t;

/// Structure for #ACC_OTC_SVC_CHANGED indication message
typedef otc_svc_changed_ind_t acc_otc_svc_changed_ind_t;

/// @} ACC_OTC_MSG

#endif // ACC_OTC_MSG_H_
