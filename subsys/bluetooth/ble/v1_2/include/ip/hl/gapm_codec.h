/**
 ****************************************************************************************
 *
 * @file gapm_codec.h
 *
 * @brief Generic Access Profile Manager - Codec information
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef GAPM_CODEC_H_
#define GAPM_CODEC_H_

#include "rom_build_cfg.h"

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "gapm.h"

/**
 ****************************************************************************************
 * @addtogroup GAPM_CODEC_API Codec
 * @ingroup GAPM_API
 * @{
 ****************************************************************************************
 */

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Version of Read Local Supported Codecs command accepted by controller
enum gapm_codec_get_version
{
    /// V1 - BT only
    GAPM_CODEC_GET_VERSION_V1 = 0u,
    /// V2
    GAPM_CODEC_GET_VERSION_V2,
};

/*
 * INTERFACES
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Callback function called once #gapm_codec_get operation has been completed
 *
 * @param[in] metainfo         Metadata information
 * @param[in] status           Status (see #hl_err enumeration)
 * @param[in] version          Version of Read Local Supported Codecs command accepted by controller (see
 *                             #gapm_codec_get_version enumeration)
 * @param[in] p_codec_list     Pointer to codec list byte-string.\n
 *
 * Has following structure for version = #GAPM_CODEC_GET_VERSION_V1:\n
 *     uint8_t nb_supp_std_codecs\n
 *     uint8_t std_codec_id[nb_supp_std_codecs]\n
 *     uint8_t nb_supp_vs_codecs\n
 *     uint8_t vs_codec_id[4][nb_supp_vs_codecs]\n
 * Has following structure for version = #GAPM_CODEC_GET_VERSION_V2:\n
 *     uint8_t nb_supp_std_codecs\n
 *     uint8_t std_codec_id[nb_supp_std_codecs]\n
 *     uint8_t std_codec_transport_id[nb_supp_std_codecs]\n
 *     uint8_t nb_supp_vs_codecs\n
 *     uint8_t vs_codec_id[4][nb_supp_vs_codecs]\n
 *     uint8_t vs_codec_transport_id[nb_supp_vs_codecs]
 ****************************************************************************************
 */
typedef void (*gapm_codec_cb_cmp_evt_get)(uint32_t metainfo, uint16_t status, uint8_t version,
                                          const uint8_t* p_codec_list);

/**
 ****************************************************************************************
 * @brief Callback function called once #gapm_codec_get_capabilities operation has been completed
 *
 * @param[in] metainfo         Metadata information
 * @param[in] status           Status (see #hl_err enumeration)
 * @param[in] nb_codec_capa    Number of codec capabilities
 * @param[in] p_codec_capa     Pointer to codec capabilities byte-string.\n
 *
 * Has following structure:\n
 *     uint8_t codec_capa_len[i]\n
 *     uint8_t codec_capa[i][codec_capa_len[i]]\n
 * with i in range [0, nb_codec_capa]
 ****************************************************************************************
 */
typedef void (*gapm_codec_cb_cmp_evt_capabilities)(uint32_t metainfo, uint16_t status, uint8_t nb_codec_capa,
                                                   const uint8_t* p_codec_capa);

/**
 ****************************************************************************************
 * @brief Callback function called once #gapm_codec_get_delay operation has been completed
 *
 * @param[in] metainfo         Metadata information
 * @param[in] status           Status (see #hl_err enumeration)
 * @param[in] delay_min_us     Minimal delay in microseconds
 * @param[in] delay_max_us     Maximal delay in microseconds
 ****************************************************************************************
 */
typedef void (*gapm_codec_cb_cmp_evt_delay)(uint32_t metainfo, uint16_t status, uint32_t delay_min_us,
                                            uint32_t delay_max_us);

/*
 * FUNCTIONS DECLARATION
 ****************************************************************************************
 */

#if (HL_CODEC_INFO)
/**
 ****************************************************************************************
 * @brief Get list of codecs supported by the controller
 *
 * @param[in] metainfo         Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] cb_cmp_evt       Callback function called once operation has been completed
 *
 * @return Execution status (see #hl_err enumeration).
 ****************************************************************************************
 */
uint16_t gapm_codec_get(uint32_t metainfo, gapm_codec_cb_cmp_evt_get cb_cmp_evt);

/**
 ****************************************************************************************
 * @brief Get list of codec capabilities supported by the controller for a given codec
 *
 * @param[in] metainfo         Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] p_codec_id       Pointer to codec ID value.\n Shall not be NULL
 * @param[in] transport_type   Transport type (see #transport_id enumeration)
 * @param[in] direction        Direction (see #data_path_direction enumeration)
 * @param[in] cb_cmp_evt       Callback function called once operation has been completed
 *
 * @return Execution status (see #hl_err enumeration).
 ****************************************************************************************
 */
uint16_t gapm_codec_get_capabilities(uint32_t metainfo, const uint8_t* p_codec_id, uint8_t transport_type,
                                     uint8_t direction, gapm_codec_cb_cmp_evt_capabilities cb_cmp_evt);

/**
 ****************************************************************************************
 * @brief Get range of supported controller delays for a specified configuration of a given codec
 *
 * @param[in] metainfo         Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] p_codec_id       Pointer to codec ID value.\n Shall not be NULL
 * @param[in] transport_type   Transport type (see #transport_id enumeration)
 * @param[in] direction        Direction (see #data_path_direction enumeration)
 * @param[in] codec_cfg_len    Length of codec configuration.\n Shall not be 0
 * @param[in] p_codec_cfg      Pointer to codec configuration.\n Shall not be NULL
 * @param[in] cb_cmp_evt       Callback function called once operation has been completed
 *
 * @return Execution status (see #hl_err enumeration).
 ****************************************************************************************
 */
uint16_t gapm_codec_get_delay(uint32_t metainfo, const uint8_t* p_codec_id, uint8_t transport_type,
                              uint8_t direction, uint8_t codec_cfg_len, const uint8_t* p_codec_cfg,
                              gapm_codec_cb_cmp_evt_delay cb_cmp_evt);
#endif // (HL_CODEC_INFO)

/// @} GAPM_CODEC_API

#endif // GAPM_CODEC_H_
