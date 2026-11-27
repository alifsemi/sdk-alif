/**
 ****************************************************************************************
 *
 * @file gapm_le_init.h
 *
 * @brief Generic Access Profile Manager - Low Energy Initiating Activities
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef GAPM_LE_INIT_H_
#define GAPM_LE_INIT_H_

#include "rom_build_cfg.h"

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "gapm_le.h"

/**
 ****************************************************************************************
 * @addtogroup GAPM_LE_INIT_ACTV_API LE Initiating
 *
 * @ingroup GAPM_ACTV_API
 *
 * @brief Create and control LE Initiating activity to discover device name or establish an LE connection
 *
 * Even if application can create several initiating activities, only one can be active (started) at a time.
 *
 * The application must have a callback structure to handle activities events
 * \snippet{lineno} app_test_le_central.c APP_INIT_ACTV_CB
 *
 * Application can create an initiating activity using #gapm_le_create_init.
 *
 * example:
 * \snippet{lineno} app_test_le_central.c APP_CREATE_INIT
 *
 * Once activity is created, application can immediately start initiating.
 *
 * An application example is available in \ref app_test_le_central.c
 *
 * @note At least #GAP_ROLE_LE_CENTRAL role is required
 * @{
 ****************************************************************************************
 */

/*
 * DEFINES
 ****************************************************************************************
 */

/// Initiating Types
enum gapm_le_init_type
{
    /// Direct connection establishment, establish a connection with an indicated device
    GAPM_INIT_TYPE_DIRECT_CONN_EST = 0u,
    /// Automatic connection establishment, establish a connection with all devices whose address is
    /// present in the filter accept list
    GAPM_INIT_TYPE_AUTO_CONN_EST,
    /// Name discovery, Establish a connection with an indicated device in order to read content of its
    /// Device Name characteristic. Connection is closed once this operation is stopped.
    GAPM_INIT_TYPE_NAME_DISC,
};

/// Initiating Properties
enum gapm_le_init_prop
{
    /// Scan connectable advertisements on the LE 1M PHY. Connection parameters for the LE 1M PHY are provided
    GAPM_INIT_PROP_1M_BIT = (1u << 0),
    /// Connection parameters for the LE 2M PHY are provided
    GAPM_INIT_PROP_2M_BIT = (1u << 1),
    /// Scan connectable advertisements on the LE Coded PHY. Connection parameters for the LE Coded PHY are provided
    GAPM_INIT_PROP_CODED_BIT = (1u << 2),
};

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// Initiating parameters
/*@TRACE*/
typedef struct
{
    /// Properties for the initiating procedure (see #gapm_le_init_prop enumeration for bit signification)
    uint8_t prop;
    /// Timeout for automatic connection establishment (in unit of 10ms). Cancel the procedure if not all
    /// indicated devices have been connected when the timeout occurs. 0 means there is no timeout
    uint16_t conn_to;
    /// Scan window opening parameters for LE 1M PHY
    gapm_le_scan_wd_op_param_t scan_param_1m;
    /// Scan window opening parameters for LE Coded PHY
    gapm_le_scan_wd_op_param_t scan_param_coded;
    /// Connection parameters for LE 1M PHY
    gapm_le_init_con_param_t conn_param_1m;
    /// Connection parameters for LE 2M PHY
    gapm_le_init_con_param_t conn_param_2m;
    /// Connection parameters for LE Coded PHY
    gapm_le_init_con_param_t conn_param_coded;
    /// Address of peer device in case filter accept list is not used for connection
    gap_bdaddr_t peer_addr;
} gapm_le_init_param_t;

/*
 * INTERFACES
 ****************************************************************************************
 */

/// Callback structure required to create an Initiating activity
typedef struct
{
    /// Inherits Activity callback interface
    gapm_le_cb_actv_t hdr;

    /**
     ****************************************************************************************
     * @brief Callback executed when Peer device name has been read on peer device
     *
     * @note Optional callback. Shall be set to a valid callback only for Name discovery procedure
     *
     * @param[in] metainfo      Metadata information provided by API user (see \glos{METAINFO})
     * @param[in] actv_idx      Activity Local identifier
     * @param[in] p_addr        Pointer to peer identity address information
     * @param[in] name_len      Length of peer device name
     * @param[in] p_name        Pointer to peer device name data
     ****************************************************************************************
     */
    void (*peer_name)(uint32_t metainfo, uint8_t actv_idx, const gap_bdaddr_t* p_addr, uint16_t name_len,
                      const uint8_t* p_name);
} gapm_le_init_cb_actv_t;

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

#if (HL_LE_CENTRAL)
/**
 ****************************************************************************************
 * @brief Create initiating connection activity
 *
 * @param[in]  metainfo         Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in]  own_addr_type    Own address type (see #gapm_le_own_addr enumeration)
 * @param[in]  p_cbs            Activity Callback interface
 * @param[out] p_actv_idx       Pointer used to return allocated activity index
 *
 * @return Execution status (see #hl_err enumeration).
 ****************************************************************************************
 */
uint16_t gapm_le_create_init(uint32_t metainfo, uint8_t own_addr_type, const gapm_le_init_cb_actv_t* p_cbs,
                             uint8_t* p_actv_idx);

/**
 ****************************************************************************************
 * @brief Start connection creation
 *
 * @param[in] actv_idx          Activity local index
 * @param[in] type              Initiating type (see #gapm_le_init_type enumeration)
 * @param[in] p_param           Initiating parameters
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapm_actv_cb_t.proc_cmp callback execution
 ****************************************************************************************
 */
uint16_t gapm_le_start_init(uint8_t actv_idx, uint8_t type, const gapm_le_init_param_t* p_param);

/**
 ****************************************************************************************
 * @brief Start direct connection establishement part of initiating activity
 *
 * @param[in] actv_idx          Activity local index
 * @param[in] p_param           Initiating parameters
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapm_actv_cb_t.proc_cmp callback execution
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint16_t gapm_le_start_direct_connection(uint8_t actv_idx, const gapm_le_init_param_t* p_param)
{
    return gapm_le_start_init(actv_idx, GAPM_INIT_TYPE_DIRECT_CONN_EST, p_param);
}

/**
 ****************************************************************************************
 * @brief Start automatic connection establishement part of initiating activity
 *
 * @param[in] actv_idx          Activity local index
 * @param[in] p_param           Initiating parameters
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapm_actv_cb_t.proc_cmp callback execution
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint16_t gapm_le_start_auto_connection(uint8_t actv_idx, const gapm_le_init_param_t* p_param)
{
    return gapm_le_start_init(actv_idx, GAPM_INIT_TYPE_AUTO_CONN_EST, p_param);
}

#if (BLE_GATT_CLI)
/**
 ****************************************************************************************
 * @brief Start remote name discovery part of initiating activity
 *
 * Establish LE connection, read Attribute device name, and finally disconnect.
 *
 * @param[in] actv_idx          Activity local index
 * @param[in] p_param           Initiating parameters
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapm_actv_cb_t.proc_cmp callback execution
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint16_t gapm_le_start_name_discovery(uint8_t actv_idx, const gapm_le_init_param_t* p_param)
{
    return gapm_le_start_init(actv_idx, GAPM_INIT_TYPE_NAME_DISC, p_param);
}
#endif // (BLE_GATT_CLI)
#endif // (HL_LE_CENTRAL)

/// @} GAPM_LE_INIT_ACTV_API

#endif /* GAPM_LE_INIT_H_ */
