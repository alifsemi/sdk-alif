/**
 ****************************************************************************************
 *
 * @file bap_uc_cli.h
 *
 * @brief Basic Audio Profile - Unicast Client - Definitions
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef BAP_UC_CLI_H_
#define BAP_UC_CLI_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup BAP_UC_CLI Unicast Client module
 * @ingroup BAP_UC
 * @brief Description of Unicast Client module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup BAP_UC_CLI_DEFINE Definitions
 * @ingroup BAP_UC_CLI
 * @brief Definitions for Unicast Client module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup BAP_UC_CLI_STRUCT Structures
 * @ingroup BAP_UC_CLI
 * @brief Structures for Unicast Client module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup BAP_UC_CLI_ENUM Enumerations
 * @ingroup BAP_UC_CLI
 * @brief Enumerations for Unicast Client module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup BAP_UC_CLI_NATIVE_API Native API
 * @ingroup BAP_UC_CLI
 * @brief Description of Native API for Unicast Client module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup BAP_UC_CLI_CALLBACK Callback Functions
 * @ingroup BAP_UC_CLI_NATIVE_API
 * @brief Description of callback functions for Unicast Client module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup BAP_UC_CLI_FUNCTION Functions
 * @ingroup BAP_UC_CLI_NATIVE_API
 * @brief Description of functions for Unicast Client module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "gaf.h"                // GAF Definitions
#include "gapi.h"                // GAPI Definitions
#include "bap_uc.h"             // Basic Audio Profile - Unicast Definitions

/*
 * DEFINES
 ****************************************************************************************
 */

/// @addtogroup BAP_UC_CLI_DEFINE
/// @{

/// Maximum number of ASE configurations
#define BAP_UC_CLI_ASE_CFG_NB_MAX       (32)

/// @} BAP_UC_CLI_DEFINE

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// @addtogroup BAP_UC_CLI_ENUM
/// @{

/// List of command types for BAP Unicast Client
enum bap_uc_cli_cmd_type
{
    /// Discover
    BAP_UC_CLI_CMD_TYPE_DISCOVER = 0,
    /// Configure Codec for an ASE
    BAP_UC_CLI_CMD_TYPE_CONFIGURE_CODEC,
    /// Configure QoS for an ASE
    BAP_UC_CLI_CMD_TYPE_CONFIGURE_QOS,
    /// Enable an ASE
    BAP_UC_CLI_CMD_TYPE_ENABLE,
    /// Update Metadata for an ASE
    BAP_UC_CLI_CMD_TYPE_UPDATE_METADATA,
    /// Disable an ASE
    BAP_UC_CLI_CMD_TYPE_DISABLE,
    /// Release an ASE
    BAP_UC_CLI_CMD_TYPE_RELEASE,
    /// Get Quality
    BAP_UC_CLI_CMD_TYPE_GET_QUALITY,
    /// Set Configuration
    BAP_UC_CLI_CMD_TYPE_SET_CFG = BAP_UC_CLI_CMD_TYPE_GET_QUALITY + 3,
    /// Get ASE State
    BAP_UC_CLI_CMD_TYPE_GET_STATE,
    /// Remove Group
    BAP_UC_CLI_CMD_TYPE_REMOVE_GROUP,
    /// Control CIS
    BAP_UC_CLI_CMD_TYPE_CIS_CONTROL,
    /// Set Group Parameters
    BAP_UC_CLI_CMD_TYPE_SET_GROUP_PARAMS,
};

/// Configuration bit field meaning for BAP Unicast Client
enum bap_uc_cli_cfg_bf
{
    /// Indicate if used of reliable write is preferred.\n
    /// Can later be changed using #bap_uc_cli_set_write_type function (#BAP_UC_CLI_SET_WRITE_TYPE request message)
    BAP_UC_CLI_CFG_RELIABLE_WR_POS = 0,
    BAP_UC_CLI_CFG_RELIABLE_WR_BIT = CO_BIT(BAP_UC_CLI_CFG_RELIABLE_WR_POS),
};

