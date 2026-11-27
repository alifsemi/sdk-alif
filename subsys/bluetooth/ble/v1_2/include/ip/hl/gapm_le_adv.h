/**
 ****************************************************************************************
 *
 * @file gapm_le_adv.h
 *
 * @brief Generic Access Profile Manager - Low Energy Advertising Activities
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef GAPM_LE_ADV_H_
#define GAPM_LE_ADV_H_

#include "rom_build_cfg.h"

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "gapm_le.h"

/**
 ****************************************************************************************
 * @addtogroup GAPM_LE_ADV_ACTV_API LE Advertising and Periodic Advertising
 *
 * @ingroup GAPM_ACTV_API
 *
 * @brief Create and control Advertising or Periodic Advertising activities.
 *
 * Application can control several advertising activities in parallel.
 *
 * The application must follow the #gapm_le_adv_cb_actv_t callback interface to handle activities events:
 * \snippet{lineno} app_connectable_adv.c APP_CONNECTABLE_ADV_ACTV_CB
 *
 * Application can then create an advertising activity using:
 *  - #gapm_le_create_adv_legacy: Legacy Advertising
 *  - #gapm_le_create_adv_extended: Extended Advertising
 *  - #gapm_le_create_adv_periodic: Periodic Advertising
 *  - #gapm_le_create_adv_periodic_with_cte: Periodic Advertising with Constant Tone Extension
 *
 * example:
 * \snippet{lineno} app_connectable_adv.c APP_CONNECTABLE_ADV_ACTV_CREATE_AND_START
 *
 *
 * Once activity is created, #gapm_le_adv_cb_actv_t.created is called, then application can set
 * advertising data and start the activity.
 *
 * An application example is available in \ref app_test_le_periph.c
 *
 * @note At least #GAP_ROLE_LE_PERIPHERAL role is required for a connectable advertising and
 *       #GAP_ROLE_LE_BROADCASTER for a non-connectable advertising.
 *
 * @{
 ****************************************************************************************
 */

/*
 * DEFINES
 ****************************************************************************************
 */

/// Length of buffer header length required by Advertising activity
#define GAPM_LE_ADV_BUFFER_HEADER_LEN (0u)
/// Length of buffer tail length required by Advertising activity
#define GAPM_LE_ADV_BUFFER_TAIL_LEN (0u)
/// Bit field for Periodic Advertising Properties field
#define GAPM_ADV_PERIOD_PROP_TX_POWER_BIT (CO_BIT(6))

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Type of advertising that can be created
enum gapm_le_adv_type
{
    /// Legacy advertising
    GAPM_ADV_TYPE_LEGACY = 0u,
    /// Extended advertising
    GAPM_ADV_TYPE_EXTENDED,
    #if (BLE_PER_ADV)
    /// Periodic advertising
    GAPM_ADV_TYPE_PERIODIC,
    #endif // (BLE_PER_ADV)
    GAPM_ADV_TYPE_MAX,
};

/// Address change reasons bit field meaning
enum gapm_le_adv_addr_change_reasons_bf
{
    /// Change the address whenever the advertising data changes
    GAPM_ADV_ADDR_CHANGE_REASON_DATA_BIT = 0x01u,
    /// See #GAPM_ADV_ADDR_CHANGE_REASON_DATA_BIT
    GAPM_ADV_ADDR_CHANGE_REASON_DATA_POS = 0u,

    /// Change the address whenever the scan response data changes
    GAPM_ADV_ADDR_CHANGE_REASON_SCAN_RSP_BIT = 0x02u,
    /// See #GAPM_ADV_ADDR_CHANGE_REASON_SCAN_RSP_BIT
    GAPM_ADV_ADDR_CHANGE_REASON_SCAN_RSP_POS = 1u,

    /// RFU
    GAPM_ADV_ADDR_CHANGE_REASON_RFU_MASK = (GAPM_ADV_ADDR_CHANGE_REASON_DATA_BIT
                                                | GAPM_ADV_ADDR_CHANGE_REASON_SCAN_RSP_BIT),
};

