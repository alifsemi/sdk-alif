/**
 ****************************************************************************************
 *
 * @file gapc_le.h
 *
 * @brief Generic Access Profile Controller - Low Energy API.
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef GAPC_LE_H_
#define GAPC_LE_H_

#include "rom_build_cfg.h"

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "gapc.h"

/**
 ****************************************************************************************
 * @addtogroup GAPC_LE_POWER_API LE Power Control
 * @ingroup GAPC_API
 *
 * @brief Control and be informed about radio power for on-going LE connection.
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup GAPC_CTE_API Constant Tone Extension
 * @ingroup GAPC_API
 *
 * @brief Manage Constant Tone Extension for on-going LE connection
 ****************************************************************************************
 */

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// @addtogroup GAPC_ENUM_API
/// @{

/// Path Loss zones.
enum gapc_le_path_loss_zone
{
    /// Entered Low zone
    GAPC_PATH_LOSS_LOW = 0u,
    /// Entered Middle zone
    GAPC_PATH_LOSS_MID = 1u,
    /// Entered High zone
    GAPC_PATH_LOSS_HIGH = 2u,
};

/// @} GAPC_ENUM_API

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// @addtogroup GAPC_STRUCT_API
/// @{

#if (BLE_SUBRATING)
/// Subrating parameters structure
typedef struct
{
    /// Subrate factor\n
    uint16_t subrate_factor;
    /// Peripheral latency\n
    uint16_t latency;
    /// Continuation number\n
    uint16_t continuation_number;
    /// Supervision timeout (in multiple of 10ms)\n
    uint16_t supervision_to_10ms;
} gapc_le_subrate_t;
#endif // (BLE_SUBRATING)

/// Peripheral preferred connection parameters
/*@TRACE*/
typedef struct
{
    /// Connection interval minimum
    uint16_t con_intv_min;
    /// Connection interval maximum
    uint16_t con_intv_max;
    /// latency
    uint16_t latency;
    /// Connection supervision timeout multiplier
    uint16_t conn_timeout;
} gapc_le_preferred_periph_param_t;

/// TX Power Report information
typedef struct
{
    /// PHY (see #gapc_phy_pwr_value enumeration)
    uint8_t phy;
    /// Transmit Power level (dBm)
    int8_t tx_pwr;
    /// Transmit Power level flags (see #gapc_pwr_ctrl_flags enumeration)
    uint8_t flags;
    /// Delta (dB)
    int8_t delta;
} gapc_le_tx_power_report_t;

/// information about IQ report
typedef struct
{
    /// Connection event counter
    uint16_t con_evt_cnt;
    /// Rx PHY  (see #gap_le_phy_val enumeration)
    uint8_t rx_phy;
    /// Data channel index
    uint8_t channel_idx;
    /// RSSI (in 0.1 dBm)
    int16_t rssi;
    /// RSSI antenna ID
    uint8_t rssi_antenna_id;
    /// CTE type (0: AOA | 1: AOD-1us | 2: AOD-2us) (see #gap_le_cte_type enumeration)
    uint8_t cte_type;
    /// Slot durations (1: 1us | 2: 2us)
    uint8_t slot_dur;
    /// Packet status
    uint8_t pkt_status;
} gapc_le_iq_report_info_t;

/// Connection Parameter used to update connection parameters
typedef struct
{
    /// Connection interval minimum
    uint16_t intv_min;
    /// Connection interval maximum
    uint16_t intv_max;
    /// Latency
    uint16_t latency;
    /// Supervision timeout
    uint16_t time_out;
} gapc_le_connection_param_t;

/// VS LE Set Tx Power
typedef struct
{
    /// TX power (in dBm)
    int8_t tx_pwr;
} gapc_le_set_tx_power_t;


/// @} GAPC_STRUCT_API

/*
 * CALLBACK DEFINITIONS
 ****************************************************************************************
 */

/// @addtogroup GAPC_CON_INFO_API
/// @{

/**
 ****************************************************************************************
 * Callback executed when get LE channel map for connection procedure is completed.
 *
 * @param[in] conidx    Connection index
 * @param[in] metainfo  Metadata information provided by API user (see \glos{METAINFO})
 * @param[in] status    Status of procedure execution (see #hl_err enumeration)
 * @param[in] p_ch_map  Pointer to the LE channel map value
 *
 ****************************************************************************************
 */
typedef void (*gapc_le_get_channel_map_cmp_cb)(uint8_t conidx, uint32_t metainfo, uint16_t status,
                                               const struct le_ch_map* p_ch_map);

/**
 ****************************************************************************************
 * Callback executed when get LE peer supported features value procedure is completed.
 *
 * @param[in] conidx      Connection index
 * @param[in] metainfo    Metadata information provided by API user (see \glos{METAINFO})
 * @param[in] status      Status of procedure execution (see #hl_err enumeration)
 * @param[in] p_features  Pointer to peer supported feature array
 *
 ****************************************************************************************
 */
typedef void (*gapc_le_get_peer_features_cmp_cb)(uint8_t conidx, uint32_t metainfo, uint16_t status,
                                                 const uint8_t* p_features);

/**
 ****************************************************************************************
 * Callback executed when get LE connection used PHY value procedure is completed.
 *
 * @param[in] conidx    Connection index
 * @param[in] metainfo  Metadata information provided by API user (see \glos{METAINFO})
 * @param[in] status    Status of procedure execution (see #hl_err enumeration)
 * @param[in] tx_phy    LE PHY for data transmission (see #gap_le_phy_bf enumeration)
 * @param[in] rx_phy    LE PHY for data reception (see #gap_le_phy_bf enumeration)
 *
 ****************************************************************************************
 */
typedef void (*gapc_le_get_phy_cmp_cb)(uint8_t conidx, uint32_t metainfo, uint16_t status,
                                       uint8_t tx_phy, uint8_t rx_phy);

