/**
 ****************************************************************************************
 *
 * @file gapm.h
 *
 * @brief Generic Access Profile Manager - Native API .
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef GAPM_H_
#define GAPM_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @addtogroup GAPM_API Generic Access Profile Manager (GAPM)
 * @ingroup GAP_API
 * @brief Generic Access Profile Manager.
 *
 * The GAP Manager module is responsible for providing an API to the application in order
 * to manage all non connected stuff such as configuring device to go in desired mode
 * (discoverable, connectable, etc.) and perform required actions (scanning, connection,
 * etc.). GAP Manager is also responsible of managing GAP Controller state according to
 * corresponding BLE connection states.
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup GAPM_ENUM_API Enumerations
 * @ingroup GAPM_API
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup GAPM_STRUCT_API Structures and types
 * @ingroup GAPM_API
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup GAPM_CONFIG_API Configuration
 * @ingroup GAPM_API
 * @brief Set of functions used to configure the device
 *
 * Prior to starting any activities or load profiles, device configuration must be setup using
 * #gapm_configure.
 *
 * Once configuration is done, it's recommended to load profiles before starting any activities.
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup GAPM_PROC_API Procedures
 *
 * @ingroup GAPM_API
 *
 * @brief Procedure interface and helpful functions
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup GAPM_ACTV_API Non-connected activities
 *
 * @ingroup GAPM_API
 *
 * @brief A GAPM activity is a non-connected activity.
 *
 * All activities can be stopped and deleted using #gapm_stop_activity and #gapm_delete_activity
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup GAPM_ACTV_ITF_API Default interface
 * @ingroup GAPM_ACTV_API
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "gap.h"
#include <stdbool.h>
#include "co_buf.h"

/*
 * DEFINES
 ****************************************************************************************
 */

/// @addtogroup GAPM_ENUM_API
/// @{

/// Privacy configuration
enum gapm_privacy_cfg
{
    /// Indicate if identity address is a public (0) or static private random (1) address
    GAPM_PRIV_CFG_PRIV_ADDR_BIT = (1u << 0),
    GAPM_PRIV_CFG_PRIV_ADDR_POS = 0u,
    /// Reserved
    GAPM_PRIV_CFG_RSVD_BIT = (1u << 1),
    GAPM_PRIV_CFG_RSVD_BIT_POS = 1u,
    /// Indicate if controller privacy is enabled
    GAPM_PRIV_CFG_PRIV_EN_BIT = (1u << 2),
    GAPM_PRIV_CFG_PRIV_EN_POS = 2u,
};

 /// Clock accuracy values
 enum gapm_clock_accuracy
 {
     /// 500 ppm
     GAPM_CLK_ACC_500 = 0u,
     /// 250 ppm
     GAPM_CLK_ACC_250,
     /// 150 ppm
     GAPM_CLK_ACC_150,
     /// 100 ppm
     GAPM_CLK_ACC_100,
     /// 75 ppm
     GAPM_CLK_ACC_75,
     /// 50 ppm
     GAPM_CLK_ACC_50,
     /// 30 ppm
     GAPM_CLK_ACC_30,
     /// 20 ppm
     GAPM_CLK_ACC_20,
 };

/// Activity procedure type
enum gapm_actv_proc_id
{
    /// Start activity
    GAPM_ACTV_START = 0u,
    /// Stop activity
    GAPM_ACTV_STOP,
    /// Delete activity
    GAPM_ACTV_DELETE,
    /// Create LE Advertising activity
    GAPM_ACTV_CREATE_LE_ADV,
    /// Set advertising data
    GAPM_ACTV_SET_ADV_DATA,
    /// Set scan response Data
    GAPM_ACTV_SET_SCAN_RSP_DATA,
    #if (BLE_PER_ADV)
    /// Set periodic advertising data
    GAPM_ACTV_SET_PERIOD_ADV_DATA,
    /// Control CTE transmission in periodic ADV activity.
    GAPM_ACTV_PERIOD_ADV_CTE_TX_CTRL,
    /// Control periodic advertising report reception
    GAPM_ACTV_PERIOD_REPORT_CTRL,
    /// Control IQ Report (manage sampling configuration)
    GAPM_ACTV_PERIOD_IQ_REPORT_CTRL,
    /// Set unchanged periodic advertising data
    GAPM_ACTV_SET_PERIOD_ADV_DATA_UNCHANGED,
    /// Pause extended advertising for Periodic (with Response) advertising
    GAPM_ACTV_PER_ADV_STOP_SYNCHRONIZABILITY,
    #endif // (BLE_PER_ADV)
    /// Set change reasons for advertising resolvable private address
    GAPM_ACTV_SET_ADV_ADDR_CHANGE_REASONS,
};