/// Advertising properties bit field bit positions
enum gapm_le_adv_prop_bf
{
    /// See #GAPM_ADV_PROP_CONNECTABLE_BIT
    GAPM_ADV_PROP_CONNECTABLE_POS = 0u,
    /// Indicate that advertising is connectable, reception of CONNECT_REQ or AUX_CONNECT_REQ
    /// PDUs is accepted. Not applicable for periodic advertising.
    GAPM_ADV_PROP_CONNECTABLE_BIT = CO_BIT(GAPM_ADV_PROP_CONNECTABLE_POS),

    /// See #GAPM_ADV_PROP_SCANNABLE_BIT
    GAPM_ADV_PROP_SCANNABLE_POS = 1u,
    /// Indicate that advertising is scannable, reception of SCAN_REQ or AUX_SCAN_REQ PDUs is
    /// accepted
    GAPM_ADV_PROP_SCANNABLE_BIT = CO_BIT(GAPM_ADV_PROP_SCANNABLE_POS),

    /// See #GAPM_ADV_PROP_DIRECTED_BIT
    GAPM_ADV_PROP_DIRECTED_POS = 2u,
    /// Indicate that advertising targets a specific device. Only apply in following cases:
    ///   - Legacy advertising: if connectable
    ///   - Extended advertising: connectable or (non connectable and non discoverable)
    GAPM_ADV_PROP_DIRECTED_BIT = CO_BIT(GAPM_ADV_PROP_DIRECTED_POS),

    /// See #GAPM_ADV_PROP_HDC_BIT
    GAPM_ADV_PROP_HDC_POS = 3u,
    /// Indicate that High Duty Cycle has to be used for advertising on primary channel
    /// Apply only if created advertising is not an extended advertising
    GAPM_ADV_PROP_HDC_BIT = CO_BIT(GAPM_ADV_PROP_HDC_POS),

    /// See #GAPM_ADV_PROP_RESERVED_4_BIT
    GAPM_ADV_PROP_RESERVED_4_POS = 4u,
    /// Bit 4 is reserved
    GAPM_ADV_PROP_RESERVED_4_BIT = CO_BIT(GAPM_ADV_PROP_RESERVED_4_POS),

    /// See #GAPM_ADV_PROP_ANONYMOUS_BIT
    GAPM_ADV_PROP_ANONYMOUS_POS = 5u,
    /// Enable anonymous mode. Device address won't appear in send PDUs
    /// Valid only if created advertising is an extended advertising
    GAPM_ADV_PROP_ANONYMOUS_BIT = CO_BIT(GAPM_ADV_PROP_ANONYMOUS_POS),

    /// See #GAPM_ADV_PROP_TX_PWR_BIT
    GAPM_ADV_PROP_TX_PWR_POS = 6u,
    /// Include TX Power in the extended header of the advertising PDU.
    /// Valid only if created advertising is not a legacy advertising
    GAPM_ADV_PROP_TX_PWR_BIT = CO_BIT(GAPM_ADV_PROP_TX_PWR_POS),

    #if (BLE_PER_ADV)
    /// See #GAPM_ADV_PROP_PER_TX_PWR_BIT
    GAPM_ADV_PROP_PER_TX_PWR_POS = 7u,
    /// Include TX Power in the periodic advertising PDU.
    /// Valid only if created advertising is a periodic advertising
    GAPM_ADV_PROP_PER_TX_PWR_BIT = CO_BIT(GAPM_ADV_PROP_PER_TX_PWR_POS),
    #endif // (BLE_PER_ADV)

    /// See #GAPM_ADV_PROP_SCAN_REQ_NTF_EN_BIT
    GAPM_ADV_PROP_SCAN_REQ_NTF_EN_POS = 8u,
    /// Indicate if application must be informed about received scan requests PDUs
    GAPM_ADV_PROP_SCAN_REQ_NTF_EN_BIT = CO_BIT(GAPM_ADV_PROP_SCAN_REQ_NTF_EN_POS),

