/**
 ****************************************************************************************
 *
 * @file gmap.h
 *
 * @brief Gaming Audio Profile - Definitions
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef GMAP_H_
#define GMAP_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup GMAP Gaming Audio Profile (GMAP)
 * @ingroup GAF
 * @brief Description of Gaming Audio Profile block
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup GMAP_COMMON Common
 * @ingroup GMAP
 * @brief Description of Gaming Audio Profile Common module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup GMAP_COMMON_ENUM Enumerations
 * @ingroup GMAP_COMMON
 * @brief Enumerations for Gaming Audio Profile Common module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup GMAP_COMMON_STRUCT Structures
 * @ingroup GMAP_COMMON
 * @brief Structures for Gaming Audio Profile Common module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup GMAP_COMMON_NATIVE_API Native API
 * @ingroup GMAP_COMMON
 * @brief Description of Native API for Gaming Audio Profile Common module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "gaf.h"             // GAF Definitions

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// @addtogroup GMAP_COMMON_ENUM
/// @{

/// Module type values for Gaming Audio Profile block
enum gmap_module_type
{
    /// Common Module
    GMAP_MODULE_COMMON = 0,
    /// Gaming Audio Service Server Module
    GMAP_MODULE_GMAS = 1,
    /// Gaming Audio Service Client Module
    GMAP_MODULE_GMAC = 2,

    /// Maximum value
    GMAP_MODULE_MAX,
};

/// Configuration bit field meaning (see #gmap_configure function)
enum gmap_cfg_bf
{
    /// Indicate if Server role is supported for Gaming Audio Service
    GMAP_CFG_GMAS_SUPP_POS = 0,
    GMAP_CFG_GMAS_SUPP_BIT = CO_BIT(GMAP_CFG_GMAS_SUPP_POS),

    /// Indicate if Client role is supported for Gaming Audio Service
    GMAP_CFG_GMAC_SUPP_POS = 1,
    GMAP_CFG_GMAC_SUPP_BIT = CO_BIT(GMAP_CFG_GMAC_SUPP_POS),
};

/// Characteristic type values for Gaming Audio Service
enum gmap_char_type
{
    /// GMAP Role characteristic
    GMAP_CHAR_TYPE_ROLE = 0,
    /// UGG Features
    GMAP_CHAR_TYPE_UGG_FEATURES,
    /// UGT Features
    GMAP_CHAR_TYPE_UGT_FEATURES,
    /// BGS Features
    GMAP_CHAR_TYPE_BGS_FEATURES,
    /// BGR Features
    GMAP_CHAR_TYPE_BGR_FEATURES,

    GMAP_CHAR_TYPE_MAX,
};

/// GMAP Role characteristic bit field meaning
enum gmap_role_bf
{
    /// Indicate if Server supports Unicast Game Gateway role (= 1) or not - Position
    GMAP_ROLE_UGG_POS = 0,
    /// Indicate if Server supports Unicast Game Gateway role (= 1) or not - Bit
    GMAP_ROLE_UGG_BIT = CO_BIT(GMAP_ROLE_UGG_POS),

    /// Indicate if Server supports Unicast Game Terminal role (= 1) or not - Position
    GMAP_ROLE_UGT_POS = 1,
    /// Indicate if Server supports Unicast Game Terminal role (= 1) or not - Bit
    GMAP_ROLE_UGT_BIT = CO_BIT(GMAP_ROLE_UGT_POS),

    /// Indicate if Server supports Broadcast Game Sender role (= 1) or not - Position
    GMAP_ROLE_BGS_POS = 2,
    /// Indicate if Server supports Broadcast Game Sender role (= 1) or not - Bit
    GMAP_ROLE_BGS_BIT = CO_BIT(GMAP_ROLE_BGS_POS),

    /// Indicate if Server supports Broadcast Game Receiver role (= 1) or not - Position
    GMAP_ROLE_BGR_POS = 3,
    /// Indicate if Server supports Broadcast Game Receiver role (= 1) or not - Bit
    GMAP_ROLE_BGR_BIT = CO_BIT(GMAP_ROLE_BGR_POS),

    /// Mask indicating valid bits
    GMAP_ROLE_VALID_MASK = GMAP_ROLE_UGG_BIT | GMAP_ROLE_UGT_BIT | GMAP_ROLE_BGS_BIT | GMAP_ROLE_BGR_BIT,
};

/// UGG Features bit field meaning
enum gmap_ugg_features_bf
{
    /// Indicate if Server supports UGG Multiplex feature (= 1) or not - Position
    GMAP_UGG_FEATURES_MULTIPLEX_POS = 0,
    /// Indicate if Server supports UGG Multiplex feature (= 1) or not - Bit\n
    /// Support transmitting multiple LC3 codec frames per block in an SDU
    GMAP_UGG_FEATURES_MULTIPLEX_BIT = CO_BIT(GMAP_UGG_FEATURES_MULTIPLEX_POS),

    /// Indicate if Server supports UGG 96 kbps Source feature (= 1) or not - Position
    GMAP_UGG_FEATURES_96KBPS_SOURCE_POS = 1,
    /// Indicate if Server supports UGG 96 kbps Source feature (= 1) or not - Bit\n
    /// Support as a source of BAP Codec Configuration Settings 48_3 and 48_4
    GMAP_UGG_FEATURES_96KBPS_SOURCE_BIT = CO_BIT(GMAP_UGG_FEATURES_96KBPS_SOURCE_POS),

    /// Indicate if Server supports UGG Multisink feature (= 1) or not - Position
    GMAP_UGG_FEATURES_MULTISINK_POS = 2,
    /// Indicate if Server supports UGG Multisink feature (= 1) or not - Bit\n
    /// Support for receiving at least two channels of audio, each in a separate CIS
    GMAP_UGG_FEATURES_MULTISINK_BIT = CO_BIT(GMAP_UGG_FEATURES_MULTISINK_POS),

    /// Mask indicating valid bits
    GMAP_UGG_FEATURES_VALID_MASK = GMAP_UGG_FEATURES_MULTIPLEX_BIT | GMAP_UGG_FEATURES_96KBPS_SOURCE_BIT
                                   | GMAP_UGG_FEATURES_MULTISINK_BIT,
};

/// UGT Features bit field meaning
enum gmap_ugt_features_bf
{
    /// Indicate if Server supports UGT Source feature (= 1) or not - Position
    GMAP_UGT_FEATURES_SOURCE_POS = 0,
    /// Indicate if Server supports UGT Source feature (= 1) or not - Bit\n
    /// Transmit Unicast Audio Streams as an Acceptor\n
    /// Mandatory to support at least one of the UGT Source feature or the UGT Sink feature
    GMAP_UGT_FEATURES_SOURCE_BIT = CO_BIT(GMAP_UGT_FEATURES_SOURCE_POS),

    /// Indicate if Server supports UGT 80 kbps Source feature (= 1) or not - Position
    GMAP_UGT_FEATURES_80KBPS_SOURCE_POS = 1,
    /// Indicate if Server supports UGT 80 kbps Source feature (= 1) or not - Bit\n
    /// Transmit Unicast Audio Streams as an Acceptor with BAP Codec Capability settings 48_1 and 48_2\n
    /// Support is excluded if UGT Source feature not supported
    GMAP_UGT_FEATURES_80KBPS_SOURCE_BIT = CO_BIT(GMAP_UGT_FEATURES_80KBPS_SOURCE_POS),

    /// Indicate if Server supports UGT Sink feature (= 1) or not - Position
    GMAP_UGT_FEATURES_SINK_POS = 2,
    /// Indicate if Server supports UGT Sink feature (= 1) or not - Bit\n
    /// Receive Unicast Audio Streams as an Acceptor\n
    /// Mandatory to support at least one of the UGT Source feature or the UGT Sink feature
    GMAP_UGT_FEATURES_SINK_BIT = CO_BIT(GMAP_UGT_FEATURES_SINK_POS),

    /// Indicate if Server supports UGT 64 kbps Sink feature (= 1) or not - Position
    GMAP_UGT_FEATURES_64KBPS_SINK_POS = 3,
    /// Indicate if Server supports UGT 64 kbps Sink feature (= 1) or not - Bit\n
    /// Receive Unicast Audio Streams as an Acceptor with BAP Codec Capability settings 32_1 and 32_2\n
    /// Support is excluded if UGT Sink feature not supported
    GMAP_UGT_FEATURES_64KBPS_SINK_BIT = CO_BIT(GMAP_UGT_FEATURES_64KBPS_SINK_POS),

    /// Indicate if Server supports UGT Multiplex feature (= 1) or not - Position
    GMAP_UGT_FEATURES_MULTIPLEX_POS = 4,
    /// Indicate if Server supports UGT Multiplex feature (= 1) or not - Bit\n
    /// Support for receiving multiple LC3 codec frames per block in an SDU\n
    /// Support is mandatory if BGR role is supported AND both BRG Multiplex and UGT Sink features are supported,
    /// otherwise optional if UGT Sink feature is supported else excluded
    GMAP_UGT_FEATURES_MULTIPLEX_BIT = CO_BIT(GMAP_UGT_FEATURES_MULTIPLEX_POS),

    /// Indicate if Server supports UGT Multisink feature (= 1) or not - Position
    GMAP_UGT_FEATURES_MULTISINK_POS = 5,
    /// Indicate if Server supports UGT Multisink feature (= 1) or not - Bit\n
    /// Support for receiving at least two audio channels, each in a separate CIS\n
    /// Support is excluded if UGT Sink feature not supported
    GMAP_UGT_FEATURES_MULTISINK_BIT = CO_BIT(GMAP_UGT_FEATURES_MULTISINK_POS),

    /// Indicate if Server supports UGT Multisource feature (= 1) or not - Position
    GMAP_UGT_FEATURES_MULTISOURCE_POS = 6,
    /// Indicate if Server supports UGT Multisource feature (= 1) or not - Bit\n
    /// Support for sending at least two audio channels, each in a separate CIS\n
    /// Support is excluded if UGT Source feature not supported
    GMAP_UGT_FEATURES_MULTISOURCE_BIT = CO_BIT(GMAP_UGT_FEATURES_MULTISOURCE_POS),

    /// Mask indicating valid bits
    GMAP_UGT_FEATURES_VALID_MASK = GMAP_UGT_FEATURES_SOURCE_BIT | GMAP_UGT_FEATURES_80KBPS_SOURCE_BIT
                                   | GMAP_UGT_FEATURES_SINK_BIT | GMAP_UGT_FEATURES_64KBPS_SINK_BIT
                                   | GMAP_UGT_FEATURES_MULTIPLEX_BIT | GMAP_UGT_FEATURES_MULTISINK_BIT
                                   | GMAP_UGT_FEATURES_MULTISOURCE_BIT,
};

/// BGS Features bit field meaning
enum gmap_bgs_features_bf
{
    /// Indicate if Server supports BGS 96 kbps feature (= 1) or not - Position
    GMAP_BGS_FEATURES_96KBPS_POS = 0,
    /// Indicate if Server supports BGS 96 kbps feature (= 1) or not - Bit/n
    /// Support of BAP Codec Configuration Settings 48_3 and 48_4
    GMAP_BGS_FEATURES_96KBPS_BIT = CO_BIT(GMAP_BGS_FEATURES_96KBPS_POS),

    /// Mask indicating valid bits
    GMAP_BGS_FEATURES_VALID_MASK = GMAP_BGS_FEATURES_96KBPS_BIT,
};

/// BGR Features bit field meaning
enum gmap_bgr_features_bf
{
    /// Indicate if Server supports BGR Multisink feature (= 1) or not - Position
    GMAP_BGR_FEATURES_MULTISINK_POS = 0,
    /// Indicate if Server supports BGR Multisink feature (= 1) or not - Bit\n
    /// Support for receiving at least two audio channels, each in a separate BIS
    GMAP_BGR_FEATURES_MULTISINK_BIT = CO_BIT(GMAP_BGR_FEATURES_MULTISINK_POS),

    /// Indicate if Server supports BGR Multiplex feature (= 1) or not - Position
    GMAP_BGR_FEATURES_MULTIPLEX_POS = 1,
    /// Indicate if Server supports BGR Multiplex feature (= 1) or not - Bit\n
    /// Support for receiving multiple LC3 codec frames per block in an SDU\n
    /// Support is mandatory if UGT Role is supported and UGT Multiplex feature is supported
    GMAP_BGR_FEATURES_MULTIPLEX_BIT = CO_BIT(GMAP_BGR_FEATURES_MULTIPLEX_POS),

    /// Mask indicating valid bits
    GMAP_BGR_FEATURES_VALID_MASK = GMAP_BGR_FEATURES_MULTISINK_BIT | GMAP_BGR_FEATURES_MULTIPLEX_BIT,
};

/// @} GMAP_COMMON_ENUM

/*
 * TYPE DEFINTIONS
 ****************************************************************************************
 */

