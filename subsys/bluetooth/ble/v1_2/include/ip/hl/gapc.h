/**
 ****************************************************************************************
 *
 * @file gapc.h
 *
 * @brief Generic Access Profile Controller - Native API .
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef GAPC_H_
#define GAPC_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @addtogroup GAPC_API Generic Access Profile Controller (GAPC)
 * @ingroup GAP_API
 * @brief  Generic Access Profile Controller.
 *
 * The GAP Controller module is responsible for providing an API to the application in
 * to perform GAP action related to a BLE connection (pairing, update parameters,
 * disconnect ...).
 *
 * @{
 * @}
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup GAPC_ENUM_API Enumerations
 * @ingroup GAPC_API
 * @{
 * @}
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup GAPC_STRUCT_API Structures and types
 * @ingroup GAPC_API
 * @{
 * @}
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup GAPC_PROC_API Procedures Handlers
 * @ingroup GAPC_API
 * @{
 * @}
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup GAPC_CON_REQ_API Connection establishment handling
 * @ingroup GAPC_API
 * @brief Function and callback to use in order to handle an \glos{LE} or \glos{BT} connection establishment
 * @{
 * @}
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup GAPC_CON_UP_API Connection Update
 * @ingroup GAPC_API
 * @brief Function and callback to use to negotiate and apply new connection parameters
 * @{
 * @}
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "gap.h"
#include "gapm.h"
#if (BLE_HOST_PRESENT)
#include "gap_le.h"
#endif // (BLE_HOST_PRESENT)
#include "co_buf.h"
#include "co_bt_defines.h"
#include <stdbool.h>
#include <stdint.h>
#include "prf_types.h"
#include "ke_msg.h"

/*
 * DEFINES
 ****************************************************************************************
 */

/// @addtogroup GAPC_ENUM_API
/// @{

/// Option for PHY configuration
enum gapc_phy_option
{
    /// No preference for rate used when transmitting on the LE Coded PHY
    GAPC_PHY_OPT_LE_CODED_ALL_RATES = 0u,
    /// 500kbps rate preferred when transmitting on the LE Coded PHY
    GAPC_PHY_OPT_LE_CODED_500K_RATE = 1u,
    /// 125kbps  when transmitting on the LE Coded PHY
    GAPC_PHY_OPT_LE_CODED_125K_RATE = 2u,
};

/// Client bond information
enum gapc_cli_info
{
    /// Service changed indication enabled
    GAPC_CLI_SVC_CHANGED_IND_EN_BIT = (1u << 0),
    GAPC_CLI_SVC_CHANGED_IND_EN_POS = 0u,
    /// Database updated since last connection
    GAPC_CLI_DB_UPDATED_BIT = (1u << 1),
    GAPC_CLI_DB_UPDATED_POS = 1u,
};

/// Client supported features
enum gapc_cli_feat
{
    /// Robust Cache feature enabled
    GAPC_CLI_ROBUST_CACHE_EN_BIT = (1u << 0),
    GAPC_CLI_ROBUST_CACHE_EN_POS = 0u,

    /// The client supports Enhanced ATT bearer
    GAPC_CLI_EATT_SUPPORTED_BIT = (1u << 1),
    GAPC_CLI_EATT_SUPPORTED_POS = 1u,
    /// The client supports Multiple Handle Value Notifications
    GAPC_CLI_MULT_NTF_SUPPORTED_BIT = (1u << 2),
    GAPC_CLI_MULT_NTF_SUPPORTED_POS = 2u,

    GAPC_CLI_NO_FEAT = 0,
};

/// Server supported features
enum gapc_srv_feat
{
    /// Enhanced ATT bearer supported
    GAPC_SRV_EATT_SUPPORTED_BIT = (1u << 0),
    GAPC_SRV_EATT_SUPPORTED_POS = 0u,

    GAPC_SRV_NO_FEAT = 0u,
};


/// Power Control Bit Field parameters.
enum gapc_pwr_ctrl_flags
{
    /// bit[0] - Sender is at the minimum supported power level
    GAPC_PWR_CTRL_MIN_BIT = 0x01u,
    GAPC_PWR_CTRL_MIN_POS = 0u,