    #if (BT_53)
    /// See #GAPM_ADV_PROP_ADDR_CHANGE_REASONS_MASK
    GAPM_ADV_PROP_ADDR_CHANGE_REASONS_LSB = 9u,
    /// Advertising address change reasons. When one bit is set, it means that a new advertising address will be generated.
    /// (see #gapm_le_adv_addr_change_reasons_bf enumeration)
    ///
    /// If #gapm_le_adv_create_param_t.peer_addr correspond to an entry in resolving address list,
    /// the advertiser address is automatically renew if:
    ///  - Advertising data is renew and #GAPM_ADV_ADDR_CHANGE_REASON_DATA_BIT is enabled
    ///  - Scan response data is renew and #GAPM_ADV_ADDR_CHANGE_REASON_SCAN_RSP_BIT is enabled
    ///
    /// Renewal of advertising data must be handled by application, using a timer for instance, to ensure that different
    /// advertising address will not be used for same data.
    GAPM_ADV_PROP_ADDR_CHANGE_REASONS_MASK = CO_MASK(2, GAPM_ADV_PROP_ADDR_CHANGE_REASONS_LSB),
    #endif // (BT_53)

    /// See #GAPM_ADV_PROP_FORCE_AD_TYPE_FLAG_BR_EDR_NOT_SUPPORTED_BIT
    GAPM_ADV_PROP_FORCE_AD_TYPE_FLAG_BR_EDR_NOT_SUPPORTED_POS = 11u,
    /// Force AD-Type BR/EDR not supported Flag to 1
    /// note Only for a BT Dual mode capable host otherwise automatically forced to 1
    GAPM_ADV_PROP_FORCE_AD_TYPE_FLAG_BR_EDR_NOT_SUPPORTED_BIT = CO_BIT(GAPM_ADV_PROP_FORCE_AD_TYPE_FLAG_BR_EDR_NOT_SUPPORTED_POS),

};

/// Advertising discovery mode
enum gapm_le_adv_discovery_mode
{
    /// Mode in non-discoverable
    GAPM_ADV_MODE_NON_DISC = 0u,
    /// Mode in general discoverable
    GAPM_ADV_MODE_GEN_DISC,
    /// Mode in limited discoverable
    GAPM_ADV_MODE_LIM_DISC,
    /// Broadcast mode without presence of AD_TYPE_FLAG in advertising data
    GAPM_ADV_MODE_BEACON,
    GAPM_ADV_MODE_MAX,
};

#if (BLE_PER_ADV)
/// Periodic advertising information bit field bit positions
enum gapm_le_adv_periodic_info_bf
{
    /// Indicate that ADI field is included in AUX_SYNC_IND PDUS.
    GAPM_PER_ADV_INFO_ADI_INCLUDED_POS = 0u,
    GAPM_PER_ADV_INFO_ADI_INCLUDED_BIT = 0x01u,
};
#endif // (BLE_PER_ADV)

/// Advertising filter policy
enum gapm_le_adv_fllter_policy
{
    ///Allow both scan and connection requests from anyone
    GAPM_ADV_ALLOW_SCAN_ANY_CON_ANY = 0x00u,
    ///Allow both scan req from Filter Accept List devices only and connection req from anyone
    GAPM_ADV_ALLOW_SCAN_WLST_CON_ANY,
    ///Allow both scan req from anyone and connection req from Filter Accept List devices only
    GAPM_ADV_ALLOW_SCAN_ANY_CON_WLST,
    ///Allow scan and connection requests from Filter Accept List devices only
    GAPM_ADV_ALLOW_SCAN_WLST_CON_WLST,
};

// -------------------------------------------------------------------------------------
// Masks for advertising properties
// -------------------------------------------------------------------------------------

