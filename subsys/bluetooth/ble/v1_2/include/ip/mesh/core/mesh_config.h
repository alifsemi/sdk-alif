/**
 ****************************************************************************************
 *
 * @file mesh_config.h
 *
 * @brief Header file for Mesh Stack Configuration
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef MESH_CONFIG_
#define MESH_CONFIG_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup MESH_CONFIG Configuration
 * @ingroup MESH
 * @brief  Mesh Stack Configuration
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"
#include "rwip_task.h"

/*
 * COMPILATION FLAGS
 ****************************************************************************************
 */

// Renamed in 5.2
#if (!defined(BLE_CONNECTION_MAX))
/// Maximum number of BLE connection
#define BLE_CONNECTION_MAX      (HOST_CONNECTION_MAX)
#endif // (!defined(BLE_CONNECTION_MAX))

/// Flag indicating that Mesh Profile of version 1.1 is enabled

/// Flag indicating that Mesh Debug is enabled
#if (defined(CFG_BLE_MESH_DBG))
#define BLE_MESH_DBG                         (1)
#else
#define BLE_MESH_DBG                         (0)
#endif //(defined(CFG_BLE_MESH_DBG))

/// Flag indicating that Mesh Message API is enabled
#if (defined(CFG_BLE_MESH_MSG_API))
#define BLE_MESH_MSG_API                     (1)
#else
#define BLE_MESH_MSG_API                     (0)
#endif //(defined(CFG_BLE_MESH_MSG_API))

/// Mesh Storage Mode APP
#if (defined(CFG_BLE_MESH_STORAGE_APP))
#define BLE_MESH_STORAGE_APP                 (1)
#else
#define BLE_MESH_STORAGE_APP                 (0)
#endif //(BLE_MESH_STORAGE_APP)

/// Mesh Storage Mode WVT
#if (defined(CFG_BLE_MESH_STORAGE_WVT))
#define BLE_MESH_STORAGE_WVT                 (1)
#else
#define BLE_MESH_STORAGE_WVT                 (0)
#endif //(BLE_MESH_STORAGE_WVT)

/// Mesh Storage Mode NVDS
#if (defined(CFG_BLE_MESH_STORAGE_NVDS))
#define BLE_MESH_STORAGE_NVDS                (1)
#else
#define BLE_MESH_STORAGE_NVDS                (0)
#endif //(CFG_BLE_MESH_STORAGE_NVDS)

/// Support of GATT Proxy feature
#if defined(CFG_BLE_MESH_GATT_PROXY)
#define BLE_MESH_GATT_PROXY                  (1)
#else
#define BLE_MESH_GATT_PROXY                  (0)
#endif //defined(CFG_BLE_MESH_GATT_PROXY)

/// Support of GATT Provisioning feature
#if defined(CFG_BLE_MESH_GATT_PROV)
#define BLE_MESH_GATT_PROV                   (1)
#else
#define BLE_MESH_GATT_PROV                   (0)
#endif //defined(CFG_BLE_MESH_GATT_PROV)

/// Used to know if GATT Bearer is present
#define BLE_MESH_GATT_BEARER                 (BLE_MESH_GATT_PROXY || BLE_MESH_GATT_PROV)

/// Support of Relay feature
#if defined(CFG_BLE_MESH_RELAY)
#define BLE_MESH_RELAY                       (1)
#else
#define BLE_MESH_RELAY                       (0)
#endif //defined(CFG_BLE_MESH_RELAY)

/// Support of Friend feature
#if defined(CFG_BLE_MESH_FRIEND)
#define BLE_MESH_FRIEND                      (1)
#else
#define BLE_MESH_FRIEND                      (0)
#endif //defined(CFG_BLE_MESH_FRIEND)

/// Support of Low Power Node feature
#if defined(CFG_BLE_MESH_LPN)
#define BLE_MESH_LPN                         (1)
#else
#define BLE_MESH_LPN                         (0)
#endif //defined(CFG_BLE_MESH_LPN)