    /// bit[1] - Sender is at the maximum supported power level
    GAPC_PWR_CTRL_MAX_BIT = 0x02u,
    GAPC_PWR_CTRL_MAX_POS = 1u,
};

/// Specify which PHY the Controller is specifying transmit power.
enum gapc_phy_pwr_value
{
    /// 1 Mbits PHY
    GAPC_PHY_PWR_1MBPS_VALUE = 1u,
    /// 2 Mbits PHY
    GAPC_PHY_PWR_2MBPS_VALUE = 2u,
    /// LE Coded PHY with S=8 data coding
    GAPC_PHY_PWR_S8_CODED_VALUE = 3u,
    /// LE Coded PHY with S=2 data coding
    GAPC_PHY_PWR_S2_CODED_VALUE = 4u,
};


/// @} GAPC_ENUM_API

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// @addtogroup GAPC_STRUCT_API
/// @{

/// Connection parameters information
/*@TRACE*/
typedef struct
{
    /// Connection interval in 1.25ms unit
    uint16_t interval;
    /// Connection latency value (in number of connection events)
    uint16_t latency;
    /// Supervision timeout in 10ms unit
    uint16_t sup_to;
} gapc_le_con_param_t;

/// LE Connection parameter negotiated
typedef struct
{
    /// Minimum Connection interval value (1.25 ms unit, range [0x0006, 0x0C80])
    uint16_t interval_min;
    /// Maximum Connection interval value (1.25 ms unit, range [0x0006, 0x0C80])
    uint16_t interval_max;
    /// Connection latency value (in number of connection event. range [0x0000, 0x01F3])
    uint16_t latency;
    /// Supervision timeout (10ms unit, range [0x000A, 0x0C80])
    uint16_t sup_to;
} gapc_le_con_param_nego_t;

/// LE Connection parameter negotiated with Connection event length
typedef struct
{
    /// Inherited connection parameters
    gapc_le_con_param_nego_t hdr;
    /// Minimum connection Event Duration (0.625 ms unit)
    uint16_t ce_len_min;
    /// Maximum connection Event Duration (0.625 ms unit)
    uint16_t ce_len_max;
} gapc_le_con_param_nego_with_ce_len_t;

/// Bond data
/*@TRACE*/
typedef struct
{
    /// Local CSRK value
    gap_sec_key_t local_csrk;
    /// Local signature counter value
    uint32_t local_sign_counter;
    /// Remote CSRK value
    gap_sec_key_t remote_csrk;
    /// Remote signature counter value
    uint32_t remote_sign_counter;
    /// Pairing level (see #gap_pairing_lvl enumeration)
    uint8_t pairing_lvl;
    /// Client bond data information (see #gapc_cli_info enumeration)
    uint8_t cli_info;
    /// LTK or link key exchanged during pairing.
    bool enc_key_present;
    /// Client supported features (see #gapc_cli_feat enumeration)
    uint8_t cli_feat;
    /// Peer GATT Service Start handle
    uint16_t gatt_start_hdl;
    /// Peer GATT Service End Handle
    uint16_t gatt_end_hdl;
    /// Peer Service Change value handle
    uint16_t svc_chg_hdl;
    /// Server supported features (see #gapc_srv_feat enumeration)
    uint8_t srv_feat;
} gapc_bond_data_t;

/// Updated bond data information
/*@TRACE*/
typedef struct
{
    /// Local SignCounter value
    uint32_t local_sign_counter;
    /// Peer SignCounter value
    uint32_t peer_sign_counter;
    /// Peer GATT Service start handle
    uint16_t gatt_start_hdl;
    /// Peer GATT Service end Handle
    uint16_t gatt_end_hdl;
    /// Peer Service Change characteristic value handle
    uint16_t svc_chg_hdl;
    /// Client bond data information (see #gapc_cli_info enumeration)
    uint8_t cli_info;
    /// Client supported features (see #gapc_cli_feat enumeration)
    uint8_t cli_feat;
    /// Server supported features (see #gapc_srv_feat enumeration)
    uint8_t srv_feat;
} gapc_bond_data_updated_t;

