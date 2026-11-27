/**
 ****************************************************************************************
 *
 * @file bap_capa_srv.h
 *
 * @brief Basic Audio Profile - Capabilities Server - Definitions
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef BAP_CAPA_SRV_H_
#define BAP_CAPA_SRV_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup BAP_CAPA_SRV Capabilities Server module
 * @ingroup BAP_CAPA
 * @brief Description of Capabilities Server module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup BAP_CAPA_SRV_DEFINE Definitions
 * @ingroup BAP_CAPA_SRV
 * @brief Definitions for Capabilities Server module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup BAP_CAPA_SRV_ENUM Enumerations
 * @ingroup BAP_CAPA_SRV
 * @brief Enumerations for Capabilities Server module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup BAP_CAPA_SRV_NATIVE_API Native API
 * @ingroup BAP_CAPA_SRV
 * @brief Description of Native API for Capabilities Server module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup BAP_CAPA_SRV_CALLBACK Callback Functions
 * @ingroup BAP_CAPA_SRV_NATIVE_API
 * @brief Description of callback functions for Capabilities Server module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup BAP_CAPA_SRV_FUNCTION Functions
 * @ingroup BAP_CAPA_SRV_NATIVE_API
 * @brief Description of functions for BAP Capabilities Server module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "gaf.h"                // GAF Defines
#include "bap_capa.h"           // Basic Audio Profile - Capabilities Definitions

/*
 * DEFINES
 ****************************************************************************************
 */

/// @addtogroup BAP_CAPA_SRV_DEFINE
/// @{

/// Maximum number of PACs
#define BAP_CAPA_SRV_PAC_NB_MAX         (15)

/// @} BAP_CAPA_SRV_DEFINE

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// @addtogroup BAP_CAPA_SRV_ENUM
/// @{

/// Configuration bit field meaning for Capabilities Server module
enum bap_capa_srv_cfg_bf
{
    /// Indicate if sending of notifications is supported for Sink/Source PACs characteristics\n
    /// Meaningful only if at least one Sink PAC or one Source PAC characteristic instance is supported
    BAP_CAPA_SRV_CFG_PAC_NTF_POS = 0,
    BAP_CAPA_SRV_CFG_PAC_NTF_BIT = CO_BIT(BAP_CAPA_SRV_CFG_PAC_NTF_POS),

    /// Indicate if sending of notifications is supported for Sink/Source Audio Locations characteristic\n
    /// Meaningful only if Sink or Source Audio Locations characteristic is supported
    BAP_CAPA_SRV_CFG_LOC_NTF_POS = 1,
    BAP_CAPA_SRV_CFG_LOC_NTF_BIT = CO_BIT(BAP_CAPA_SRV_CFG_LOC_NTF_POS),

    /// Indicate if sending of notifications is supported for Supported Audio Contexts characteristic
    BAP_CAPA_SRV_CFG_SUPP_CONTEXT_NTF_POS = 2,
    BAP_CAPA_SRV_CFG_SUPP_CONTEXT_NTF_BIT = CO_BIT(BAP_CAPA_SRV_CFG_SUPP_CONTEXT_NTF_POS),

    /// Indicate if Sink/Source Audio Locations characteristics are writable\n
    /// If these characteristic are writable, sending of notifications will also be supported
    BAP_CAPA_SRV_CFG_LOC_WR_POS = 3,
    BAP_CAPA_SRV_CFG_LOC_WR_BIT = CO_BIT(BAP_CAPA_SRV_CFG_LOC_WR_POS),

    /// Indicate if it is required (= 1) to check the Lock state\n
    /// Meaningful only if Coordinated Set Identification Profile Set Member Role is supported
    BAP_CAPA_SRV_CFG_CHECK_LOCK_POS = 5,
    BAP_CAPA_SRV_CFG_CHECK_LOCK_BIT = CO_BIT(BAP_CAPA_SRV_CFG_CHECK_LOCK_POS),
};

/// @} BAP_CAPA_SRV_ENUM

/*
 * CALLBACK FUNCTIONS DEFINITION
 ****************************************************************************************
 */

/// @addtogroup BAP_CAPA_SRV_CALLBACK
/// @{