/// Support of Provisioner Role
#if defined(CFG_BLE_MESH_PROVER)
    #define BLE_MESH_PROVER                  (1)
    /// Support of GATT Bearer Client Role
    #define BLE_MESH_GATT_BEARER_CLIENT      (1)
    /// Support of Foundation Client Model
    #define BLE_MESH_FND_CLI                 (1)
    /// Support of GATT Proxy Client Role
    #define BLE_MESH_GATT_PROXY_CLI          (1)

    /// Support of Unprovisioned Role
    #if defined(CFG_BLE_MESH_PROVEE)
        #define BLE_MESH_PROVEE              (1)
        /// Support of GATT Bearer Server Role
        #define BLE_MESH_GATT_BEARER_SERVER  (1)
        /// Support of Configuration Server Model
        #define BLE_MESH_FND_SVR             (1)
        /// Support of GATT Proxy Server Role
        #define BLE_MESH_GATT_PROXY_SVR      (1)
    #else
        #define BLE_MESH_PROVEE              (0)
        #define BLE_MESH_GATT_BEARER_SERVER  (0)
        #define BLE_MESH_FND_SVR             (0)
        #define BLE_MESH_GATT_PROXY_SVR      (0)
    #endif //defined(CFG_BLE_MESH_PROVEE)

    /// Support of Directed Forwarding Configuration feature
#else
    #define BLE_MESH_PROVER                  (0)
    /// Support of GATT Bearer Client Role
    #define BLE_MESH_GATT_BEARER_CLIENT      (0)
    /// Support of Foundation Client Model
    #define BLE_MESH_FND_CLI                 (0)
    /// Support of GATT Proxy Client Role
    #define BLE_MESH_GATT_PROXY_CLI          (0)
    /// Support of Directed Forwarding Configuration feature
    /// Force to Unprovisioned Role
    #define BLE_MESH_PROVEE                  (1)
    /// Support of GATT Bearer Server Role
    #define BLE_MESH_GATT_BEARER_SERVER      (1)
    /// Support of Configuration Server Model
    #define BLE_MESH_FND_SVR                 (1)
    /// Support of GATT Proxy Server Role
    #define BLE_MESH_GATT_PROXY_SVR          (1)
#endif // defined(CFG_BLE_MESH_PROVER)

/// Support of Directed Forwarding Node feature

/// Support of Directed Proxy feature
#define BLE_MESH_FWD_PROXY                   (0)

/// Support of Directed Friend feature
#define BLE_MESH_FWD_FRIEND                  (0)

/// Supported feature mask
#define BLE_MESH_FEAT_MASK   (  (BLE_MESH_MSG_API    * M_FEAT_MSG_API_SUP        )\
                              | (BLE_MESH_RELAY      * M_FEAT_RELAY_NODE_SUP     )\
                              | (BLE_MESH_GATT_PROXY * M_FEAT_PROXY_NODE_SUP     )\
                              | (BLE_MESH_GATT_PROV  * M_FEAT_PB_GATT_SUP        )\
                              | (BLE_MESH_FRIEND     * M_FEAT_FRIEND_NODE_SUP    )\
                              | (BLE_MESH_LPN        * M_FEAT_LOW_POWER_NODE_SUP )\
                              | (BLE_MESH_PROVER     * M_FEAT_PROVER_SUP         )\
                              | (BLE_MESH_PROVEE     * M_FEAT_PROVEE_SUP         )\
                              | ((0)        * M_FEAT_FWD_SUP            )\
                              | ((0)    * M_FEAT_FWD_CFG_SUP        ))

/*
 * COMPILATION FLAGS FOR MODELS
 ****************************************************************************************
 */

/// Support of server role for models
#if (defined(CFG_BLE_MESH_MDL_SERVER))
#define BLE_MESH_MDL_SERVER                  (1)
#else
#define BLE_MESH_MDL_SERVER                  (0)
#endif //(BLE_MESH_MDL_SERVER)

/// Support of client role for models
#if (defined(CFG_BLE_MESH_MDL_CLIENT))
#define BLE_MESH_MDL_CLIENT                  (1)
#else
#define BLE_MESH_MDL_CLIENT                  (0)
#endif //(BLE_MESH_MDL_CLIENT)

/// Support of models
#define BLE_MESH_MDL                         (BLE_MESH_MDL_SERVER || BLE_MESH_MDL_CLIENT)