/// Peer version information
typedef struct
{
    /// Manufacturer company identifier
    uint16_t company_id;
    /// LMP subversion
    uint16_t lmp_subversion;
    /// LMP version
    uint8_t lmp_version;
} gapc_version_t;


/// @} GAPC_STRUCT_API

/*
 * CALLBACK DEFINITIONS
 ****************************************************************************************
 */

/// @addtogroup GAPC_PROC_API
/// @{

/**
 ****************************************************************************************
 * Callback executed when a procedure is completed.
 *
 * @param[in] conidx    Connection index
 * @param[in] metainfo  Metadata information provided by API user (see \glos{METAINFO})
 * @param[in] status    Status of procedure execution (see #hl_err enumeration)
 *
 ****************************************************************************************
 */
typedef void (*gapc_proc_cmp_cb)(uint8_t conidx, uint32_t metainfo, uint16_t status);

/// @} GAPC_PROC_API

/// @addtogroup GAPC_CON_REQ_API
/// @{

/// Callback structure required to handle BT-Classic or LE connection request events
struct gapc_connection_req_cb
{
    /**
     ****************************************************************************************
     * @brief Callback executed once a connection has been established. The upper layer software shall
     *        execute #gapc_le_connection_cfm to enable ACL data reception and restore bond data.
     *
     * @note Mandatory for a connectable activity, optional otherwise
     *
     * @param[in] conidx        allocated connection index
     * @param[in] metainfo      Metadata information provided by API user (see \glos{METAINFO}) `
     *                          during device configuration (see #gapm_configure)
     * @param[in] actv_idx      Activity Local identifier
     * @param[in] role          Connection role (see #gap_con_role enumeration)
     * @param[in] p_con_params  Pointer to connection parameters
     * @param[in] clk_accuracy  Master clock accuracy in PPM
     ****************************************************************************************
     */
    void (*le_connection_req)(uint8_t conidx, uint32_t metainfo, uint8_t actv_idx, uint8_t role,
                              const gap_bdaddr_t* p_peer_addr, const gapc_le_con_param_t* p_con_params,
                              uint8_t clk_accuracy);

};

/// @} GAPC_CON_REQ_API

/**
 ****************************************************************************************
 * @addtogroup GAPC_CON_INFO_API Connection information
 * @ingroup GAPC_API
 * @brief Function and callback to use to get information about active connection or answer peer device requests.
 * @{
 ****************************************************************************************
 */

/// Callback structure required to handle general connection events
struct gapc_connection_info_cb
{
    /**
     ****************************************************************************************
     * @brief Callback executed when link is disconnected
     *
     * @note Mandatory callback.
     *
     * @param[in] conidx        Connection index
     * @param[in] metainfo      Metadata information provided by API user (see \glos{METAINFO})
     * @param[in] reason        Disconnection reason (see #hl_err enumeration)
     ****************************************************************************************
     */
    void (*disconnected)(uint8_t conidx, uint32_t metainfo, uint16_t reason);

    /**
     ****************************************************************************************
     * @brief Callback executed when connection bond data is updated.
     *
     * @note Optional callback.
     *
     * @param[in] conidx        Connection index
     * @param[in] metainfo      Metadata information provided by API user (see \glos{METAINFO})
     * @param[in] p_data        Pointer to updated bond data
     ****************************************************************************************
     */
    void (*bond_data_updated)(uint8_t conidx, uint32_t metainfo, const gapc_bond_data_updated_t* p_data);

    /**
     ****************************************************************************************
     * @brief Callback executed when an authenticated payload timeout has been detected.
     * (no encrypted data received after a specific duration ; see LE-PING)
     *
     * @note Optional callback.
     *
     * @param[in] conidx        Connection index
     * @param[in] metainfo      Metadata information provided by API user (see \glos{METAINFO})
     ****************************************************************************************
     */
    void (*auth_payload_timeout)(uint8_t conidx, uint32_t metainfo);

