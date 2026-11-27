/**
 ****************************************************************************************
 *
 * @file bap_capa_cli.h
 *
 * @brief Basic Audio Profile - Capabilities Client - Definitions
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef BAP_CAPA_CLI_H_
#define BAP_CAPA_CLI_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup BAP_CAPA_CLI Capabilities Client module
 * @ingroup BAP_CAPA
 * @brief Description of Capabilities Client module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup BAP_CAPA_CLI_ENUM Enumerations
 * @ingroup BAP_CAPA_CLI
 * @brief Enumerations for Capabilities Client module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup BAP_CAPA_CLI_STRUCT Structure
 * @ingroup BAP_CAPA_CLI
 * @brief Structures for Capabilities Client module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup BAP_CAPA_CLI_NATIVE_API Native API
 * @ingroup BAP_CAPA_CLI
 * @brief Description of Native API for Capabilities Client module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup BAP_CAPA_CLI_CALLBACK Callback Functions
 * @ingroup BAP_CAPA_CLI_NATIVE_API
 * @brief Description of callback functions for Capabilities Client module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup BAP_CAPA_CLI_FUNCTION Functions
 * @ingroup BAP_CAPA_CLI_NATIVE_API
 * @brief Description of functions for BAP Capabilities Client module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "gaf.h"                // GAF Defines
#include "bap_capa.h"           // Basic Audio Profile - Capabilities Definitions

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// @addtogroup BAP_CAPA_CLI_ENUM
/// @{

/// List of command type values for Capabilities Client module
enum bap_capa_cli_cmd_type
{
    /// Discover
    BAP_CAPA_CLI_CMD_TYPE_DISCOVER = 0,
    /// Get
    BAP_CAPA_CLI_CMD_TYPE_GET,
    /// Set Configuration
    BAP_CAPA_CLI_CMD_TYPE_SET_CFG = 3,
    /// Set Audio Locations
    BAP_CAPA_CLI_CMD_TYPE_SET_LOCATION,
};

/// Optional features bit field meaning
enum bap_capa_cli_feat_bf
{
    /// Indicate if Sink Audio Locations characteristic is writable or not
    BAP_CAPA_CLI_FEAT_SINK_LOC_WR_POS = 0,
    BAP_CAPA_CLI_FEAT_SINK_LOC_WR_BIT = CO_BIT(BAP_CAPA_CLI_FEAT_SINK_LOC_WR_POS),

    /// Indicate if Source Audio Locations characteristic is writable or not
    BAP_CAPA_CLI_FEAT_SRC_LOC_WR_POS = 1,
    BAP_CAPA_CLI_FEAT_SRC_LOC_WR_BIT = CO_BIT(BAP_CAPA_CLI_FEAT_SRC_LOC_WR_POS),
};

/// @} BAP_CAPA_CLI_ENUM

/*
 * TYPES DEFINITION
 ****************************************************************************************
 */

/// @addtogroup BAP_CAPA_CLI_STRUCT
/// @{

/// Published Audio Capabilities Service characteristic description structure
typedef struct bap_capa_cli_pacs_char
{
    /// Characteristic value handle
    uint16_t val_hdl;
    /// Client Characteristic Configuration descriptor handle
    uint16_t desc_hdl;
} bap_capa_cli_pacs_char_t;

/// Published Audio Capabilities Service content description structure
typedef struct bap_capa_cli_pacs
{
    /// Service description
    prf_svc_t svc_info;
    /// Number of PAC Groups for Sink direction
    uint8_t nb_pacs_sink;
    /// Number of PAC Group for Source direction
    uint8_t nb_pacs_src;
    /// Optional features bit field (see #bap_capa_cli_feat_bf enumeration)
    uint8_t opt_feat_bf;
    /// Characteristic information structures
    bap_capa_cli_pacs_char_t char_info[__ARRAY_EMPTY];
} bap_capa_cli_pacs_t;

/// @} BAP_CAPA_CLI_STRUCT


/*
 * CALLBACK FUNCTIONS DEFINITION
 ****************************************************************************************
 */

/// @addtogroup BAP_CAPA_CLI_CALLBACK
/// @{