/**
 ****************************************************************************************
 * @brief Callback function called when client configuration for Published Audio Capabilities Service has been
 * updated
 *
 * @param[in] con_lid           Connection local index
 * @param[in] cli_cfg_bf        Client configuration bit field
 * @param[in] pac_cli_cfg_bf    Client configuration bit field for Sink/Source PAC characteristic instances
 ****************************************************************************************
 */
typedef void (*bap_capa_srv_cb_bond_data)(uint8_t con_lid, uint8_t cli_cfg_bf, uint16_t pac_cli_cfg_bf);

/**
 ****************************************************************************************
 * @brief Callback function called when either Sink Audio Locations or Source Audio Locations characteristic has been
 * written by a client device.\n
 * Upper layer is expected to reply with call of #bap_capa_srv_location_cfm function
 *
 * @param[in] con_lid           Connection local index
 * @param[in] token             Token that must be provided back in #bap_capa_srv_location_cfm function
 * @param[in] direction         Direction (see #gaf_direction enumeration)
 * @param[in] location_bf       Location bit field (see #gaf_loc_bf enumeration)
 ****************************************************************************************
 */
typedef void (*bap_capa_srv_cb_location_req)(uint8_t con_lid, uint16_t token, uint8_t direction, uint32_t location_bf);

/*
 * CALLBACK SET DEFINITION
 ****************************************************************************************
 */

/// Set of callback functions for BAP Capabilities Server module
struct bap_capa_srv_cb
{
    /// Callback function called when client configuration for Published Audio Capabilities Service has been updated
    bap_capa_srv_cb_bond_data cb_bond_data;
    /// Callback function called when either Sink Audio Locations or Source Audio Locations characteristic has been
    /// written by a client device
    bap_capa_srv_cb_location_req cb_location_req;
};

/// @} BAP_CAPA_SRV_CALLBACK

/*
 * API FUNCTIONS DECLARATION
 ****************************************************************************************
 */

#if (GAF_BAP_CAPA_SRV)
/// @addtogroup BAP_CAPA_SRV_FUNCTION
/// @{