    /**
     ****************************************************************************************
     * @brief Callback executed when all ATT bearer are closed onto a connection
     *
     * @note Optional callback.
     *
     * @param[in] conidx        Connection index
     * @param[in] metainfo      Metadata information provided by API user (see \glos{METAINFO})
     ****************************************************************************************
     */
    void (*no_more_att_bearer)(uint8_t conidx, uint32_t metainfo);

    #if (!(0))
    /**
     ****************************************************************************************
     * @brief Callback executed when peer database hash value is read
     *
     * @note Optional callback.
     *
     * @param[in] conidx        Connection index
     * @param[in] metainfo      Metadata information provided by API user (see \glos{METAINFO})
     * @param[in] handle        Handle value of HASH attribute
     * @param[in] p_hash        Pointer to the peer HASH attribute value
     ****************************************************************************************
     */
    void (*cli_hash_info)(uint8_t conidx, uint32_t metainfo, uint16_t handle, const uint8_t* p_hash);
    #endif // (!(0))

    /**
     ****************************************************************************************
     * @brief Callback executed when peer request device name information
     * Upper layer SW shall call #gapc_le_get_name_cfm to provide expected information
     *
     * @note Mandatory callback if attribute present.
     *
     * @param[in] conidx        Connection index
     * @param[in] metainfo      Metadata information provided by API user (see \glos{METAINFO})
     * @param[in] token         Token value that must be returned in confirmation
     * @param[in] offset        Device Name data offset
     * @param[in] max_length    Maximum name length to return (starting from offset)
     ****************************************************************************************
     */
    void (*name_get)(uint8_t conidx, uint32_t metainfo, uint16_t token, uint16_t offset, uint16_t max_length);

    /**
     ****************************************************************************************
     * @brief Callback executed when peer request appearance information
     * Upper layer SW shall call #gapc_le_get_appearance_cfm to provide expected information
     *
     * @note Mandatory callback if attribute present
     *
     * @param[in] conidx        Connection index
     * @param[in] metainfo      Metadata information provided by API user (see \glos{METAINFO})
     * @param[in] token         Token value that must be returned in confirmation
     ****************************************************************************************
     */
    void (*appearance_get)(uint8_t conidx, uint32_t metainfo, uint16_t token);

    /**
     ****************************************************************************************
     * @brief Callback executed when peer request slave preferred connection parameters information
     * Upper layer SW shall call #gapc_le_get_preferred_periph_params_cfm to provide expected information
     *
     * @note Mandatory callback if attribute present
     *
     * @param[in] conidx        Connection index
     * @param[in] metainfo      Metadata information provided by API user (see \glos{METAINFO})
     * @param[in] token         Token value that must be returned in confirmation
     ****************************************************************************************
     */
    void (*slave_pref_param_get)(uint8_t conidx, uint32_t metainfo, uint16_t token);

    /**
     ****************************************************************************************
     * @brief Callback executed when peer request modification of device name information
     * Upper layer SW shall call #gapc_le_set_name_cfm to accept or reject request
     *
     * @note Optional callback - Automatically rejected if not set
     *
     * @param[in] conidx        Connection index
     * @param[in] metainfo      Metadata information provided by API user (see \glos{METAINFO})
     * @param[in] token         Token value that must be returned in confirmation
     * @param[in] p_buf         Pointer to buffer that contains new device name
     ****************************************************************************************
     */
    void (*name_set)(uint8_t conidx, uint32_t metainfo, uint16_t token, co_buf_t* p_buf);

    /**
     ****************************************************************************************
     * @brief Callback executed when peer request modification of device appearance information
     * Upper layer SW shall call #gapc_le_set_appearance_cfm to accept or reject request
     *
     * @note Optional callback - Automatically rejected if not set
     *
     * @param[in] conidx        Connection index
     * @param[in] metainfo      Metadata information provided by API user (see \glos{METAINFO})
     * @param[in] token         Token value that must be returned in confirmation
     * @param[in] appearance    New appearance value
     ****************************************************************************************
     */
    void (*appearance_set)(uint8_t conidx, uint32_t metainfo, uint16_t token, uint16_t appearance);

};