/// CIS state update event values
enum bap_uc_cli_cis_event
{
    /// An ASE has been bound with the Stream
    BAP_UC_CLI_CIS_EVENT_ASE_BOUND = 0,
    /// An ASE has been unbound from the Stream
    BAP_UC_CLI_CIS_EVENT_ASE_UNBOUND,
    /// CIS has been successfully established
    BAP_UC_CLI_CIS_EVENT_ESTABLISHED,
    /// CIS has failed to be established
    BAP_UC_CLI_CIS_EVENT_FAILED,
    /// CIS has been disconnected or has been lost
    BAP_UC_CLI_CIS_EVENT_DISCONNECTED,
};

/// @} BAP_UC_CLI_ENUM

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// @addtogroup BAP_UC_CLI_STRUCT
/// @{

/// Audio Stream Control Service characteristic description structure
typedef struct bap_uc_cli_ascs_char
{
    /// Characteristic value handle
    uint16_t val_hdl;
    /// Client Characteristic Configuration descriptor handle
    uint16_t desc_hdl;
    /// ASE ID
    uint8_t ase_id;
} bap_uc_cli_ascs_char_t;

/// Audio Stream Control Service content description structure
typedef struct bap_uc_cli_ascs
{
    /// Service description
    prf_svc_t svc_info;
    /// Number of discovered Sink ASE characteristics
    uint8_t nb_ases_sink;
    /// Number of discovered Source ASE characteristics
    uint8_t nb_ases_src;
    /// Characteristic information structures
    bap_uc_cli_ascs_char_t char_info[__ARRAY_EMPTY];
} bap_uc_cli_ascs_t;

/// Unicast Group parameters
typedef struct bap_uc_cli_grp_param
{
    /// SDU interval from Master to Slave in microseconds
    /// From 0xFF (255us) to 0xFFFF (1.048575s)
    uint32_t sdu_intv_m2s_us;
    /// SDU interval from Slave to Master in microseconds
    /// From 0xFF (255us) to 0xFFFF (1.048575s)
    uint32_t sdu_intv_s2m_us;
    /// Maximum time (in milliseconds) for an SDU to be transported from master controller to slave
    /// controller. From 0x5 (5ms) to 0xFA0 (4s)
    uint16_t tlatency_m2s_ms;
    /// Maximum time (in milliseconds) for an SDU to be transported from slave controller to master
    /// controller. From 0x5 (5ms) to 0xFA0 (4s)
    uint16_t tlatency_s2m_ms;
    /// Sequential or interleaved scheduling
    uint8_t packing;
    /// Unframed or framed mode
    uint8_t framing;
    /// Worst slow clock accuracy of slaves
    uint8_t sca;
} bap_uc_cli_grp_param_t;

/// QoS Configuration structure (short)
typedef struct bap_uc_cli_qos_cfg
{
    /// PHY
    uint8_t phy;
    /// Maximum number of retransmissions for each CIS Data PDU
    /// From 0 to 0xFF
    uint8_t retx_nb;
    /// Maximum SDU size
    /// From 0 to 4095 bytes (0xFFF)
    uint16_t max_sdu_size;
    /// Presentation Delay in microseconds
    uint32_t pres_delay_us;
} bap_uc_cli_qos_cfg_t;

/// @} BAP_UC_STRUCT

/*
 * CALLBACK FUNCTIONS DEFINITION
 ****************************************************************************************
 */

/// @addtogroup BAP_UC_CLI_CALLBACK
/// @{

/**
 ****************************************************************************************
 * @brief Callback function called each time a command has been completed
 *
 * @param[in] cmd_type      Command type (see #bap_uc_cli_cmd_type enumeration)
 * @param[in] status        Status
 * @param[in] lid           Connection local index or Group local index
 * @param[in] ase_info      ASE information (ASE local index or ASE instance index)
 * @param[in] char_type     Characteristic type (see #bap_uc_char_type enumeration)
 ****************************************************************************************
 */
