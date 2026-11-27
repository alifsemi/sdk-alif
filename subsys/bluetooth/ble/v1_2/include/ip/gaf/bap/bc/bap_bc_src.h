/**
 ****************************************************************************************
 *
 * @file bap_bc_src.h
 *
 * @brief Basic Audio Profile - Broadcast Source - Definitions
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef BAP_BC_SRC_H_
#define BAP_BC_SRC_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup BAP_BC_SRC Broadcast Source module
 * @ingroup BAP_BC
 * @brief Description of Broadcast Source module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup BAP_BC_SRC_ENUM Enumerations
 * @ingroup BAP_BC_SRC
 * @brief Enumerations for Broadcast Source module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup BAP_BC_SRC_NATIVE_API Native API
 * @ingroup BAP_BC_SRC
 * @brief Description of Native API for Broadcast Source module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup BAP_BC_SRC_CALLBACK Callback Functions
 * @ingroup BAP_BC_SRC_NATIVE_API
 * @brief Description of callback functions for Broadcast Source module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup BAP_BC_SRC_FUNCTION Functions
 * @ingroup BAP_BC_SRC_NATIVE_API
 * @brief Description of functions for Broadcast Source module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "gaf.h"                // GAF Defines
#include "bap.h"                // Basic Audio Profile Definitions
#include "bap_bc.h"             // Basic Audio Profile Broadcast Definitions
#include "gap.h"                // GAP Definitions
#include "gapi.h"                // GAPI Definitions

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// @addtogroup BAP_BC_SRC_ENUM
/// @{

/// List of command type values for Broadcast Source module
enum bap_bc_src_cmd_type
{
    /// Enable Periodic Advertising for a Broadcast Group.
    BAP_BC_SRC_CMD_TYPE_ENABLE_PA = 0,
    /// Enable a Broadcast Group.
    BAP_BC_SRC_CMD_TYPE_ENABLE,
    /// Start transmission of audio data on either all streams or on a subset of streams within a Broadcast Group.
    BAP_BC_SRC_CMD_TYPE_START_STREAMING,
    /// Stop transmission of audio data for either all streams or for a subset of streams within a Broadcast Group.
    BAP_BC_SRC_CMD_TYPE_STOP_STREAMING,
    /// Disable Periodic Advertising for a Broadcast Group.
    BAP_BC_SRC_CMD_TYPE_DISABLE_PA,
    /// Disable a Broadcast Group.
    BAP_BC_SRC_CMD_TYPE_DISABLE,
    /// Update Metadata
    BAP_BC_SRC_CMD_TYPE_UPDATE_METADATA,
    /// Remove a Broadcast Group.
    BAP_BC_SRC_CMD_TYPE_REMOVE_GROUP,

    /// INTERNAL - Transfer Periodic Advertising
    BAP_BC_SRC_CMD_TYPE_PA_TRANSFER,
};

/// @} BAP_BC_SRC_ENUM

/*
 * CALLBACK FUNCTIONS DEFINITION
 ****************************************************************************************
 */

/// @addtogroup BAP_BC_SRC_CALLBACK
/// @{

/**
 ****************************************************************************************
 * @brief Callback function called each time a Broadcast Source command has been completed
 *
 * @param[in] cmd_type          Command type (see #bap_bc_src_cmd_type enumeration)
 * @param[in] status            Status
 * @param[in] grp_lid           Group local index
 * @param[in] sgrp_lid          Subgroup local index
 ****************************************************************************************
 */
typedef void (*bap_bc_src_cb_cmp_evt)(uint8_t cmd_type, uint16_t status, uint8_t grp_lid, uint8_t sgrp_lid);

/**
 ****************************************************************************************
 * @brief Callback function called when a group has been created
 *
 * @param[in] grp_lid           Group local index
 * @param[in] p_bg_cfg          Pointer to Broadcast Group configuration
 * @param[in] nb_bis            Number of BISes
 * @param[in] p_conhdl          Pointer to list of Connection Handle values provided by the Controller\n
 *                              List of nb_bis values
 ****************************************************************************************
 */