/**
 ****************************************************************************************
 * Callback executed when get authenticated payload timeout value procedure is completed.
 *
 * @param[in] conidx    Connection index
 * @param[in] metainfo  Metadata information provided by API user (see \glos{METAINFO})
 * @param[in] status    Status of procedure execution (see #hl_err enumeration)
 * @param[in] timeout   Authenticated payload timeout (10ms unit)
 *
 ****************************************************************************************
 */
typedef void (*gapc_get_ping_to_cmp_cb)(uint8_t conidx, uint32_t metainfo, uint16_t status, uint16_t timeout);

/**
 ****************************************************************************************
 * Callback executed when read attribute name procedure is completed.
 *
 * @param[in] conidx    Connection index
 * @param[in] metainfo  Metadata information provided by API user (see \glos{METAINFO})
 * @param[in] status    Status of procedure execution (see #hl_err enumeration)
 * @param[in] handle    Attribute handle
 * @param[in] p_name    Pointer to buffer that contains peer device name
 *
 ****************************************************************************************
 */
typedef void (*gapc_get_peer_name_cmp_cb)(uint8_t conidx, uint32_t metainfo, uint16_t status, uint16_t handle,
                                          co_buf_t* p_name);

/**
 ****************************************************************************************
 * Callback executed when get peer version procedure is completed.
 *
 * @param[in] conidx    Connection index
 * @param[in] metainfo  Metadata information provided by API user (see \glos{METAINFO})
 * @param[in] status    Status of procedure execution (see #hl_err enumeration)
 * @param[in] p_version Pointer to peer version information
 *
 ****************************************************************************************
 */
typedef void (*gapc_get_peer_version_cmp_cb)(uint8_t conidx, uint32_t metainfo, uint16_t status,
                                             const gapc_version_t* p_version);

/**
 ****************************************************************************************
 * Callback executed when get RSSI value procedure is completed.
 *
 * @param[in] conidx    Connection index
 * @param[in] metainfo  Metadata information provided by API user (see \glos{METAINFO})
 * @param[in] status    Status of procedure execution (see #hl_err enumeration)
 * @param[in] rssi      Latest measured RSSI value (in dBm)
 *
 ****************************************************************************************
 */
typedef void (*gapc_get_rssi_cmp_cb)(uint8_t conidx, uint32_t metainfo, uint16_t status, int8_t rssi);

/// @} GAPC_CON_INFO_API

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/// @addtogroup GAPC_CON_UP_API Connection Update
/// @{
///
/**
 ****************************************************************************************
 * @brief Ask for BT classic or LE link disconnection.
 *
 * @param[in] conidx    Connection index
 * @param[in] metainfo  Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] reason    Disconnection error (@ref hl_err). Only following error codes should be used:
 *                      - #LL_ERR_AUTH_FAILURE
 *                      - #LL_ERR_REMOTE_USER_TERM_CON
 *                      - #LL_ERR_REMOTE_DEV_TERM_LOW_RESOURCES
 *                      - #LL_ERR_REMOTE_DEV_POWER_OFF
 *                      - #LL_ERR_UNSUPPORTED_REMOTE_FEATURE
 *                      - #LL_ERR_PAIRING_WITH_UNIT_KEY_NOT_SUP
 *                      - #LL_ERR_UNACCEPTABLE_CONN_PARAM
 * @param[in] cmp_cb    Function called when procedure is over.
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapc_proc_cmp_cb callback execution
 ****************************************************************************************
 */
uint16_t gapc_disconnect(uint8_t conidx, uint32_t metainfo, uint16_t reason, gapc_proc_cmp_cb cmp_cb);

/// @} GAPC_CON_UP_API

/// @addtogroup GAPC_CON_INFO_API
/// @{
/**
 ****************************************************************************************
 * @brief Retrieve connection index from connection handle.
 *
 * @param[in] conhdl Connection handle
 *
 * @return Return found connection index, GAP_INVALID_CONIDX if not found.
 ****************************************************************************************
 */