/**
 ****************************************************************************************
 * Callback executed when get LE connection local transmit power level information procedure is completed.
 *
 * @param[in] conidx          Connection index
 * @param[in] metainfo        Metadata information provided by API user (see \glos{METAINFO})
 * @param[in] status          Status of procedure execution (see #hl_err enumeration)
 * @param[in] phy             Transmission PHY (see #gapc_phy_pwr_value enumeration)
 * @param[in] power_level     Current transmit power level (in dBm)
 * @param[in] max_power_level Max transmit power level (in dBm)
 *
 ****************************************************************************************
 */
typedef void (*gapc_le_get_local_tx_power_level_cmp_cb)(uint8_t conidx, uint32_t metainfo, uint16_t status,
                                                         uint8_t phy, int8_t power_level, int8_t max_power_level);

/**
 ****************************************************************************************
 * Callback executed when LE remote transmit power level read procedure is completed.
 *
 * @param[in] conidx          Connection index
 * @param[in] metainfo        Metadata information provided by API user (see \glos{METAINFO})
 * @param[in] status          Status of procedure execution (see #hl_err enumeration)
 * @param[in] phy             Transmission PHY (see #gapc_phy_pwr_value enumeration)
 * @param[in] power_level     Current transmit power level (in dBm)
 * @param[in] flags           Transmit Power level flags (see #gapc_pwr_ctrl_flags enumeration)
 *
 ****************************************************************************************
 */
typedef void (*gapc_le_get_peer_tx_power_level_cmp_cb)(uint8_t conidx, uint32_t metainfo, uint16_t status,
                                                         uint8_t phy, int8_t power_level, uint8_t flags);

#if (!(0))
/**
 ****************************************************************************************
 * Callback executed when read attribute peripheral preferred parameters procedure is completed.
 *
 * @param[in] conidx      Connection index
 * @param[in] metainfo    Metadata information provided by API user (see \glos{METAINFO})
 * @param[in] status      Status of procedure execution (see #hl_err enumeration)
 * @param[in] handle      Attribute handle
 * @param[in] p_param     Pointer to read peer device slave preferred parameters characteristic value
 *
 ****************************************************************************************
 */
typedef void (*gapc_le_get_periph_preferred_params_cmp_cb)(uint8_t conidx, uint32_t metainfo,
                                                           uint16_t status, uint16_t handle,
                                                           const gapc_le_preferred_periph_param_t* p_param);

/**
 ****************************************************************************************
 * Callback executed when read attribute appearance procedure is completed.
 *
 * @param[in] conidx      Connection index
 * @param[in] metainfo    Metadata information provided by API user (see \glos{METAINFO})
 * @param[in] status      Status of procedure execution (see #hl_err enumeration)
 * @param[in] handle      Attribute handle
 * @param[in] appearance  Peer device appearance characteristic value
 *
 ****************************************************************************************
 */
typedef void (*gapc_le_get_peer_appearance_cmp_cb)(uint8_t conidx, uint32_t metainfo, uint16_t status, uint16_t handle,
                                                   uint16_t appearance);

/**
 ****************************************************************************************
 * Callback executed when read central address resolution supported parameters procedure is completed.
 *
 * @param[in] conidx               Connection index
 * @param[in] metainfo             Metadata information provided by API user (see \glos{METAINFO})
 * @param[in] status               Status of procedure execution (see #hl_err enumeration)
 * @param[in] handle               Attribute handle
 * @param[in] central_addr_resol   Read peer central address resolution characteristic value
 *
 ****************************************************************************************
 */
typedef void (*gapc_le_get_peer_addr_resolution_supported_cmp_cb)(uint8_t conidx, uint32_t metainfo, uint16_t status,
                                                                  uint16_t handle, uint8_t central_addr_resol);

/**
 ****************************************************************************************
 * Callback executed when read attribute resolvable private address only procedure is completed.
 *
 * @param[in] conidx              Connection index
 * @param[in] metainfo            Metadata information provided by API user (see \glos{METAINFO})
 * @param[in] status              Status of procedure execution (see #hl_err enumeration)
 * @param[in] handle              Attribute handle
 * @param[in] rslv_priv_addr_only Peer device resolvable private address only characteristic value
 *
 ****************************************************************************************
 */
typedef void (*gapc_le_get_peer_rpa_only_cmp_cb)(uint8_t conidx, uint32_t metainfo, uint16_t status, uint16_t handle,
                                                    uint8_t rslv_priv_addr_only);
#endif // (!(0))

/**
 ****************************************************************************************
 * Callback executed when read attribute database hash procedure is completed.
 *
 * @param[in] conidx      Connection index
 * @param[in] metainfo    Metadata information provided by API user (see \glos{METAINFO})
 * @param[in] status      Status of procedure execution (see #hl_err enumeration)
 * @param[in] handle      Attribute handle
 * @param[in] p_hash      Pointer to read peer device database hash characteristic value (128-bit value)
 *
 ****************************************************************************************
 */
typedef void (*gapc_le_get_peer_db_hash_cmp_cb)(uint8_t conidx, uint32_t metainfo, uint16_t status, uint16_t handle,
                                               const uint8_t* p_hash);

/**
 ****************************************************************************************
 * Callback executed when set TX power procedure is completed.
 *
 * @param[in] conidx               Connection index
 * @param[in] metainfo             Metadata information provided by API user (see \glos{METAINFO})
 * @param[in] status               Status of procedure execution (see #hl_err enumeration)
 * @param[in] new_tx_pwr           New Transmit Power
 *
 ****************************************************************************************
 */
typedef void (*gapc_le_set_tx_power_cmp_cb)(uint8_t conidx, uint32_t metainfo, uint16_t status, int8_t new_tx_pwr);


/// @} GAPC_CON_INFO_API

/// @addtogroup GAPC_CON_UP_API
/// @{

