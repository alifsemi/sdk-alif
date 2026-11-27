/**
 ****************************************************************************************
 *
 * @file bap_bc_deleg.h
 *
 * @brief Basic Audio Profile - Broadcast Delegator - Definitions
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef BAP_BC_DELEG_H_
#define BAP_BC_DELEG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup BAP_BC_DELEG Broadcast Delegator module
 * @ingroup BAP_BC
 * @brief Description of Broadcast Delegator module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup BAP_BC_DELEG_ENUM Enumerations
 * @ingroup BAP_BC_DELEG
 * @brief Enumerations for Broadcast Delegator module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup BAP_BC_DELEG_NATIVE_API Native API
 * @ingroup BAP_BC_DELEG
 * @brief Description of Native API for Broadcast Delegator module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup BAP_BC_DELEG_CALLBACK Callback Functions
 * @ingroup BAP_BC_DELEG_NATIVE_API
 * @brief Description of callback functions for Broadcast Delegator module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup BAP_BC_DELEG_FUNCTION Functions
 * @ingroup BAP_BC_DELEG_NATIVE_API
 * @brief Description of functions for Broadcast Delegator module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "gaf.h"            // GAF Definitions
#include "bap.h"            // BAP Definitions
#include "bap_bc.h"         // BAP Broadcast Definitions

#if (GAF_BAP_BC_DELEG)

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// @addtogroup BAP_BC_DELEG_ENUM
/// @{

#if (!(0))
/// List of command type values for Broadcast Delegator module
enum bap_bc_deleg_cmd_type
{
    /// Start sending of Solicitation Requests
    BAP_BC_DELEG_CMD_TYPE_START_SOLICITE = 0,
    /// Stop sending of Solicitation Requests
    BAP_BC_DELEG_CMD_TYPE_STOP_SOLICITE,
};

/// Solicitation stop reason
enum bap_bc_deleg_stop_reason
{
    /// Connection has been established with a Broadcast Assistant
    BAP_BC_DELEG_STOP_REASON_ESTABLISHED = 0x00,
    /// Stopped due to upper layer request
    BAP_BC_DELEG_STOP_REASON_UPPER_TERMINATE = 0x01,
    /// Stopped due to a timeout
    BAP_BC_DELEG_STOP_REASON_TIMEOUT = 0x02,
};
#endif //(!(0))

/// Broadcast Assistant device scan state
enum bap_bc_deleg_scan_state
{
    /// Broadcast assistant device has stopped to scan on behalf of device
    BAP_BC_DELEG_SCAN_STOPPED = 0x00,
    /// Broadcast assistant device has started to scan on behalf of device
    BAP_BC_DELEG_SCAN_STARTED = 0x01,
};

/// Configuration bit field meaning for Broadcast Delegator module
enum bap_bc_deleg_cfg_bf
{
    /// Indicate if it is required (= 1) to check the Lock state
    /// Meaningful only if Coordinated Set Identification Profile Set Member Role is supported
    BAP_BC_DELEG_CFG_CHECK_LOCK_POS = 0,
    BAP_BC_DELEG_CFG_CHECK_LOCK_BIT = CO_BIT(BAP_BC_DELEG_CFG_CHECK_LOCK_POS),
};

/// Information bit field for Add Source request
enum bap_bc_deleg_add_info_bf
{
    /// Indicate if Broadcast Core is required
    BAP_BC_DELEG_ADD_INFO_NEED_CODE_POS = 0,
    BAP_BC_DELEG_ADD_INFO_NEED_CODE_BIT = CO_BIT(BAP_BC_DELEG_ADD_INFO_NEED_CODE_POS),
};

/// @} BAP_BC_DELEG_ENUM

/*
 * CALLBACK FUNCTIONS DEFINITION
 ****************************************************************************************
 */

/// @addtogroup BAP_BC_DELEG_CALLBACK
/// @{

#if (!(0))
/**
 ****************************************************************************************
 * @brief Callback function called when a command execution completes
 *
 * @param[in] cmd_type      Command type (see #bap_bc_deleg_cmd_type enumeration)
 * @param[in] status        Status (see #gaf_err enumeration)
 * @param[in] src_lid       Source local index
 ****************************************************************************************
 */
typedef void (*bap_bc_deleg_cb_cmp_evt)(uint8_t cmd_type, uint16_t status, uint8_t src_lid);

/**
 ****************************************************************************************
 * @brief Callback function called when sending of Solicitation Requests has been stopped.
 *
 * @param[in] reason        Reason why sending of Solicitation Requests has been stopped
 * (see #bap_bc_deleg_stop_reason enumeration)
 ****************************************************************************************
 */
