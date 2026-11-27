/**
 ****************************************************************************************
 *
 * @file rwble_hl_config.h
 *
 * @brief Configuration of the BLE protocol stack (max number of supported connections,
 * type of partitioning, etc.)
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 *
 ****************************************************************************************
 */

#ifndef RWHL_CONFIG_H_
#define RWHL_CONFIG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @addtogroup ROOT
 * @{
 * @name BLE stack configuration
 * @{
 ****************************************************************************************
 */

#include "hl_error.h"
#include "co_list.h"

// Redefine Roles to HL defines
#if(BLE_HOST_PRESENT)
#define HL_LE_OBSERVER      (BLE_OBSERVER)
#define HL_LE_BROADCASTER   (BLE_BROADCASTER)
#define HL_LE_CENTRAL       (BLE_CENTRAL)
#define HL_LE_PERIPHERAL    (BLE_PERIPHERAL)
#else
#define HL_LE_OBSERVER      (0)
#define HL_LE_BROADCASTER   (0)
#define HL_LE_CENTRAL       (0)
#define HL_LE_PERIPHERAL    (0)
#endif // (BLE_HOST_PRESENT)

/*
 * DEFINES
 ****************************************************************************************
 */

/// Maximum time to remain advertising when in the Limited
/// Discover able mode: TGAP(lim_adv_timeout)
/// required value: 180s: (18000 in 10 ms step)
#define GAP_TMR_LIM_ADV_TIMEOUT                             (18000)

/// Minimum time to perform scanning when performing
/// the General Discovery procedure on 1M PHY: TGAP(gen_disc_scan_min)
/// recommended value: 10.24s: (1024 in 10 ms step)
#define GAP_TMR_GEN_DISC_SCAN_1M                            (1024)

/// Minimum time to perform scanning when performing
/// the General Discovery procedure on Coded PHY: TGAP(gen_disc_scan_min_coded)
/// recommended value: 30.72s: (3072 in 10 ms step)
#define GAP_TMR_GEN_DISC_SCAN_CODED                         (3072)

/// Minimum time to perform scanning when performing the
/// Limited Discovery procedure on 1M PHY: TGAP(lim_disc_scan_min)
/// recommended value: 10.24s: (1024 in 10 ms step)
#define GAP_TMR_LIM_DISC_SCAN_1M                            (1024)

/// Minimum time to perform scanning when performing the
/// Limited Discovery procedure on Coded PHY: TGAP(lim_disc_scan_min_coded)
/// recommended value: 30.72s: (3072 in 10 ms step)
#define GAP_TMR_LIM_DISC_SCAN_CODED                         (3072)

/// TGAP(private_addr_int)
/// recommended value: 15 minutes

/// Minimum time interval between private address change
/// Minimum value 1s
#define GAP_TMR_PRIV_ADDR_MIN                             (0x0001)
/// Maximum time interval between private address change
/// 0xA1B8 (approximately 11.5 hours)
#define GAP_TMR_PRIV_ADDR_MAX                             (0xA1B8)

/// L2CAP Signaling transaction Timer duration in milliseconds
/// TGAP(conn_param_timeout) recommended value: 30 s: (30 000 ms)
#define GAP_SIG_TRANS_TIMEOUT_MS                          (30000)

/// SMP L2CAP transaction Timer duration in milliseconds  30 s: (30 000 ms)
#define GAP_SMP_TRANS_TIMEOUT_MS                          (30000)

/// Maximal authorized MTU / MPS value - Depends on memory size available
#ifndef CFG_MAX_LE_MTU
#define GAP_LE_MTU_MAX                                     (2048)
#define GAP_LE_MPS_MAX                                     (2048)
#else
#define GAP_LE_MTU_MAX                                     (CFG_MAX_LE_MTU)
#define GAP_LE_MPS_MAX                                     (CFG_MAX_LE_MTU)
#endif // (CFG_MAX_LE_MTU)

/// Maximum GAP device name size
#define GAP_MAX_NAME_SIZE                                  (0x20)

/// 30 seconds transaction timer (30000 ms)
#define GATT_TRANSACTION_TIMEOUT                           (30000)

/// Maximum attribute value length
#ifndef CFG_ATT_VAL_MAX
#define GATT_MAX_VALUE                                     (GAP_LE_MTU_MAX)
#else
#define GATT_MAX_VALUE                                     (CFG_ATT_VAL_MAX)
#endif // CFG_ATT_VAL_MAX

/// Maximum number of services returned by callback
#define GATT_DISCOVER_SVC_ATT_MAX                          (20)

/// Number of L2CAP COC channel that can be created per connection
#ifndef CFG_L2CAP_COC_CHAN_PER_CON_NB
#define L2CAP_COC_CHAN_PER_CON_NB                                  (10)
#else
#define L2CAP_COC_CHAN_PER_CON_NB                                  (CFG_L2CAP_COC_CHAN_PER_CON_NB)
#endif // CFG_L2CAP_COC_CHAN_PER_CON_NB

/// Total Number of L2CAP channel and GATT bearer that can be allocated in environment heap
#ifndef CFG_L2CAP_CHAN_IN_ENV_NB
#define L2CAP_CHAN_IN_ENV_NB                                   (10)
#else
#define L2CAP_CHAN_IN_ENV_NB                                   (CFG_L2CAP_CHAN_IN_ENV_NB)
#endif // CFG_L2CAP_CHAN_IN_ENV_NB

/// Memory limit of prepare write queue
#define GATT_PREP_WRITE_QUEUE_MEM_LIMIT                    (GATT_MAX_VALUE)
/// Memory limit of GATT operation (to consider that there insufficient memory to perform procedure)
#define GATT_MEM_LIMIT                                     (CO_BUF_BIG_POOL_SIZE + (CO_BUF_SMALL_POOL_SIZE >> 1))


/******************************************************************************************/
/* -------------------------   TEST MODE             -------------------------------------*/
/******************************************************************************************/

#if defined(CFG_HOST_TEST_MODE)
#define HOST_TEST_MODE           (1)
#else
#define HOST_TEST_MODE           (0)
#endif // defined(CFG_HOST_TEST_MODE)

/******************************************************************************************/
/* -------------------------   BLE PARTITIONING      -------------------------------------*/
/******************************************************************************************/

/******************************************************************************************/
/* --------------------------   INTERFACES        ----------------------------------------*/
/******************************************************************************************/

#ifndef APP_MAIN_TASK
#if APP_PRESENT
#define APP_MAIN_TASK       TASK_APP
#elif AHI_TL_SUPPORT
#define APP_MAIN_TASK       TASK_AHI
#else
#define APP_MAIN_TASK       TASK_NONE
#endif // APP_PRESENT
#endif

// Host Controller Interface (Host side)
#define HOST_HCIH_ITF            HCIH_ITF

/******************************************************************************************/
/* --------------------------   COEX SETUP        ----------------------------------------*/
/******************************************************************************************/

///WLAN coex
#define HOST_WLAN_COEX          RW_WLAN_COEX
///WLAN test mode
#define HOST_WLAN_COEX_TEST     RW_WLAN_COEX_TEST

/******************************************************************************************/
/* --------------------------        GAF          ----------------------------------------*/
/******************************************************************************************/
#if BLE_GAF_PRESENT
#define BLE_GAF           (BLE_HOST_PRESENT && BLE_ISO_PRESENT)
#else //(CFG_GAF)
#define BLE_GAF           (0)
#endif //(CFG_GAF)

/******************************************************************************************/
/* --------------------------   PROFILES          ----------------------------------------*/
/******************************************************************************************/
#ifdef CFG_PRF
#define HOST_PROFILES      (1)
/// Number of Profile tasks managed by GAP manager.
#define HOST_NB_PROFILES   (CFG_NB_PRF)
#else
#define HOST_PROFILES      (0)
#define HOST_NB_PROFILES   (0)
#endif // CFG_PRF
#include "rwprf_config.h"

/******************************************************************************************/
/* --------------------------   HOST MODULES      ----------------------------------------*/
/******************************************************************************************/

#if (HL_LE_CENTRAL)
// must be equals to sizeof(gapm_init_actv_t)
#define GAPM_HEAP_ENV_SIZE      (100 + KE_HEAP_MEM_RESERVED)
#elif (HL_LE_OBSERVER)
// must be equals to sizeof(gapm_scan_actv_t)
#define GAPM_HEAP_ENV_SIZE      (44 + KE_HEAP_MEM_RESERVED)
#else // (HL_LE_BROADCASTER || HL_LE_PERIPHERAL)
// must be equals to sizeof(gapm_actv_adv_t)
#define GAPM_HEAP_ENV_SIZE      (32 + KE_HEAP_MEM_RESERVED)
#endif

#if (HL_LE_CENTRAL || HL_LE_PERIPHERAL || (0))
#define GAPC_PRESENT                (1)
#else
#define GAPC_PRESENT                (0)
#endif // (HL_LE_CENTRAL || HL_LE_PERIPHERAL || (0))

#if (BLE_HOST_PRESENT && (HL_LE_CENTRAL || HL_LE_PERIPHERAL))
#define BLE_GAPC                    (1)
#else // (BLE_HOST_PRESENT && (HL_LE_CENTRAL || HL_LE_PERIPHERAL))
#define BLE_GAPC                    (0)
#endif // (BLE_HOST_PRESENT && (HL_LE_CENTRAL || HL_LE_PERIPHERAL))

#if (BLE_HOST_PRESENT && BLE_ISO_PRESENT)
#define BLE_GAPI                    (1)
#if (defined(CFG_GAPI_TEST))
#define GAPI_TEST                   (1)
#else // (defined(CFG_GAPI_TEST))
#define GAPI_TEST                   (0)
#endif // (defined(CFG_GAPI_TEST))
#else //(BLE_HOST_PRESENT && BLE_ISO_PRESENT)
#define BLE_GAPI                    (0)
#define GAPI_TEST                   (0)
#endif //(BLE_HOST_PRESENT && BLE_ISO_PRESENT)

#if (GAPC_PRESENT)
#if (BLE_GAPC)
// must be equals to sizeof(gapc_le_con_t) - handle variable size of co_list
#define GAPC_LE_HEAP_ENV_SIZE       (sizeof(co_list_t) + 104 + KE_HEAP_MEM_RESERVED)
#else
#define GAPC_LE_HEAP_ENV_SIZE       (0)
#endif // (BLE_GAPC)

#define GAPC_BT_HEAP_ENV_SIZE       (0)

#if (BLE_GAPC)
#define GAPC_HEAP_ENV_SIZE          (GAPC_LE_HEAP_ENV_SIZE)
#else  // ((0))
#define GAPC_HEAP_ENV_SIZE          (GAPC_BT_HEAP_ENV_SIZE)
#endif // (BLE_GAPC)

#else // !(GAPC_PRESENT)
#define GAPC_HEAP_ENV_SIZE          (0)
#endif // (GAPC_PRESENT)

#if (BLE_GAPC)
#define BLE_GATT                    1
#if (BLE_GAF)
#define BLE_GATT_USER_NB            (10 + BLE_GATT + (BLE_GATT_CLI || BLE_GATT_MTU_NEGO)) // 2 users reserved for GATT and GAP services / clients
#define BLE_GATT_BEARER_PER_CON     (10 + BLE_GATT)
#else //(BLE_GAF)
#if (HOST_PROFILES)
#define BLE_GATT_USER_NB            (HOST_NB_PROFILES + BLE_GATT + (BLE_GATT_CLI || BLE_GATT_MTU_NEGO)) // 2 users reserved for GATT and GAP services / clients
#if (HOST_NB_PROFILES > L2CAP_COC_CHAN_PER_CON_NB)
#define BLE_GATT_BEARER_PER_CON     (L2CAP_COC_CHAN_PER_CON_NB + BLE_GATT)
#else // (HOST_NB_PROFILES <= L2CAP_COC_CHAN_PER_CON_NB)
#define BLE_GATT_BEARER_PER_CON     (HOST_NB_PROFILES + BLE_GATT)
#endif // (HOST_NB_PROFILES > L2CAP_COC_CHAN_PER_CON_NB)
#else // (HOST_PROFILES)
#define BLE_GATT_USER_NB            ((BLE_GATT * 2) + ((BLE_GATT_CLI || BLE_GATT_MTU_NEGO) * 2)) // 1 client + service for GATT and GAP + 1 client + 1 service for upper layer application
#define BLE_GATT_BEARER_PER_CON     ((BLE_GATT * 2) + BLE_GATT_CLI)       // 1 Legacy bearer + 2 Enhanced Bearer: for server and one client for upper layer application
#endif // (HOST_PROFILES)
#endif //(BLE_GAF)

// Must be equals to (sizeof(gatt_con_env_t) and consider 4 * BLE_GATT_BEARER_PER_CON)
#define BLE_GATT_CON_ENV_SIZE       ((sizeof(co_list_t) * 3) + 24 + (4 * BLE_GATT_BEARER_PER_CON))
#define BLE_GATT_BEARER_ENV_SIZE    (sizeof(co_list_t) + 8 + KE_HEAP_MEM_RESERVED)
#define BLE_GATT_HEAP_ENV_SIZE      (BLE_GATT_CON_ENV_SIZE + KE_HEAP_MEM_RESERVED)
#else //(BLE_GAPC)
#define BLE_GATT                    0
#define BLE_GATT_CLI                0
#define BLE_GATT_HEAP_ENV_SIZE      0
#define BLE_GATT_BEARER_ENV_SIZE    0
#endif //(BLE_GAPC)

#if (GAPC_PRESENT)
#define L2CAP_PRESENT               (1)
#define BLE_L2CAP                   (BLE_GAPC)
#define L2CAP_CHAN_PER_CON          \
    (GAPC_PRESENT + BLE_GATT + ((L2CAP_COC_CHAN_PER_CON_NB + 1) * BLE_L2CAP) + (0))

// Must be equals to (sizeof(l2cap_con_env_t) and consider 4 * L2CAP_CHAN_PER_CON)
#define L2CAP_CON_ENV_SIZE          (28 + (4 * L2CAP_CHAN_PER_CON) + ((HOST_MSG_API + 3) * sizeof(co_list_t)))
#define L2CAP_CHANNEL_ENV_SIZE      (sizeof(co_list_t) + (HOST_MSG_API * 4) + 28 + KE_HEAP_MEM_RESERVED)
#define L2CAP_HEAP_ENV_SIZE         (L2CAP_CON_ENV_SIZE + KE_HEAP_MEM_RESERVED)
#define L2CAP_NB_RX_BUF_AVAIL       10
#else // !(GAPC_PRESENT)
#define L2CAP_PRESENT               0
#define BLE_L2CAP                   0
#define L2CAP_HEAP_ENV_SIZE         0
#define L2CAP_CHANNEL_ENV_SIZE      0
#endif // (GAPC_PRESENT)

// support of IQ generator in host
#define BLE_HOST_IQ_GEN (BLE_IQ_GEN && BLE_HOST_PRESENT)

/******************************************************************************************/
/* --------------------------   HL MESSAGE API    ----------------------------------------*/
/******************************************************************************************/

/// Support HL Message API
#if defined(CFG_HL_MSG_API)
    #define HOST_MSG_API        1
#else
    #define HOST_MSG_API        0
#endif // defined(CFG_HL_MSG_API)

/******************************************************************************************/
/* --------------------------   GATT              ----------------------------------------*/
/******************************************************************************************/

/// Support GATT Client
#if (defined(CFG_GATT_CLI) && BLE_GAPC)
    #define BLE_GATT_CLI         1
#else // !(defined(CFG_GATT_CLI) && BLE_GAPC)
    #define BLE_GATT_CLI         0
#endif // (defined(CFG_GATT_CLI) && BLE_GAPC)

/// Support GATT MTU Negociation
#if (defined(CFG_GATT_MTU_NEGO) && BLE_GAPC)
    #define BLE_GATT_MTU_NEGO         1
#else
    #define BLE_GATT_MTU_NEGO         (BLE_GATT_CLI)
#endif // (defined(CFG_GATT_MTU_NEGO) && BLE_GAPC)

/// Support GATT Caching
#if (defined(CFG_GATT_CACHING_SUPPORT) && BLE_GAPC)
#define GATT_CACHING_SUPPORT 1
#else
#define GATT_CACHING_SUPPORT 0
#endif // (defined(CFG_GATT_CACHING_SUPPORT) && BLE_GAPC)

/// Support Enhanced ATT
#if (defined(CFG_EATT_SUPPORT) && BLE_GAPC)
#define EATT_SUPPORT         1
#else
#define EATT_SUPPORT         0
#endif // (defined(CFG_EATT_SUPPORT) && BLE_GAPC)

/// Support Encrypted Advertising Data

/// Support of post 5.3 API

/// Support of LE GATT Security Levels characteristic in host

/// Support of Prepare write disabled
#if (defined(CFG_ATT_PREPARE_WRITE_DISABLED))
#define ATT_PREPARE_WRITE_SUPPORT  (0)
#else
#define ATT_PREPARE_WRITE_SUPPORT  (1)
#endif // (defined(CFG_ATT_PREPARE_WRITE_DISABLED))

/// Indicate to application that MTU on Legacy ATT bearer has changed
#if (defined(CFG_GATT_INDICATE_LEGACY_MTU_CHANGED))
#define GATT_INDICATE_LEGACY_MTU_CHANGED  (1)
#else
#define GATT_INDICATE_LEGACY_MTU_CHANGED  (0)
#endif // (defined(CFG_GATT_INDICATE_LEGACY_MTU_CHANGED))

/// Add support of bond data
#if (defined(CFG_HL_BONDABLE))
#define HL_BONDABLE (1)
#else
#define HL_BONDABLE (0)
#endif // (defined(CFG_HL_BONDABLE))

/// Enable checks on API usage
#if (defined(CFG_HL_API_CHECKS))
#define HL_API_CHECKS (1)
#else
#define HL_API_CHECKS (0)
#endif // (defined(CFG_HL_API_CHECKS))

/// Add support of functions allowing to retrieve information about codecs supported by controller
#if (defined(CFG_HL_CODEC_INFO))
#define HL_CODEC_INFO (1)
#else
#define HL_CODEC_INFO (0)
#endif // (defined(CFG_HL_CODEC_INFO))

/// Automatic sending of Service Changed indication (DEPRECATED)
#if (defined(CFG_HL_DEPRECATED_SERVICE_CHANGED))
#define HL_DEPRECATED_SERVICE_CHANGED (1)
#else
#define HL_DEPRECATED_SERVICE_CHANGED (0)
#endif // (defined(CFG_HL_DEPRECATED_SERVICE_CHANGED))

/// Old list content (FAL / RAL / PAL) management API (DEPRECATED)
#if (defined(CFG_HL_DEPRECATED_LIST))
#define HL_DEPRECATED_LIST (1)
#else
#define HL_DEPRECATED_LIST (0)
#endif // (defined(CFG_HL_DEPRECATED_LIST))

/// Automatically add AD Type Flags in advertising data
#if (defined(CFG_HL_ADV_ADD_FLAGS))
#define HL_ADV_ADD_FLAGS (1)
#else
#define HL_ADV_ADD_FLAGS (0)
#endif // (defined(CFG_HL_ADV_ADD_FLAGS))

/******************************************************************************************/
/* --------------------------   GAP               ----------------------------------------*/
/******************************************************************************************/

/// Support of legacy (<5.0) advertising/scanning/connection commands/events

/// Support of LE PAWR broadcaster in host

/// Support of LE PAWR observer in host

/******************************************************************************************/
/* --------------------------        GAPI         ----------------------------------------*/
/******************************************************************************************/

/// Number of interface available for GAPI
#define GAPI_INTF_NB              (CFG_GAPI_INTF_NB)

/// Number of streams that can be created using GAPI
#define GAPI_NB_STREAMS           (BLE_ISO_STREAM_MAX)

/// Unicast mode supported
#if (BLE_CIS && BLE_GAPC)
#define GAPI_UNICAST_SUPP         BLE_GAPI
#else //(BLE_CIS && BLE_GAPC)
#define GAPI_UNICAST_SUPP         (0)
#endif //(BLE_CIS && BLE_GAPC)

/// Broadcast mode supported
#if (BLE_BIS && BLE_PER_ADV && BLE_GAPI)
#define GAPI_BROADCAST_SUPP          (1)
# else
#define GAPI_BROADCAST_SUPP          (0)
#endif // (BLE_BIS && BLE_PER_ADV && BLE_GAPI)

#if (BLE_BIS && BLE_PER_ADV && BLE_OBSERVER && BLE_GAPI)
#define GAPI_BROADCAST_SINK_SUPP     (1)
#else
#define GAPI_BROADCAST_SINK_SUPP     (0)
#endif // (BLE_BIS && BLE_PER_ADV && BLE_OBSERVER && BLE_GAPI)

#if (BLE_BIS && BLE_PER_ADV && BLE_BROADCASTER && BLE_GAPI)
#define GAPI_BROADCAST_SOURCE_SUPP   (1)
#else
#define GAPI_BROADCAST_SOURCE_SUPP   (0)
#endif // (BLE_BIS && BLE_PER_ADV && BLE_BROADCASTER && BLE_GAPI)

/// Size of the heap
#if (HL_LE_CENTRAL || HL_LE_PERIPHERAL)
    #if (BLE_MESH)
    /// Can be tuned based on supported profiles
    #define HOST_HEAP_PROFILE_SIZE                (5108)
    #else //(BLE_MESH)
    #define HOST_HEAP_PROFILE_SIZE                (4096)
    #endif //(BLE_MESH)
    /// Needed "security" heap size for reception of max supported MTU through prepare write procedure
    /// If retention memory is used, this additional heap can be part of the size allocated for the retention memory
    #define HOST_HEAP_DATA_THP_SIZE               (GAP_LE_MTU_MAX)
    /// message size per connection
    #define HOST_HEAP_MSG_SZ_PER_CON              (100)

    #define HOST_HEAP_MSG_SIZE                    (((HOST_HEAP_MSG_SZ_PER_CON * HOST_CONNECTION_MAX) > HOST_HEAP_DATA_THP_SIZE) \
                                                        ? (HOST_HEAP_MSG_SZ_PER_CON * HOST_CONNECTION_MAX) : HOST_HEAP_DATA_THP_SIZE)
#elif (BLE_MESH)
    #define HOST_HEAP_MSG_SIZE                    (2048)
    #define HOST_HEAP_PROFILE_SIZE                (5108)
#else
    #define HOST_HEAP_MSG_SIZE                    (256)
    #define HOST_HEAP_PROFILE_SIZE                (0)
#endif /* #if (HL_LE_CENTRAL || HL_LE_PERIPHERAL) */

/// Number of BLE HL tasks
#define HOST_TASK_SIZE       BLE_HOST_TASK_SIZE + BLE_PRF_TASK_SIZE

/// Size of environment variable needed on BLE Host Stack for one link
#define HOST_HEAP_ENV_SIZE  ( HOST_CONNECTION_MAX * ( GAPC_HEAP_ENV_SIZE       \
                                                    + BLE_GATT_HEAP_ENV_SIZE       \
                                                    + L2CAP_HEAP_ENV_SIZE )    \
                             + HOST_ACTIVITY_MAX * GAPM_HEAP_ENV_SIZE \
                             + L2CAP_CHAN_IN_ENV_NB * (BLE_GATT_BEARER_ENV_SIZE + L2CAP_CHANNEL_ENV_SIZE) + 2048)

/// @} BLE stack configuration
/// @} ROOT

#endif // RWHL_CONFIG_H_
