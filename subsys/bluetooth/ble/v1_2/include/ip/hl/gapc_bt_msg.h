/**
 ****************************************************************************************
 *
 * @file gapc_bt_msg.h
 *
 * @brief Generic Access Profile Controller  Message API. - BT-Classic
 *
 * Copyright (C) RivieraWaves 2009-2022
 *
 ****************************************************************************************
 */

#ifndef _GAPC_BT_MSG_H_
#define _GAPC_BT_MSG_H_

#include "rom_build_cfg.h"

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "gapc_msg.h"
#include "gapc_bt.h"
#include "gap_bt.h"

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// @addtogroup GAPC_MSG_STRUCT_API Message Structures
/// @ingroup GAPC_MSG_API
/// @{

/// Parameter of the #GAPC_BT_CONNECTION_REQ_IND message
/*@TRACE*/
typedef struct gapc_bt_connection_req_ind
{
    /// Connection index
    uint8_t conidx;
    /// Connection handle
    uint16_t conhdl;
    /// Peer BT address
    gap_bdaddr_t peer_addr;
    ///  Is initiator of the connection (1 = Initiator / 0 = Responder)
    bool is_initator;
} gapc_bt_connection_req_ind_t;

/// Parameter of the #GAPC_BT_PERIPH_CONNECTION_ESTABLISHED_IND message
/*@TRACE*/
typedef struct gapc_bt_periph_connection_established_ind
{
    /// Connection index
    uint8_t conidx;
    /// Connection handle
    uint16_t conhdl;
} gapc_bt_periph_connection_established_ind_t;

/// Parameter of the #GAPC_BT_SET_REQUIRED_SECURITY_LEVEL_CMD message
/*@TRACE*/
typedef struct gapc_bt_set_required_security_level_cmd
{
    /// Connection index
    uint8_t conidx;
    /// GAP request type:
    /// - #GAPC_BT_SET_REQUIRED_SECURITY_LEVEL
    uint8_t operation;
    /// Required security level (see enum #gap_sec_lvl)
    uint8_t sec_lvl;
} gapc_bt_set_required_security_level_cmd_t;

/// Parameter of the #GAPC_BT_ENCRYPT_REQ_IND message
/*@TRACE*/
typedef struct gapc_bt_encrypt_req_ind
{
    /// Connection index
    uint8_t conidx;
} gapc_bt_encrypt_req_ind_t;

/// Parameter of the #GAPC_BT_ENCRYPT_CFM message
/*@TRACE*/
typedef struct gapc_bt_encrypt_cfm
{
    /// Connection index
    uint8_t conidx;
    /// Indicate if a link key has been found for the peer device
    bool found;
    /// BT-Classic link key value
    gap_sec_key_t link_key;
} gapc_bt_encrypt_cfm_t;

/// Parameter of the #GAPC_BT_CHANGE_PACKET_TYPE_CMD message
/*@TRACE*/
typedef struct gapc_bt_change_packet_type_cmd
{
    /// Connection index
    uint8_t conidx;
    /// GAP request type:
    /// - #GAPC_BT_CHANGE_PACKET_TYPE
    uint8_t operation;
    /// Packet type bit field (see #gapm_bt_packet_type_bf enumeration)
    uint16_t packet_type_bf;
} gapc_bt_change_packet_type_cmd_t;

/// Parameter of the #GAPC_BT_PACKET_TYPE_CHANGED_IND message
/*@TRACE*/
typedef struct gapc_bt_packet_type_changed_ind
{
    /// Connection index
    uint8_t conidx;
    /// Packet type bit field (see #gapm_bt_packet_type_bf enumeration)
    uint16_t packet_type_bf;
} gapc_bt_packet_type_changed_ind_t;

/// Parameter of the #GAPC_BT_PEER_FEATURES_IND message
/*@TRACE*/
typedef struct gapc_bt_peer_features_ind
{
    /// Connection index
    uint8_t conidx;
    /// Packet type bit field (see #gapm_bt_packet_type_bf enumeration)
    uint8_t features[GAP_BT_FEATS_LEN];
} gapc_bt_peer_features_ind_t;

/// Parameter of the #GAPC_BT_SWITCH_ROLE_CMD message
/*@TRACE*/
typedef struct gapc_bt_switch_role_cmd
{
    /// Connection index
    uint8_t conidx;
    /// GAP request type:
    /// - #GAPC_BT_SWITCH_ROLE
    uint8_t operation;
    /// Expected new role
    uint8_t role;
} gapc_bt_switch_role_cmd_t;

/// Parameter of the #GAPC_BT_ROLE_CHANGED_IND message
/*@TRACE*/
typedef struct gapc_bt_role_changed_ind
{
    /// Connection index
    uint8_t conidx;
    /// New role
    uint8_t role;
} gapc_bt_role_changed_ind_t;

/// Parameters of the @ref GAPC_BT_CONFIGURE_SNIFF_MODE_CMD message
/*@TRACE*/
typedef struct gapc_bt_configure_sniff_mode_cmd
{
    /// Connection index
    uint8_t conidx;
    /// GAP request type:
    /// - GAPC_BT_SNIFF_MODE_ENTER: Enter in Sniff Mode
    /// - GAPC_BT_SNIFF_MODE_EXIT: Exit Sniff Mode
    uint8_t operation;
    /// Sniff parameters\n
    /// Meaningless for #GAPC_BT_SNIFF_MODE_EXIT operation
    gapc_bt_sniff_t sniff_params;
} gapc_bt_configure_sniff_mode_cmd_t;

/// Parameters of the @ref GAPC_BT_MODE_CHANGED_IND message
/*@TRACE*/
typedef struct gapc_bt_mode_changed_ind
{
    /// Connection index
    uint8_t conidx;
    /// Sniff interval (in slots)\n
    /// 0 means that ACL connection is in Active Mode\n
    /// > 0 means that ACL connection is in Sniff Mode
    uint16_t sniff_interval_slots;
} gapc_bt_mode_changed_ind_t;

/// Parameters of the @ref GAPC_BT_SETUP_QOS_CMD message
/*@TRACE*/
typedef struct gapc_bt_setup_qos_cmd
{
    /// Connection index
    uint8_t conidx;
    /// GAP request type:
    /// - GAPC_BT_SETUP_QOS: Setup QoS
    uint8_t operation;
    /// QoS parameters
    gapc_bt_qos_t qos_params;
} gapc_bt_setup_qos_cmd_t;

/// Parameters of the @ref GAPC_BT_QOS_SETUP_IND message
/*@TRACE*/
typedef struct gapc_bt_qos_setup_ind
{
    /// Connection index
    uint8_t conidx;
    /// QoS parameters
    gapc_bt_qos_t qos_params;
} gapc_bt_qos_setup_ind_t;

/// @} GAPC_MSG_STRUCT_API

#endif /* _GAPC_BT_MSG_H_ */