typedef void (*bap_bc_deleg_cb_solicite_stopped)(uint8_t reason);
#endif //(!(0))

/**
 ****************************************************************************************
 * @brief Callback function called when client configuration for Broadcast Audio Scan Service has been updated
 *
 * @param[in] con_lid           Connection local index
 * @param[in] cli_cfg_bf        Client configuration bit field
 ****************************************************************************************
 */
typedef void (*bap_bc_deleg_cb_bond_data)(uint8_t con_lid, uint16_t cli_cfg_bf);

/**
 ****************************************************************************************
 * @brief Callback function called when Broadcast Assistant device has updated its scanning status
 *
 * @param[in] con_lid       Connection local index
 * @param[in] state         Broadcast Assistant device scan state (see #bap_bc_deleg_scan_state enumeration)
 ****************************************************************************************
 */
typedef void (*bap_bc_deleg_cb_remote_scan)(uint8_t con_lid, uint8_t state);

/**
 ****************************************************************************************
 * @brief Callback function called when Broadcast Assistant device has provided its Broadcast Code
 *
 * @param[in] src_lid           Source local index
 * @param[in] con_lid           Connection local index
 * @param[in] p_bcast_code      Pointer to Broadcast Code
 ****************************************************************************************
 */
typedef void (*bap_bc_deleg_cb_bcast_code)(uint8_t src_lid, uint8_t con_lid, const gaf_bcast_code_t* p_bcast_code);

/**
 ****************************************************************************************
 * @brief Callback function called when a Broadcast Assistant requests to add a Broadcast Source.\n
 * #bap_bc_deleg_modify_source_cfm function is expected to be called.\n
 * Information about each subgroup should be retrieved using #bap_bc_deleg_get_sgrp_info function
 *
 * @param[in] src_lid           Source local index
 * @param[in] con_lid           Connection local index
 * @param[in] p_adv_id          Pointer to Periodic Advertising identification structure
 * @param[in] p_bcast_id        Pointer to Broadcast ID
 * @param[in] pa_sync_req       Requested synchronization state for Periodic Advertising (see #bap_bc_pa_sync
 *                              enumeration)
 * @param[in] pa_intv_frames    Periodic Advertising interval in frames
 * @param[in] nb_subgroups      Number of Subgroups
 * @param[in] metadata_len      Total length of Metadata received
 ****************************************************************************************
 */
typedef void (*bap_bc_deleg_cb_add_source_req)(uint8_t src_lid, uint8_t con_lid, const bap_adv_id_t* p_adv_id,
                                               const bap_bcast_id_t* p_bcast_id, uint8_t pa_sync_req,
                                               uint16_t pa_intv_frames, uint8_t nb_subgroups, uint16_t metadata_len);

/**
 ****************************************************************************************
 * @brief Callback function called when a Broadcast Assistant requests to update a Broadcast Source.\n
 * #bap_bc_deleg_add_source_cfm function is expected to be called.\n
 * Information about each subgroup should be retrieved using #bap_bc_deleg_get_sgrp_info function
 *
 * @param[in] src_lid           Source local index
 * @param[in] con_lid           Connection local index
 * @param[in] pa_sync_req       Requested synchronization state for Periodic Advertising (see #bap_bc_pa_sync
 *                              enumeration)
 * @param[in] pa_intv_frames    Periodic Advertising interval in frames
 * @param[in] nb_subgroups      Number of Subgroups
 * @param[in] metadata_len      Total length of Metadata received
 ****************************************************************************************
 */
typedef void (*bap_bc_deleg_cb_modify_source_req)(uint8_t src_lid, uint8_t con_lid, uint8_t pa_sync_req,
                                                  uint16_t pa_intv_frames, uint8_t nb_subgroups, uint16_t metadata_len);

/**
 ****************************************************************************************
 * @brief Callback function called when a Broadcast Assistant requests to remove a Broadcast Source.\n
 * #bap_bc_deleg_remove_source_cfm function is expected to be called
 *
 * @param[in] src_lid       Source local index
 * @param[in] con_lid       Connection local index
 ****************************************************************************************
 */
typedef void (*bap_bc_deleg_cb_remove_source_req)(uint8_t src_lid, uint8_t con_lid);

/*
 * CALLBACK SET DEFINITION
 ****************************************************************************************
 */

