/**
 ****************************************************************************************
 *
 * @file bap_bc_assist.h
 *
 * @brief Basic Audio Profile - Broadcast Assistant - Definitions
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef BAP_BC_ASSIST_H_
#define BAP_BC_ASSIST_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup BAP_BC_ASSIST Broadcast Assistant module
 * @ingroup BAP_BC
 * @brief Description of Broadcast Assistant module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup BAP_BC_ASSIST_ENUM Enumerations
 * @ingroup BAP_BC_ASSIST
 * @brief Enumerations for Broadcast Assistant module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup BAP_BC_ASSIST_STRUCT Structures
 * @ingroup BAP_BC_ASSIST
 * @brief Structures for Broadcast Assistant module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup BAP_BC_ASSIST_NATIVE_API Native API
 * @ingroup BAP_BC_ASSIST
 * @brief Description of Native API for Broadcast Assistant module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup BAP_BC_ASSIST_CALLBACK Callback Functions
 * @ingroup BAP_BC_ASSIST_NATIVE_API
 * @brief Description of callback functions for Broadcast Assistant module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup BAP_BC_ASSIST_FUNCTION Functions
 * @ingroup BAP_BC_ASSIST_NATIVE_API
 * @brief Description of functions for Broadcast Assistant module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "gaf.h"             // GAF Defines
#include "bap.h"             // Basic Audio Profile Definitions
#include "bap_bc.h"          // Basic Audio Profile Broadcast Definitions

#if (GAF_BAP_BC_ASSIST)

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// @addtogroup BAP_BC_ASSIST_ENUM
/// @{

/// Command type values
enum bap_bc_assist_cmd_type
{
    /// Start discovery of Solicitation Requests sent by Delegator devices
    BAP_BC_ASSIST_CMD_TYPE_START_SCAN = 0,
    /// Stop discovery of Solicitation Requests
    BAP_BC_ASSIST_CMD_TYPE_STOP_SCAN,
    /// Discover Broadcast Audio Scan Service in peer device database
    BAP_BC_ASSIST_CMD_TYPE_DISCOVER,
    /// Get value of an instance of Broadcast Receive State characteristic
    BAP_BC_ASSIST_CMD_TYPE_GET_STATE,
    /// Enable or disable sending of notifications for an instance of the Broadcast Receive State characteristic
    BAP_BC_ASSIST_CMD_TYPE_SET_CFG = 5,
    /// Inform peer Delegator device that Assistant has either started or stopped scan on its behalf
    BAP_BC_ASSIST_CMD_TYPE_UPDATE_SCAN,
    /// Add a Broadcast Source
    BAP_BC_ASSIST_CMD_TYPE_ADD_SOURCE,
    /// Add a Broadcast Source locally created (Source and Assistant colocated)
    BAP_BC_ASSIST_CMD_TYPE_ADD_SOURCE_LOCAL,
    /// Remove a Broadcast Source
    BAP_BC_ASSIST_CMD_TYPE_REMOVE_SOURCE,
    /// Update a Broadcast Source
    BAP_BC_ASSIST_CMD_TYPE_MODIFY_SOURCE,
    /// Update a Broadcast Source locally created (Source and Assistant colocated)
    BAP_BC_ASSIST_CMD_TYPE_MODIFY_SOURCE_LOCAL,

    BAP_BC_ASSIST_CMD_TYPE_MAX
};

/// List of Broadcast Audio Scan Service characteristics
enum bap_bc_char_type
{
    /// Broadcast Audio Scan Control Point characteristic
    BAP_BC_CHAR_TYPE_CP = 0,

    /// Number of mono-instantiated characteristics
    BAP_BC_CHAR_TYPE_MAX_MONO,
};

/// Values for PA Sync field used in #bap_bc_assist_add_source, #bap_bc_assist_add_source_local and
/// #bap_bc_assist_modify_source functions
enum bap_bc_assist_pa_sync
{
    /// Do not synchronize to PA
    BAP_BC_ASSIST_PA_SYNC_NO_SYNC = 0,
    /// Synchronize to PA
    BAP_BC_ASSIST_PA_SYNC_SYNC,

    /// Maximum value
    BAP_BC_ASSIST_PA_SYNC_MAX,
};

/// Configuration bit field meaning for BAP Broadcast Assistant
enum bap_bc_assist_cfg_bf
{
    /// Indicate if use of write without response is preferred
    BAP_BC_ASSIST_CFG_WRITE_NO_RSP_POS = 0,
    BAP_BC_ASSIST_CFG_WRITE_NO_RSP_BIT = CO_BIT(BAP_BC_ASSIST_CFG_WRITE_NO_RSP_POS),
};

/// @} BAP_BC_ASSIST_ENUM

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// @addtogroup BAP_BC_ASSIST_STRUCT
/// @{

/// Broadcast Receive State characteristic description structure
typedef struct bap_bc_assist_rx_state_char
{
    /// Characteristic value handle
    uint16_t val_hdl;
    /// Client characteristic configuration descriptor handle
    uint16_t desc_hdl;
} bap_bc_assist_rx_state_char_t;

/// Broadcast Audio Scan Service content description structure
typedef struct bap_bc_assist_bass
{
    /// Service description
    prf_svc_t svc_info;
    /// Characteristic value handle for Broadcast Audio Scan Control Point characteristic
    uint16_t cp_val_hdl;
    /// Number of discovered Broadcast Receive State characteristics
    uint8_t nb_rx_state;
    /// Pointer to information structures for Broadcast Receive State characteristic
    bap_bc_assist_rx_state_char_t rx_state_char_info[__ARRAY_EMPTY];
} bap_bc_assist_bass_t;

/// @} BAP_BC_ASSIST_STRUCT

/*
 * CALLBACK FUNCTIONS DEFINITION
 ****************************************************************************************
 */

