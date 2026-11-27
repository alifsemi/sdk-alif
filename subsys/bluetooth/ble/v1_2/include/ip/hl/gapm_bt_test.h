/**
 ****************************************************************************************
 *
 * @file gapm_bt_test.h
 *
 * @brief Generic Access Profile Manager - Test mode for BT-Classic
 *
 * Copyright (C) RivieraWaves 2009-2022
 *
 ****************************************************************************************
 */

#ifndef _GAPM_BT_TEST_H_
#define _GAPM_BT_TEST_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @addtogroup GAPM_BT_TEST_API Test mode
 *
 * @ingroup GAPM_BT_API
 *
 * @brief Control BT-Classic test mode from application
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "gapm.h"

/*
 * DEFINES
 ****************************************************************************************
 */

/// @addtogroup GAPM_BT_TEST_API
/// @{

/// Loopback mode value
enum gapm_bt_loopback_mode
{
    /// No Loopback mode enabled (default).
    GAPM_BT_LOOPBACK_DISABLED = 0x00,
    /// Enable Local Loopback.
    GAPM_BT_LOOPBACK_ENABLE_LOCAL = 0x01,
    /// Enable Remote Loopback.
    GAPM_BT_LOOPBACK_ENABLE_REMOTE = 0x02,
};

/*
 * INTERFACES
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * Callback executed when reads controller's loopback mode procedure is completed.
 *
 * @param[in] metainfo      Metadata information provided by API user (see \glos{METAINFO})
 * @param[in] status        Status of procedure execution (see enum #hl_err)
 * @param[in] loopback_mode Loopback mode value read (see #gapm_bt_loopback_mode)
 *
 ****************************************************************************************
 */
typedef void (*gapm_bt_read_loopback_res_cb)(uint32_t metainfo, uint16_t status, uint8_t loopback_mode);

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ***************************************************************************************
 * @brief Reads the value for the setting of the controller's loopback mode.
 *
 * @param[in] metainfo  Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] res_cb    Function called when procedure is over.
 *
 * @return Execution status (see enum #hl_err).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapm_bt_read_loopback_res_cb callback execution
 ***************************************************************************************
 */
uint16_t gapm_bt_read_loopback_mode(uint32_t metainfo, gapm_bt_read_loopback_res_cb res_cb);

/**
 ***************************************************************************************
 * @brief Writes the value for the setting of the BR/EDR controller's loopback mode. The setting of the loopback
 *        mode parameter shall determine the path of information. In Local loopback mode, every data packet
 *        (ACL, SCO and eSCO) and command packet that is sent from the Host to the BR/EDR controller is sent
 *        back with no modifications by the BR/EDR  controller.
 *
 * @param[in] metainfo      Metadata information that will be returned in procedure callback function (see \glos{METAINFO})
 * @param[in] loopback_mode Loopback mode value (see #gapm_bt_loopback_mode)
 * @param[in] cmp_cb        Function called when procedure is over.
 *
 * @return Execution status (see enum #hl_err).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapm_proc_cmp_cb callback execution
 ***************************************************************************************
 */
uint16_t gapm_bt_write_loopback_mode(uint32_t metainfo, uint8_t loopback_mode, gapm_proc_cmp_cb cmp_cb);

/**
 ***************************************************************************************
 * @brief Allows the local BR/EDR controller to enter test mode via LMP test commands.
 *
 * @param[in] metainfo      Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] cmp_cb        Function called when procedure is over.
 *
 * @return Execution status (see enum #hl_err).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapm_proc_cmp_cb callback execution
 ***************************************************************************************
 */
uint16_t gapm_bt_enable_device_under_test_mode(uint32_t metainfo, gapm_proc_cmp_cb cmp_cb);

/**
 ***************************************************************************************
 * @brief Configures the BR/EDR controller to use a predefined Diffie Hellman private key for
 *        simple pairing to enable debug equipment to monitor the encrypted connection.
 *
 * @param[in] metainfo          Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] enable_debug_mode True to enable debug mode, false otherwise.
 * @param[in] cmp_cb            Function called when procedure is over.
 *
 * @return Execution status (see enum #hl_err).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapm_proc_cmp_cb callback execution
 ***************************************************************************************
 */
uint16_t gapm_bt_write_simple_pairing_debug_mode(uint32_t metainfo, bool enable_debug_mode, gapm_proc_cmp_cb cmp_cb);

/**
 ***************************************************************************************
 * @brief Configures the BR/EDR controller to enable and disable the two test modes used for verifying the
 *        secure connections feature during qualification.
 *
 * @param[in] metainfo                  Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] conidx                    Connection index
 * @param[in] enable_dm1_acl_u_mode     Enables or disables the use of DM1 packets for transmitting ACL-U data.
 * @param[in] enable_esco_loopback_mode Enables and disables the loopback of received eSCO payloads.
 * @param[in] cmp_cb                    Function called when procedure is over.
 *
 * @return Execution status (see enum #hl_err).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapm_proc_cmp_cb callback execution
 ***************************************************************************************
 */
uint16_t gapm_bt_write_secure_connections_test_mode(uint32_t metainfo, uint8_t conidx, bool enable_dm1_acl_u_mode,
                                                    bool enable_esco_loopback_mode, gapm_proc_cmp_cb cmp_cb);

/// @} GAPM_BT_TEST_API

#endif /* _GAPM_BT_TEST_H_ */