/// Device Attribute write permission requirement
enum gapm_write_att_perm
{
    /// Disable write access
    GAPM_WRITE_DISABLE = 0u,
    /// Enable write access - no encryption required
    GAPM_WRITE_NOT_ENC = 1u,
    /// Write access requires unauthenticated link
    GAPM_WRITE_UNAUTH = 2u,
    /// Write access requires authenticated link
    GAPM_WRITE_AUTH = 3u,
    /// Write access requires secure connected link
    GAPM_WRITE_SEC_CON = 4u
};

/// Attribute database configuration
/// @verbatim
///      15     14   13   12   11   10    9    8    7    6    5    4    3    2    1    0
/// +---------+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
/// |BOND_INFO|      RFU     | KM |RPAO|EATT| FE |MTU |PCP |   APP_PERM   |   NAME_PERM  |
/// +---------+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
/// @endverbatim
enum gapm_att_cfg_flag
{
    /// Bit [0-2]: See #GAPM_ATT_NAME_PERM_MASK
    GAPM_ATT_NAME_PERM_LSB = 0u,
    /// Bit [0-2]: Device Name write permission requirements for peer device (#gapm_write_att_perm)
    GAPM_ATT_NAME_PERM_MASK = CO_MASK(3, GAPM_ATT_NAME_PERM_LSB),

    /// Bit [3-5]: See #GAPM_ATT_APPEARENCE_PERM_MASK
    GAPM_ATT_APPEARENCE_PERM_LSB = 3u,
    /// Bit [3-5]: Device Appearance write permission requirements for peer device (#gapm_write_att_perm)
    GAPM_ATT_APPEARENCE_PERM_MASK = CO_MASK(3, GAPM_ATT_APPEARENCE_PERM_LSB),

    /// Bit [6]: See #GAPM_ATT_SLV_PREF_CON_PAR_EN_MASK
    GAPM_ATT_SLV_PREF_CON_PAR_EN_LSB = 6u,
    /// Bit [6]: Slave Preferred Connection Parameters present in GAP attribute database.
    GAPM_ATT_SLV_PREF_CON_PAR_EN_MASK = CO_MASK(1, GAPM_ATT_SLV_PREF_CON_PAR_EN_LSB),

    /// Bit [7]: See #GAPM_ATT_CLI_DIS_AUTO_MTU_EXCH_MASK
    GAPM_ATT_CLI_DIS_AUTO_MTU_EXCH_LSB = 7u,
    /// Bit [7]: Disable automatic MTU exchange at connection establishment (on legacy ATT bearer)
    GAPM_ATT_CLI_DIS_AUTO_MTU_EXCH_MASK = CO_MASK(1, GAPM_ATT_CLI_DIS_AUTO_MTU_EXCH_LSB),

    /// Bit [8]: See #GAPM_ATT_CLI_DIS_AUTO_FEAT_EN_MASK
    GAPM_ATT_CLI_DIS_AUTO_FEAT_EN_LSB = 8u,
    /// Bit [8]: Disable automatic client feature enable setup at connection establishment
    GAPM_ATT_CLI_DIS_AUTO_FEAT_EN_MASK = CO_MASK(1, GAPM_ATT_CLI_DIS_AUTO_FEAT_EN_LSB),

    #if (EATT_SUPPORT)
    /// Bit [9]: See #GAPM_ATT_CLI_DIS_AUTO_EATT_MASK
    GAPM_ATT_CLI_DIS_AUTO_EATT_LSB = 9u,
    /// Bit [9]: Disable automatic establishment of Enhanced ATT bearers
    GAPM_ATT_CLI_DIS_AUTO_EATT_MASK = CO_MASK(1, GAPM_ATT_CLI_DIS_AUTO_EATT_LSB),
    #endif // (EATT_SUPPORT)