/// Callback structure used to be notified about connection configuration events
struct gapc_le_config_cb
{
    /**
     ****************************************************************************************
     * Callback executed when connection parameter update is requested
     * #gapc_le_update_params_cfm shall be called to confirm new parameters.
     *
     * @note Optional callback - Parameters automatically accepted if callback not provided on peripheral side.
     *                           Automatically rejected on central side.
     *
     * @param[in] conidx        Connection index
     * @param[in] metainfo      Metadata information provided by API user (see \glos{METAINFO})
     * @param[in] p_param       Pointer to negotiated LE connection parameters
     ****************************************************************************************
     */
    void (*param_update_req)(uint8_t conidx, uint32_t metainfo, const gapc_le_con_param_nego_t* p_param);

    /**
     ****************************************************************************************
     * Callback executed when connection parameter are updated
     *
     * @note Optional callback.
     *
     * @param[in] conidx        Connection index
     * @param[in] metainfo      Metadata information provided by API user (see \glos{METAINFO})
     * @param[in] p_param       Pointer to new connection parameters
     ****************************************************************************************
     */
    void (*param_updated)(uint8_t conidx, uint32_t metainfo, const gapc_le_con_param_t* p_param);

    /**
     ****************************************************************************************
     * Callback executed when data length over the air has been updated
     *
     * @note Optional callback.
     *
     * @param[in] conidx        Connection index
     * @param[in] metainfo      Metadata information provided by API user (see \glos{METAINFO})
     * @param[in] max_tx_octets The maximum number of payload octets in TX
     * @param[in] max_tx_time   The maximum time that the local Controller will take to TX (in microseconds)
     * @param[in] max_tx_octets The maximum number of payload octets in RX
     * @param[in] max_tx_time   The maximum time that the local Controller will take to RX (in microseconds)
     ****************************************************************************************
     */
    void (*packet_size_updated)(uint8_t conidx, uint32_t metainfo, uint16_t max_tx_octets , uint16_t max_tx_time,
                                uint16_t max_rx_octets , uint16_t max_rx_time);

    /**
     ****************************************************************************************
     * Callback executed when LE PHY is updated
     *
     * @note Optional callback.
     *
     * @param[in] conidx        Connection index
     * @param[in] metainfo      Metadata information provided by API user (see \glos{METAINFO})
     * @param[in] tx_phy        LE PHY for data transmission (see #gap_le_phy_bf enumeration)
     * @param[in] rx_phy        LE PHY for data reception (see #gap_le_phy_bf enumeration)
     ****************************************************************************************
     */
    void (*phy_updated)(uint8_t conidx, uint32_t metainfo, uint8_t tx_phy , uint8_t rx_phy);

    #if (BLE_SUBRATING)
    /**
     ****************************************************************************************
     * Callback executed when applied subrating parameters are updated for a connection
     *
     * @note Optional callback - Automatically rejected if not set
     *
     * @param[in] conidx            Connection index
     * @param[in] metainfo          Metadata information provided by API user (see \glos{METAINFO})
     * @param[in] p_subrate_param   Pointer to subrating parameters applied on a connection
     ****************************************************************************************
     */
    void (*subrate_updated)(uint8_t conidx, uint32_t metainfo, const gapc_le_subrate_t* p_subrate_params);
    #endif // (BLE_SUBRATING)

    #if (GATT_INDICATE_LEGACY_MTU_CHANGED)
    /**
     ****************************************************************************************
     * Callback executed when MTU changed on legacy attribute bearer .
     *
     * @param[in] conidx        Connection index
     * @param[in] metainfo      Metadata information provided by API user (see \glos{METAINFO})
     * @param[in] mtu           New value of attribute bearer
     ****************************************************************************************
     */
    void (*att_legacy_bearer_mtu_changed_cb)(uint8_t conidx, uint32_t metainfo, uint16_t mtu);
    #endif // (GATT_INDICATE_LEGACY_MTU_CHANGED)
};

/// Callback structure used to be notified about LE Events (from a profile or an application module)
typedef struct
{
    /**
     ****************************************************************************************
     * Callback executed to provide initial LE connection parameter or updated one
     *
     * @note Optional callback.
     *
     * @param[in] conidx        Connection index
     * @param[in] p_param       Pointer to connection parameters
     ****************************************************************************************
     */
    void (*con_param)(uint8_t conidx, const gapc_le_con_param_t* p_param);
} gapc_le_event_cb_t;

/// Structure that must be followed by a client of LE events
typedef struct
{
    /// List header element
    co_list_hdr_t hdr;
    /// Pointer to the callback structure, SHALL NOT BE NULL
    const gapc_le_event_cb_t* p_cbs;
} gapc_le_event_client_t;

/// @} GAPC_CON_UP_API

/// @addtogroup GAPC_LE_POWER_API
/// @{

/// Callback structure used to be notified about LE Power events
typedef struct
{
    /**
     ****************************************************************************************
     * Callback executed when a TX power change report is received
     *
     * @note  Mandatory callback
     *
     * @param[in] conidx        Connection index
     * @param[in] metainfo      Metadata information provided by API user (see \glos{METAINFO})
     * @param[in] local         True if local TX power change report, remote TX power change report otherwise
     * @param[in] p_report      Pointer to TX Power report information
     ****************************************************************************************
     */
    void (*tx_change_report)(uint8_t conidx, uint32_t metainfo, bool local, const gapc_le_tx_power_report_t* p_report);

    /**
     ****************************************************************************************
     * Callback executed when a Path Loss threshold report event is received
     *
     * @note Mandatory callback
     *
     * @param[in] conidx         Connection index
     * @param[in] metainfo       Metadata information provided by API user (see \glos{METAINFO})
     * @param[in] curr_path_loss Current path loss (dB)
     * @param[in] zone_entered   Zone entered (see #gapc_le_path_loss_zone enumeration)
     ****************************************************************************************
     */
    void (*path_loss_threshold_report)(uint8_t conidx, uint32_t metainfo, uint8_t curr_path_loss, uint8_t zone_entered);
} gapc_le_power_cb_t;
/// @} GAPC_LE_POWER_API