/// @addtogroup GMAP_COMMON_STRUCT
/// @{

typedef struct gmap_gmac_cb gmap_gmac_cb_t;

/// Configuration Parameter structure for Gaming Audio Service Server
typedef struct gmap_gmas_cfg_param
{
    /// Bit field indicating supported GMAP roles (see #gmap_role_bf enumeration)
    uint8_t role_bf;
    /// UGG Features bit field meaning (see #gmap_ugg_features_bf enumeration)\n
    /// Meaningful only if UGT role is supported
    uint8_t ugg_features_bf;
    /// UGT Features bit field meaning (see #gmap_ugt_features_bf enumeration)\n
    /// Meaningful only if UGT role is supported
    uint8_t ugt_features_bf;
    /// BGS Features bit field meaning (see #gmap_bgs_features_bf enumeration)\n
    /// Meaningful only if UGT role is supported
    uint8_t bgs_features_bf;
    /// BGR Features bit field meaning (see #gmap_bgr_features_bf enumeration)\n
    /// Meaningful only if UGT role is supported
    uint8_t bgr_features_bf;
    /// Required start handle\n
    /// If set to GATT_INVALID_HANDLE, the start handle will be automatically chosen
    uint16_t shdl;
} gmap_gmas_cfg_param_t;

/// @} GMAP_COMMON_STRUCT

/*
 * EXTERNAL FUNCTIONS DECLARATION
 ****************************************************************************************
 */

#if (GAF_GMAP)
/// @addtogroup GMAP_COMMON_NATIVE_API
/// @{

/**
 ****************************************************************************************
 * @brief Configure Gaming Audio Profile block
 *
 * @param[in] cfg_bf            Configuration bit field (see #gmap_cfg_bf enumeration)
 * @param[in] p_cfg_param_gmas  Pointer to Configuration Parameters for Gaming Audio Service Server\n
 *                              Cannot be NULL if support of Server Role for Gaming Audio Service is
 *                              indicated as supported in cfg_bf
 * @param[in] p_cb_gmac         Pointer to set of callback functions for Gaming Audio Service Client\n
 *                              Cannot be NULL if support of Client Role for Gaming Audio Service is
 *                              indicated as supported in cfg_bf
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t gmap_configure(uint8_t cfg_bf, const gmap_gmas_cfg_param_t* p_cfg_param_gmas,
                        const gmap_gmac_cb_t* p_cb_gmac);

/// @} GMAP_COMMON_NATIVE_API

#endif //(GAF_GMAP)

#endif // GMAP_H_