    /// Bit [10]: See #GAPM_ATT_RSLV_PRIV_ADDR_ONLY_MASK
    GAPM_ATT_RSLV_PRIV_ADDR_ONLY_LSB = 10u,
    /// Bit [10]: Enable presence of Resolvable private address only.
    /// This means that after a bond, device must only use resolvable private address
    GAPM_ATT_RSLV_PRIV_ADDR_ONLY_MASK = CO_MASK(1, GAPM_ATT_RSLV_PRIV_ADDR_ONLY_LSB),


    /// Bit [15]: See #GAPM_DBG_BOND_INFO_TRIGGER_BIT
    GAPM_DBG_BOND_INFO_TRIGGER_POS = 15u,
    /// Bit [15]: Trigger bond information to application even if devices are not bonded
    GAPM_DBG_BOND_INFO_TRIGGER_BIT = CO_BIT(GAPM_DBG_BOND_INFO_TRIGGER_POS),
};

/// Pairing mode authorized on the device bit field
/// @verbatim
///    7    6    5    4    3    2    1    0
/// +----+----+----+----+----+----+----+----+
/// |        RFU        | SSP| CT2| SCP| LP |
/// +----+----+----+----+----+----+----+----+
/// @endverbatim
enum gapm_pairing_mode
{
    /// No pairing authorized
    GAPM_PAIRING_DISABLE = 0u,
    /// Legacy pairing Authorized
    GAPM_PAIRING_LEGACY = (1u << 0),
    /// Secure Connection pairing Authorized
    GAPM_PAIRING_SEC_CON = (1u << 1),
    /// Dual-mode only feature used to support cross transport key derivation 2nd algorithm
    GAPM_PAIRING_CT2 = (1u << 2),
    /// BT-Classic secure simple pairing (forced to 1 if secure connection enabled)
    GAPM_PAIRING_BT_SSP = (1u << 3),

    /// All security mode enabled
    GAPM_PAIRING_MODE_ALL = (GAPM_PAIRING_LEGACY | GAPM_PAIRING_SEC_CON | GAPM_PAIRING_CT2 | GAPM_PAIRING_BT_SSP),
};

/// Type of activities that can be created
/*@TRACE*/
enum gapm_actv_type
{
    // LE - Activities
    /// Advertising activity
    GAPM_ACTV_TYPE_ADV = 0u,
    /// Scanning activity
    GAPM_ACTV_TYPE_SCAN = 1u,
    /// Initiating activity
    GAPM_ACTV_TYPE_INIT = 2u,
    #if (BLE_PER_ADV)
    /// Periodic synchronization activity
    GAPM_ACTV_TYPE_PER_SYNC = 3u,
    #endif // (BLE_PER_ADV)

    // BT Classic - Activities
    /// Inquiry activity
    GAPM_ACTV_TYPE_INQUIRY = 50u,
    /// Inquiry scan activity
    GAPM_ACTV_TYPE_INQUIRY_SCAN = 51u,
    /// Page activity
    GAPM_ACTV_TYPE_PAGE = 52u,
    /// Page scan activity
    GAPM_ACTV_TYPE_PAGE_SCAN = 53u,

    // LE -Test Mode
    /// TX Test Mode
    GAPM_ACTV_TYPE_TX_TEST = 100u,
    /// RX Test Mode
    GAPM_ACTV_TYPE_RX_TEST = 101u,
};

/// List of optional features that may be disabled using #gapm_disable_feature
enum gapm_optional_features
{
    /// GATT caching feature
    GAPM_OPT_FEAT_GATT_CACHING = 0u,
    /// ATT Prepare write feature
    GAPM_OPT_FEAT_ATT_PREPARE_WRITE = 1u,

    // Total number of feature that can be disabled, additional features need to be added before GAPM_LE_MAX_FEAT_NUM
    GAPM_MAX_OPT_FEAT_NUM
};

/// @} GAPM_ENUM_API

/*
 * TYPE DEFINITION
 ****************************************************************************************
 */

typedef struct gapc_connection_req_cb gapc_connection_req_cb_t;
typedef struct gapc_security_cb gapc_security_cb_t;
typedef struct gapc_connection_info_cb gapc_connection_info_cb_t;
typedef struct gapc_le_config_cb gapc_le_config_cb_t;
typedef struct gapc_bt_config_cb gapc_bt_config_cb_t;

/// @addtogroup GAPM_STRUCT_API
/// @{