/// @addtogroup GAPC_CTE_API
/// @{
#if (BLE_CON_CTE_REQ)
/// Callback structure used to be notified about constant tone extension events
typedef struct
{
    /**
     ****************************************************************************************
     * Callback executed when an IQ report has been received
     *
     * @note Mandatory callback
     *
     * @param[in] conidx        Connection index
     * @param[in] metainfo      Metadata information provided by API user (see \glos{METAINFO})
     * @param[in] p_report      Pointer to IQ report information
     * @param[in] nb_samples    Number of samples
     * @param[in] p_samples     Pointer to samples data
     ****************************************************************************************
     */
    void (*iq_report_received)(uint8_t conidx, uint32_t metainfo, const gapc_le_iq_report_info_t* p_report, uint8_t nb_samples,
                               const gap_le_iq_sample_t* p_samples);

    /**
     ****************************************************************************************
     * Callback executed when a CTE request failed event is triggered by controller
     *
     * @note Mandatory callback
     *
     * @param[in] conidx        Connection index
     * @param[in] metainfo      Metadata information provided by API user (see \glos{METAINFO})
     * @param[in] reason        Failed reason (see #hl_err enumeration)
     ****************************************************************************************
     */
    void (*request_failed_event)(uint8_t conidx, uint32_t metainfo, uint16_t reason);
} gapc_le_cte_cb_t;
#endif // (BLE_CON_CTE_REQ)
/// @} GAPC_CTE_API

/**
 ****************************************************************************************
 * @brief Send discovery command complete event
 *
 * @param[in] conidx        Connection index
 * @param[in] status        Discovery status
 ****************************************************************************************
 */
typedef void (*gapc_le_cli_cb_cmp_evt)(uint8_t conidx, uint16_t status);

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/// @addtogroup GAPC_CON_REQ_API
/// @{

/**
 ****************************************************************************************
 * @brief Upper layer SW confirmation of Low Energy link creation with bond data if available.
 *
 * @note After confirmation of connection establishment, it is recommended to require peer version (see
 * #gapc_get_peer_version) as well as exchange of supported features (see #gapc_le_get_peer_features).
 * @note When BAP Delegator or Assistant role is used, exchange of features is mandatory in order to enable use of
 * PAST feature for synchronization.
 *
 * @param[in] conidx    Connection index
 * @param[in] metainfo  Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] p_data    Pointer to bond data if present, NULL otherwise
 *
 * @return Return function execution status (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t gapc_le_connection_cfm(uint8_t conidx, uint32_t metainfo, const gapc_bond_data_t* p_data);

/// @} GAPC_CON_REQ_API

/// @addtogroup GAPC_CON_INFO_API
/// @{

/**
 ****************************************************************************************
 * @brief Get if connection is an LE connection.
 *
 * @param[in] conidx Connection index
 *
 * @return Return true if connection is an LE connection; false otherwise.
 ****************************************************************************************
 */
bool gapc_is_le_connection(uint8_t conidx);

/**
 ****************************************************************************************
 * @brief Get if peer device supports a specific Low Energy feature.
 *
 * @param[in] conidx    Connection index
 * @param[in] feature   Feature bit (see #gap_le_feature enumeration)
 *
 * @return Return true if remote features has been read and if supported, false otherwise
 ****************************************************************************************
 */
bool gapc_is_le_feat_supported(uint8_t conidx, uint8_t feature);

/**
 ****************************************************************************************
 * @brief Get LE channel selection algorithm used for a given connection identified
 *        by its connection index.
 *
 * @param[in] conidx    Connection index
 *
 * @return Channel selection algorithm used (0 if algo #1, 1 if algo #2, 0xFF if invalid connection)
 ****************************************************************************************
 */
uint8_t gapc_le_get_channel_selection_algo(uint8_t conidx);

/**
 ****************************************************************************************
 * @brief Get connection LE channel map used.
 *
 * @param[in] conidx    Connection index
 * @param[in] metainfo  Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] cmp_cb    Function called when procedure is over.
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapc_get_ping_to_cmp_cb callback execution
 ****************************************************************************************
 */
uint16_t gapc_le_get_channel_map(uint8_t conidx, uint32_t metainfo, gapc_le_get_channel_map_cmp_cb cmp_cb);

/**
 ****************************************************************************************
 * @brief Get connection LE peer supported features
 *
 * @param[in] conidx    Connection index
 * @param[in] metainfo  Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] cmp_cb    Function called when procedure is over.
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapc_get_ping_to_cmp_cb callback execution
 ****************************************************************************************
 */
uint16_t gapc_le_get_peer_features(uint8_t conidx, uint32_t metainfo, gapc_le_get_peer_features_cmp_cb cmp_cb);

/**
 ****************************************************************************************
 * @brief Get LE connection used PHY value
 *
 * @param[in] conidx    Connection index
 * @param[in] metainfo  Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] cmp_cb    Function called when procedure is over.
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapc_get_ping_to_cmp_cb callback execution
 ****************************************************************************************
 */
uint16_t gapc_le_get_phy(uint8_t conidx, uint32_t metainfo, gapc_le_get_phy_cmp_cb cmp_cb);

/**
 ****************************************************************************************
 * @brief Get LE connection local transmit power level information for a specific PHY.
 *
 * @param[in] conidx    Connection index
 * @param[in] metainfo  Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] phy       Specific transmission PHY (see #gapc_phy_pwr_value enumeration)
 * @param[in] cmp_cb    Function called when procedure is over.
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapc_get_ping_to_cmp_cb callback execution
 ****************************************************************************************
 */
uint16_t gapc_le_get_local_tx_power_level(uint8_t conidx, uint32_t metainfo, uint8_t phy,
                                          gapc_le_get_local_tx_power_level_cmp_cb cmp_cb);

 /**
 ****************************************************************************************
 * @brief Get LE connection remote transmit power level information for a specific PHY.
 *
 * @param[in] conidx    Connection index
 * @param[in] metainfo  Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] phy       Specific transmission PHY (see #gapc_phy_pwr_value enumeration)
 * @param[in] cmp_cb    Function called when procedure is over.
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapc_get_ping_to_cmp_cb callback execution
 ****************************************************************************************
 */