/// Support of Generic Server Models
#if (defined(CFG_BLE_MESH_MDL_GENS))
#define BLE_MESH_MDL_GENS                    (1)
#else
#define BLE_MESH_MDL_GENS                    (0)
#endif //(BLE_MESH_MDL_GENS)

/// Support of Generic Client Models
#if (defined(CFG_BLE_MESH_MDL_GENC))
#define BLE_MESH_MDL_GENC                    (1)
#else
#define BLE_MESH_MDL_GENC                    (0)
#endif //(BLE_MESH_MDL_GENC)

/// Support of Sensor Server Models
#if (defined(CFG_BLE_MESH_MDL_SENSS))
#define BLE_MESH_MDL_SENSS                   (1)
#else
#define BLE_MESH_MDL_SENSS                   (0)
#endif //(BLE_MESH_MDL_SENSS)

/// Support of Sensor Client Models
#if (defined(CFG_BLE_MESH_MDL_SENSC))
#define BLE_MESH_MDL_SENSC                   (1)
#else
#define BLE_MESH_MDL_SENSC                   (0)
#endif //(BLE_MESH_MDL_SENSC)

/// Support of Time and Scenes Server Models
#if (defined(CFG_BLE_MESH_MDL_TSCNS))
#define BLE_MESH_MDL_TSCNS                   (1)
#else
#define BLE_MESH_MDL_TSCNS                   (0)
#endif //(BLE_MESH_MDL_TSCNS)

/// Support of Time and Scenes Client Models
#if (defined(CFG_BLE_MESH_MDL_TSCNC))
#define BLE_MESH_MDL_TSCNC                   (1)
#else
#define BLE_MESH_MDL_TSCNC                   (0)
#endif //(BLE_MESH_MDL_TSCNC)

/// Support of Light Server Models
#if (defined(CFG_BLE_MESH_MDL_LIGHTS))
#define BLE_MESH_MDL_LIGHTS                  (1)
#else
#define BLE_MESH_MDL_LIGHTS                  (0)
#endif //(BLE_MESH_MDL_LIGHTS)

/// Support of Light Client Models
#if (defined(CFG_BLE_MESH_MDL_LIGHTC))
#define BLE_MESH_MDL_LIGHTC                  (1)
#else
#define BLE_MESH_MDL_LIGHTC                  (0)
#endif //(BLE_MESH_MDL_LIGHTS)

/*
 * MESH PROFILE BLOCK CONFIGURATION
 ****************************************************************************************
 */

/// Maximum number of subnets the node can belong to
#define MESH_SUBNET_NB_MAX                   (5)
/// Maximum number of models that can be registered on the node
/// Shall be at least 2 (for Configuration Server Model and Health Server Model)
#define MESH_MODEL_NB_MAX                    (14)
/// Maximum number of Application Key that may be stored
#define MESH_APP_KEY_NB_MAX                  (MESH_MODEL_NB_MAX)

/// Maximum number of buffer block that can be allocated by the Buffer Manager
#define MESH_BUF_BLOCK_NB_MAX                (5)
/// Size of data part of "long" buffers
/// Value must be a multiple of 4
#define MESH_BUF_LONG_SIZE                   (152)
/// Size of data part of "small" buffers
/// Value must be a multiple of 4
#define MESH_BUF_SMALL_SIZE                  (32)

#if (BLE_MESH_FRIEND)
/// Number of "small" buffer to allocate during mesh stack initialization
#define MESH_BUF_INIT_LONG_NB                (8)
/// Number of "long" buffer to allocate during mesh stack initialization
#define MESH_BUF_INIT_SMALL_NB               (32)
#else
/// Number of "small" buffer to allocate during mesh stack initialization
#define MESH_BUF_INIT_LONG_NB                (4)
/// Number of "long" buffer to allocate during mesh stack initialization
#define MESH_BUF_INIT_SMALL_NB               (16)
#endif //(BLE_MESH_FRIEND)

/// Maximum number of buffers that can be dynamically allocated
#define MESH_BUF_DYN_NB_MAX                  (4)


/*
 * MESH MODEL BLOCK CONFIGURATION
 ****************************************************************************************
 */
/// Mesh Model Default Number of Replay Elements
#define MESH_MDL_DFLT_NB_REPLAY_ELMT         (5)

/// @} MESH_CONFIG

#endif /* MESH_CONFIG_ */
