/**
 ****************************************************************************************
 *
 * @file bap_bc_sink.h
 *
 * @brief Basic Audio Profile - Broadcast Sink - Definitions
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef BAP_BC_SINK_H_
#define BAP_BC_SINK_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup BAP_BC_SINK Broadcast Sink module
 * @ingroup BAP_BC
 * @brief Description of Broadcast Sink module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup BAP_BC_SINK_ENUM Enumerations
 * @ingroup BAP_BC_SINK
 * @brief Enumerations for Broadcast Sink module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup BAP_BC_SINK_NATIVE_API Native API
 * @ingroup BAP_BC_SINK
 * @brief Description of Native API for Broadcast Sink module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup BAP_BC_SINK_CALLBACK Callback Functions
 * @ingroup BAP_BC_SINK_NATIVE_API
 * @brief Description of callback functions for Broadcast Sink module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup BAP_BC_SINK_FUNCTION Functions
 * @ingroup BAP_BC_SINK_NATIVE_API
 * @brief Description of functions for Broadcast Sink module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "gaf.h"                // GAF Defines
#include "bap.h"                // Basic Audio Profile Definitions
#include "gap.h"                // GAP Definitions
#include "gapi.h"                // GAPI Definitions
#include "bap_bc.h"             // BAP Broadcast Definitions

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// @addtogroup BAP_BC_SINK_ENUM
/// @{

/// List of command type values for Broadcast Sink module
enum bap_bc_sink_cmd_type
{
    /// Enable a Broadcast Group Sink - it creates a corresponding Broadcast group.
    BAP_BC_SINK_CMD_TYPE_ENABLE = 0,
    /// Disable a Broadcast Group Sink - it removes Broadcast group and included streams.
    BAP_BC_SINK_CMD_TYPE_DISABLE,
    /// Start reception of audio data onto a Broadcast stream.
    BAP_BC_SINK_CMD_TYPE_START_STREAMING,
    /// Stop reception of audio data onto a Broadcast stream.
    BAP_BC_SINK_CMD_TYPE_STOP_STREAMING,
    /// Get reception quality for a Broadcast stream.
    BAP_BC_SINK_CMD_TYPE_GET_QUALITY,
};

/// Broadcast group sink state
enum bap_bc_sink_state
{
    /// Synchronization has been established
    BAP_BC_SINK_ESTABLISHED = 0x00,
    /// Synchronization has failed
    BAP_BC_SINK_FAILED = 0x01,
    /// Synchronization establishment has been cancelled
    BAP_BC_SINK_CANCELLED = 0x02,
    /// Synchronization has been lost
    BAP_BC_SINK_LOST = 0x03,
    /// Synchronization stopped due to peer termination
    BAP_BC_SINK_PEER_TERMINATE = 0x04,
    /// Synchronization stopped due to upper layer termination
    BAP_BC_SINK_UPPER_TERMINATE = 0x05,
    /// Synchronization stopped due to an encryption error
    BAP_BC_SINK_MIC_FAILURE = 0x06,
};

/// @} BAP_BC_SINK_ENUM

/*
 * CALLBACK FUNCTIONS DEFINITION
 ****************************************************************************************
 */

/// @addtogroup BAP_BC_SINK_CALLBACK
/// @{

/**
 ****************************************************************************************
 * @brief Callback function called each time a Broadcast sink command has been completed
 *
 *
 * @param[in] cmd_type      Command type (see #bap_bc_sink_cmd_type enumeration)
 * @param[in] status        Status (see #gaf_err enumeration)
 * @param[in] grp_lid       Group local index
 * @param[in] stream_pos    Stream position in group BAP_BC_SINK_START_STREAMING and BAP_BC_SINK_STOP_STREAMING
 *                          commands
 ****************************************************************************************
 */
typedef void (*bap_bc_sink_cb_cmp_evt)(uint8_t cmd_type, uint16_t status, uint8_t grp_lid, uint8_t stream_pos);

/**
 ****************************************************************************************
 * @brief Callback function called when BAP_BC_SINK_GET_QUALITY command has been completed
 *
 * @param[in] status                Status (see #gaf_err enumeration)
 * @param[in] grp_lid               Group local index
 * @param[in] stream_pos            Stream position
 * @param[in] crc_error_packets     Number of packets received with a CRC error
 * @param[in] rx_unrx_packets       Number of unreceived packets
 * @param[in] duplicate_packets     Number of duplicate packets received
 ****************************************************************************************
 */
typedef void (*bap_bc_sink_cb_quality_cmp_evt)(uint16_t status, uint8_t grp_lid, uint8_t stream_pos,
                                               uint32_t crc_error_packets, uint32_t rx_unrx_packets,
                                               uint32_t duplicate_packets);