/// Advertising properties configurations for legacy advertising
enum gapm_le_adv_legacy_prop
{
    /// Non connectable non scannable advertising
    GAPM_ADV_PROP_NON_CONN_NON_SCAN_MASK = 0x0000u,
    /// Broadcast non scannable advertising - Discovery mode must be Non Discoverable
    GAPM_ADV_PROP_BROADCAST_NON_SCAN_MASK = GAPM_ADV_PROP_NON_CONN_NON_SCAN_MASK,
    /// Non connectable scannable advertising
    GAPM_ADV_PROP_NON_CONN_SCAN_MASK = GAPM_ADV_PROP_SCANNABLE_BIT,
    /// Broadcast non scannable advertising - Discovery mode must be Non Discoverable
    GAPM_ADV_PROP_BROADCAST_SCAN_MASK = GAPM_ADV_PROP_NON_CONN_SCAN_MASK,
    /// Undirected connectable advertising
    GAPM_ADV_PROP_UNDIR_CONN_MASK = GAPM_ADV_PROP_CONNECTABLE_BIT | GAPM_ADV_PROP_SCANNABLE_BIT,
    /// Directed connectable advertising
    GAPM_ADV_PROP_DIR_CONN_MASK = GAPM_ADV_PROP_DIRECTED_BIT | GAPM_ADV_PROP_CONNECTABLE_BIT,
    /// Directed connectable with Low Duty Cycle
    GAPM_ADV_PROP_DIR_CONN_LDC_MASK = GAPM_ADV_PROP_DIR_CONN_MASK,
    /// Directed connectable with High Duty Cycle
    GAPM_ADV_PROP_DIR_CONN_HDC_MASK = GAPM_ADV_PROP_DIR_CONN_MASK | GAPM_ADV_PROP_HDC_BIT,
};

/// Advertising properties configurations for extended advertising
enum gapm_le_adv_extended_prop
{
    /// Non connectable non scannable extended advertising
    GAPM_EXT_ADV_PROP_NON_CONN_NON_SCAN_MASK = 0x0000u,
    /// Non connectable scannable extended advertising
    GAPM_EXT_ADV_PROP_NON_CONN_SCAN_MASK = GAPM_ADV_PROP_SCANNABLE_BIT,
    /// Non connectable scannable directed extended advertising
    GAPM_EXT_ADV_PROP_NON_CONN_SCAN_DIR_MASK = GAPM_ADV_PROP_SCANNABLE_BIT | GAPM_ADV_PROP_DIRECTED_BIT,
    /// Non connectable anonymous directed extended advertising
    GAPM_EXT_ADV_PROP_ANONYM_DIR_MASK = GAPM_ADV_PROP_ANONYMOUS_BIT | GAPM_ADV_PROP_DIRECTED_BIT,
    /// Undirected connectable extended advertising
    GAPM_EXT_ADV_PROP_UNDIR_CONN_MASK = GAPM_ADV_PROP_CONNECTABLE_BIT,
    /// Directed connectable extended advertising
    GAPM_EXT_ADV_PROP_DIR_CONN_MASK = GAPM_ADV_PROP_CONNECTABLE_BIT | GAPM_ADV_PROP_DIRECTED_BIT,
};

#if (BLE_PER_ADV)
/// Advertising properties configurations for periodic advertising
enum gapm_le_adv_periodic_prop
{
    /// Undirected periodic advertising
    GAPM_PER_ADV_PROP_UNDIR_MASK = 0x0000u,
    /// Directed periodic advertising
    GAPM_PER_ADV_PROP_DIR_MASK = GAPM_ADV_PROP_DIRECTED_BIT,
};
#endif // (BLE_PER_ADV)


/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// Configuration for advertising on primary channel
/*@TRACE*/
typedef struct
{
    /// Minimum advertising interval (in unit of 625us). Must be greater than 20ms
    uint32_t adv_intv_min;
    /// Maximum advertising interval (in unit of 625us). Must be greater than 20ms
    uint32_t adv_intv_max;
    /// Bit field indicating the channel mapping
    uint8_t ch_map;
    /// Indicate on which PHY primary advertising has to be performed (see #gapm_le_phy_type enumeration)
    /// Note that LE 2M PHY is not allowed and that legacy advertising only support LE 1M PHY
    uint8_t phy;
} gapm_le_adv_primary_cfg_t;

/// Configuration for advertising on secondary channel
/*@TRACE*/
typedef struct
{
    /// Maximum number of advertising events the controller can skip before sending the
    /// AUX_ADV_IND packets. 0 means that AUX_ADV_IND PDUs shall be sent prior each
    /// advertising events
    uint8_t max_skip;
    /// Indicate on which PHY secondary advertising has to be performed (see #gapm_le_phy_type enumeration)
    uint8_t phy;
    /// Advertising SID
    uint8_t adv_sid;
} gapm_le_adv_secondary_cfg_t;