uint16_t gapc_le_get_peer_tx_power_level(uint8_t conidx, uint32_t metainfo, uint8_t phy,
                                         gapc_le_get_peer_tx_power_level_cmp_cb cmp_cb);

/**
 ****************************************************************************************
 * @brief VS LE Set Tx Power.
 *
 * @param[in] conidx    Connection index
 * @param[in] metainfo  Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] tx_pwr    Tx Power
 * @param[in] cmp_cb    Function called when procedure is over.
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapc_get_ping_to_cmp_cb callback execution
 ****************************************************************************************
 */
uint16_t gapc_le_set_tx_power(uint8_t conidx, uint32_t metainfo, int8_t tx_pwr, gapc_le_set_tx_power_cmp_cb cmp_cb);

#if (!(0))
/**
 ****************************************************************************************
 * @brief Read peer device peripheral preferred parameters characteristic present in attribute database
 *
 * @param[in] conidx    Connection index
 * @param[in] metainfo  Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] cmp_cb    Function called when procedure is over.
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapc_le_get_periph_preferred_params_cmp_cb callback execution
 ****************************************************************************************
 */
uint16_t gapc_le_get_periph_preferred_params(uint8_t conidx, uint32_t metainfo,
                                             gapc_le_get_periph_preferred_params_cmp_cb cmp_cb);
#endif // (!(0))

/**
 ****************************************************************************************
 * @brief Provide slave preferred connection parameters to peer device in response of request received.
 *
 * @param[in] conidx          Connection index
 * @param[in] token           Token value provided in request indication
 * @param[in] status          Status code used to accept (GAP_ERR_NO_ERROR) or reject request (see #hl_err enumeration).
 * @param[in] pref            Slave preferred connection parameters
 *
 * @return Execution status (see #hl_err enumeration).
 ****************************************************************************************
 */
uint16_t gapc_le_get_preferred_periph_params_cfm(uint8_t conidx, uint16_t token, uint16_t status,
                                                 gapc_le_preferred_periph_param_t pref);

/**
 ****************************************************************************************
 * @brief Provide name to peer device after in response of request received.
 *
 * @param[in] conidx          Connection index
 * @param[in] token           Token value provided in request indication
 * @param[in] status          Status code used to accept (GAP_ERR_NO_ERROR) or reject request (see #hl_err enumeration).
 * @param[in] complete_length Complete device name length
 * @param[in] length          Length of the value to transmit (should be less or equals requested maximum length)
 * @param[in] p_name          Pointer to array that contains device name starting from requested offset
 *
 * @return Execution status (see #hl_err enumeration).
 ****************************************************************************************
 */
uint16_t gapc_le_get_name_cfm(uint8_t conidx, uint16_t token, uint16_t status, uint16_t complete_length,
                                uint8_t length, const uint8_t* p_name);

/**
 ****************************************************************************************
 * @brief Provide appearance to peer device after in response of request received.
 *
 * @param[in] conidx          Connection index
 * @param[in] token           Token value provided in request indication
 * @param[in] status          Status code used to accept (GAP_ERR_NO_ERROR) or reject request (see #hl_err enumeration).
 * @param[in] appearance      Appearance icon value
 *
 * @return Execution status (see #hl_err enumeration).
 ****************************************************************************************
 */
uint16_t gapc_le_get_appearance_cfm(uint8_t conidx, uint16_t token, uint16_t status, uint16_t appearance);



/**
 ****************************************************************************************
 * @brief Retrieve BD address used by peer device on current link.
 *
 * @param[in] conidx Connection index
 *
 * @return Return found connection address
 ****************************************************************************************
 */
const gap_bdaddr_t* gapc_le_get_peer_bdaddr(uint8_t conidx);

/**
 ****************************************************************************************
 * @brief Retrieve BD address used by local device on current link.
 *
 * @param[in] conidx Connection index
 *
 * @return Return found connection address
 ****************************************************************************************
 */
const gap_bdaddr_t* gapc_le_get_local_bdaddr(uint8_t conidx);

/**
 ****************************************************************************************
 * @brief Read peer device attribute database hash characteristic present in attribute database
 *
 * @param[in] conidx    Connection index
 * @param[in] metainfo  Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] cmp_cb    Function called when procedure is over.
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapc_le_get_peer_db_hash_cmp_cb callback execution
 ****************************************************************************************
 */
uint16_t gapc_le_get_peer_db_hash(uint8_t conidx, uint32_t metainfo, gapc_le_get_peer_db_hash_cmp_cb cmp_cb);

#if (!(0))
/**
 ****************************************************************************************
 * @brief Read peer device appearance characteristic present in attribute database
 *
 * @param[in] conidx    Connection index
 * @param[in] metainfo  Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] cmp_cb    Function called when procedure is over.
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapc_le_get_peer_appearance_cmp_cb callback execution
 ****************************************************************************************
 */
uint16_t gapc_le_get_peer_appearance(uint8_t conidx, uint32_t metainfo, gapc_le_get_peer_appearance_cmp_cb cmp_cb);

/**
 ****************************************************************************************
 * @brief Read peer device central address resolution supported characteristic present in attribute database
 *
 * @param[in] conidx    Connection index
 * @param[in] metainfo  Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] cmp_cb    Function called when procedure is over.
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapc_le_get_peer_addr_resolution_supported_cmp_cb
 *         callback execution
 ****************************************************************************************
 */
uint16_t gapc_le_get_peer_addr_resolution_supported(uint8_t conidx, uint32_t metainfo,
                                                    gapc_le_get_peer_addr_resolution_supported_cmp_cb cmp_cb);

/**
 ****************************************************************************************
 * @brief Read peer device attribute resolvable private address only characteristic present in attribute database
 *
 * @param[in] conidx    Connection index
 * @param[in] metainfo  Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] cmp_cb    Function called when procedure is over.
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapc_le_get_peer_rpa_only_cmp_cb callback execution
 ****************************************************************************************
 */