/**
 ****************************************************************************************
 * @brief Inform upper layer about status of synchronization with a Broadcast Group
 *
 * Group local index is removed in case of sync termination (if state != BAP_BC_SINK_ESTABLISHED).
 *
 * @param[in] grp_lid           Group local index
 * @param[in] state             Synchronization state (see #bap_bc_sink_state enumeration)
 * @param[in] stream_pos_bf     Stream position bit field indicating Streams with which synchronization has been
 *                              established
 *                              Meaningful only if synchronization has been established
 * @param[in] p_bg_cfg          Pointer to Broadcast Group synchronization configuration
 *                              Meaningful only if synchronization has been established
 * @param[in] nb_bis            Number of BISes synchronization has been established with\n
 *                              Meaningful only if synchronization has been established
 * @param[in] p_conhdl          Pointer to list of Connection Handle values provided by the Controller\n
 *                              List of nb_bis values
 *****************************************************************************************
 */
typedef void (*bap_bc_sink_cb_status)(uint8_t grp_lid, uint8_t state, uint32_t stream_pos_bf,
                                      const gapi_bg_sync_config_t* p_bg_cfg, uint8_t nb_bis, const uint16_t* p_conhdl);

/**
 ****************************************************************************************
 * @brief Callback executed when Broadcast Delegator role is supported and request to establish synchronization with
 * a Broadcast Group has been received from a Broadcast Assistant
 * #bap_bc_sink_enable_cfm must be called to accept or reject synchronization establishment
 *
 * @param[in] grp_lid           Group local index
 * @param[in] src_lid           Source local index
 * @param[in] con_lid           Connection local index of Broadcast Assistant requiring synchronization
 * @param[in] stream_pos_bf     Bit field indicating streams with which synchronization is mandatory
 * @param[in] stream_pos_bf_opt Bit field indicating streams with which synchronization is optional.\n
 *                              List of selected streams to synchronize with must be provided in
 *                              #bap_bc_sink_enable_cfm function
 ****************************************************************************************
 */
typedef void (*bap_bc_sink_cb_enable_req)(uint8_t grp_lid, uint8_t src_lid, uint8_t con_lid, uint32_t stream_pos_bf,
                                          uint32_t stream_pos_bf_opt);

/**
 ****************************************************************************************
 * @brief Callback executed when Broadcast Delegator role is supported and request to stop synchronization with
 * a Broadcast Group has been received from a Broadcast Assistant
 * #bap_bc_sink_disable_cfm must be called to accept or reject synchronization stop
 *
 * @param[in] grp_lid           Group local index
 * @param[in] con_lid           Connection local index of Broadcast Assistant requiring synchronization
 ****************************************************************************************
 */
typedef void (*bap_bc_sink_cb_disable_req)(uint8_t grp_lid, uint8_t con_lid);

/*
 * CALLBACK SET DEFINITION
 ****************************************************************************************
 */

/// Set of callback functions for Basic Audio Profile - Broadcast Sink role
struct bap_bc_sink_cb
{
    /// Callback function called when a command has been completed
    bap_bc_sink_cb_cmp_evt cb_cmp_evt;
    /// Callback function called when BAP_BC_SINK_GET_QUALITY command has been completed
    bap_bc_sink_cb_quality_cmp_evt cb_quality_cmp_evt;
    /// Callback function called when status of synchronization with a Broadcast Group has changed
    bap_bc_sink_cb_status cb_status;
    #if (GAF_BAP_BC_DELEG)
    /// Callback function called when peer scan assistant requests to establish a broadcast sink
    bap_bc_sink_cb_enable_req cb_enable_req;
    /// Callback function called when peer scan assistant requests to terminate broadcast sink
    bap_bc_sink_cb_disable_req cb_disable_req;
    #endif //(GAF_BAP_BC_DELEG)
};

/// @} BAP_BC_SINK_CALLBACK

/*
 * API FUNCTIONS DECLARATION
 ****************************************************************************************
 */

#if (GAF_BAP_BC_SINK)
/// @addtogroup BAP_BC_SINK_FUNCTION
/// @{