#if (BLE_PER_ADV)
/// Configuration for periodic advertising
/*@TRACE*/
typedef struct
{
    /// Minimum periodic advertising interval (in unit of 1.25ms). Must be at least equal to 7.5ms
    uint16_t interval_min;
    /// Maximum periodic advertising interval (in unit of 1.25ms). Must be at least equal to 7.5ms
    uint16_t interval_max;
} gapm_le_adv_periodic_cfg_t;

/// Configuration for constant tone extension
/*@TRACE*/
typedef struct
{
    /// CTE count (number of CTEs to transmit in each periodic advertising interval, range 0x01 to 0x10)
    /// 0 to disable CTE transmission
    uint8_t count;
    /// CTE type (0: AOA | 1: AOD-1us | 2: AOD-2us) (see #gap_le_cte_type enumeration)
    uint8_t type;
    /// CTE length (in 8us unit)
    uint8_t length;
} gapm_le_adv_cte_cfg_t;
#endif // (BLE_PER_ADV)

/// Advertising parameters for advertising creation
/*@TRACE*/
typedef struct
{
    /// Bit field value provided advertising properties (see #gapm_le_adv_prop_bf)
    /// Pre-defined configuration
    /// - Legacy Adv: #gapm_le_adv_legacy_prop
    /// - Extended Adv: #gapm_le_adv_extended_prop
    /// - Periodic Adv: #gapm_le_adv_periodic_prop
    uint16_t prop;
    /// Discovery mode (see #gapm_le_adv_discovery_mode enumeration)
    uint8_t disc_mode;
    /// Required TX Power level in dBm (from -127 to 20 dBm)\n
    /// ADV_TX_PWR_NO_PREF (0x7F) means no preference\n
    /// TX Power level value chosen by controller reported in created callback function
    int8_t tx_pwr;
    /// Advertising filtering policy (see #gapm_le_adv_fllter_policy enumeration)
    uint8_t filter_pol;
    /// Peer address configuration (only used in case of directed advertising)
    gap_bdaddr_t peer_addr;
    /// Configuration for primary advertising
    gapm_le_adv_primary_cfg_t prim_cfg;
} gapm_le_adv_create_param_t;

/// Additional advertising parameters
/*@TRACE*/
typedef struct
{
    /// Advertising duration (in unit of 10ms). 0 means that advertising continues
    /// until the host disable it
    uint16_t duration;
    /// Maximum number of extended advertising events the controller shall attempt to send prior to
    /// terminating the extending advertising
    /// Valid only if extended advertising
    uint8_t max_adv_evt;
    /// Periodic advertising information bit field (see #gapm_le_adv_periodic_info_bf enumeration)
    /// Meaningful only if periodic advertising
    uint8_t per_adv_info_bf;
} gapm_le_adv_param_t;

/*
 * INTERFACES
 ****************************************************************************************
 */

/// Callback structure required to create an advertising activity
typedef struct
{
    /// Inherits Activity callback interface
    gapm_le_cb_actv_t hdr;

    /**
     ****************************************************************************************
     * Callback executed when advertising activity is created
     *
     * @note Mandatory callback. Shall be set to a valid callback
     *
     * @param[in] metainfo   Metadata information provided by API user (see \glos{METAINFO})
     * @param[in] actv_idx   Activity local index
     * @param[in] tx_pwr     Selected TX power for advertising activity
     *
     ****************************************************************************************
     */
    void (*created)(uint32_t metainfo, uint8_t actv_idx, int8_t tx_pwr);

    /**
     ****************************************************************************************
     * Callback executed when receiving a scan request (if enabled in advertising properties)
     *
     * @note Optional callback. Set it to NULL to ignore event reception
     *
     * @param[in] metainfo      Metadata information provided by API user (see \glos{METAINFO})
     * @param[in] actv_idx      Activity local index
     * @param[in] p_addr        Pointer to transmitter device identity address
     ****************************************************************************************
     */
    void (*scan_req_received)(uint32_t metainfo, uint8_t actv_idx, const gap_bdaddr_t* p_addr);

    #if (BLE_PER_ADV)
    /**
     ****************************************************************************************
     * Callback executed for periodic ADV to indicate that non periodic advertising is stopped.
     *
     * @note Optional callback. Mandatory for a periodic ADV
     *
     * @param[in] metainfo      Metadata information provided by API user (see \glos{METAINFO})
     * @param[in] actv_idx      Activity local index
     * @param[in] reason        Activity stop reason (see #hl_err enumeration)
     ****************************************************************************************
     */
    void (*ext_adv_stopped)(uint32_t metainfo, uint8_t actv_idx, uint16_t reason);
    #endif // (BLE_PER_ADV)

} gapm_le_adv_cb_actv_t;