typedef void (*bap_bc_src_cb_info)(uint8_t grp_lid, const gapi_bg_config_t* p_bg_cfg, uint8_t nb_bis,
                                   const uint16_t* p_conhdl);

/*
 * CALLBACK SET DEFINITION
 ****************************************************************************************
 */

/// Set of callback functions for Broadcast Source module
struct bap_bc_src_cb
{
    /// Callback function called when a command has been completed
    bap_bc_src_cb_cmp_evt cb_cmp_evt;
    /// Callback function called when a group has been created
    bap_bc_src_cb_info cb_info;
};

/// @} BAP_BC_SRC_CALLBACK

/*
 * API FUNCTIONS DECLARATION
 ****************************************************************************************
 */

#if (GAF_BAP_BC_SRC)
/// @addtogroup BAP_BC_SRC_FUNCTION
/// @{

/**
 ****************************************************************************************
 * @brief Configure use of BAP Broadcast Source module
 *
 * @param[in] p_cb          Pointer to set to callback functions for communication with upper layer
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t bap_bc_src_configure(const bap_bc_src_cb_t* p_cb);

/**
 ****************************************************************************************
 * @brief Add a Broadcast Group.
 *
 * A Broadcast Group may contain one or several Broadcast Subgroup(s).
 * A Broadcast Subgroup may contain one or several Broadcast Stream(s).
 *
 * A local index is allocated for the added Broadcast Group and is returned.
 * Local index for a Broadcast Subgroup belonging to the group (named sgrp_lid in other messages) is in the range
 * [0, nb_subgroups[.
 *
 * Local index for a Broadcast Stream belonging to the group (named stream_lid in other messages) is in the range
 * [0, nb_streams[.
 *
 * By default, all Broadcast Streams are part of the first Subgroup (sgrp_lid = 0).
 *
 * A Broadcast Subgroup can be configured using #bap_bc_src_set_subgroup function.
 * A Broadcast Streams can be configured using #bap_bc_src_set_stream function.
 *
 * @param[in] p_bcast_id         Pointer to Broadcast ID value
 * @param[in] p_bcast_code       Pointer to Broadcast Code\n
 *                               NULL if Broadcast Group is not encrypted
 * @param[in] nb_streams         Number of Streams in the group
 *                               At least one Stream must be present in the group
 * @param[in] nb_subgroups       Number of Subgroups in the Broadcast Group
 *                               At least one Subgroup must be present in the group.
 * @param[in] p_grp_param        Pointer to Broadcast Group parameters
 * @param[in] p_adv_param        Pointer to Advertising parameters
 * @param[in] p_per_adv_param    Pointer to Periodic Advertising parameters
 * @param[in] pres_delay_us      Presentation Delay in microseconds
 * @param[out] p_grp_lid         Pointer at which allocated Group local index is returned
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t bap_bc_src_add_group(const bap_bcast_id_t* p_bcast_id, const gaf_bcast_code_t* p_bcast_code,
                              uint8_t nb_streams, uint8_t nb_subgroups, const bap_bc_grp_param_t* p_grp_param,
                              const bap_bc_adv_param_t* p_adv_param, const bap_bc_per_adv_param_t* p_per_adv_param,
                              uint32_t pres_delay_us, uint8_t* p_grp_lid);

/**
 ****************************************************************************************
 * @brief Remove a Broadcast Group.
 *
 * The Broadcast Group (including the Periodic Advertising) must have been disabled.
 *
 * @param[in] grp_lid            Group local index
 *
 * @return An error status (see #gaf_err enumeration)
 * If no error, no new function should be executed until #bap_bc_src_cb_cmp_evt callback function with
 * BAP_BC_SRC_REMOVE command code is called.
 ****************************************************************************************
 */
uint16_t bap_bc_src_remove_group(uint8_t grp_lid);