/// Set device configuration command
/*@TRACE*/
typedef struct
{
    /// Device Role: Central, Peripheral, Observer, Broadcaster or All roles (see #gap_role enumeration)
    uint8_t role;

    // -------------- Security Config ------------------------------------
    /// Pairing mode authorized (see #gapm_pairing_mode enumeration)
    uint8_t pairing_mode;
    /// If supported by controller, minimum required key size for LE and BT-Classic pairing
    /// (valid range: [7:16], value provided automatically update to valid range if outside)
    uint8_t pairing_min_req_key_size;

    // -------------- Privacy Config -------------------------------------
    /// Privacy configuration bit field (see #gapm_privacy_cfg enumeration for bit signification)
    uint8_t privacy_cfg;
    /// Duration before regenerate device address when privacy is enabled. - in seconds
    uint16_t renew_dur;
    /// Private static identity address - meaningful if #GAPM_PRIV_CFG_PRIV_ADDR_BIT set in #gapm_config_t.privacy_cfg
    /// bit field, otherwise ignored.
    /// \if btdm
    /// @note A device that supports #GAP_ROLE_BT_CLASSIC role cannot use a private static identity
    ///       if #GAPM_PRIV_CFG_PRIV_ADDR_BIT is set in #gapm_config_t.privacy_cfg, configuration returns #GAP_ERR_PRIVACY_CFG_PB
    /// \endif
    gap_addr_t private_identity;
    /// Device IRK used for resolvable random BD address generation (LSB first)
    gap_sec_key_t irk;

    // -------------- ATT Database Config --------------------------------
    /// GAP service start handle
    uint16_t gap_start_hdl;
    /// GATT service start handle
    uint16_t gatt_start_hdl;
    /// Attribute database configuration (see #gapm_att_cfg_flag enumeration)
    uint16_t att_cfg;

    // -------------- LE Data Length Extension ---------------------------
    ///Suggested value for the Controller's maximum transmitted number of payload octets to be used
    uint16_t sugg_max_tx_octets;
    ///Suggested value for the Controller's maximum packet transmission time to be used
    uint16_t sugg_max_tx_time;

    // ------------------ LE PHY Management  -----------------------------
    /// Preferred LE PHY for data transmission (see #gap_le_phy_bf enumeration)
    uint8_t tx_pref_phy;
    /// Preferred LE PHY for data reception (see #gap_le_phy_bf enumeration)
    uint8_t rx_pref_phy;

    // ------------------ Radio Configuration ----------------------------
    /// RF TX Path Compensation value (from -128dB to 128dB, unit is 0.1dB)
    uint16_t tx_path_comp;
    /// RF RX Path Compensation value (from -128dB to 128dB, unit is 0.1dB)
    uint16_t rx_path_comp;

    // ------------------ BT classic configuration ----------------------
    /// Bluetooth Class of device
    uint32_t class_of_device;
    /// Default link policy
    uint16_t dflt_link_policy;

} gapm_config_t;

/// List of callbacks that will handle gapm events
typedef struct
{
    /**
     ****************************************************************************************
     * Callback executed when hardware error hci event is received
     *
     * @note Mandatory callback -
     *
     * @param[in] metainfo    Metadata information provided by API user (see \glos{METAINFO})
     * @param[in] code        Hardware error code
     ****************************************************************************************
     */
    void (*cb_hw_error)(uint32_t metainfo, uint8_t code);
} gapm_cb_t;

/// List of callbacks that will handle GAP events
typedef struct
{
    /// Connection request event callback functions provided by upper layer software
    /// Mandatory if #GAP_ROLE_LE_CENTRAL or #GAP_ROLE_LE_PERIPHERAL or #GAP_ROLE_BT_CLASSIC is supported
    const gapc_connection_req_cb_t* p_con_req_cbs;
    /// Security event callback functions provided by upper layer software
    /// Mandatory if #GAP_ROLE_LE_CENTRAL or #GAP_ROLE_LE_PERIPHERAL or #GAP_ROLE_BT_CLASSIC is supported
    const gapc_security_cb_t* p_sec_cbs;
    /// Connection event callback functions provided by upper layer software
    /// Mandatory if #GAP_ROLE_LE_CENTRAL or #GAP_ROLE_LE_PERIPHERAL or #GAP_ROLE_BT_CLASSIC is supported
    const gapc_connection_info_cb_t* p_info_cbs;
    /// LE Connection configuration event callback functions provided by upper layer software
    /// Mandatory if #GAP_ROLE_LE_CENTRAL or #GAP_ROLE_LE_PERIPHERAL
    const gapc_le_config_cb_t* p_le_config_cbs;
    /// BT Connection configuration callback functions provided by upper layer software
    /// Mandatory if #GAP_ROLE_BT_CLASSIC is supported
    const gapc_bt_config_cb_t* p_bt_config_cbs;
    /// GAPM configuration callback functions provided by upper layer software
    /// Mandatory
    const gapm_cb_t* p_gapm_cbs;

} gapm_callbacks_t;