/**
 ****************************************************************************************
 * @brief Configure use of BAP Capabilities Server module
 *
 * @param[in] p_cb          Pointer to set to callback functions for communication with upper layer
 * @param[in] p_cfg         Pointer to configuration structure
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t bap_capa_srv_configure(const bap_capa_srv_cb_t* p_cb, bap_capa_srv_cfg_t* p_cfg);

/**
 ****************************************************************************************
 * @brief Add or update a PAC Record\n
 * PAC Record is identified using a Record ID provided by the upper layer. The provided Record ID shall be unique
 * (meaning not used for any other PAC Record whatever the PAC Group it belongs to).\n
 * At least 1 PAC Record should be added for each Sink/Source PAC characteristic
 *
 * @param[in] pac_lid             PAC local index
 * @param[in] record_id           Record identifier
 * @param[in] p_codec_id          Pointer to Codec ID value
 * @param[in] p_capa              Pointer to Codec Capabilities structure\n
 *                                Cannot be NULL\n
 *                                Structure shall be allocated by Upper Layer and maintained until an update of the
 *                                record with a new call of #bap_capa_srv_set_record or a call of
 *                                #bap_capa_srv_remove_record
 * @param[in] p_metadata          Pointer to Codec Capabilities Metadata structure (must remain allocated by Upper
 *                                Layer)\n
 *                                Can be NULL\n
 *                                Structure shall be allocated by Upper Layer and maintained until a new call of
 *                                #bap_capa_srv_set_record or a call of #bap_capa_srv_remove_record
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t bap_capa_srv_set_record(uint8_t pac_lid, uint8_t record_id, const gaf_codec_id_t* p_codec_id,
                                 const bap_capa_t* p_capa, const bap_capa_metadata_t* p_metadata);

/**
 ****************************************************************************************
 * @brief Remove a PAC record\n
 * If the PAC record to remove in the only remaining record for the Sink/Source PAC characteristic it belongs to,
 * a #GAF_ERR_COMMAND_DISALLOWED will be returned. Content of this last report can be updated using
 * #bap_capa_srv_set_record function.
 *
 * @param[in] record_id             Record identifier
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t bap_capa_srv_remove_record(uint8_t record_id);

/**
 ****************************************************************************************
 * @brief Set bonding information related to Published Audio Capabilities Service after connection with a peer device
 *
 * @param[in] con_lid           Connection local index
 * @param[in] cli_cfg_bf        Client configuration bit field\n
 *                              Each bit corresponds to a characteristic in the range [0, BAP_CAPA_CHAR_TYPE_PAC[
 * @param[in] pac_cli_cfg_bf    Client configuration bit field for Sink/Source PAC characteristic instances\n
 *                              Each bit corresponds to an instance of the Sink/Source PAC characteristic
 *                              (Sink placed first)
 * @param[in] evt_bf            Event bit field\n
 *                              Each bit corresponds to a characteristic in the range [0, BAP_CAPA_CHAR_TYPE_PAC[
 * @param[in] pac_evt_bf        Event configuration bit field for Sink/Source PAC characteristics\n
 *                              Each bit corresponds to an instance of the Sink/Source PAC characteristic
 *                              (Sink placed first)
 * @param[in] context_bf_sink   Available Audio Contexts for Sink direction (see #bap_context_type_bf enumeration)
 * @param[in] context_bf_src    Available Audio Contexts for Source direction (see #bap_context_type_bf enumeration)
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t bap_capa_srv_restore_bond_data(uint8_t con_lid, uint8_t cli_cfg_bf, uint16_t pac_cli_cfg_bf, uint8_t evt_bf,
                                        uint16_t pac_evt_bf, uint16_t context_bf_sink, uint16_t context_bf_src);

/**
 ****************************************************************************************
 * @brief Update value of either Available Audio Contexts or Supported Audio Contexts characteristic
 *
 * @param[in] context_type      Context type (see #bap_capa_context_type enumeration)
 * @param[in] con_lid           Connection local index\n
 *                              Meaningful only for Available Audio Contexts value\n
 *                              Must be set to GAP_INVALID_CONIDX if indicated audio contexts are available for all
 *                              connections
 * @param[in] context_bf_sink   Context bit field for Sink direction (see #bap_context_type_bf enumeration)
 * @param[in] context_bf_src    Context bit field for Source direction (see #bap_context_type_bf enumeration)
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t bap_capa_srv_set_context(uint8_t context_type, uint8_t con_lid, uint16_t context_bf_sink,
                                  uint16_t context_bf_src);

/**
 ****************************************************************************************
 * @brief Update value of either Sink Audio Locations or Source Audio Locations characteristic
 *
 * @param[in] direction     Direction (see #gaf_direction enumeration)
 * @param[in] location_bf   Location bit field (see #gaf_loc_bf enumeration)
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t bap_capa_srv_set_location(uint8_t direction, uint32_t location_bf);

/**
 ****************************************************************************************
 * @brief Accept or not new Audio Location value provided in cb_location_req callback.
 *
 * @param[in] status        Status
 * @param[in] token         Token
 * @param[in] con_lid       Connection local index
 * @param[in] direction     Direction (see #gaf_direction enumeration)
 * @param[in] location_bf   Location bit field (see #gaf_loc_bf enumeration)
 ****************************************************************************************
 */
void bap_capa_srv_location_cfm(uint16_t status, uint16_t token, uint8_t con_lid, uint8_t direction,
                               uint32_t location_bf);

/**
 ****************************************************************************************
 * @return Return if BAP Capabilities Server module has been configured
 ****************************************************************************************
 */
bool bap_capa_srv_is_configured(void);

/**
 ****************************************************************************************
 * @return Bit field indicating list of supported contexts for a given direction
 *
 * @param[in] direction     Direction (see #gaf_direction enumeration)
 ****************************************************************************************
 */
uint16_t bap_capa_srv_get_supported_context(uint8_t direction);

#if (!HOST_MSG_API)
/**
 ****************************************************************************************
 * @return Bit field indicating list of available contexts for a given direction and a given connection
 *
 * @param[in] con_lid       Connection local index
 * @param[in] direction     Direction (see #gaf_direction enumeration)
 ****************************************************************************************
 */
uint16_t bap_capa_srv_get_available_context(uint8_t con_lid, uint8_t direction);
#endif //(!HOST_MSG_API)

/// @} BAP_CAPA_SRV_FUNCTION
#endif //(GAF_BAP_CAPA_SRV)

#endif // BAP_CAPA_SRV_H_