/**
 ****************************************************************************************
 * @brief Configure or reconfigure a Broadcast Subgroup.
 *
 * The Broadcast Group (including the Periodic Advertising) must not be enabled in case of a reconfiguration.
 *
 * @param[in] grp_lid            Group local index
 * @param[in] sgrp_lid           Subgroup local index
 * @param[in] p_codec_id         Pointer to Codec ID
 *                               Cannot be NULL
 * @param[in] p_cfg              Pointer to Codec Configuration structure\n
 *                               Can be NULL\n
 *                               Structure shall be allocated by Upper Layer and maintained until group is removed
 *                               using #bap_bc_src_remove_group function or until a new call of
 *                               #bap_bc_src_set_subgroup for the Subgroup
 * @param[in] p_metadata         Pointer to Metadata structure\n
 *                               Can be NULL\n
 *                               Structure shall be allocated by Upper Layer and maintained until a new call of
 *                               #bap_bc_src_set_subgroup or a call of #bap_bc_src_update_metadata function or
 *                               until group is removed using #bap_bc_src_remove_group function
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t bap_bc_src_set_subgroup(uint8_t grp_lid, uint8_t sgrp_lid, const gaf_codec_id_t* p_codec_id,
                                 const bap_cfg_t* p_cfg, const bap_cfg_metadata_t* p_metadata);

/**
 ****************************************************************************************
 * @brief Configure or reconfigure a Broadcast Stream and set the Broadcast Subgroup it belongs to.
 *
 * The Broadcast Group (including the Periodic Advertising) must not be enabled in case of a reconfiguration.
 *
 * @param[in] grp_lid            Group local index
 * @param[in] stream_lid         Stream local index
 * @param[in] sgrp_lid           Subgroup local index
 * @param[in] dp_cfg_bf          Data Path configuration bit field (Data Path ID, Codec location, ...)\n
 *                               See #bap_dp_cfg_bf for bit field meaning
 * @param[in] ctl_delay_us       Controller delay in microseconds
 * @param[in] p_cfg              Pointer to Codec Configuration structure\n
 *                               Can be NULL\n
 *                               Structure shall be allocated by Upper Layer and maintained until group is removed
 *                               using #bap_bc_src_remove_group function or until a new call of
 *                               #bap_bc_src_set_stream for the Stream.
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t bap_bc_src_set_stream(uint8_t grp_lid, uint8_t stream_lid, uint8_t sgrp_lid, uint16_t dp_cfg_bf,
                               uint32_t ctl_delay_us, const bap_cfg_t* p_cfg);

/**
 ****************************************************************************************
 * @brief Enable Periodic Advertising
 *
 * @param[in] grp_lid            Group local index.
 * @param[in] adv_data_len       Length of additional Advertising Data
 * @param[in] per_adv_data_len   Length of additional Periodic Advertising Data
 * @param[in] p_adv_data         Pointer to additional Advertising Data
 * @param[in] p_per_adv_data     Pointer to additional Periodic Advertising Data
 *                               Memory containing the additional Periodic Advertising Data must have been allocated
 *                               by the upper layer
 * @param[in] broadcast_name_len Length of Public Broadcast Name
 * @param[in] p_broadcast_name   Pointer to Public Broadcast Name
 * @param[in] metadata_len       Length of Public Broadcast Metadata
 * @param[in] p_metadata         Pointer to Public Broadcast Metadata
 *
 * @return An error status (see #gaf_err enumeration).
 *         If no error, no new function should be executed until #bap_bc_src_cb_cmp_evt callback function with
 *         BAP_BC_SRC_ENABLE_PA command code is called.
 ****************************************************************************************
 */
uint16_t bap_bc_src_enable_pa(uint8_t grp_lid, uint16_t adv_data_len, uint16_t per_adv_data_len,
                              const uint8_t* p_adv_data, const uint8_t* p_per_adv_data,
                              uint8_t broadcast_name_len, const uint8_t* p_broadcast_name,
                              uint8_t metadata_len, const uint8_t* p_metadata);

/**
 ****************************************************************************************
 * @brief Disable Periodic Advertising for a Broadcast Group.
 *
 * @param[in] grp_lid            Group local index
 *
 * @return An error status (see #gaf_err enumeration).
 *         If no error, no new function should be executed until #bap_bc_src_cb_cmp_evt callback function with
 *         BAP_BC_SRC_DISABLE_PA command code is called.
 ****************************************************************************************
 */
uint16_t bap_bc_src_disable_pa(uint8_t grp_lid);