#if (!(0))
/**
 ****************************************************************************************
 * @brief Callback function allowing to inform a module that address is about to be renew for an advertising
 * activity
 ****************************************************************************************
 */
typedef void (*gapm_le_adv_cb_addr_renewed)(uint8_t actv_idx);
#endif // (!(0))

/**
 ***************************************************************************************
 * @brief Function executed when procedure execution is over.
 *
 * @param[in] metainfo      Metadata information provided by API user (see \glos{METAINFO})
 * @param[in] status        Procedure execution status  (see #hl_err enumeration)
 * @param[in] nb_adv_set    Number of advertising set supported by controller
 ***************************************************************************************
 */
typedef void (*gapm_le_adv_cb_nb_sets)(uint32_t metainfo, uint16_t status, uint8_t nb_adv_set);

/**
 ***************************************************************************************
 * @brief Function executed when procedure execution is over.
 *
 * @param[in] metainfo      Metadata information provided by API user (see \glos{METAINFO})
 * @param[in] status        Procedure execution status  (see #hl_err enumeration)
 * @param[in] max_adv_len   Maximum advertising data length
 ***************************************************************************************
 */
typedef void (*gapm_le_adv_cb_max_len)(uint32_t metainfo, uint16_t status, uint16_t max_adv_len);

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

#if (HL_LE_BROADCASTER)
/**
 ****************************************************************************************
 * @brief Create an Legacy advertising activity.
 *
 * @param[in] metainfo          Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] own_addr_type     Own address type (see #gapm_le_own_addr enumeration)
 * @param[in] p_param           Pointer to advertising parameters
 * @param[in] p_cbs             Activity Callback interface
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapm_actv_cb_t.proc_cmp callback execution
 ****************************************************************************************
 */
uint16_t gapm_le_create_adv_legacy(uint32_t metainfo, uint8_t own_addr_type,
                                   const gapm_le_adv_create_param_t* p_param, const gapm_le_adv_cb_actv_t* p_cbs);

/**
 ****************************************************************************************
 * @brief Create an extended advertising activity.
 *
 * @param[in] metainfo          Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] own_addr_type     Own address type (see #gapm_le_own_addr enumeration)
 * @param[in] p_param           Pointer to advertising parameters
 * @param[in] p_second_cfg      Pointer to configuration for secondary advertising
 * @param[in] p_cbs             Activity Callback interface
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapm_actv_cb_t.proc_cmp callback execution
 ****************************************************************************************
 */
uint16_t gapm_le_create_adv_extended(uint32_t metainfo, uint8_t own_addr_type,
                                     const gapm_le_adv_create_param_t* p_param,
                                     const gapm_le_adv_secondary_cfg_t* p_second_cfg,
                                     const gapm_le_adv_cb_actv_t* p_cbs);

#if (BLE_PER_ADV)
/**
 ****************************************************************************************
 * @brief Create a Periodic advertising activity.
 *
 * @param[in] metainfo          Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] own_addr_type     Own address type (see #gapm_le_own_addr enumeration)
 * @param[in] p_param           Pointer to advertising parameters
 * @param[in] p_second_cfg      Pointer to configuration for secondary advertising
 * @param[in] p_period_cfg      Pointer to configuration for periodic advertising
 * @param[in] p_cbs             Activity Callback interface
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapm_actv_cb_t.proc_cmp callback execution
 ****************************************************************************************
 */