/// Device SW/HW version information
typedef struct
{
    /// HCI version
    uint8_t hci_ver;
    /// LMP version
    uint8_t lmp_ver;
    /// Host version
    uint8_t host_ver;
    /// HCI revision
    uint16_t hci_subver;
    /// LMP subversion
    uint16_t lmp_subver;
    /// Host revision
    uint16_t host_subver;
    /// Manufacturer name
    uint16_t manuf_name;
} gapm_version_t;

/// @} GAPM_STRUCT_API
/*
 * INTERFACES
 ****************************************************************************************
 */

/*
 * RESULT CALLBACK FUNCTIONS
 ****************************************************************************************
 */

/// @addtogroup GAPM_PROC_API
/// @{

/**
 ****************************************************************************************
 * Callback executed when a procedure is completed.
 *
 * @param[in] metainfo  Metadata information provided by API user (see \glos{METAINFO})
 * @param[in] status    Status of procedure execution (see #hl_err enumeration)
 *
 ****************************************************************************************
 */
typedef void (*gapm_proc_cmp_cb)(uint32_t metainfo, uint16_t status);

/// @} GAPM_PROC_API

/**
 ****************************************************************************************
 * @addtogroup GAPM_INFO_API Device Information
 * @ingroup GAPM_API
 * @brief Set of functions used to retrieve device information
 * @{
 ****************************************************************************************
 */

/**
 ***************************************************************************************
 * @brief Function executed when procedure execution is over.
 *
 * @param[in] metainfo      Metadata information provided by API user (see \glos{METAINFO})
 * @param[in] status        Procedure execution status  (see #hl_err enumeration)
 * @param[in] p_version     Pointer to local device version
 *                          (NULL if status != GAP_ERR_NO_ERROR)
 ***************************************************************************************
 */
typedef void (*gapm_version_cb)(uint32_t metainfo, uint16_t status, const gapm_version_t* p_version);

/**
 ***************************************************************************************
 * @brief Function executed when controller address has been read
 *
 * @param[in] metainfo      Metadata information provided by API user (see \glos{METAINFO})
 * @param[in] status        Procedure execution status  (see #hl_err enumeration)
 * @param[in] p_addr        Pointer to controller address read
 *                          (NULL if status != GAP_ERR_NO_ERROR)
 ***************************************************************************************
 */
typedef void (*gapm_bdaddr_cb)(uint32_t metainfo, uint16_t status, const gap_addr_t* p_addr);

/// @} GAPM_INFO_API

/// @addtogroup GAPM_ACTV_ITF_API
/// @{

/// Callback structure required to create an activity
typedef struct
{
    /**
     ****************************************************************************************
     * Callback executed when a procedure is completed.
     *
     * @note Mandatory callback. Shall be set to a valid callback
     *
     * @param[in] metainfo  Metadata information provided by API user (see \glos{METAINFO})
     * @param[in] proc_id   Procedure identifier (see #gapm_actv_proc_id enumeration)
     * @param[in] actv_idx  Activity local index
     * @param[in] status    Status of procedure execution (see #hl_err enumeration)
     *
     ****************************************************************************************
     */
    void (*proc_cmp)(uint32_t metainfo, uint8_t proc_id, uint8_t actv_idx, uint16_t status);

    /**
     ****************************************************************************************
     * Callback executed when a procedure is completed.
     *
     * @note Mandatory callback. Shall be set to a valid callback
     *
     * @param[in] metainfo      Metadata information provided by API user (see \glos{METAINFO})
     * @param[in] actv_idx      Activity local index
     * @param[in] reason        Activity stop reason (see #hl_err enumeration)
     ****************************************************************************************
     */
    void (*stopped)(uint32_t metainfo, uint8_t actv_idx, uint16_t reason);
} gapm_actv_cb_t;

/// @} GAPM_ACTV_ITF_API

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/// @addtogroup GAPM_PROC_API
/// @{

/**
 ****************************************************************************************
 * @brief Generate a new token for any HL procedures
 *
 * @return New Generated token
 ***************************************************************************************
 */