uint16_t gapc_le_get_peer_rpa_only(uint8_t conidx, uint32_t metainfo, gapc_le_get_peer_rpa_only_cmp_cb cmp_cb);
#endif // (!(0))


/**
 ****************************************************************************************
 * @brief Inform if name modification in response of request received is accepted or rejected.
 *
 * @param[in] conidx          Connection index
 * @param[in] token           Token value provided in request indication
 * @param[in] status          Status code used to accept (GAP_ERR_NO_ERROR) or reject request (see #hl_err enumeration).
 *
 * @return Execution status (see #hl_err enumeration).
 ****************************************************************************************
 */
uint16_t gapc_le_set_name_cfm(uint8_t conidx, uint16_t token, uint16_t status);

/**
 ****************************************************************************************
 * @brief Inform if appearance modification in response of request received is accepted or rejected.
 *
 * @param[in] conidx          Connection index
 * @param[in] token           Token value provided in request indication
 * @param[in] status          Status code used to accept (GAP_ERR_NO_ERROR) or reject request (see #hl_err enumeration).
 *
 * @return Execution status (see #hl_err enumeration).
 ****************************************************************************************
 */
uint16_t gapc_le_set_appearance_cfm(uint8_t conidx, uint16_t token, uint16_t status);

#if (!(0))
/**
 ****************************************************************************************
 * @brief Enable usage of supported client features
 *
 * @param[in] conidx          Connection index
 * @param[in] metainfo        Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] cmp_cb          Function called when procedure is over.
 *
 * @return Return function execution status (see #hl_err enumeration)
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapc_proc_cmp_cb callback execution
 ****************************************************************************************
 */
uint16_t gapc_le_enable_central_features(uint8_t conidx, uint32_t metainfo, gapc_proc_cmp_cb cmp_cb);
#endif // (!(0))

#if (!HL_DEPRECATED_SERVICE_CHANGED)
/**
 ****************************************************************************************
 * @brief Send indication for Service Changed characteristic to a peer device
 *
 * @param[in] conidx          Connection index
 * @param[in] metainfo        Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] shdl            Start handle
 * @param[in] ehdl            End handle
 * @param[in] cb_cmp_evt      Function called when procedure is over
 *
 * @return Return function execution status (see #hl_err enumeration)
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapc_proc_cmp_cb callback execution
 ****************************************************************************************
 */
uint16_t gapc_le_send_service_changed(uint8_t conidx, uint32_t metainfo, uint16_t shdl, uint16_t ehdl,
                                      gapc_proc_cmp_cb cb_cmp_evt);
#endif // (!HL_DEPRECATED_SERVICE_CHANGED)

/// @} GAPC_CON_INFO_API

/// @addtogroup GAPC_CON_UP_API Connection Update
/// @{
/**
 ****************************************************************************************
 * @brief Function used to negotiate new BLE connection parameters
 *
 * @param[in] conidx        Connection index
 * @param[in] metainfo      Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] p_param       Pointer to new LE connection parameters to negotiate.
 * @param[in] cmp_cb        Function called when procedure is over.
 *
 * @return Return function execution status (see #hl_err enumeration)
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapc_proc_cmp_cb callback execution
 ****************************************************************************************
 */
uint16_t gapc_le_update_params(uint8_t conidx, uint32_t metainfo, const gapc_le_con_param_nego_with_ce_len_t* p_param,
                               gapc_proc_cmp_cb cmp_cb);
/**
 ****************************************************************************************
 * @brief Function used to accept or reject LE connection parameter update
 *
 * @param[in] conidx      Connection index
 * @param[in] accept      True to accept peer connection parameters, False otherwise
 * @param[in] ce_len_min  Minimum connection Event Duration (0.625 ms unit)
 * @param[in] ce_len_max  Maximum connection Event Duration (0.625 ms unit)
 *
 * @return Return function execution status (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t gapc_le_update_params_cfm(uint8_t conidx, bool accept, uint16_t ce_len_min, uint16_t ce_len_max);

/**
 ****************************************************************************************
 * @brief Function used to negotiate new BLE phy for connection
 *
 * @param[in] conidx        Connection index
 * @param[in] metainfo      Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] tx_phy        Supported LE PHY for data transmission (see #gap_le_phy_bf enumeration)
 * @param[in] rx_phy        Supported LE PHY for data reception (see #gap_le_phy_bf enumeration)
 * @param[in] phy_opt       PHY options (see #gapc_phy_option enumeration)
 * @param[in] cmp_cb        Function called when procedure is over.
 *
 * @return Return function execution status (see #hl_err enumeration)
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapc_proc_cmp_cb callback execution
 ****************************************************************************************
 */
uint16_t gapc_le_set_phy(uint8_t conidx, uint32_t metainfo, uint8_t tx_phy, uint8_t rx_phy, uint8_t phy_opt,
                         gapc_proc_cmp_cb cmp_cb);

/**
 ****************************************************************************************
 * @brief Function used to set the preferred BLE connection peripheral latency (dynamically without negotiation)
 * @note Can be initiated only by a peripheral
 *
 * @param[in] conidx        Connection index
 * @param[in] metainfo      Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] latency       Preferred latency (in number of connection events)
 * @param[in] cmp_cb        Function called when procedure is over.
 *
 * @return Return function execution status (see #hl_err enumeration)
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapc_proc_cmp_cb callback execution
 ****************************************************************************************
 */
uint16_t gapc_le_set_preferred_periph_latency(uint8_t conidx, uint32_t metainfo, uint16_t latency, gapc_proc_cmp_cb cmp_cb);

/**
 ****************************************************************************************
 * @brief Function used to set the preferred slave event duration (dynamically without negotiation)
 * @note Can be initiated only by a peripheral
 *
 * @param[in] conidx        Connection index
 * @param[in] metainfo      Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] duration      Preferred event duration (N * 0.625 ms)
 * @param[in] single_tx      Slave transmits a single packet per connection event (False/True)
 * @param[in] cmp_cb        Function called when procedure is over.
 *
 * @return Return function execution status (see #hl_err enumeration)
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapc_proc_cmp_cb callback execution
 ****************************************************************************************
 */