/**
 ****************************************************************************************
 * @brief Configure use of BAP Broadcast Sink module
 *
 * @param[in] role_bf       Supported roles bit field (see #bap_role_bf enumeration)
 * @param[in] p_cb          Pointer to set to callback functions for communication with upper layer
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t bap_bc_sink_configure(uint32_t role_bf, const bap_bc_sink_cb_t* p_cb);

/**
 ****************************************************************************************
 * @brief Synchronize with a Broadcast Group
 *        Synchronization with the Periodic Advertising must have been established using
 *        #bap_bc_scan_pa_synchronize
 *        Synchronization status is provided with #bap_bc_sink_cb.cb_status callback function
 *
 * @param[in] pa_lid            Periodic Advertising local index
 * @param[in] p_bcast_id        Pointer to Broadcast ID
 * @param[in] stream_pos_bf     Stream position bit field indicating Streams to synchronize with
 * @param[in] p_bcast_code      Pointer to Broadcast Code, NULL if not encrypted
 * @param[in] mse               Maximum number of subevents the controller should use to received data payloads in
 *                              each interval
 * @param[in] timeout_10ms      Timeout duration (in multiple of 10ms) before considering synchronization has been
 *                              lost
 *                              From 100ms to 163.84s
 * @param[out] p_grp_lid        Pointer at which allocated Group local index is returned
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t bap_bc_sink_enable(uint8_t pa_lid, const bap_bcast_id_t* p_bcast_id, uint32_t stream_pos_bf,
                            const gaf_bcast_code_t* p_bcast_code, uint8_t mse, uint16_t timeout_10ms,
                            uint8_t* p_grp_lid);

/**
 ****************************************************************************************
 * @brief Stop synchronization with a Broadcast Group or cancel synchronization establishment
 *
 * @param[in] grp_lid           Group local index
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t bap_bc_sink_disable(uint8_t grp_lid);

/**
 ****************************************************************************************
 * @brief Start reception of audio data on a Broadcast Stream
 *
 * @param[in] grp_lid           Group local index
 * @param[in] stream_pos        Position of the Stream in the group
 *                              From 1 to 32
 * @param[in] p_codec_id        Pointer to Codec ID\n
 *                              NULL means that transparent mode is used
 * @param[in] dp_id             Data Path ID (see #data_path_id enumeration)
 * @param[in] ctl_delay_us      Controller delay in microseconds
 * @param[in] p_cfg_ltv         Pointer to Codec Configuration (in LTV format)\n
 *                              Meaningful only if first byte of Codec ID is not #GAPI_CODEC_FORMAT_TRANSPARENT or
 *                              if p_codec_id is not NULL
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t bap_bc_sink_start_streaming(uint8_t grp_lid, uint8_t stream_pos, const gaf_codec_id_t* p_codec_id,
                                     uint8_t dp_id, uint32_t ctl_delay_us, const gaf_ltv_t* p_cfg_ltv);

/**
 ****************************************************************************************
 * @brief Stop reception of audio data on a Broadcast Stream
 *
 * @param[in] grp_lid           Group local index
 * @param[in] stream_pos        Position of the Stream in the group
 *                              From 1 to 32
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t bap_bc_sink_stop_streaming(uint8_t grp_lid, uint8_t stream_pos);

/**
 ****************************************************************************************
 * @brief Get reception quality for a Broadcast Stream
 *
 * @param[in] grp_lid           Group local index
 * @param[in] stream_pos        Position of the Stream in the group
 *                              From 1 to 32
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t bap_bc_sink_get_quality(uint8_t grp_lid, uint8_t stream_pos);

#if (GAF_BAP_BC_DELEG)
/**
 ****************************************************************************************
 * @brief This function must be called in response to #bap_bc_sink_cb.cb_enable_req callback execution to accept or
 *        reject synchronization establishment with a Broadcast Group
 *
 *        Synchronization status is then provided with #bap_bc_sink_cb.cb_status callback
 *
 * @param[in] grp_lid            Group local index
 * @param[in] accept             True to accept broadcast sink establishment, False otherwise
 * @param[in] stream_sync_bf_opt Bit field indicating optional streams with which synchronization is requested
 * @param[in] timeout_10ms       Timeout duration (in multiple of 10ms) before considering synchronization has been
 *                               lost
 * @param[in] mse                Maximum number of subevents the controller should use to received data
 *                               payloads in each interval
 ****************************************************************************************
 */
void bap_bc_sink_enable_cfm(uint8_t grp_lid, bool accept, uint32_t stream_sync_bf_opt, uint16_t timeout_10ms,
                            uint8_t mse);

/**
 ****************************************************************************************
 * @brief This function must be called in response to #bap_bc_sink_cb.cb_disable_req callback execution to accept or
 *        reject end of synchronization with a Broadcast Group
 *
 * @param[in] grp_lid            Group local index
 * @param[in] accept             True to accept broadcast sink termination, False otherwise
 ****************************************************************************************
 */
void bap_bc_sink_disable_cfm(uint8_t grp_lid, bool accept);
#endif //(GAF_BAP_BC_DELEG)

/// @} BAP_BC_SINK_FUNCTION
#endif //(GAF_BAP_BC_SINK)

#endif // BAP_BC_SINK_H_