/// @addtogroup BAP_BC_ASSIST_CALLBACK
/// @{

/**
 ****************************************************************************************
 * @brief Callback function called each time a command has been completed
 *
 * @param[in] cmd_type      Command type (see #bap_bc_assist_cmd_type enumeration)
 * @param[in] status        Status
 * @param[in] con_lid       Connection local index
 * @param[in] src_lid       Source local index
 ****************************************************************************************
 */
typedef void (*bap_bc_assist_cb_cmp_evt)(uint8_t cmd_type, uint16_t status, uint8_t con_lid, uint8_t src_lid);

/**
 ****************************************************************************************
 * @brief Callback function called when Broadcast Scan Audio Service has been discovered in a peer service device
 * database
 *
 * @param[in] con_lid           Connection local index
 * @param[in] p_bass_info       Pointer to Broadcast Audio Scan Service description structure
 ****************************************************************************************
 */
typedef void (*bap_bc_assist_cb_bond_data)(uint8_t con_lid, const bap_bc_assist_bass_t* p_bass_info);

/**
 ****************************************************************************************
 * @brief Callback function called when state for a Broadcast Source has been received
 *
 * @param[in] con_lid               Connection local index
 * @param[in] src_lid               Source local index
 * @param[in] is_used               Indicate if the instance of Broadcast Receive State characteristic is used\n
 *                                  Below parameters are meaningful only if is_used = true
 * @param[in] src_id                Source ID
 * @param[in] p_adv_id              Pointer to Advertising identification structure
 * @param[in] p_bcast_id            Pointer to Broadcast ID
 * @param[in] pa_sync               Synchronization state for PA (see #bap_bc_rx_state_pa enumeration)
 * @param[in] big_enc               Encryption state for BIG (see #bap_bc_big_encrypt_state enumeration)
 * @param[in] p_bad_code            Pointer to Bad Broadcast code\n
 *                                  NULL if big_enc != BAP_BC_BIG_ENCRYPT_STATE_BAD_CODE
 * @param[in] nb_subgroups          Number of Subgroups\n
 *                                  From 0 to 32
 * @param[in] bis_sync_bf           BIS synchronization bit field for the first Subgroup\n
 *                                  #bap_bc_assist_cb_state_subgroup callback function immediately called following
 *                                  this callback for each remaining Subgroup\n
 *                                  Meaningful only if nb_subgroups != 0
 * @param[in] p_metadata            Pointer to Metadata for the first Subgroup\n
 *                                  Can be NULL
 ****************************************************************************************
 */
typedef void (*bap_bc_assist_cb_state)(uint8_t con_lid, uint8_t src_lid, bool is_used, uint8_t src_id,
                                       const bap_adv_id_t* p_adv_id, const bap_bcast_id_t* p_bcast_id,
                                       uint8_t pa_sync, uint8_t big_enc, const uint8_t* p_bad_code,
                                       uint8_t nb_subgroups, uint32_t bis_sync_bf,
                                       const bap_cfg_metadata_ptr_t* p_metadata);