/**
 ****************************************************************************************
 * @brief Callback function called each time a command has been completed
 *
 * @param[in] cmd_type      Command type (see #bap_capa_cli_cmd_type enumeration)
 * @param[in] status        Status
 * @param[in] con_lid       Connection local index
 * @param[in] param_1       Characteristic type or Direction
 *     - Direction: Meaningful only for BAP_CAPA_CLI_SET_LOCATION command (see #gaf_direction enumeration)
 *     - Characteristic type: Meaningless for BAP_CAPA_CLI_SET_LOCATION and BAP_CAPA_CLI_DISCOVER
 *     commands (see #bap_capa_char_type enumeration)
 * @param[in] pac_lid       PAC local index
 ****************************************************************************************
 */
typedef void (*bap_capa_cli_cb_cmp_evt)(uint8_t cmd_type, uint16_t status, uint8_t con_lid,
                                        uint8_t param_1, uint8_t pac_lid);

/**
 ****************************************************************************************
 * @brief Callback function called when Published Audio Capabilities Service has been discovered in a peer service
 * device database
 *
 * @param[in] con_lid           Connection local index
 * @param[in] p_pacs_info       Pointer to Published Audio Capabilities Service content description structure
 ****************************************************************************************
 */
typedef void (*bap_capa_cli_cb_bond_data)(uint8_t con_lid, bap_capa_cli_pacs_t* p_pacs_info);

/**
 ****************************************************************************************
 * @brief Callback function called when a PAC record has been received from a peer server device
 *
 * @param[in] con_lid               Connection local index
 * @param[in] pac_lid               PAC local index
 * @param[in] record_lid            Record local index
 * @param[in] nb_records            Number of records
 * @param[in] p_codec_id            Pointer to Codec ID
 * @param[in] p_capa                Pointer to Codec Capabilities structure
 * @param[in] p_metadata            Pointer to Metadata structure
 ****************************************************************************************
 */
typedef void (*bap_capa_cli_cb_record)(uint8_t con_lid, uint8_t pac_lid, uint8_t record_lid, uint8_t nb_records,
                                       const gaf_codec_id_t* p_codec_id, const bap_capa_ptr_t* p_capa,
                                       const bap_capa_metadata_ptr_t* p_metadata);

/**
 ****************************************************************************************
 * @brief Callback function called when value for Sink/Source Audio Locations characteristic has been received from
 * a peer server device
 *
 * @param[in] con_lid       Connection local index
 * @param[in] direction     Direction (see #gaf_direction enumeration)
 * @param[in] location_bf   Location bit field (see #gaf_loc_bf enumeration)
 ****************************************************************************************
 */
typedef void (*bap_capa_cli_cb_location)(uint8_t con_lid, uint8_t direction, uint32_t location_bf);

/**
 ****************************************************************************************
 * @brief Callback function called when value for Sink/Source Audio Locations or Audio Content Availability or
 * Supported Audio Content characteristic has been received from a peer server device
 *
 * @param[in] con_lid           Connection local index
 * @param[in] context_type      Context type (see #bap_capa_context_type enumeration)
 * @param[in] context_bf_sink   Context bit field for Sink direction (see #bap_context_type_bf enumeration)
 * @param[in] context_bf_src    Context bit field for Source direction (see #bap_context_type_bf enumeration)
 ****************************************************************************************
 */
typedef void (*bap_capa_cli_cb_context)(uint8_t con_lid, uint8_t context_type,
                                        uint16_t context_bf_sink, uint16_t context_bf_src);

/**
 ****************************************************************************************
 * @brief Callback function called when service changed indication for the Published Audio Capabilities Service has
 * been received
 *
 * @param[in] con_lid           Connection local index
 ****************************************************************************************
 */
typedef void (*bap_capa_cli_cb_svc_changed)(uint8_t con_lid);

/*
 * CALLBACK SET DEFINITION
 ****************************************************************************************
 */

/// Set of callback functions for BAP Capabilities Client module
struct bap_capa_cli_cb
{
    /// Callback function called when a command has been completed
    bap_capa_cli_cb_cmp_evt cb_cmp_evt;
    /// Callback function called when Published Audio Capabilities Service instance has been discovered in a peer
    /// server database
    bap_capa_cli_cb_bond_data cb_bond_data;
    /// Callback function called when a PAC record has been received from a peer server device
    bap_capa_cli_cb_record cb_record;
    /// Callback function called when value for Sink/Source Audio Locations characteristic has been received from a
    /// peer server device
    bap_capa_cli_cb_location cb_location;
    /// Callback function called when value for Available Audio Contexts or Supported Audio Contexts characteristic
    /// has been received from a peer server device
    bap_capa_cli_cb_context cb_context;
    /// Callback function called when service changed indication for the Published Audio Capabilities Service has been
    /// received
    bap_capa_cli_cb_svc_changed cb_svc_changed;
};

