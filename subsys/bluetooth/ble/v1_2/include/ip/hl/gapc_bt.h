/**
 ****************************************************************************************
 *
 * @file gapc_bt.h
 *
 * @brief Generic Access Profile Controller - BT-Classic API.
 *
 * Copyright (C) RivieraWaves 2009-2022
 *
 ****************************************************************************************
 */

#ifndef _GAPC_BT_H_
#define _GAPC_BT_H_

#include "rom_build_cfg.h"

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "gapc.h"

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// @addtogroup GAPC_ENUM_API
/// @{

/// Service type values
enum gapc_bt_service_type
{
    /// No traffic
    GAPC_BT_SERVICE_TYPE_NO_TRAFFIC = 0,
    /// Best effort
    GAPC_BT_SERVICE_TYPE_BEST_EFFORT,
    /// Guaranteed
    GAPC_BT_SERVICE_TYPE_GUARANTEED,

    GAPC_BT_SERVICE_TYPE_MAX,
};

/// @} GAPC_ENUM_API

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// Sniff parameters structure
typedef struct gapc_bt_sniff
{
    /// Minimum Sniff Interval (in slots)
    uint16_t interval_min_slots;
    /// Maximum Sniff Interval (in slots)
    uint16_t interval_max_slots;
    /// Sniff Attempts (in frames)
    uint16_t attempts_frames;
    /// Sniff Timeout (in frames)
    uint16_t timeout_frames;
} gapc_bt_sniff_t;

/// QoS parameters structure
typedef struct gapc_bt_qos
{
    /// Service type (see #gapc_bt_service_type enumeration)
    uint8_t service_type;
    /// Token rate (in octets per second)
    uint32_t token_rate;
    /// Peak bandwidth (in octets per second)
    uint32_t peak_bandwidth;
    /// Latency (in microseconds)
    uint32_t latency_us;
    /// Delay variation (in microseconds)
    uint32_t delay_variation_us;
} gapc_bt_qos_t;

/// Callback structure used to be notified about connection configuration events
typedef struct gapc_bt_config_cb
{
    /**
     ****************************************************************************************
     * @brief Callback executed when list of packet types supported for a connection has been updated
     *
     * @param[in] conidx            Connection index
     * @param[in] metainfo          Metadata information provided by API user (see \glos{METAINFO})
     * @param[in] packet_type_bf    Packet type bit field (see #gapm_bt_packet_type_bf enumeration)
     ****************************************************************************************
     */
    void (*cb_packet_type_changed)(uint8_t conidx, uint32_t metainfo, uint16_t packet_type_bf);

    /**
     ****************************************************************************************
     * @brief Callback executed when role has changed for a connection
     *
     * @param[in] conidx            Connection index
     * @param[in] metainfo          Metadata information provided by API user (see \glos{METAINFO})
     * @param[in] role              New role
     ****************************************************************************************
     */
    void (*cb_role_changed)(uint8_t conidx, uint32_t metainfo, uint8_t role);

    /**
     ****************************************************************************************
     * @brief Callback executed when mode changed changed for a connection
     *
     * @param[in] conidx                Connection index
     * @param[in] metainfo              Metadata information provided by API user (see \glos{METAINFO})
     * @param[in] sniff_interval_slots  Sniff interval (in slots)\n
     *                                  0 means that ACL connection is in Active Mode\n
     *                                  > 0 means that ACL connection is in Sniff Mode
     ****************************************************************************************
     */
    void (*cb_mode_changed)(uint8_t conidx, uint32_t metainfo, uint16_t sniff_interval_slots);

    /**
     ****************************************************************************************
     * @brief Callback executed when QoS has been setup
     *
     * @param[in] conidx                Connection index
     * @param[in] metainfo              Metadata information provided by API user (see \glos{METAINFO})
     * @param[in] p_qos_params          Pointer to QoS parameters
     ****************************************************************************************
     */
    void (*cb_qos_setup)(uint8_t conidx, uint32_t metainfo, gapc_bt_qos_t* p_qos_params);
} gapc_bt_config_cb_t;

/**
 ****************************************************************************************
 * Callback executed when get Get Peer Supported Features procedure is completed.
 *
 * @param[in] conidx      Connection index
 * @param[in] metainfo    Metadata information provided by API user (see \glos{METAINFO})
 * @param[in] status      Status of procedure execution (see enum #hl_err)
 * @param[in] p_features  Pointer to peer supported feature array
 *
 ****************************************************************************************
 */