uint16_t gapc_le_set_preferred_periph_event_duration(uint8_t conidx, uint32_t metainfo, uint16_t duration, bool single_tx,
                                        gapc_proc_cmp_cb cmp_cb);

/**
 ****************************************************************************************
 * @brief Function used to set the maximum reception size and time
 *
 * @param[in] conidx        Connection index
 * @param[in] metainfo      Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] rx_octets     Maximum RX size (in Bytes)
 * @param[in] rx_time       Maximum RX time (in microseconds)
 * @param[in] cmp_cb        Function called when procedure is over.
 *
 * @return Return function execution status (see #hl_err enumeration)
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapc_proc_cmp_cb callback execution
 ****************************************************************************************
 */
uint16_t gapc_le_set_max_rx_size_and_time(uint8_t conidx, uint32_t metainfo, uint16_t rx_octets, uint16_t rx_time,
                                       gapc_proc_cmp_cb cmp_cb);

/**
 ****************************************************************************************
 * @brief Function used to set the maximum transmit size and time
 *
 * @param[in] conidx        Connection index
 * @param[in] metainfo      Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] tx_octets     Preferred maximum number of payload octets that the local Controller should include
 *                          in a single Link Layer Data Channel PDU.
 * @param[in] tx_time       Preferred maximum number of microseconds that the local Controller should use to transmit
 *                          a single Link Layer Data Channel PDU
 * @param[in] cmp_cb        Function called when procedure is over.
 *
 * @return Return function execution status (see #hl_err enumeration)
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapc_proc_cmp_cb callback execution
 ****************************************************************************************
 */
uint16_t gapc_le_set_packet_size(uint8_t conidx, uint32_t metainfo, uint16_t tx_octets, uint16_t tx_time,
                              gapc_proc_cmp_cb cmp_cb);

/**
 ****************************************************************************************
 * @brief Register a client waiting for LE events.
 *
 * Corresponding callback will be executed when an LE event is triggered
 *
 * @param[in] p_client      Pointer to the LE event client structure
 *
 * @return Return function execution status (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t gapc_le_register_event_client(gapc_le_event_client_t* p_client);

/**
 ****************************************************************************************
 * @brief Un-Register a client waiting for LE events.
 *
 * @param[in] p_client      Pointer to the LE event client structure
 *
 * @return Return function execution status (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t gapc_le_unregister_event_client(gapc_le_event_client_t* p_client);

#if (BLE_SUBRATING)
/**
 ****************************************************************************************
 * @brief Request update of the subrating parameters applied on a connection
 *
 * @param[in] conidx           Connection index
 * @param[in] metainfo         Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] p_subrate_req    Pointer to subrating requirements
 * @param[in] cmp_cb           Callback function called when procedure has been completed
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapc_proc_cmp_cb callback execution
 ****************************************************************************************
 */
uint16_t gapc_le_update_subrate(uint8_t conidx, uint32_t metainfo, const gap_le_subrate_req_t* p_subrate_req,
                                gapc_proc_cmp_cb cmp_cb);
#endif // (BLE_SUBRATING)

/**
 ****************************************************************************************
 * @brief Set priority elevation level for a given LE connection
 *
 * @param[in] conidx        Connection index
 * @param[in] metainfo      Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] elevation     Priority elevation (in units of increment)
 * @param[in] cmp_cb        Function called when procedure is over.
 *
 * @return Return function execution status (see #hl_err enumeration)
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapc_proc_cmp_cb callback execution
 ****************************************************************************************
 */
uint16_t gapc_le_set_priority_elevation(uint8_t conidx, uint32_t metainfo, uint8_t elevation, gapc_proc_cmp_cb cmp_cb);

/// @} GAPC_CON_UP_API

#if (BLE_PAST)
/// @addtogroup GAPC_PAST_API Periodic Advertising Sync Transfer
/// @ingroup GAPC_API
/// @{

/**
 ****************************************************************************************
 * @brief Transfer periodic advertising sync information to peer device.
 *        Either a periodic advertising or a periodic sync activity.
 *
 * @param[in] conidx        Connection index
 * @param[in] metainfo      Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] actv_idx      Periodic Advertising or Periodic Sync activity index
 * @param[in] service_data  A value provided by application given to peer device
 * @param[in] cmp_cb        Function called when procedure is over.
 *
 * @return Return function execution status (see #hl_err enumeration)
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapc_proc_cmp_cb callback execution
 ****************************************************************************************
 */
uint16_t gapc_le_start_past(uint8_t conidx, uint32_t metainfo, uint8_t actv_idx, uint16_t service_data,
                            gapc_proc_cmp_cb cmp_cb);

/// @} GAPC_PAST_API
#endif // (BLE_PAST)

/// @addtogroup GAPC_LE_POWER_API LE Power Control
/// @{

/**
 ****************************************************************************************
 * @brief Set Callback that will handle path loss and tx power change reports
 *
 * @param[in] p_cbs         Pointer to the callback set
 *
 * @return Return function execution status (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t gapc_le_power_set_callbacks(const gapc_le_power_cb_t* p_cbs);

/**
 ****************************************************************************************
 * @brief Control reception of TX Local and/or remote power report
 *        #gapc_le_power_cb_t.tx_change_report must be valid
 *
 * @param[in] conidx        Connection index
 * @param[in] metainfo      Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] local_en      True to enable remote power changes reporting, false to disable.
 * @param[in] remote_en     True to enable remote power changes reporting, false to disable.
 * @param[in] cmp_cb        Function called when procedure is over.
 *
 * @return Return function execution status (see #hl_err enumeration)
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapc_proc_cmp_cb callback execution
 ****************************************************************************************
 */