typedef void (*bap_uc_cli_cb_cmp_evt)(uint8_t cmd_type, uint16_t status, uint8_t lid, uint8_t ase_info,
                                      uint8_t char_type);

/**
 ****************************************************************************************
 * @brief Callback function called each time BAP_UC_CLI_GET_QUALITY command has been completed
 *
 * @param[in] status                    Status
 * @param[in] ase_lid                   ASE local index
 * @param[in] tx_unacked_packets        Number of packets transmitted and unacked
 * @param[in] tx_flushed_packets        Number of flushed transmitted packets
 * @param[in] tx_last_subevent_packets  Number of packets transmitted during last subevent
 * @param[in] retransmitted_packets     Number of retransmitted packets
 * @param[in] crc_error_packets         Number of packets received with a CRC error
 * @param[in] rx_unreceived_packets     Number of unreceived packets
 * @param[in] duplicate_packets         Number of duplicate packet received
 ****************************************************************************************
 */
typedef void (*bap_uc_cli_cb_quality_cmp_evt)(uint16_t status, uint8_t ase_lid, uint32_t tx_unacked_packets,
                                              uint32_t tx_flushed_packets, uint32_t tx_last_subevent_packets,
                                              uint32_t retransmitted_packets, uint32_t crc_error_packets,
                                              uint32_t rx_unreceived_packets, uint32_t duplicate_packets);

/**
 ****************************************************************************************
 * @brief Callback function called when Audio Stream Control Service has been discovered in a peer server device
 * database
 *
 * @param[in] con_lid           Connection local index
 * @param[in] p_ascs_info       Pointer to Audio Stream Control Service content description
 * structure
 ****************************************************************************************
 */
typedef void (*bap_uc_cli_cb_bond_data)(uint8_t con_lid, bap_uc_cli_ascs_t* p_ascs_info);

/**
 ****************************************************************************************
 * @brief Callback function called when an error has been received for an ASE
 *
 * @param[in] ase_lid           ASE local index
 * @param[in] opcode            Operation code (see #bap_uc_opcode enumeration)
 * @param[in] rsp_code          Response code (see #bap_uc_cp_rsp_code enumeration)
 * @param[in] reason            Reason (see #bap_uc_cp_reason enumeration)
 ****************************************************************************************
 */
typedef void (*bap_uc_cli_cb_error)(uint8_t ase_lid, uint8_t opcode, uint8_t rsp_code, uint8_t reason);

/**
 ****************************************************************************************
 * @brief Callback function called when state of a CIS has been updated
 *
 * @param[in] stream_lid        Stream local index
 * @param[in] event             Event that has trigerred CIS state update (see #bap_uc_cli_cis_event enumeration)
 * @param[in] con_lid           Connection local index of LE connection the CIS is bound with
 * @param[in] ase_lid_sink      ASE local index for Sink direction
 * @param[in] ase_lid_src       ASE local index for Source direction
 * @param[in] grp_lid           Group local index
 * @param[in] cis_id            CIS ID
 * @param[in] conhdl            Connection handle allocated for the CIS by Controller
 *                              GAP_INVALID_CONHDL indicates that CIS is not connected
 * @param[in] p_cig_cfg         Pointer to CIG parameters. NULL if conhdl == GAP_INVALID_CONHDL
 * @param[in] p_cis_cfg         Pointer to CIS parameters. NULL if conhdl == GAP_INVALID_CONHDL
 ****************************************************************************************
 */
typedef void (*bap_uc_cli_cb_cis_state)(uint8_t stream_lid, uint8_t event, uint8_t con_lid, uint8_t ase_lid_sink,
                                        uint8_t ase_lid_src, uint8_t grp_lid, uint8_t cis_id, uint16_t conhdl,
                                        gapi_ug_config_t* p_cig_cfg, gapi_us_config_t* p_cis_cfg);