uint8_t gapc_get_conidx(uint16_t conhdl);

/**
 ****************************************************************************************
 * @brief Retrieve connection handle from connection index.
 *
 * @param[in] conidx Connection index
 *
 * @return Return found connection handle, GAP_INVALID_CONHDL if not found.
 ****************************************************************************************
 */
uint16_t gapc_get_conhdl(uint8_t conidx);

/**
 ****************************************************************************************
 * @brief Check if a connection for provided connection index is established
 *
 * @param[in] conidx Connection index
 *
 * @return Return true if connection is established; false otherwise.
 ****************************************************************************************
 */
bool gapc_is_established(uint8_t conidx);

/**
 ****************************************************************************************
 * @brief Retrieve connection role from connection index.
 *
 * @param[in] conidx Connection index
 *
 * @return Return found connection role (see #gap_con_role enumeration)
 ****************************************************************************************
 */
uint8_t gapc_get_role(uint8_t conidx);

/**
 ****************************************************************************************
 * @brief Get if peer device and local device are bonded
 *
 * @param[in] conidx    Connection index
 *
 * @return Return true if a bond exists with peer device, false otherwise
 ****************************************************************************************
 */
bool gapc_is_bonded(uint8_t conidx);

/**
 ****************************************************************************************
 * @brief Read peer device name characteristic present in attribute database
 *
 * @param[in] conidx    Connection index
 * @param[in] metainfo  Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] cmp_cb    Function called when procedure is over.
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapc_get_peer_name_cmp_cb callback execution
 ****************************************************************************************
 */
uint16_t gapc_get_peer_name(uint8_t conidx, uint32_t metainfo, gapc_get_peer_name_cmp_cb cmp_cb);

/**
 ****************************************************************************************
 * @brief Read peer version information
 *
 * @param[in] conidx    Connection index
 * @param[in] metainfo  Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] cmp_cb    Function called when procedure is over.
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapc_get_peer_version_cmp_cb callback execution
 ****************************************************************************************
 */
uint16_t gapc_get_peer_version(uint8_t conidx, uint32_t metainfo, gapc_get_peer_version_cmp_cb cmp_cb);

/**
 ****************************************************************************************
 * @brief Get latest measured RSSI value onto connection
 *
 * @param[in] conidx    Connection index
 * @param[in] metainfo  Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] cmp_cb    Function called when procedure is over.
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapc_get_rssi_cmp_cb callback execution
 ****************************************************************************************
 */
uint16_t gapc_get_rssi(uint8_t conidx, uint32_t metainfo, gapc_get_rssi_cmp_cb cmp_cb);

/**
 ****************************************************************************************
 * @brief Get configured authentication timeout value
 *
 * @param[in] conidx    Connection index
 * @param[in] metainfo  Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] cmp_cb    Function called when procedure is over.
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapc_get_ping_to_cmp_cb callback execution
 ****************************************************************************************
 */
uint16_t gapc_get_ping_to(uint8_t conidx, uint32_t metainfo, gapc_get_ping_to_cmp_cb cmp_cb);

/// @} GAPC_CON_INFO_API

/// @addtogroup GAPC_CON_UP_API
/// @{

/**
 ****************************************************************************************
 * @brief Function used to set authenticated payload timeout (BT-Ping / LE Ping).
 *        Feature used to ensure that encrypted packet are exchanged before timer expiration.
 *
 * @param[in] conidx        Connection index
 * @param[in] metainfo      Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] timeout       Authenticated payload timeout (N*10ms)
 * @param[in] cmp_cb        Function called when procedure is over.
 *
 * @return Return function execution status (see #hl_err enumeration)
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapc_proc_cmp_cb callback execution
 ****************************************************************************************
 */
uint16_t gapc_set_ping_to(uint8_t conidx, uint32_t metainfo, uint16_t timeout, gapc_proc_cmp_cb cmp_cb);

/// @} GAPC_CON_UP_API

#endif /* GAPC_H_ */