uint16_t gapc_le_control_tx_power_report(uint8_t conidx, uint32_t metainfo, bool local_en, bool remote_en, gapc_proc_cmp_cb cmp_cb);

/**
 ****************************************************************************************
 * @brief Enable path loss detection
 *        #gapc_le_power_cb_t.path_loss_threshold_report must be valid
 *
 * @param[in] conidx          Connection index
 * @param[in] metainfo        Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] high_threshold  High threshold (dB)
 * @param[in] high_hysteresis High hysteresis (dB)
 * @param[in] low_threshold   Low threshold (dB)
 * @param[in] low_hysteresis  Low hysteresis (dB)
 * @param[in] min_time        Min time spent (Number of connection events)
 * @param[in] cmp_cb          Function called when procedure is over.
 *
 * @return Return function execution status (see #hl_err enumeration)
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapc_proc_cmp_cb callback execution
 ****************************************************************************************
 */
uint16_t gapc_le_enable_path_loss(uint8_t conidx, uint32_t metainfo, uint8_t high_threshold, uint8_t high_hysteresis,
                                  uint8_t low_threshold, uint8_t low_hysteresis, uint16_t min_time,
                                  gapc_proc_cmp_cb cmp_cb);

/**
 ****************************************************************************************
 * @brief Disable path loss detection
 *
 * @param[in] conidx          Connection index
 * @param[in] metainfo        Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] cmp_cb          Function called when procedure is over.
 *
 * @return Return function execution status (see #hl_err enumeration)
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapc_proc_cmp_cb callback execution
 ****************************************************************************************
 */
uint16_t gapc_le_disable_path_loss(uint8_t conidx, uint32_t metainfo, gapc_proc_cmp_cb cmp_cb);

/// @} GAPC_LE_POWER_API

#if (BLE_CON_CTE_RSP || BLE_CON_CTE_REQ)
/// @addtogroup GAPC_CTE_API Constant Tone Extension
/// @{

#if (BLE_CON_CTE_REQ)
/**
 ****************************************************************************************
 * @brief Set Callbacks used to handle IQ reports
 *
 * @param[in] p_cbs         Pointer to the callback set
 *
 * @return Return function execution status (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t gapc_le_set_cte_callbacks(const gapc_le_cte_cb_t* p_cbs);

/**
 ****************************************************************************************
 * @brief Configure constant tone extension reception parameters.
 *
 * @param[in] conidx                Connection index
 * @param[in] metainfo              Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] sample_enable         True to enable sampling, false otherwise
 * @param[in] slot_dur              Slot durations (1: 1us | 2: 2us)
 * @param[in] switching_pattern_len Length of switching pattern (number of antenna IDs in the pattern)
 * @param[in] p_antenna_id          Pointer to antenna IDs
 * @param[in] cmp_cb                Function called when procedure is over.
 *
 * @return Return function execution status (see #hl_err enumeration)
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapc_proc_cmp_cb callback execution
 ****************************************************************************************
 */
uint16_t gapc_le_configure_cte_rx(uint8_t conidx, uint32_t metainfo, bool sample_enable, uint8_t slot_dur,
                                  uint8_t switching_pattern_len, const uint8_t* p_antenna_id, gapc_proc_cmp_cb cmp_cb);

/**
 ****************************************************************************************
 * @brief Control transmission of constant tone extension requests initiated by controller.
 *
 * @param[in] conidx      Connection index
 * @param[in] metainfo    Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] enable      True to enable CTE requests, false otherwise
 * @param[in] interval    CTE request interval (in number of connection events)
 * @param[in] cte_length  Requested CTE length (in 8us unit)
 * @param[in] cte_type    Requested CTE type (0: AOA | 1: AOD-1us | 2: AOD-2us)
 * @param[in] cmp_cb      Function called when procedure is over.
 *
 * @return Return function execution status (see #hl_err enumeration)
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapc_proc_cmp_cb callback execution
 ****************************************************************************************
 */
uint16_t gapc_le_control_cte_request(uint8_t conidx, uint32_t metainfo, bool enable, uint16_t interval,
                                     uint8_t cte_length, uint8_t cte_type, gapc_proc_cmp_cb cmp_cb);
#endif //(BLE_CON_CTE_REQ)
#if (BLE_CON_CTE_RSP)
/**
 ****************************************************************************************
 * @brief Configure constant tone extension transmission parameters.
 *
 * @param[in] conidx                Connection index
 * @param[in] metainfo              Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] cte_types             CTE types (bit0: AOA | bit1: AOD-1us | bit2: AOD-2us) (see #gap_le_cte_type_bf enumeration)
 * @param[in] switching_pattern_len Length of switching pattern (number of antenna IDs in the pattern)
 * @param[in] p_antenna_id          Pointer to antenna IDs
 * @param[in] cmp_cb                Function called when procedure is over.
 *
 * @return Return function execution status (see #hl_err enumeration)
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapc_proc_cmp_cb callback execution
 ****************************************************************************************
 */
uint16_t gapc_le_configure_cte_tx(uint8_t conidx, uint32_t metainfo, uint8_t cte_types, uint8_t switching_pattern_len,
                                  const uint8_t* p_antenna_id, gapc_proc_cmp_cb cmp_cb);

/**
 ****************************************************************************************
 * @brief Control if controller answers constant tone extension requests.
 *
 * @param[in] conidx      Connection index
 * @param[in] metainfo    Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] enable      True to enable CTE responses, false otherwise
 * @param[in] cmp_cb      Function called when procedure is over.
 *
 * @return Return function execution status (see #hl_err enumeration)
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapc_proc_cmp_cb callback execution
 ****************************************************************************************
 */
uint16_t gapc_le_control_cte_response(uint8_t conidx, uint32_t metainfo, bool enable, gapc_proc_cmp_cb cmp_cb);
#endif // (BLE_CON_CTE_RSP)
/// @} GAPC_CTE_API
#endif // (BLE_CON_CTE_RSP || BLE_CON_CTE_REQ)


#endif /* GAPC_LE_H_ */