/**
 ****************************************************************************************
 * @brief Callback function called when a state for a Broadcast Source has been received and more than one Subgroup
 * is exposed
 *
 * @param[in] con_lid               Connection local index
 * @param[in] src_lid               Source local index
 * @param[in] sgrp_idx              Subgroup index\n
 *                                  From 1 to (nb_subgroups - 1)\n
 *                                  nb_subgroups value provided in #bap_bc_assist_cb_state callback function
 * @param[in] bis_sync_bf           BIS synchronization bit field for the Subgroup
 * @param[in] p_metadata            Pointer to Metadata for the Subgroup\n
 *                                  Can be NULL
 ****************************************************************************************
 */
typedef void (*bap_bc_assist_cb_state_subgroup)(uint8_t con_lid, uint8_t src_lid, uint8_t sgrp_idx,
                                                uint32_t bis_sync_bf, const bap_cfg_metadata_ptr_t* p_metadata);

/**
 ****************************************************************************************
 * @brief Callback function called when a Delegator requires Broadcast Code for a given Source
 *
 * @param[in] con_lid           Connection local index
 * @param[in] src_lid           Source local index
 ****************************************************************************************
 */
typedef void (*bap_bc_assist_cb_bcast_code_req)(uint8_t con_lid, uint8_t src_lid);

/**
 ****************************************************************************************
 * @brief Callback function called when a Solicitation Request has been received
 *
 * @param[in] addr_type         Address type
 * @param[in] p_addr            Pointer to address
 * @param[in] length            Length of advertising data
 * @param[in] p_adv_data        Pointer to advertising data
 ****************************************************************************************
 */
typedef void (*bap_bc_assist_cb_solicitation)(uint8_t addr_type, const uint8_t* p_addr, uint16_t length,
                                              const uint8_t* p_adv_data);

/**
 ****************************************************************************************
 * @brief Callback function called when scan allowing to find Solicitation Requests has been stopped due to a timeout
 ****************************************************************************************
 */
typedef void (*bap_bc_assist_cb_timeout)(void);

/**
 ****************************************************************************************
 * @brief Callback function called when a service changed indication has been received from a Delegator device
 *
 * @param[in] con_lid       Connection local index
 ****************************************************************************************
 */
typedef void (*bap_bc_assist_cb_svc_changed)(uint8_t con_lid);

/*
 * CALLBACK SET DEFINITION
 ****************************************************************************************
 */

/// Set of callback functions for BAP BC Assistant module
struct bap_bc_assist_cb
{
    /// Callback function called when a command has been completed
    bap_bc_assist_cb_cmp_evt cb_cmp_evt;
    /// Callback function called when state for a Broadcast Source has been received
    bap_bc_assist_cb_state cb_state;
    /// Callback function called when a state for a Broadcast Source has been received and more than one Subgroup
    /// is exposed
    bap_bc_assist_cb_state_subgroup cb_state_subgroup;
    /// Callback function called when a Delegator requires Broadcast Code for a given Source
    bap_bc_assist_cb_bcast_code_req cb_bcast_code_req;
    /// Callback function called when Broadcast Scan Audio Service has been discovered in a peer service device
    /// database
    bap_bc_assist_cb_bond_data cb_bond_data;
    /// Callback function called when a Solicitation Request has been received
    bap_bc_assist_cb_solicitation cb_solicitation;
    /// Callback function called when scan allowing to find Solicitation Requests has been stopped due to a timeout
    bap_bc_assist_cb_timeout cb_timeout;
    /// Callback function called when a service changed indication has been received from a Delegator device
    bap_bc_assist_cb_svc_changed cb_svc_changed;
};

/// @} BAP_BC_ASSIST_CALLBACK

/*
 * API FUNCTIONS DECLARATION
 ****************************************************************************************
 */

/// @addtogroup BAP_BC_ASSIST_FUNCTION
/// @{

