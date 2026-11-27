/**
 ****************************************************************************************
 *
 * @file gapi_audio_sync.h
 *
 * @brief GAP Isochronous Audio Synchronization - Header file
 *
 * Copyright (C) RivieraWaves 2019-2022
 *
 ****************************************************************************************
 */

#ifndef GAPI_AUDIO_SYNC_H_
#define GAPI_AUDIO_SYNC_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @addtogroup AUDIO_SYNC
 * @{
 ****************************************************************************************
 */

/*
 * DEFINES
 ****************************************************************************************
 */

/// @addtogroup GAPI_AUDIO_SYNC_DEFINE
/// @{

/// Module type values
#define GAPI_MODULE_AUSY         (6)

/// @} GAPI_AUDIO_SYNC_DEFINE

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"    // SW configuration
#if (AUDIO_SYNC_SUPPORT && BLE_BIS)
#include <stdint.h>         // integer definition
#include "audio_sync.h"     // Audio Sync API
#include "ke_msg.h"         // Kernel message definitions

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// Structure for #GAPI_AUSY_CONFIG_BIS command message
typedef struct gapi_ausy_config_bis_cmd
{
    /// Command code (shall be set to #GAPI_TM_STOP)
    uint16_t cmd_code;
    /// Enable or Disable reception of BIS establishment parameters (1: enable,  0: disable)
    uint8_t  enable;
} gapi_ausy_config_bis_cmd_t;

/// Structure for gapi BIS establishment parameters used in GAPI_AUSY_ESTAB_BIS
typedef struct gapi_bis_estab_param
{
    /// Used to identify a Connected Isochronous Stream. (Range 0x00-0xEF)
    uint8_t  bis_id;
    /// Stream Local Identifier
    uint8_t  stream_lid;
    /// Expected BIS start time instant audio clock value (0.613us step)
    uint32_t exp_bis_start_time;
} gapi_bis_estab_param_t;

/// Structure for #GAPI_AUSY_ESTAB_BIS indication message
typedef struct gapi_ausy_estab_bis_ind
{
    /// Indication code (shall be set to #GAPI_AUSY_ESTAB_BIS)
    uint16_t ind_code;
    /// Used to identify the group local identifier. (Range 0x00-0xEF)
    uint8_t  group_lid;
    /// Number of subevent
    uint8_t  nse;
    /// ISO Interval in microseconds
    uint32_t iso_interval;
    /// Subevent interval in microseconds
    uint32_t sub_interval;
    /// Rate used (0: 1Mbps | 1: 2 Mbps | 2: 125 Kbps | 3: 500 Kbps)
    uint8_t  rate;
    ///  Number from which the Access Addresses for all Broadcast
    uint32_t seed_access_addr;
    /// Number of BIS transmitted. (range 1 to 31)
    uint8_t  num_bis;
    /// BIS establishment parameters
    gapi_bis_estab_param_t bis_estab_param[__ARRAY_EMPTY];
} gapi_ausy_estab_bis_ind_t;

/// This event provide BIS parameters when all BIS negotiations succeeds (associated to the same BIG).
/*@TRACE*/
typedef struct gapi_ausy_estab_bis_evt
{
    /// Used to identify the Connected Isochronous Group. (Range 0x00-0xEF)
    uint8_t  big_hdl;
    /// Number of subevent
    uint8_t  nse;
    /// ISO Interval in microseconds
    uint32_t iso_interval;
    /// Subevent interval in microseconds
    uint32_t sub_interval;
    /// Rate used (0: 1Mbps | 1: 2 Mbps | 2: 125 Kbps | 3: 500 Kbps)
    uint8_t  rate;
    ///  Number from which the Access Addresses for all Broadcast
    uint32_t seed_access_addr;
    /// Number of BIS transmitted. (range 1 to 31)
    uint8_t  num_bis;
    /// Pointer to BIS establishment parameters
    const struct bis_estab_param* p_bis_estab_param;
} gapi_ausy_estab_bis_evt_t;

/*
 * CALLBACK FUNCTIONS DEFINITION
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Callback executed when new BIS activities are granted during a BIG start
 *
 * @param[in]  metainfo             Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in]  group_lid            Pointer to BIG Sync Info format
 * @param[in]  nse                  Number of subevent
 * @param[in]  iso_interval         ISO Interval in microsecond
 * @param[in]  sub_interval         Subevent interval in microseconds
 * @param[in]  rate                 Rate used (0: 1Mbps | 1: 2 Mbps | 2: 125 Kbps | 3: 500 Kbps)
 * @param[in]  seed_access_addr     Number from which the Access Addresses for all Broadcast
 * @param[in]  num_bis              Number of BIS transmitted. (range 1 to 31)
 * @param[in]  p_estab_bis_param    Pointer to BIS establishment parameters (see struct bis_estab_param_t)
 *
 ****************************************************************************************
 */
typedef void (*gapi_ausy_estab_bis_evt_cb)(uint32_t metainfo, uint8_t group_lid, uint8_t nse, uint32_t iso_interval,
                                            uint32_t sub_interval, uint8_t rate, uint32_t seed_access_addr,
                                            uint8_t num_bis, const gapi_bis_estab_param_t* p_bis_estab_param);

/**
 ****************************************************************************************
 * @brief Callback executed when BIS config event is completed
 *
 * @param[in]  metainfo             Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in]  status               Status of procedure execution
 *
 ****************************************************************************************
 */
typedef void (*gapi_ausy_config_bis_cmp_evt_cb) (uint32_t metainfo, uint8_t status);

#if (HOST_MSG_API)
/*
 * COMMAND HANDLERS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
* @brief Handler for GAPI_CMD(GAPI_AUSY_CONFIG_BIS) message
*
* @param[in] p_cmd             Pointer to command parameters
* @param[in] src_id            Source Index
*
* @return An error status
*****************************************************************************************
 */
uint16_t gapi_msg_ausy_config_bis_cmd_handler(gapi_ausy_config_bis_cmd_t const* p_cmd, ke_task_id_t const src_id);

#endif // (HOST_MSG_API)
#endif // (AUDIO_SYNC_SUPPORT && BLE_BIS)
/// @} AUDIO_SYNC

#endif // GAPI_AUDIO_SYNC_H_