/**
 ****************************************************************************************
 * @brief Callback function called when state of an ASE has been update and state is Idle or Releasing\n
 * It is recommended to postpone any action triggerred by reception of this state (see use of delayed job co_djob.h)
 *
 * @param[in] con_lid           Connection local index
 *                              GAF_INVALID_LID indicates that the ASE is no more used
 * @param[in] ase_instance_idx  ASE instance index
 *                              Meaningful only if con_lid is not equal to GAF_INVALID_LID
 * @param[in] ase_lid           ASE local index
 * @param[in] state             State (see #bap_uc_ase_state enumeration)
 *                              Meaningful only if con_lid is not equal to GAF_INVALID_LID
 ****************************************************************************************
 */
typedef void (*bap_uc_cli_cb_state_empty)(uint8_t con_lid, uint8_t ase_instance_idx, uint8_t ase_lid, uint8_t state);

/**
 ****************************************************************************************
 * @brief Callback function called when state of an ASE has been update and state is Codec Configured\n
 * It is recommended to postpone any action triggerred by reception of this state (see use of delayed job co_djob.h)
 *
 * @param[in] con_lid           Connection local index
 * @param[in] ase_instance_idx  ASE instance index
 * @param[in] ase_lid           ASE local index
 * @param[in] p_codec_id        Pointer to Codec ID
 * @param[in] p_qos_req         Pointer to QoS requirements structure
 * @param[in] p_cfg             Pointer to Codec Configuration structure
 ****************************************************************************************
 */
typedef void (*bap_uc_cli_cb_state_codec)(uint8_t con_lid, uint8_t ase_instance_idx, uint8_t ase_lid,
                                          gaf_codec_id_t* p_codec_id, bap_qos_req_t* p_qos_req,
                                          const bap_cfg_ptr_t* p_cfg);

/**
 ****************************************************************************************
 * @brief Callback function called when state of an ASE has been update and state is QoS Configured\n
 * It is recommended to postpone any action triggerred by reception of this state (see use of delayed job co_djob.h)
 *
 * @param[in] ase_lid           ASE local index
 * @param[in] p_qos_cfg         Pointer to QoS configuration structure
 ****************************************************************************************
 */
typedef void (*bap_uc_cli_cb_state_qos)(uint8_t ase_lid, const bap_qos_cfg_t* p_qos_cfg);

/**
 ****************************************************************************************
 * @brief Callback function called when state of an ASE has been update and state is Enable or Streaming or
 * Disabling\n
 * It is recommended to postpone any action triggerred by reception of this state (see use of delayed job co_djob.h)
 *
 * @param[in] ase_lid           ASE local index
 * @param[in] state             ASE state
 * @param[in] p_metadata        Pointer to Metadata structure
 ****************************************************************************************
 */
typedef void (*bap_uc_cli_cb_state_metadata)(uint8_t ase_lid, uint8_t state,
                                             const bap_cfg_metadata_ptr_t* p_metadata);

/**
 ****************************************************************************************
 * @brief Callback function called when service changed indication has been received for Audio Stream Control Service
 * from a peer server device
 *
 *
 * @param[in] con_lid       Connection local index
 ****************************************************************************************
 */
typedef void (*bap_uc_cli_cb_svc_changed)(uint8_t con_lid);

/**
 ****************************************************************************************
 * @brief Callback function called when data path is about to be started or stopped for an ASE
 *
 * @param[in] ase_lid       ASE local index
 * @param[in] start         Indicate if Data Path is about to be started or stopped for the ASE
 ****************************************************************************************
 */
typedef void (*bap_uc_cli_cb_dp_update_req)(uint8_t ase_lid, bool start);

/*
 * CALLBACK SET DEFINITION
 ****************************************************************************************
 */