uint16_t gapm_get_token_id(void);

/// @} GAPM_PROC_API

/// @addtogroup GAPM_CONFIG_API
/// @{

/**
 ***************************************************************************************
 * @brief Optional and is present only for testing purpose.
 *
 * It's used to stop all device activity, but new activity cannot be started before
 * setting device configuration (#gapm_configure)
 *
 * This will initialize the RW-BLE Host stack - rearrange to default settings the ATT, GAP, GATT, L2CAP and SMP blocks.
 * Furthermore, if the link layer part is not present on the device, it will cause the host to send a reset command
 * down to the external controller.
 *
 * Platform reset: Use platform mechanism to reset hardware.
 *
 * @param[in] metainfo  Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] cmp_cb    Function called when procedure is over.
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapm_proc_cmp_cb callback execution
 ***************************************************************************************
 */
uint16_t gapm_reset(uint32_t metainfo, gapm_proc_cmp_cb cmp_cb);

/**
 ***************************************************************************************
 * @brief Setup initial device configuration
 *
 * Set the device configuration such as:
 *     - Device role
 *     - Manage device address type: Public, Private static or Generated for Privacy
 *     - Internal IRK used to generate resolvable random address
 *     - Set Internal GAP / GATT service start
 *     - Set specific write permissions on the appearance and name attributes in internal GAP database.
 *     - Manage presence of some attribute.
 *     - Configure Data Length Extension features
 *
 * The set device configuration command must be sent before adding profiles and start air activities.
 *
 * If the link layer part is not present on the device, this command will firstly send a reset command
 * down to the external controller.
 *
 * For reconfiguration purpose, #gapm_reset command is required to be called before any new configuration.
 *
 * Application shall wait #gapm_proc_cmp_cb callback execution before starting any activities
 *
 * example:
 * \snippet{lineno} app.c APP_SET_DEV_CFG
 *
 * @param[in] metainfo  Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 *                      This parameter is provided to application when a connection request is received (see #gapc_connection_req_cb)
 *                      Or in case of hardware error event (see #gapm_cb_t).
 * @param[in] p_cfg     Pointer to device configuration.
 * @param[in] p_cbs     Pointer to callbacks that handles events
 * @param[in] cmp_cb    Function called when procedure is over.
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapm_proc_cmp_cb callback execution
 ***************************************************************************************
 */
uint16_t gapm_configure(uint32_t metainfo, const gapm_config_t* p_cfg, const gapm_callbacks_t* p_cbs, gapm_proc_cmp_cb cmp_cb);

/**
 ***************************************************************************************
 * @brief Set device name
 *
 * Device name pointer life cycle must be handled by application, it shall be valid until a name modification or a reset
 * is performed.
 *
 * Device name is set for both BT Classic and LE.
 *
 * Application should wait #gapm_proc_cmp_cb callback execution before starting a new procedure
 *
 * @param[in] metainfo  Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] name_len  Length of the name array
 * @param[in] p_name    Pointer to device name pointer in UTF-8 format.
 * @param[in] cmp_cb    Function called when procedure is over.
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapm_proc_cmp_cb callback execution
 ***************************************************************************************
 */
uint16_t gapm_set_name(uint32_t metainfo, uint8_t name_len, const uint8_t* p_name, gapm_proc_cmp_cb cmp_cb);

#if (BLE_GAPC)
/**
 ****************************************************************************************
 * @brief Configure security level required for LE connections
 *
 * LE security mode 1
 *   - 1. No security (No authentication and no encryption)
 *   - 2. Unauthenticated pairing with encryption
 *   - 3. Authenticated pairing with encryption
 *   - 4. Authenticated LE Secure Connections pairing with encryption using a 128-bit strength encryption key.
 *
 * LE security mode 2
 *   - 1. Unauthenticated pairing with data signing
 *   - 2. Authenticated pairing with data signing
 *
 * @param[in] sec_req_level LE connection security requirements (minimum security level - see #gap_sec_req enumeration)
 ****************************************************************************************
 */
void gapm_le_configure_security_level(uint8_t sec_req_level);
#endif // (BLE_GAPC)

/// @} GAPM_CONFIG_API

/// @addtogroup GAPM_INFO_API
/// @{