/// Set of callback functions for Broadcast Audio Scan Service Server
struct bap_bc_deleg_cb
{
    #if (!(0))
    /// Callback function called when a command execution completes
    bap_bc_deleg_cb_cmp_evt cb_cmp_evt;
    /// Callback function called when sending of Solicitation Requests has been stopped.
    bap_bc_deleg_cb_solicite_stopped cb_solicite_stopped;
    #endif //(!(0))
    /// Callback function called when client configuration for Broadcast Audio Scan Service has been updated
    bap_bc_deleg_cb_bond_data cb_bond_data;
    /// Callback function called when Broadcast Assistant device updates its scanning status
    bap_bc_deleg_cb_remote_scan cb_remote_scan;
    /// Callback function called when Broadcast Assistant device provides its Broadcast Code
    bap_bc_deleg_cb_bcast_code cb_bcast_code;
    /// Callback function called when a Broadcast Assistant requests to add a Broadcast Source
    bap_bc_deleg_cb_add_source_req cb_add_source_req;
    /// Callback function called when a Broadcast Assistant requests to update a Broadcast Source
    bap_bc_deleg_cb_modify_source_req cb_modify_source_req;
    /// Callback function called when a Broadcast Assistant requests to remove a Broadcast Source
    bap_bc_deleg_cb_remove_source_req cb_remove_source_req;
};

/// @} BAP_BC_DELEG_CALLBACK

/*
 * API FUNCTIONS DECLARATION
 ****************************************************************************************
 */

/// @addtogroup BAP_BC_DELEG_FUNCTION
/// @{

/**
 ****************************************************************************************
 * @brief Configure use of BAP Broadcast Delegator module
 *
 * @param[in] p_cb          Pointer to set to callback functions for communication with upper layer
 * @param[in] p_cfg         Pointer to configuration structure
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t bap_bc_deleg_configure(const bap_bc_deleg_cb_t* p_cb, const bap_bc_deleg_cfg_t* p_cfg);

/**
 ****************************************************************************************
 * @brief Set bonding information related to Broadcast Audio Scan Service after connection with a peer device
 *
 * @param[in] con_lid           Connection local index
 * @param[in] cli_cfg_bf        Configuration bit field (1 bit per Broadcast Receive State characteristic instance)
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t bap_bc_deleg_restore_bond_data(uint8_t con_lid, uint16_t cli_cfg_bf);

/**
 ****************************************************************************************
 * @brief Add a Broadcast Source
 * Must be directly followed by a call of #bap_bc_deleg_set_sgrp_bis_sync function for each subgroup (if bit field
 * is not 0)
 * Must be directly followed by a call of #bap_bc_deleg_set_sgrp_metadata function for each subgroup (if Metadata
 * exists)
 *
 * @param[in] p_adv_id          Pointer to Periodic Advertising identification structure
 * @param[in] p_bcast_id        Pointer Broadcast ID
 * @param[in] info_bf           Information bit field (see #bap_bc_deleg_add_info_bf enumeration)
 * @param[in] nb_subgroups      Number of Subgroups
 * @param[out] p_src_lid        Pointer at which allocated Source local index will be returned
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t bap_bc_deleg_add_source(const bap_adv_id_t* p_adv_id, const bap_bcast_id_t* p_bcast_id, uint8_t info_bf,
                                 uint8_t nb_subgroups, uint8_t* p_src_lid);

/**
 ****************************************************************************************
 * @brief Modify a Broadcast Source
 * Must be directly followed by a call of #bap_bc_deleg_set_sgrp_bis_sync function for each subgroup (if bit field
 * has changed)
 * Must be directly followed by a call of #bap_bc_deleg_set_sgrp_metadata function for each subgroup (if Metadata
 * has changed)
 *
 * @param[in] src_lid           Source local index
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t bap_bc_deleg_modify_source(uint8_t src_lid);

/**
 ****************************************************************************************
 * @brief Remove a Broadcast Source
 *
 * @param[in] src_lid       Source local index
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t bap_bc_deleg_remove_source(uint8_t src_lid);

/**
 ****************************************************************************************
 * @brief Set BIS synchronization bit field for a Subgroup.
 * This function must be called directly after call of #bap_bc_deleg_add_source or #bap_bc_deleg_modify_source
 * function
 *
 * @param[in] src_lid       Source local index
 * @param[in] sgrp_idx      Subgroup index (between 0 and (nb_subgroups - 1))
 * @param[in] bis_sync_bf   BIS synchronization bit field
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t bap_bc_deleg_set_sgrp_bis_sync(uint8_t src_lid, uint8_t sgrp_idx, uint32_t bis_sync_bf);

/**
 ****************************************************************************************
 * @brief Set Metadata for a Subgroup.
 * This function must be called directly after call of #bap_bc_deleg_add_source or #bap_bc_deleg_modify_source
 * function
 *
 * @param[in] src_lid       Source local index
 * @param[in] sgrp_idx      Subgroup index (between 0 and (nb_subgroups - 1))
 * @param[in] p_metadata    Pointer to Metadata\n
 *                          Can be NULL\n
 *                          Structure shall be allocated by Upper Layer and maintained until a new call of
 *                          #bap_bc_deleg_set_sgrp_metadata for the Subgroup
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t bap_bc_deleg_set_sgrp_metadata(uint8_t src_lid, uint8_t sgrp_idx, const bap_cfg_metadata_t* p_metadata);

/**
 ****************************************************************************************
 * @brief Get Subgroup information provided by Broadcast Assistant device\n
 * This function must be called by Upper Layer inside #bap_bc_deleg_cb_add_source_req
 * or #bap_bc_deleg_cb_modify_source_req callback function\n
 * First time this function is called, information for Subgroup 0 are returned\n
 * Second time this function is called, information for Subgroup 1 are returned\n
 * Etc...
 *
 * @param[in] src_lid            Source local index
 * @param[in] p_bis_sync_bf      Pointer to BIS synchronization bit field
 * @param[in] p_metadata         Pointer to Metadata
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t bap_bc_deleg_get_sgrp_info(uint8_t src_lid, uint32_t* p_bis_sync_bf, bap_cfg_metadata_ptr_t* p_metadata);

#if (!(0))
/**
 ****************************************************************************************
 * @brief Start sending of Solicitation Requests
 *
 *        When a connection is established or a timeout is raised, #cb_solicite_stopped is called
 *
 * @param[in] timeout_s           Timeout duration in seconds
 *                                0 means that sending of Solicitation Requests will last until stopped by the upper
 *                                layer
 * @param[in] p_adv_param         Pointer to Advertising parameters
 * @param[in] p_adv_data          Pointer to additional advertising data
 *
 * @return An error status (see #gaf_err enumeration)
           if function returns GAF_ERR_NO_ERROR, wait for #bap_bc_deleg_cb.cb_cmp_evt execution
 ****************************************************************************************
 */