/// Set of callback functions for Audio Stream Control Service Client
struct bap_uc_cli_cb
{
    /// Callback function called when a command has been completed
    bap_uc_cli_cb_cmp_evt cb_cmp_evt;
    /// Callback function called when BAP_UC_CLI_GET_QUALITY command has been completed
    bap_uc_cli_cb_quality_cmp_evt cb_quality_cmp_evt;
    /// Callback function called when Audio Stream Control Service has been discovered in a peer server device
    /// database
    bap_uc_cli_cb_bond_data cb_bond_data;
    /// Callback function called when an error has been received for an ASE
    bap_uc_cli_cb_error cb_error;
    /// Callback function called when state of a CIS has been updated
    bap_uc_cli_cb_cis_state cb_cis_state;
    /// Callback function called when state of an ASE has been update and state is Idle or Releasing
    bap_uc_cli_cb_state_empty cb_state_empty;
    /// Callback function called when state of an ASE has been update and state is Codec Configured
    bap_uc_cli_cb_state_codec cb_state_codec;
    /// Callback function called when state of an ASE has been update and state is QoS Configured
    bap_uc_cli_cb_state_qos cb_state_qos;
    /// Callback function called when state of an ASE has been update and state is Enable or Streaming or Disabling
    bap_uc_cli_cb_state_metadata cb_state_metadata;
    /// Callback function called when service changed indication has been received for Audio Stream Control Service
    /// from a peer server device
    bap_uc_cli_cb_svc_changed cb_svc_changed;
    /// Callback function called when data path is about to be started or stopped for an ASE
    bap_uc_cli_cb_dp_update_req cb_dp_update_req;
};

/// @} BAP_UC_CLI_CALLBACK

/*
 * API FUNCTIONS DECLARATION
 ****************************************************************************************
 */

#if (GAF_BAP_UC_CLI)
/// @addtogroup BAP_UC_CLI_FUNCTION
/// @{

/**
 ****************************************************************************************
 * @brief Configure use of BAP Unicast Client module
 *
 * @param[in] p_cb          Pointer to set to callback functions for communication with upper layer
 * @param[in] p_cfg         Pointer to configuration structure
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t bap_uc_cli_configure(const bap_uc_cli_cb_t* p_cb, bap_uc_cli_cfg_t* p_cfg);

/**
 ****************************************************************************************
 * @brief Enable use of Audio Stream Control Service as Client and start discovery of the service in peer server device
 * database
 *
 * @param[in] con_lid       Connection local index
 * @param[in] shdl          Start handle for the discovery. Set GATT_INVALID_HDL if not provided
 * @param[in] ehdl          End handle for the discovery. Set GATT_INVALID_HDL if not provided
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t bap_uc_cli_discover(uint8_t con_lid, uint16_t shdl, uint16_t ehdl);

/**
 ****************************************************************************************
 * @brief Enable use of BAP Unicast Client module and set bonding information after reconnection with peer server
 * device
 *
 * @param[in] con_lid           Connection local index
 * @param[in] p_ascs_info       Pointer to Audio Stream Control Service content description
 * structure
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t bap_uc_cli_restore_bond_data(uint8_t con_lid, bap_uc_cli_ascs_t* p_ascs_info);

/**
 ****************************************************************************************
 * @brief Restore bond data about an ASE in Codec Configured state
 *
 * @param[in] con_lid               Connection local index
 * @param[in] ase_instance_idx      ASE instance index
 * @param[in] ase_lid               ASE local index
 * @param[in] dp_cfg_bf             Data Path configuration bit field (Data Path ID, Codec location, ...)\n
 *                                  See #bap_dp_cfg_bf for bit field meaning
 * @param[in] p_codec_id            Pointer to Codec ID
 * @param[in] ctl_delay_us          Controller Delay in microseconds
 * @param[in] p_cfg                 Pointer to Codec Configuration structure\n
 *                                  Can be NULL\n
 *                                  Structure shall be allocated by Upper Layer and maintained until a call of
 *                                  #bap_uc_cli_configure_codec or a new call of #bap_uc_cli_restore_bond_data_codec
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t bap_uc_cli_restore_bond_data_codec(uint8_t con_lid, uint8_t ase_instance_idx, uint8_t ase_lid,
                                            uint16_t dp_cfg_bf, gaf_codec_id_t* p_codec_id,
                                            uint32_t ctl_delay_us, const bap_cfg_t* p_cfg);

/**
 ****************************************************************************************
 * @brief Create a group
 *
 * @param[in] cig_id            CIG ID
 * @param[in] p_param           Pointer to Group parameters structure
 * @param[out] p_grp_lid        Pointer at which allocated Group local index is returned
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t bap_uc_cli_create_group(uint8_t cig_id, bap_uc_cli_grp_param_t* p_param, uint8_t* p_grp_lid);

/**
 ****************************************************************************************
 * @brief Configure a Stream before any ASE configuration\n
 * See #bap_msc_cig_configuration\n
 * Up to application to ensure that QoS parameters set using #bap_uc_cli_configure_qos are compatible with
 * provided configuration
 *
 * @param[in] grp_lid           Group local index
 * @param[in] cis_id            CIS ID
 * @param[out] p_params         Pointer to Stream parameters
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t bap_uc_cli_create_stream(uint8_t grp_lid, uint8_t cis_id, gapi_us_param_t* p_params);

/**
 ****************************************************************************************
 * @brief Provide Group parameters (set using #bap_uc_cli_create_group and #bap_uc_cli_create_stream function) to
 * the Controller (using HCI LE Set CIG Parameters command).
 * See #bap_msc_cig_configuration\n

 * @param[in] grp_lid           Group local index
 *
 * @return An error status (see #gaf_err enumeration).
 *         If #GAF_ERR_NO_ERROR is returned, operation will be considered as completed with call of cb_cmp_evt
 *         callback function
 ****************************************************************************************
 */