/**
 ****************************************************************************************
 * @brief Configure use of BAP Broadcast Assistant module
 *
 * @param[in] p_cb          Pointer to set to callback functions for communication with upper layer
 * @param[in] p_cfg         Pointer to configuration structure
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t bap_bc_assist_configure(const bap_bc_assist_cb_t* p_cb, const bap_bc_assist_cfg_t* p_cfg);

/**
 ****************************************************************************************
 * @brief Enable use of Broadcast Scan Audio Service as Client and start discovery of the service in peer server
 * device database
 *
 * @param[in] con_lid       Connection local index
 * @param[in] shdl          Start handle for the discovery. Set GATT_INVALID_HDL if not provided
 * @param[in] ehdl          End handle for the discovery. Set GATT_INVALID_HDL if not provided
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t bap_bc_assist_discover(uint8_t con_lid, uint16_t shdl, uint16_t ehdl);

/**
 ****************************************************************************************
 * @brief Enable use of Broadcast Scan Audio Service as Client and set bonding information after reconnection with
 * peer server device
 *
 * @param[in] con_lid           Connection local index
 * @param[in] p_bass_info       Pointer to Broadcast Audio Scan Service description structure
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t bap_bc_assist_restore_bond_data(uint8_t con_lid, const bap_bc_assist_bass_t* p_bass_info);

/**
 ****************************************************************************************
 * @brief Start scanning for Solicitation Requests sent by Broadcast Delegator devices
 *
 * @param[in] timeout_s     Timeout in seconds
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t bap_bc_assist_start_scan(uint16_t timeout_s);

/**
 ****************************************************************************************
 * @brief Stop scanning for Solicitation Requests
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t bap_bc_assist_stop_scan(void);

#if (GAF_DBG)
/**
 ****************************************************************************************
 * @brief Get value for a Broadcast Receive Source instance of a peer server device
 *
 * @param[in] con_lid       Connection local index
 * @param[in] src_lid       Source local index
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t bap_bc_assist_get_state(uint8_t con_lid, uint8_t src_lid);

/**
 ****************************************************************************************
 * @brief Set Client Characteristic Configuration Descriptor value for for a Broadcast Receive Source instance of a
 * peer server device
 *
 * @param[in] con_lid           Connection local index
 * @param[in] src_lid           Source local index
 * @param[in] enable            Indicate if sending of notifications must be enabled (!= 0) or disabled
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t bap_bc_assist_set_cfg(uint8_t con_lid, uint8_t src_lid, uint8_t enable);
#endif //(GAF_DBG)

/**
 ****************************************************************************************
 * @brief Inform server that scanning on its behalf has been either started or stopped
 *
 * @param[in] con_lid       Connection local index
 * @param[in] started       Indicate if scan has been started (!= 0) or stopped
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t bap_bc_assist_update_scan(uint8_t con_lid, uint8_t started);

/**
 ****************************************************************************************
 * @brief Inform the server about discovery of a Broadcast Source
 *
 * @param[in] con_lid           Connection local index
 * @param[in] p_adv_id          Pointer to Advertising identification structure
 * @param[in] p_bcast_id        Pointer to Broadcast ID
 * @param[in] pa_sync           Required PA synchronization state (see #bap_bc_assist_pa_sync enumeration)
 * @param[in] pa_intv_frames    Periodic Advertising interval in frames\n
 *                              From 0x0006 to 0xFFFE\n
 *                              BAP_BC_UNKNOWN_PA_INTV indicates that interval is unknown
 * @param[in] nb_subgroups      Number of Subgroups\n
 *                              From 0 to 32
 * @param[in] bis_sync_bf       Required BIS synchronization bit field for the first Subgroup\n
 *                              Meaningful only if nb_subgroups != 0\n
 *                              #bap_bc_assist_set_sgrp_info function must be called after this function for each
 *                              additional Subgroup
 * @param[in] p_metadata        Pointer to Metadata\n
 *                              Meaningful only if nb_subgroups != 0\n
 *                              Can be NULL
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t bap_bc_assist_add_source(uint8_t con_lid, const bap_adv_id_t* p_adv_id,
                                  const bap_bcast_id_t* p_bcast_id, uint8_t pa_sync,
                                  uint16_t pa_intv_frames, uint8_t nb_subgroups, uint32_t bis_sync_bf,
                                  const bap_cfg_metadata_t* p_metadata);

#if (GAF_BAP_BC_SRC)
/**
 ****************************************************************************************
 * @brief Inform the server about a local Broadcast Source
 *
 * @param[in] con_lid           Connection local index
 * @param[in] grp_lid           Group local index
 * @param[in] pa_sync           Required PA synchronization state (see #bap_bc_assist_pa_sync enumeration)
 * @param[in] nb_subgroups      Number of Subgroups\n
 *                              From 0 to 32
 * @param[in] p_bis_sync_bfs    Pointer to BIS synchronization bit fields for each Subgroup\n
 *                              Cannot be NULL if nb_subgroups != 0
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t bap_bc_assist_add_source_local(uint8_t con_lid, uint8_t grp_lid, uint8_t pa_sync, uint8_t nb_subgroups,
                                    const uint32_t* p_bis_sync_bfs);
#endif //(GAF_BAP_BC_SRC)

/**
 ****************************************************************************************
 * @brief Request the server to remove information about a Broadcast Source
 *
 * @param[in] con_lid       Connection local index
 * @param[in] src_lid       Source local index
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t bap_bc_assist_remove_source(uint8_t con_lid, uint8_t src_lid);

/**
 ****************************************************************************************
 * @brief Request the server to synchronize to, or to stop synchronization to, a PA and/or a BIS
 *
 * @param[in] con_lid           Connection local index
 * @param[in] src_lid           Source local index
 * @param[in] pa_sync           Required PA synchronization state (see #bap_bc_assist_pa_sync enumeration)
 * @param[in] pa_intv_frames    Periodic Advertising interval in frames\n
 *                              From 0x0006 to 0xFFFE\n
 *                              BAP_BC_UNKNOWN_PA_INTV indicates that interval is unknown
 * @param[in] nb_subgroups      Number of Subgroups\n
 *                              From 0 to 32
 * @param[in] bis_sync_bf       Required BIS synchronization bit field for the first Subgroup\n
 *                              Meaningful only if nb_subgroups != 0\n
 *                              #bap_bc_assist_set_sgrp_info function must be called after this function for each
 *                              additional Subgroup
 * @param[in] p_metadata        Pointer to Metadata\n
 *                              Meaningful only if nb_subgroups != 0
 *                              Can be NULL
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t bap_bc_assist_modify_source(uint8_t con_lid, uint8_t src_lid, uint8_t pa_sync, uint16_t pa_intv_frames,
                                     uint8_t nb_subgroups, uint32_t bis_sync_bf,
                                     const bap_cfg_metadata_t* p_metadata);

#if (GAF_BAP_BC_SRC)
/**
 ****************************************************************************************
 * @brief Request the server to synchronize to, or to stop synchronization to, a PA and/or a BIS when Broadcast Group
 * has been locally created
 *
 * @param[in] con_lid           Connection local index
 * @param[in] src_lid           Source local index
 * @param[in] grp_lid           Group local index
 * @param[in] pa_sync           Required PA synchronization state (see #bap_bc_assist_pa_sync enumeration)
 * @param[in] nb_subgroups      Number of Subgroups\n
 *                              From 0 to 32
 * @param[in] p_bis_sync_bfs    Pointer to BIS synchronization bit fields for each Subgroup\n
 *                              Cannot be NULL if nb_subgroups != 0
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t bap_bc_assist_modify_source_local(uint8_t con_lid, uint8_t src_lid, uint8_t grp_lid, uint8_t pa_sync,
                                           uint8_t nb_subgroups, const uint32_t* p_bis_sync_bfs);
#endif //(GAF_BAP_BC_SRC)

/**
 ****************************************************************************************
 * @brief Set Subgroup information for Broadcast Source being added or updated
 *
 * @param[in] con_lid           Connection local index
 * @param[in] sgrp_idx          Subgroup index
 * @param[in] bis_sync_bf       Required BIS synchronization bit field for the Subgroup\n
 * @param[in] p_metadata        Pointer to Metadata for the Subgroup\n
 *                              Can be NULL
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t bap_bc_assist_set_sgrp_info(uint8_t con_lid, uint8_t sgrp_idx, uint32_t bis_sync_bf,
                                     const bap_cfg_metadata_t* p_metadata);

/**
 ****************************************************************************************
 * @brief Confirmation for BAP_BC_ASSIST_BCAST_CODE request indication
 *
 * @param[in] accept        Indicate if request is accepted or not
 * @param[in] con_lid       Connection local index
 * @param[in] src_lid       Source local index
 * @param[in] p_bcast_code  Pointer to Broadcast Code value
 ****************************************************************************************
 */
void bap_bc_assist_bcast_code_cfm(bool accept, uint8_t con_lid, uint8_t src_lid, const uint8_t* p_bcast_code);

/**
 ****************************************************************************************
 * @brief Set write type to be used for write of the Broadcast Source Control Point characteristic
 *
 * @param[in] reliable           Indicate if used of reliable write is preferred
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t bap_bc_assist_set_write_type(bool reliable);

/// @} BAP_BC_ASSIST_FUNCTION
#endif //(GAF_BAP_BC_ASSIST)

#endif // BAP_BC_ASSIST_H_