uint16_t bap_bc_deleg_start_solicite(uint16_t timeout_s, const bap_bc_adv_param_t* p_adv_param,
                                     const gaf_ltv_t* p_adv_data);

/**
 ****************************************************************************************
 * @brief Stop sending of Solicitation Requests
 *
 *        When sending of Solication Requests has stopped, #bap_bc_deleg_cb.cb_solicite_stopped is called
 *
 * @return An error status (see #gaf_err enumeration)
           if function returns GAF_ERR_NO_ERROR, wait for #bap_bc_deleg_cb.cb_cmp_evt execution
 ****************************************************************************************
 */
uint16_t bap_bc_deleg_stop_solicite(void);

/**
 ****************************************************************************************
 * @return Activity index allocated by GAP for advertising activity created in order to send Solicitation Requests.\n
 * After #bap_bc_deleg_start_solicite has been called, returned index is valid only during or after
 * #bap_bc_deleg_cb.cb_cmp_evt execution with no error returned.
 ****************************************************************************************
 */
uint8_t bap_bc_deleg_get_actv_idx(void);
#endif //(!(0))

/**
 ****************************************************************************************
 * @brief Confirm addition of a new Broadcast Source
 *
 * @param[in] src_lid               Source local index
 * @param[in] accept                True to accept new Broadcast Source, False to reject
 ****************************************************************************************
 */
void bap_bc_deleg_add_source_cfm(uint8_t src_lid, bool accept);

/**
 ****************************************************************************************
 * @brief Confirm update of Broadcast Source
 *
 * @param[in] src_lid               Source local index
 * @param[in] accept                True to accept update, False to reject
 ****************************************************************************************
 */
void bap_bc_deleg_modify_source_cfm(uint8_t src_lid, bool accept);

/**
 ****************************************************************************************
 * @brief Confirm removal of a Broadcast Source
 *
 * @param[in] src_lid               Source local index
 * @param[in] accept                True to accept Broadcast Source removal, False to reject
 ****************************************************************************************
 */
void bap_bc_deleg_remove_source_cfm(uint8_t src_lid, bool accept);

/**
 ****************************************************************************************
 * @return Return if BAP Broadcast Delegator module has been configured
 ****************************************************************************************
 */
bool bap_bc_deleg_is_configured(void);

/// @} BAP_BC_DELEG_FUNCTION
#endif //(GAF_BAP_BC_DELEG)

#endif // BAP_BC_DELEG_H_