/**
 ****************************************************************************************
 * @brief Enable a Broadcast Group.
 *
 * @param[in] grp_lid            Group local index
 *
 * @return An error status (see #gaf_err enumeration).
 *         If no error, no new function should be executed until #bap_bc_src_cb_cmp_evt callback function with
 *         BAP_BC_SRC_ENABLE command code is called.
 ****************************************************************************************
 */
uint16_t bap_bc_src_enable(uint8_t grp_lid);

/**
 ****************************************************************************************
 * @brief Disable a Broadcast Group\n
 * Periodic Advertising is also disabled if still enabled when this command is received\n
 * Streaming is also stopped for all Streams on which it has been started\n
 *
 * @param[in] grp_lid            Group local index
 * @param[in] disable_pa         Indicate if Periodic Advertising must be disabled
 *
 * @return An error status (see #gaf_err enumeration).
 *         If no error, no new function should be executed until #bap_bc_src_cb_cmp_evt callback function with
 *         BAP_BC_SRC_DISABLE command code is called.
 ****************************************************************************************
 */
uint16_t bap_bc_src_disable(uint8_t grp_lid, bool disable_pa);

/**
 ****************************************************************************************
 * @brief Start transmission of audio data on either all Streams or on a subset of Streams within a Broadcast Group.
 *
 * @param[in] grp_lid            Group local index
 * @param[in] stream_lid_bf      Stream local index bit field indicating for which Stream streaming must be started
 *                               0xFFFFFFFF means that streaming must be started for all Streams
 *
 * @return An error status (see #gaf_err enumeration).
 *         If no error, no new function should be executed until #bap_bc_src_cb_cmp_evt callback function with
 *         BAP_BC_SRC_START_STREAMING command code is called.
 ****************************************************************************************
 */
uint16_t bap_bc_src_start_streaming(uint8_t grp_lid, uint32_t stream_lid_bf);

/**
 ****************************************************************************************
 * @brief Stop transmission of audio data for either all Streams or for a subset of Streams within a Broadcast Group.
 *
 * @param[in] grp_lid            Group local index
 * @param[in] stream_lid_bf      Stream local index bit field indicating for which Stream streaming must be stopped
 *                               0xFFFFFFFF means that streaming must be stopped for all Streams
 *
 * @return An error status (see #gaf_err enumeration).
 *         If no error, no new function should be executed until #bap_bc_src_cb_cmp_evt callback function with
 *         BAP_BC_SRC_STOP_STREAMING command code is called.
 ****************************************************************************************
 */
uint16_t bap_bc_src_stop_streaming(uint8_t grp_lid, uint32_t stream_lid_bf);

/**
 ****************************************************************************************
 * @brief Update Metadata for a Broadcast Subgroup for which the Periodic Advertising has been started
 *
 * @param[in] grp_lid           Group local index
 * @param[in] sgrp_lid          Subgroup local index
 * @param[in] p_metadata        Pointer to Metadata structure\n
 *                              Can be NULL\n
 *                              Structure shall be allocated by Upper Layer and maintained until a new call of
 *                              #bap_bc_src_update_metadata function for the Subgroup or until group is removed using
 *                              #bap_bc_src_remove_group function
 *
 * @return An error status (see #gaf_err enumeration).
 *         If no error, no new function should be executed from the Group until #bap_bc_src_cb_cmp_evt callback
 *         function with BAP_BC_SRC_UPDATE_METADATA command code is called.
 ****************************************************************************************
 */
uint16_t bap_bc_src_update_metadata(uint8_t grp_lid, uint8_t sgrp_lid, const bap_cfg_metadata_t* p_metadata);

/**
 ****************************************************************************************
 * @brief Get Codec ID set for a Broadcast Subgroup
 *
 * @param[in] grp_lid       Group local index
 * @param[in] sgrp_lid      Subgroup locak index
 * @param[out] pp_codec_id  Pointer at which pointer to Codec ID will be returned
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t bap_bc_src_get_codec_id(uint8_t grp_lid, uint8_t sgrp_lid, gaf_codec_id_t** pp_codec_id);

/// @} BAP_BC_SRC_FUNCTION
#endif //(GAF_BAP_BC_SRC)

#endif // BAP_BC_SRC_H_