/// @} BAP_CAPA_CLI_CALLBACK

/*
 * API FUNCTIONS DECLARATION
 ****************************************************************************************
 */

#if (GAF_BAP_CAPA_CLI)
/// @addtogroup BAP_CAPA_CLI_FUNCTION
/// @{

/**
 ****************************************************************************************
 * @brief Configure use of BAP Capabilities Client module
 *
 * @param[in] p_cb          Pointer to set to callback functions for communication with upper layer
 * @param[in] p_cfg         Pointer to configuration structure
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t bap_capa_cli_configure(const bap_capa_cli_cb_t* p_cb, bap_capa_cli_cfg_t* p_cfg);

/**
 ****************************************************************************************
 * @brief Discover Audio Capabilities exposed by a Server device.
 *
 * @param[in] con_lid       Connection local index
 * @param[in] shdl          Start handle for the discovery. Set GATT_INVALID_HDL if not provided
 * @param[in] ehdl          End handle for the discovery. Set GATT_INVALID_HDL if not provided
 *
 * @return An error status (see #gaf_err enumeration). cb_cmp_evt callback function called once procedure is finished.
 ****************************************************************************************
 */
uint16_t bap_capa_cli_discover(uint8_t con_lid, uint16_t shdl, uint16_t ehdl);

/**
 ****************************************************************************************
 * @brief Restore Published Capabilities Audio Service information after connection with a server device with which
 * discovery has already been performed and with which a trusted relationship has been established.
 *
 * @param[in] con_lid           Connection local index
 * @param[in] p_pacs_info       Pointer to Service information structure
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t bap_capa_cli_restore_bond_data(uint8_t con_lid, bap_capa_cli_pacs_t* p_pacs_info);

/**
 ****************************************************************************************
 * @brief Set value for either Source Audio Locations or Sink Audio Locations characteristic of a peer server device
 *
 * @param[in] con_lid       Connection local index
 * @param[in] direction     Direction
 * @param[in] location_bf   Location bit field (see #gaf_loc_bf enumeration)
 *
 * @return An error status (see #gaf_err enumeration). cb_cmp_evt callback function called once procedure is finished.
 ****************************************************************************************
 */
uint16_t bap_capa_cli_set_location(uint8_t con_lid, uint8_t direction, uint32_t location_bf);

#if (GAF_DBG)
/**
 ****************************************************************************************
 * @brief Get value for either Source Audio Locations or Sink Audio Locations or Sink PAC or Source PAC or Available
 * Audio Contexts or Supported Audio Contexts characteristic of a peer server device
 *
 * @param[in] con_lid       Connection local index
 * @param[in] char_type     Characteristic type
 * @param[in] pac_lid       PAC local index for Sink/Source PAC characteristic
 *
 * @return An error status (see #gaf_err enumeration). cb_cmp_evt callback function called once procedure is finished.
 ****************************************************************************************
 */
uint16_t bap_capa_cli_get(uint8_t con_lid, uint8_t char_type, uint8_t pac_lid);

/**
 ****************************************************************************************
 * @brief Set Client Characteristic Configuration Descriptor value for either Source Audio Locations or Sink Audio
 * Locations or Sink PAC or Source PAC or Available Audio Contexts or Supported Audio Contexts characteristic of a peer
 * server device
 *
 * @param[in] con_lid       Connection local index
 * @param[in] char_type     Characteristic type
 * @param[in] pac_lid       PAC local index for Sink/Source PAC characteristic
 * @param[in] enable        Indicate if sending of events must be enabled (!= 0) or disabled
 *
 * @return An error status (see #gaf_err enumeration). cb_cmp_evt callback function called once procedure is finished.
 ****************************************************************************************
 */
uint16_t bap_capa_cli_set_cfg(uint8_t con_lid, uint8_t char_type, uint8_t pac_lid, uint8_t enable);
#endif //(GAF_DBG)

/// @} BAP_CAPA_CLI_FUNCTION
#endif //(GAF_BAP_CAPA_CLI)

#endif // BAP_CAPA_CLI_H_