uint16_t bap_uc_cli_set_group_params(uint8_t grp_lid);

/**
 ****************************************************************************************
 * @brief Remove a group
 *
 * @param[in] grp_lid       Group local index
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t bap_uc_cli_remove_group(uint8_t grp_lid);

/**
 ****************************************************************************************
 * @brief Establish or disconnect a CIS bound with an indicated ASE.
 *
 * @param[in] ase_lid           ASE local index
 * @param[in] establish         Indicate if CIS the indicated ASE is bound with must be
 * established (!= 0) or disconnected
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t bap_uc_cli_cis_control(uint8_t ase_lid, bool establish);

/**
 ****************************************************************************************
 * @brief Configure Codec for an ASE
 *
 * @param[in] con_lid           Connection local index
 * @param[in] ase_instance_idx  ASE instance index
 * @param[in] ase_lid           ASE local index
 * @param[in] dp_cfg_bf         Data Path configuration bit field (Data Path ID, Codec location, ...)\n
 *                              See #bap_dp_cfg_bf for bit field meaning
 * @param[in] p_codec_id        Pointer to Codec ID
 * @param[in] tgt_latency       Target Latency (see #bap_uc_tgt_latency enumeration)
 * @param[in] tgt_phy           Target PHY (see #bap_uc_tgt_phy enumeration)
 * @param[in] ctl_delay_us      Controller Delay in microseconds
 * @param[in] p_cfg             Pointer to Codec Configuration structure\n
 *                              Can be NULL
 *                              Structure shall be allocated by Upper Layer and maintained until a call of
 *                              #bap_uc_cli_configure_codec or #bap_uc_cli_cb.cb_state_codec
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t bap_uc_cli_configure_codec(uint8_t con_lid, uint8_t ase_instance_idx, uint8_t ase_lid,
                                    uint16_t dp_cfg_bf, gaf_codec_id_t* p_codec_id, uint8_t tgt_latency,
                                    uint8_t tgt_phy, uint32_t ctl_delay_us, const bap_cfg_t* p_cfg);

/**
 ****************************************************************************************
 * @brief Configure QoS for an ASE
 *
 * @param[in] ase_lid       ASE local index
 * @param[in] grp_lid       Group local index
 * @param[in] cis_id        CIS ID
 * @param[in] p_qos_cfg     Pointer to QoS configuration structure
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t bap_uc_cli_configure_qos(uint8_t ase_lid, uint8_t grp_lid, uint8_t cis_id,
                                  const bap_uc_cli_qos_cfg_t* p_qos_cfg);

/**
 ****************************************************************************************
 * @brief Enable an ASE
 *
 * @param[in] ase_lid       ASE local index
 * @param[in] p_metadata    Pointer to Metadata structure
 *                          List of CCIDs for the ASE can be provided as part of the additional Metadata
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t bap_uc_cli_enable(uint8_t ase_lid, const bap_cfg_metadata_t* p_metadata);

/**
 ****************************************************************************************
 * @brief Update metadata for an ASE
 *
 * @param[in] ase_lid       ASE local index
 * @param[in] p_metadata    Pointer to Metadata structure
 *                          List of CCIDs for the ASE can be provided as part of the additional Metadata
 *
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t bap_uc_cli_update_metadata(uint8_t ase_lid, const bap_cfg_metadata_t* p_metadata);

/**
 ****************************************************************************************
 * @brief Disable an ASE (stop streaming)
 *
 * @param[in] ase_lid       ASE local index
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t bap_uc_cli_disable(uint8_t ase_lid);

/**
 ****************************************************************************************
 * @brief Release an ASE
 *
 * @param[in] ase_lid       ASE local index
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t bap_uc_cli_release(uint8_t ase_lid);

/**
 ****************************************************************************************
 * @brief Get quality information about an established CIS
 *
 * @param[in] ase_lid       ASE local index
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t bap_uc_cli_get_quality(uint8_t ase_lid);

#if (GAF_DBG)
/**
 ****************************************************************************************
 * @brief Get value of an ASE characteristic instance
 *
 * @param[in] con_lid           Connection local index
 * @param[in] ase_instance_idx  ASE instance index
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t bap_uc_cli_get_state(uint8_t con_lid, uint8_t ase_instance_idx);

/**
 ****************************************************************************************
 * @brief Set Client Characteristic Configuration Descriptor value for either an ASE or ASE Control Point
 * characteristic of a peer server device
 *
 * @param[in] con_lid           Connection local index
 * @param[in] char_type         Characteristic type
 * @param[in] ase_instance_idx  ASE instance index for ASE characteristic
 * @param[in] enable            Indicate if sending of events must be enabled (!= 0) or disabled
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t bap_uc_cli_set_cfg(uint8_t con_lid, uint8_t char_type, uint8_t ase_instance_idx, uint8_t enable);
#endif //(GAF_DBG)

/**
 ****************************************************************************************
 * @brief Confirmation function for BAP_UC_CLI_DP_UPDATE request indication
 *
 * @param[in] ase_lid           ASE local index
 * @param[in] accept            Indicate if data path update is accepted or not
 * During ENABLE procedure if accept = true, data path is started,
 * Else nothing is done. Application will then have to initiate either start of DISABLE or RELEASE procedure.
 *
 * During DISABLE procedure if accept = true, data path is started,
 * Else nothing is done. Application will then have to initiate start of RELEASE procedure
 ****************************************************************************************
 */
void bap_uc_cli_dp_update_cfm(uint8_t ase_lid, bool accept);

/**
 ****************************************************************************************
 * @brief Set write type to be used for write configuration of ASEs through the ASE Control Point characteristic
 *
 * @param[in] reliable           Indicate if used of reliable write is preferred
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t bap_uc_cli_set_write_type(bool reliable);

/// @} BAP_UC_CLI_FUNCTION
#endif //(GAF_BAP_UC_CLI)

#endif // BAP_UC_CLI_H_