uint16_t gapm_le_create_adv_periodic(uint32_t metainfo, uint8_t own_addr_type,
                                     const gapm_le_adv_create_param_t* p_param,
                                     const gapm_le_adv_secondary_cfg_t* p_second_cfg,
                                     const gapm_le_adv_periodic_cfg_t* p_period_cfg,
                                     const gapm_le_adv_cb_actv_t* p_cbs);

/**
 ****************************************************************************************
 * @brief Create a Periodic advertising activity with constant tone extension.
 *
 * @param[in] metainfo              Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] own_addr_type         Own address type (see #gapm_le_own_addr enumeration)
 * @param[in] p_param               Pointer to advertising parameters
 * @param[in] p_second_cfg          Pointer to configuration for secondary advertising
 * @param[in] p_period_cfg          Pointer to configuration for periodic advertising
 * @param[in] p_cte_cfg             Pointer to CTE Configuration
 * @param[in] switching_pattern_len Length of switching pattern (number of antenna IDs in the pattern)
 * @param[in] p_antenna_id          Pointer to array of antenna IDs
 * @param[in] p_cbs                 Activity Callback interface
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapm_actv_cb_t.proc_cmp callback execution
 ****************************************************************************************
 */
uint16_t gapm_le_create_adv_periodic_with_cte(uint32_t metainfo, uint8_t own_addr_type,
                                              const gapm_le_adv_create_param_t* p_param,
                                              const gapm_le_adv_secondary_cfg_t* p_second_cfg,
                                              const gapm_le_adv_periodic_cfg_t* p_period_cfg,
                                              const gapm_le_adv_cte_cfg_t* p_cte_cfg, uint8_t switching_pattern_len,
                                              const uint8_t* p_antenna_id, const gapm_le_adv_cb_actv_t* p_cbs);

/**
 ****************************************************************************************
 * @brief Stop synchronizability of a periodic advertising (with response) activity.\n
 * Once stopped, synchronizability may be restarted using #gapm_le_start_adv function
 *
 * @param[in] actv_idx      Activity local index
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapm_actv_cb_t.proc_cmp callback execution
 ****************************************************************************************
 */
uint16_t gapm_le_per_adv_stop_synchronizability(uint8_t actv_idx);
#endif // (BLE_PER_ADV)

/**
 ****************************************************************************************
 * @brief Start advertising activity.
 *
 * @param[in] actv_idx          Activity local index
 * @param[in] p_param           Additional Advertising start parameters
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapm_actv_cb_t.proc_cmp callback execution
 ****************************************************************************************
 */
uint16_t gapm_le_start_adv(uint8_t actv_idx, const gapm_le_adv_param_t* p_param);

/**
 ****************************************************************************************
 * @brief Set advertising data
 *
 * @note AD Type Flag is automatically added according to advertising mode (if code not compiled with HL_ADV_ADD_FLAGS
 * flag)
 *
 * @note Minimum buffer head: #GAPM_LE_ADV_BUFFER_HEADER_LEN, and minimum buffer tail: #GAPM_LE_ADV_BUFFER_TAIL_LEN

 * @param[in] actv_idx          Activity local index
 * @param[in] p_data            Pointer to buffer that contains Advertising data
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapm_actv_cb_t.proc_cmp callback execution
 ****************************************************************************************
 */
uint16_t gapm_le_set_adv_data(uint8_t actv_idx, co_buf_t* p_data);

/**
 ****************************************************************************************
 * @brief Set scan response data
 *
 * @note Minimum buffer head: #GAPM_LE_ADV_BUFFER_HEADER_LEN, 
 *       and minimum buffer tail: #GAPM_LE_ADV_BUFFER_TAIL_LEN

 * @param[in] actv_idx          Activity local index
 * @param[in] p_data            Pointer to buffer that contains Advertising data
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapm_actv_cb_t.proc_cmp callback execution
 ****************************************************************************************
 */
uint16_t gapm_le_set_scan_response_data(uint8_t actv_idx, co_buf_t* p_data);