/**
 ***************************************************************************************
 * @brief Get device version information. Version is returned in res_cb function
 *
 * @note In a configuration where lower layer are present, result is provided before function returns
 *
 * @param[in] metainfo  Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] res_cb    Function called when version is available
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapm_version_cb callback execution
 ***************************************************************************************
 */
uint16_t gapm_get_version(uint32_t metainfo, gapm_version_cb res_cb);

#if (!EMB_PRESENT)
/**
 ***************************************************************************************
 * @brief Get list of features supported by controller
 *
 * @note Shall be sent after #GAPM_CONFIGURE procedure has been completed
 *
 * @return Pointer to structure containing requested information
 ***************************************************************************************
 */
const le_features_t* gapm_get_features(void);
#endif // (!EMB_PRESENT)

/**
 ***************************************************************************************
 * @brief Get Local device Identity address
 *
 * @note Identity available once device is configured (see #gapm_configure)
 *
 * @param[out] p_addr   Pointer to the address structure to fill with local address information
 *
 * @return Execution status (see #hl_err enumeration).
 ***************************************************************************************
 */
uint16_t gapm_get_identity(gap_bdaddr_t* p_addr);

/**
 ***************************************************************************************
 * @brief Read Controller public static address.
 *
 * @note In a configuration where lower layer are present, result is provided before function returns
 *
 * @param[in] metainfo  Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] res_cb    Function called when controller address has been read
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapm_bdaddr_cb callback execution
 ***************************************************************************************
 */
uint16_t gapm_get_controller_bdaddr(uint32_t metainfo, gapm_bdaddr_cb res_cb);

/// @} GAPM_INFO_API

/// @addtogroup GAPM_ACTV_ITF_API
/// @{

/**
 ****************************************************************************************
 * @brief Stop an activity
 *
 * @param[in] actv_idx          Activity local index
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapm_actv_cb_t.proc_cmp callback execution
 ****************************************************************************************
 */
uint16_t gapm_stop_activity(uint8_t actv_idx);

/**
 ****************************************************************************************
 * @brief Delete an activity
 *
 * @param[in] actv_idx          Activity local index
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapm_actv_cb_t.proc_cmp callback execution
 ****************************************************************************************
 */
uint16_t gapm_delete_activity(uint8_t actv_idx);

/**
 ****************************************************************************************
 * @brief Into a buffer that contains several field using LTV format (Length, Type, Value)
 *        retrieve value of a specific type
 *
 * @param[in]  type      Searched Type
 * @param[in]  data_size Buffer data size
 * @param[in]  p_data    Pointer to buffer data
 * @param[out] p_length  Pointer where length of found value is returned
 *
 * @return Pointer to the first byte of value found that correspond to given type.
 *         NULL if nothing found.
 ****************************************************************************************
 */
const uint8_t* gapm_get_ltv_value(uint8_t type, uint16_t data_size, const uint8_t* p_data, uint8_t* p_length);

#if (RW_DEBUG && (BT_EMB_PRESENT || BLE_EMB_PRESENT))
/**
 ****************************************************************************************
 * @brief Provide all needed priority/increment pairs as a buffer that is copied directly to the priority table.\n
 * This function should be called before any activity is started in order to avoid unexpected scheduling issues.\n
 * Activities whose priority increment is 0 no longer benefit from the random increment to ensure that their
 * priority stays the same.
 *
 * @param[in] metainfo      Metadata information that will be returned in procedure callback functions
 * (see \glos{METAINFO})
 * @param[in] pair_count    Number of priority/increment pairs\n
 *                          Must be set to RWIP_PRIO_IDX_MAX
 * @param[in] p_prio_data   Priority/increment pairs
 * @param[in] cmp_cb        Function called when procedure is over.
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for cmp_cb callback execution
 ****************************************************************************************
 */
uint16_t gapm_vs_set_sched_prio(uint32_t metainfo, uint8_t pair_count, const uint8_t* p_prio_data,
                                gapm_proc_cmp_cb cmp_cb);
#endif // (RW_DEBUG && (BT_EMB_PRESENT || BLE_EMB_PRESENT))

/**
 ****************************************************************************************
 * @brief Disable a feature\n
 * Must be called before #gapm_configure to disable a given feature.
 *
 * @param[in] feature_idx          Supported feature index (see #gapm_optional_features)
 ****************************************************************************************
 */
void gapm_disable_feature(uint16_t feature_idx);

/// @} GAPM_ACTV_ITF_API

#endif /* GAPM_H_ */