typedef void (*gapc_bt_get_peer_features_cmp_cb)(uint8_t conidx, uint32_t metainfo, uint16_t status,
                                                 const uint8_t* p_features);

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/// @addtogroup GAPC_CON_REQ_API
/// @{

/**
 ****************************************************************************************
 * @brief Upper layer SW confirmation of Bluetooth classic link creation with bond data if available.
 *
 * @param[in] conidx    Connection index
 * @param[in] metainfo  Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] p_data    Pointer to bond data if present, NULL otherwise
 *
 * @return Return function execution status (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t gapc_bt_connection_cfm(uint8_t conidx, uint32_t metainfo, const gapc_bond_data_t* p_data);

/// @} GAPC_CON_REQ_API

/// @addtogroup GAPC_CON_INFO_API
/// @{

/**
 ****************************************************************************************
 * @brief Get BT address of peer device
 *
 * @param[in] conidx Connection index
 *
 * @return Return peer device address if active BT-Classic connection, NULL otherwise
 ****************************************************************************************
 */
const gap_addr_t* gapc_bt_get_peer_addr(uint8_t conidx);

/**
 ****************************************************************************************
 * @brief Get peer supported Bluetooth classic features
 *
 * @param[in] conidx    Connection index
 * @param[in] metainfo  Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] cmp_cb    Function called when procedure is over.
 *
 * @return Execution status (see enum #hl_err).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapc_get_ping_to_cmp_cb callback execution
 ****************************************************************************************
 */
uint16_t gapc_bt_get_peer_features(uint8_t conidx, uint32_t metainfo, gapc_bt_get_peer_features_cmp_cb cmp_cb);

/// @} GAPC_CON_INFO_API

/// @addtogroup GAPC_CON_UP_API
/// @{

/**
 ****************************************************************************************
 * @brief Change packet types supported for a connection
 *
 * @param[in] conidx            Connection index
 * @param[in] metainfo          Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] packet_type_bf    Packet type bit field (see #gapm_bt_packet_type_bf enumeration)
 * @param[in] cmp_cb            Callback function called when procedure has been completed
 *
 * @return Return function execution status (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t gapc_bt_change_packet_type(uint8_t conidx, uint32_t metainfo, uint16_t packet_type_bf, gapc_proc_cmp_cb cmp_cb);

/**
 ****************************************************************************************
 * @brief Require to switch role
 *
 * @param[in] conidx            Connection index
 * @param[in] metainfo          Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] role              New expected role
 * @param[in] cmp_cb            Callback function called when procedure has been completed
 *
 * @return Return function execution status (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t gapc_bt_switch_role(uint8_t conidx, uint32_t metainfo, uint8_t role, gapc_proc_cmp_cb cmp_cb);

/**
 ****************************************************************************************
 * @brief Configure sniff mode
 *
 * @param[in] conidx            Connection index
 * @param[in] metainfo          Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] p_sniff_params    Pointer to Sniff parameters\n
 *                              NULL if exit Sniff Mode is required
 * @param[in] cmp_cb            Callback function called when procedure has been completed
 *
 * @return Return function execution status (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t gapc_bt_configure_sniff_mode(uint8_t conidx, uint32_t metainfo, gapc_bt_sniff_t* p_sniff_params,
                                      gapc_proc_cmp_cb cmp_cb);

/**
 ****************************************************************************************
 * @brief Setup Quality of Service (QoS)
 *
 * @param[in] conidx            Connection index
 * @param[in] metainfo          Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] p_qos_params      Pointer to QoS parameters\n
 *                              NULL if exit Sniff Mode is required
 * @param[in] cmp_cb            Callback function called when procedure has been completed
 *
 * @return Return function execution status (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t gapc_bt_setup_qos(uint8_t conidx, uint32_t metainfo, gapc_bt_qos_t* p_qos_params, gapc_proc_cmp_cb cmp_cb);

/**
 ****************************************************************************************
 * @brief Get if connection is a BT-Classic connection.
 *
 * @param[in] conidx Connection index
 *
 * @return Return true if connection is an BT-Classic connection; false otherwise.
 ****************************************************************************************
 */
bool gapc_is_bt_connection(uint8_t conidx);

/// @} GAPC_CON_UP_API

#endif /* _GAPC_BT_H_ */