/**
 ****************************************************************************************
 * @brief Set periodic advertising data
 *
 * @note Minimum buffer head: #GAPM_LE_ADV_BUFFER_HEADER_LEN, 
 *       and minimum buffer tail: #GAPM_LE_ADV_BUFFER_TAIL_LEN

 * @param[in] actv_idx  Activity local index
 * @param[in] p_data    Pointer to buffer that contains Advertising data
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapm_actv_cb_t.proc_cmp callback execution
 ****************************************************************************************
 */
uint16_t gapm_le_set_per_adv_data(uint8_t actv_idx, co_buf_t* p_data);

/**
 ****************************************************************************************
 * @brief Set unchanged periodic advertising data
 *
 * @param[in] actv_idx  Activity local index
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapm_actv_cb_t.proc_cmp callback execution
 ****************************************************************************************
 */
uint16_t gapm_le_set_per_adv_data_fake(uint8_t actv_idx);

/**
 ***************************************************************************************
 * @brief Control transmission of constant tone extension with periodic advertising data
 *
 * @param[in] actv_idx  Activity local index
 * @param[in] enable    True to enable CTE transmission, False otherwise
 *
 * @return Execution status (see #hl_err enumeration).
 ***************************************************************************************
 */
uint16_t gapm_le_control_per_adv_cte_tx(uint8_t actv_idx, bool enable);

/**
 ***************************************************************************************
 * @brief Get advertising handle
 *
 * @param[in] actv_idx   Activity index
 *
 * @return Required advertising handle (see #hl_err enumeration).
 ***************************************************************************************
 */
uint8_t gapm_le_get_adv_hdl(uint8_t actv_idx);

/**
 ***************************************************************************************
 * @brief Get address set for an advertising activity.\n
 * Note that the returned address may differ from address used over the air if controller privacy is used and a
 * valid entry exists in the Resolving List for indicated peer device.
 *
 * @param[in] actv_idx   Activity index
 *
 * @return Pointer to required address
 ***************************************************************************************
 */
gap_addr_t* gapm_le_get_adv_addr(uint8_t actv_idx);

#if (!(0))
/**
 ***************************************************************************************
 * @brief Set callback function allowing to inform a block that BD Address is about to be renewed for an
 * advertising activity\n
 * !!!! FOR INTERNAL USE ONLY !!!!
 *
 * @param[in] actv_idx          Activity index
 * @param[in] cb_addr_renew     Callback function
 *
 * @return Execution status (see #hl_err enumeration).
 ***************************************************************************************
 */
uint16_t gapm_le_set_adv_cb_addr_renew(uint8_t actv_idx, gapm_le_adv_cb_addr_renewed cb_addr_renew);
#endif // (!(0))

/**
 ***************************************************************************************
 * @brief Get number of advertising set. Information returned in res_cb function
 *
 * @param[in] metainfo  Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] res_cb    Function called when procedure is over
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapm_le_adv_cb_nb_sets callback execution
 ***************************************************************************************
 */
uint16_t gapm_le_get_nb_adv_sets(uint32_t metainfo, gapm_le_adv_cb_nb_sets res_cb);

/**
 ***************************************************************************************
 * @brief Get maximum advertising data length. Information returned in res_cb function
 *
 * @param[in] metainfo  Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] res_cb    Function called when procedure is over
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapm_le_adv_cb_max_len callback execution
 ***************************************************************************************
 */
uint16_t gapm_le_get_max_adv_data_len(uint32_t metainfo, gapm_le_adv_cb_max_len res_cb);

#if (BT_53)
/**
 ***************************************************************************************
 * @brief Update reason why address is updated for an advertising activity if resolvable private address is used
 *
 * @param[in] actv_idx          Activity index
 * @param[in] change_reasons    Change reasons
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapm_le_adv_cb_max_len callback execution
 ***************************************************************************************
 */
uint16_t gapm_le_set_adv_addr_change_reasons(uint8_t actv_idx, uint8_t change_reasons);
#endif // (BT_53)


#endif // (HL_LE_BROADCASTER)

/// @} GAPM_LE_ADV_ACTV_API

#endif /* GAPM_LE_ADV_H_ */
