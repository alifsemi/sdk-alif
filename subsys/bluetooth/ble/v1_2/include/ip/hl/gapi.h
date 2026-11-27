/**
 ****************************************************************************************
 *
 * @file gapi.h
 *
 * @brief GAP Isochronous - Header file
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef GAPI_H_
#define GAPI_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup GAPI Generic Access Profile Isochronous (GAPI)
 * @ingroup GAP_API
 * @brief Description of GAP Isochronous block
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup GAPI_DEFINE Definitions
 * @ingroup GAPI
 * @brief Definitions for GAP Isochronous block
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup GAPI_ENUM Enumerations
 * @ingroup GAPI
 * @brief Enumerations for GAP Isochronous block
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup GAPI_STRUCT Structures
 * @ingroup GAPI
 * @brief Structures for GAP Isochronous block
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup GAPI_NATIVE_API Native API
 * @ingroup GAPI
 * @brief Description of Native API for GAP Isochronous block
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup GAPI_CALLBACK Callback Functions
 * @ingroup GAPI_NATIVE_API
 * @brief Description of Callback Functions for GAP Isochronous block
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup GAPI_FUNCTION Functions
 * @ingroup GAPI_NATIVE_API
 * @brief Description of Functions for GAP Isochronous block
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "co_utils.h"         // Utilitaries Library
#include "co_math.h"          // Math Library
#include "co_bt_defines.h"    // Bluetooth Definitions
#if (AUDIO_SYNC_SUPPORT && BLE_BIS)
#include "gapi_audio_sync.h"  // GAPI Interface Audio Synchronization
#endif // (AUDIO_SYNC_SUPPORT && BLE_BIS)

/*
 * DEFINES
 ****************************************************************************************
 */

/// @addtogroup GAPI_DEFINE
/// @{

/// Length of broadcast code
#define GAPI_BROADCAST_CODE_LEN         (16u)

/// @} GAPI_DEFINE

/*
 * MACROS
 ****************************************************************************************
 */

/// Generate a command/request/request indication/indication code
#define GAPI_CODE(module, idx) ((GAPI_MODULE_ ## module << 8u) | idx)
/// Retrieve module from message code
#define GAPI_MODULE(code) ((code & 0x0F00u) >> 8u)
/// Retrieve code index from message code
#define GAPI_CODE_IDX(code) (code & 0x00FFu)

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// @addtogroup GAPI_ENUM
/// @{

/// Module type values
enum gapi_module_type
{
    /// Common section
    GAPI_MODULE_COMMON = 0u,
    /// Unicast group management section
    GAPI_MODULE_UG = 1u,
    /// Broadcast group management section
    GAPI_MODULE_BG = 2u,
    /// Data path management section
    GAPI_MODULE_DP = 3u,
    /// Test mode management section
    GAPI_MODULE_TM = 4u,
    /// Debug section
    GAPI_MODULE_DBG = 5u,
    /// Reserved for customer design service
    GAPI_MODULE_RESERVED = 6u,
    /// Maximum value
    GAPI_MODULE_MAX,
};

/// List of command type values for GAP Isochronous block
enum gapi_cmd_type
{
    /// Get Quality
    GAPI_CMD_TYPE_GET_QUALITY = 0u,

    /// Unicast - Update Group
    GAPI_CMD_TYPE_UG_UPDATE = 0u,
    /// Unicast - Enable Stream
    GAPI_CMD_TYPE_US_ENABLE = 1u,
    /// Unicast - Disable Stream
    GAPI_CMD_TYPE_US_DISABLE = 2u,
    /// Unicast - Remove Group
    GAPI_CMD_TYPE_UG_REMOVE = 3u,

    /// Broadcast - Enable Group
    GAPI_CMD_TYPE_BG_ENABLE = 0u,
    /// Broadcast - Synchronize with Group
    GAPI_CMD_TYPE_BG_SYNC = 1u,
    /// Broadcast - Disable Group
    GAPI_CMD_TYPE_BG_DISABLE = 2u,
    /// Broadcast - Remove Group
    GAPI_CMD_TYPE_BG_REMOVE = 3u,

    /// Test Mode - Start
    GAPI_CMD_TYPE_TM_START = 0u,
    /// Test Mode - Get Counters
    GAPI_CMD_TYPE_TM_CNT_GET = 1u,
    /// Test Mode - Stop
    GAPI_CMD_TYPE_TM_STOP = 2u,

    /// AUSY - Start
    GAPI_CMD_TYPE_AUSY_CONFIG_BIS = 0u,

};

/// List of GAPI_CMD command code values for GAP Isochronous block
enum gapi_cmd_code
{
    /// Get Quality
    GAPI_GET_QUALITY = GAPI_CODE(COMMON, GAPI_CMD_TYPE_GET_QUALITY),

    /// Unicast - Update Group
    GAPI_UG_UPDATE = GAPI_CODE(UG, GAPI_CMD_TYPE_UG_UPDATE),
    /// Unicast - Enable Stream
    GAPI_US_ENABLE = GAPI_CODE(UG, GAPI_CMD_TYPE_US_ENABLE),
    /// Unicast - Disable Stream
    GAPI_US_DISABLE = GAPI_CODE(UG, GAPI_CMD_TYPE_US_DISABLE),
    /// Unicast - Remove Group
    GAPI_UG_REMOVE = GAPI_CODE(UG, GAPI_CMD_TYPE_UG_REMOVE),

    /// Broadcast - Enable Group
    GAPI_BG_ENABLE = GAPI_CODE(BG, GAPI_CMD_TYPE_BG_ENABLE),
    /// Broadcast - Synchronize with Group
    GAPI_BG_SYNC = GAPI_CODE(BG, GAPI_CMD_TYPE_BG_SYNC),
    /// Broadcast - Disable Group
    GAPI_BG_DISABLE = GAPI_CODE(BG, GAPI_CMD_TYPE_BG_DISABLE),
    /// Broadcast - Remove Group
    GAPI_BG_REMOVE = GAPI_CODE(BG, GAPI_CMD_TYPE_BG_REMOVE),

    #if (GAPI_TEST)
    /// Test Mode - Start
    GAPI_TM_START = GAPI_CODE(TM, GAPI_CMD_TYPE_TM_START),
    /// Test Mode - Get Counters
    GAPI_TM_CNT_GET = GAPI_CODE(TM, GAPI_CMD_TYPE_TM_CNT_GET),
    /// Test Mode - Stop
    GAPI_TM_STOP = GAPI_CODE(TM, GAPI_CMD_TYPE_TM_STOP),
    #endif // (GAPI_TEST)

    #if (AUDIO_SYNC_SUPPORT && BLE_BIS)
    /// AUSY - Start
    GAPI_AUSY_CONFIG_BIS = GAPI_CODE(AUSY, GAPI_CMD_TYPE_AUSY_CONFIG_BIS),
    #endif // (AUDIO_SYNC_SUPPORT && BLE_BIS)
};

/// Group type values
enum gapi_group_types
{
    /// Unicast group
    GAPI_GROUP_TYPE_UG = 0u,
    /// Broadcast group
    GAPI_GROUP_TYPE_BG,
};

/// Codec Format values
enum gapi_codec_format
{
    /// u-Law log
    GAPI_CODEC_FORMAT_U_LAW = CODEC_ID_U_LAW,
    /// A-Lay log
    GAPI_CODEC_FORMAT_A_LAW = CODEC_ID_A_LAW,
    /// CVSD
    GAPI_CODEC_FORMAT_CVSD = CODEC_ID_CVSD,
    /// Transparent (controller does not do any transcoding or resampling)
    GAPI_CODEC_FORMAT_TRANSPARENT = CODEC_ID_TRANSPARENT,
    /// Linear PCM
    GAPI_CODEC_FORMAT_LINEAR_PCM = CODEC_ID_LINEAR_PCM,
    /// mSBC
    GAPI_CODEC_FORMAT_MSBC = CODEC_ID_MSBC,
    /// LC3
    GAPI_CODEC_FORMAT_LC3 = CODEC_ID_LC3,
    /// G.729A
    GAPI_CODEC_FORMAT_G729A = CODEC_ID_G729A,

    /// Maximum Codec Format
    GAPI_CODEC_FORMAT_SIG_MAX,

    /// Vendor Specific Codec
    GAPI_CODEC_FORMAT_VENDOR = CODEC_ID_VS,
};

/// Data path values
enum gapi_dp
{
    /// ISO over HCI Data Path
    GAPI_DP_ISOOHCI = 0x00,
    /// Example Data Path
    GAPI_DP_EXAMPLE = 0xF0,
    /// PCM
    GAPI_DP_PCM = 0xF1,
    /// ISO Payload Generator
    GAPI_DP_ISOGEN = 0xF2,
    /// ISO over shared-memory Data Path
    GAPI_DP_ISOOSHM = 0xF6,
    /// Audio Forwarding
    GAPI_DP_TWS = 0xF3,
};

/// Data path direction values
enum gapi_dp_direction
{
    /// Input (Host to Controller)
    GAPI_DP_DIRECTION_INPUT = 0,
    /// Output (Controller to Host)
    GAPI_DP_DIRECTION_OUTPUT,

    GAPI_DP_DIRECTION_MAX,
};

/// Data path direction bit field
enum gapi_dp_direction_bf
{
    /// Input data path
    GAPI_DP_DIRECTION_IN_POS = DATA_PATH_INPUT,
    GAPI_DP_DIRECTION_IN_BIT = CO_BIT(GAPI_DP_DIRECTION_IN_POS),

    /// Output data path
    GAPI_DP_DIRECTION_OUT_POS = DATA_PATH_OUTPUT,
    GAPI_DP_DIRECTION_OUT_BIT = CO_BIT(GAPI_DP_DIRECTION_OUT_POS),

    /// Both direction
    GAPI_DP_DIRECTION_BOTH = GAPI_DP_DIRECTION_IN_BIT | GAPI_DP_DIRECTION_OUT_BIT,
};

/// Data path update types
enum gapi_dp_update
{
    /// Setup
    GAPI_DP_UPDATE_SETUP = 0u,
    /// Remove
    GAPI_DP_UPDATE_REMOVE,
};

/// Broadcast Group status
enum gapi_bg_sync_status
{
    /// Synchronization has been established
    GAPI_BG_SYNC_STATUS_ESTABLISHED = 0u,
    /// Synchronization has failed
    GAPI_BG_SYNC_STATUS_FAILED,
    /// Synchronization establishment has been cancelled
    GAPI_BG_SYNC_STATUS_CANCELLED,
    /// Synchronization has been lost
    GAPI_BG_SYNC_STATUS_LOST,
    /// Synchronization stopped due to peer termination
    GAPI_BG_SYNC_STATUS_PEER_TERMINATE,
    /// Synchronization stopped due to upper layer termination
    GAPI_BG_SYNC_STATUS_UPPER_TERMINATE,
    /// Synchronization stopped due to an encryption error
    GAPI_BG_SYNC_STATUS_MIC_FAILURE,
};

/// @} GAPI_ENUM

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// @addtogroup GAPI_STRUCT
/// @{

/// CIS/Connection binding information structure
typedef struct
{
    /// Stream local index
    uint8_t stream_lid;
    /// Connection index
    uint8_t conidx;
} gapi_us_bind_info_t;

/// Unicast group common parameters structure
typedef struct
{
    /// SDU interval from Master to Slave in microseconds
    /// From 0xFF (255us) to 0xFFFF (1.048575s)
    uint32_t sdu_intv_m2s_us;
    /// SDU interval from Slave to Master in microseconds
    /// From 0xFF (255us) to 0xFFFF (1.048575s)
    uint32_t sdu_intv_s2m_us;
    /// Sequential or interleaved scheduling
    uint8_t packing;
    /// Unframed or framed mode
    uint8_t framing;
    /// Worst slow clock accuracy of slaves
    uint8_t sca;
} gapi_ug_param_common_t;

/// Unicast group information structure
typedef struct
{
    /// Common parameters
    gapi_ug_param_common_t common;
    /// Maximum time (in milliseconds) for an SDU to be transported from master controller to slave
    /// controller. From 0x5 (5ms) to 0xFA0 (4s)
    uint16_t tlatency_m2s_ms;
    /// Maximum time (in milliseconds) for an SDU to be transported from slave controller to master
    /// controller. From 0x5 (5ms) to 0xFA0 (4s)
    uint16_t tlatency_s2m_ms;
} gapi_ug_param_t;

/// Unicast group test information structure
typedef struct
{
    /// Common information
    gapi_ug_param_common_t common;
    /// Flush timeout in milliseconds for each payload sent from Master to Slave
    uint16_t ft_m2s_ms;
    /// Flush timeout in milliseconds for each payload sent from Slave to Master
    uint16_t ft_s2m_ms;
    /// ISO interval in frames
    uint16_t iso_intv_frames;
} gapi_ug_test_param_t;

/// Unicast stream common information structure
typedef struct
{
    /// Maximum size of an SDU provided by master host
    uint16_t max_sdu_m2s;
    /// Maximum size of an SDU provided by slave host
    uint16_t max_sdu_s2m;
    /// PHYs on which packets may be transmitted from master to slave
    uint8_t phy_m2s;
    /// PHYs on which packets may be transmitted from slave to master
    uint8_t phy_s2m;
} gapi_us_param_common_t;

/// Unicast stream information structure
typedef struct
{
    /// Common information
    gapi_us_param_common_t common;
    /// Maximum number of times every data PDU should be retransmitted for master to slave. From 0x0 to 0xF
    uint8_t rtn_m2s;
    /// Maximum number of times every data PDU should be retransmitted for slave to master. From 0x0 to 0xF
    uint8_t rtn_s2m;
} gapi_us_param_t;

/// Unicast stream test information structure
typedef struct
{
    /// Common information
    gapi_us_param_common_t common;
    /// Maximum size of the payload from master to slave. From 0x0 to 0xFB
    uint8_t max_pdu_m2s;
    /// Maximum size of the payload from slave to master. From 0x0 to 0xFB
    uint8_t max_pdu_s2m;
    /// Burst number from master to slave. From 0x0 to 0xF
    uint8_t bn_m2s;
    /// Burst number from slave to master. From 0x0 to 0xF
    uint8_t bn_s2m;
    /// Maximum number of subevents in each stream interval. From 0x1 to 0x1F
    uint8_t nse;
} gapi_us_test_param_t;

/// Unicast group configuration structure (provided by controller after stream establisment)
typedef struct
{
    /// Group synchronization delay time in microseconds
    uint32_t sync_delay_us;
    /// The maximum time, in microseconds, for transmission of SDUs of all CISes from master to slave
    /// (range 0x0000EA to 0x7FFFFF)
    uint32_t tlatency_m2s_us;
    /// The maximum time, in microseconds, for transmission of SDUs of all CISes from slave to master
    /// (range 0x0000EA to 0x7FFFFF)
    uint32_t tlatency_s2m_us;
    /// ISO interval (1.25ms unit, range: 5ms to 4s)
    uint16_t iso_intv_frames;
} gapi_ug_config_t;

/// Unicast stream configuration structure (provided by controller after stream establishment)
typedef struct
{
    /// Stream synchronization delay time in microseconds
    uint32_t sync_delay_us;
    /// Maximum size, in octets, of the payload from master to slave (Range: 0x00-0xFB)
    uint16_t max_pdu_m2s;
    /// Maximum size, in octets, of the payload from slave to master (Range: 0x00-0xFB)
    uint16_t max_pdu_s2m;
    /// Master to slave PHY, bit 0: 1Mbps, bit 1: 2Mbps, bit 2: LE-Coded
    uint8_t phy_m2s;
    /// Slave to master PHY, bit 0: 1Mbps, bit 1: 2Mbps, bit 2: LE-Coded
    uint8_t phy_s2m;
    /// The burst number for master to slave transmission (0x00: no isochronous data from the master to the slave, range 0x01-0x0F)
    uint8_t bn_m2s;
    /// The burst number for slave to master transmission (0x00: no isochronous data from the slave to the master, range 0x01-0x0F)
    uint8_t bn_s2m;
    /// The flush timeout, in multiples of the ISO_Interval, for each payload sent from the master to the slave (Range: 0x01-0x1F)
    uint8_t ft_m2s;
    /// The flush timeout, in multiples of the ISO_Interval, for each payload sent from the slave to the master (Range: 0x01-0x1F)
    uint8_t ft_s2m;
    /// Maximum number of subevents in each isochronous interval. From 0x1 to 0x1F
    uint8_t nse;
} gapi_us_config_t;

/// Broadcast group common parameter structure
typedef struct
{
    /// SDU interval in microseconds
    uint32_t sdu_intv_us;
    /// Maximum size of an SDU
    uint16_t max_sdu;
    /// Sequential or interleaved scheduling
    uint8_t packing;
    /// Unframed or framed mode
    uint8_t framing;
    /// Bitfield indicating PHYs than can be used by the controller for transmissions of SDUs
    uint8_t phy_bf;
} gapi_bg_param_common_t;

/// Broadcast group information structure
typedef struct
{
    /// Common information
    gapi_bg_param_common_t common_param;
    /// Maximum time (in milliseconds) between the first transmission of an SDU to the end of the last
    /// transmission of the same SDU
    uint16_t max_tlatency_ms;
    /// Number of times every PDU should be transmitted
    uint8_t rtn;
} gapi_bg_param_t;

/// Broadcast group test information structure
typedef struct
{
    /// Common information
    gapi_bg_param_common_t common_param;
    /// ISO interval in multiple of 1.25ms. From 0x4 (5ms) to 0xC80 (4s)
    uint16_t iso_intv_frame;
    /// Number of subevents in each interval of each stream in the group
    uint8_t nse;
    /// Maximum size of a PDU
    uint8_t max_pdu;
    /// Burst number (number of new payload in each interval). From 1 to 7
    uint8_t bn;
    /// Number of times the scheduled payload is transmitted in a given event. From 0x1 to 0xF
    uint8_t irc;
    /// Isochronous Interval spacing of payloads transmitted in the pre-transmission subevents.
    /// From 0x00 to 0x0F
    uint8_t pto;
} gapi_bg_test_param_t;

/// Broadcast Group synchronization configuration structure
typedef struct
{
    /// The maximum delay time, in microseconds, for transmission of SDUs of all BISes
    /// (in us range 0x0000EA-0x7FFFFF)
    uint32_t tlatency_us;
    /// ISO interval in frames\n
    /// From 5ms to 4s
    uint16_t iso_interval_frames;
    /// The number of subevents in each BIS event in the BIG, range 0x01-0x1E
    uint8_t nse;
    /// The number of new payloads in each BIS event, range 0x01-0x07
    uint8_t bn;
    /// Offset used for pre-transmissions, range 0x00-0x0F
    uint8_t pto;
    /// The number of times a payload is transmitted in a BIS event, range 0x01-0x0F
    uint8_t irc;
    /// Maximum size of the payload in octets, range 0x00-0xFB
    uint8_t max_pdu;
} gapi_bg_sync_config_t;

/// Broadcast Group configuration structure
typedef struct
{
    /// Transmission delay time in microseconds of all BISs in the BIG (in us range 0x0000EA-0x7FFFFF)
    uint32_t sync_delay_us;
    /// The maximum delay time, in microseconds, for transmission of SDUs of all BISes
    /// (in us range 0x0000EA-0x7FFFFF)
    uint32_t tlatency_us;
    /// ISO interval in frames\n
    /// From 5ms to 4s
    uint16_t iso_interval_frames;
    /// The number of subevents in each BIS event in the BIG, range 0x01-0x1E
    uint8_t nse;
    /// The number of new payloads in each BIS event, range 0x01-0x07
    uint8_t bn;
    /// Offset used for pre-transmissions, range 0x00-0x0F
    uint8_t pto;
    /// The number of times a payload is transmitted in a BIS event, range 0x01-0x0F
    uint8_t irc;
    /// Maximum size of the payload in octets, range 0x00-0xFB
    uint8_t max_pdu;
    /// PHY
    uint8_t phy;
} gapi_bg_config_t;

/// Data value in LTV format
typedef struct
{
    /// Length of data value
    uint8_t len;
    /// Data value
    uint8_t data[__ARRAY_EMPTY];
} gapi_ltv_t;

/// Data Path Codec Configuration structure
typedef struct
{
    /// Codec ID
    uint8_t codec_id[CODEC_ID_LEN];
    /// Number of Codec Configuration pieces
    uint8_t nb_cfg;
    /// Codec Configuration (in LTV format)
    const gapi_ltv_t* p_cfg[__ARRAY_EMPTY];
} gapi_dp_codec_cfg_t;

/// Ausy BIS configuration structure
typedef struct
{
    /// Enable or Disable reception of BIS establishment parameters (1: enable,  0: disable)
    uint8_t enable;
} gapi_ausy_config_bis_t;

/// Data Path Codec Configuration structure with one single codec configuration
typedef struct
{
    /// Codec ID
    uint8_t codec_id[CODEC_ID_LEN];
    /// Number of Codec Configuration pieces
    uint8_t nb_cfg;
    /// Codec Configuration (in LTV format)
    const gapi_ltv_t* p_cfg;
} gapi_dp_codec_cfg_single_t;

/// @} GAPI_STRUCT

/*
 * CALLBACK FUNCTIONS DEFINITION
 ****************************************************************************************
 */

/// @addtogroup GAPI_CALLBACK
/// @{

/**
 ****************************************************************************************
 * @brief Callback function called when a command has been fully processed
 *
 * @param[in] intf_lid      Interface local index
 * @param[in] cmd_code      Command code
 * @param[in] status        Status
 * @param[in] group_lid     Group local index
 * @param[in] stream_lid    Stream local index
 ****************************************************************************************
 */
typedef void (*gapi_cb_cmp_evt)(uint8_t intf_lid, uint16_t cmd_code, uint16_t status,
                               uint8_t group_lid, uint8_t stream_lid);

/**
 ****************************************************************************************
 * @brief Callback function called when establishment of an unicast stream is required by a peer device
 *
 * @param[in] intf_lid      Interface local index
 * @param[in] cig_id        CIG ID
 * @param[in] cis_id        CIS ID
 * @param[in] conidx        Connection index of connection on which stream establishment has been required
 * @param[in] group_lid     Allocated Group local index
 * @param[in] stream_lid    Allocated Stream local index
 ****************************************************************************************
 */
typedef void (*gapi_cb_us_establish_req)(uint8_t intf_lid, uint8_t cig_id, uint8_t cis_id, uint8_t conidx,
                                         uint8_t group_lid, uint8_t stream_lid);

/**
 ****************************************************************************************
 * @brief Callback function called when an unicast stream has been enabled
 *
 * @param[in] intf_lid      Interface local index
 * @param[in] group_lid     Group local index
 * @param[in] stream_lid    Stream local index
 * @param[in] p_ug_cfg      Pointer to Group parameters provided by the Controller
 * @param[in] p_us_cfg      Pointer to Stream parameters provided by the Controller
 ****************************************************************************************
 */
typedef void (*gapi_cb_us_enabled)(uint8_t intf_lid, uint8_t group_lid, uint8_t stream_lid,
                                  gapi_ug_config_t* p_ug_cfg, gapi_us_config_t* p_us_cfg);

/**
 ****************************************************************************************
 * @brief Callback function called when an unicast stream has been disabled
 *
 * @param[in] intf_lid      Interface local index
 * @param[in] stream_lid    Stream local index
 * @param[in] reason        Reason
 ****************************************************************************************
 */
typedef void (*gapi_cb_us_disabled)(uint8_t intf_lid, uint8_t stream_lid, uint8_t reason);

/**
 ****************************************************************************************
 * @brief Callback function called when a Broadcast Group synchronization status has been updated
 *
 * @param[in] intf_lid     Interface local index
 * @param[in] group_lid    Group local index
 * @param[in] status       Status (see #gapi_bg_sync_status enumeration)
 * @param[in] p_cfg        Pointer to Broadcast Group configuration structure
 * @param[in] nb_bis       Number of BISes synchronization has been established with\n
 *                         Meaningful only if synchronization has been established
 * @param[in] p_conhdl     Pointer to list of Connection Handle values provided by the Controller\n
 *                         List of nb_bis values
 ****************************************************************************************
 */
typedef void (*gapi_cb_bg_sync_status)(uint8_t intf_lid, uint8_t group_lid, uint8_t status,
                                      gapi_bg_sync_config_t* p_cfg, uint8_t nb_bis, const uint16_t* p_conhdl);

/**
 ****************************************************************************************
 * @brief Callback function called when a Broadcast Group has been created
 *
 * @param[in] intf_lid     Interface local index
 * @param[in] group_lid    Group local index
 * @param[in] p_cfg        Pointer to Broadcast Group configuration structure
 * @param[in] nb_bis       Number of BISes synchronization has been established with\n
 *                         Meaningful only if synchronization has been established
 * @param[in] p_conhdl     Pointer to list of Connection Handle values provided by the Controller\n
 *                         List of nb_bis values
 ****************************************************************************************
 */
typedef void (*gapi_cb_bg_created)(uint8_t intf_lid, uint8_t group_lid, gapi_bg_config_t* p_cfg,
                                  uint8_t nb_bis, const uint16_t* p_conhdl);

/**
 ****************************************************************************************
 * @brief Callback function called when data path state for a stream has been updated
 *
 * @param[in] intf_lid      Interface local index
 * @param[in] group_lid     Group local index
 * @param[in] stream_lid    Stream local index
 * @param[in] dp_update     Data path update type
 * @param[in] direction     Direction for setup update, direction bit field for remove update
 * @param[in] status        Update status
 ****************************************************************************************
 */
typedef void (*gapi_cb_dp_update)(uint8_t intf_lid, uint8_t group_lid, uint8_t stream_lid, uint8_t dp_update,
                                 uint8_t direction, uint16_t status);

/**
 ****************************************************************************************
 * @brief Callback function called when test mode counter have been received
 *
 * @param[in] intf_lid      Interface local index
 * @param[in] stream_lid    Stream local index
 * @param[in] nb_rx         Number of received packets
 * @param[in] nb_missed     Number of missed packets
 * @param[in] nb_failed     Number of failed packets
 ****************************************************************************************
 */
typedef void (*gapi_cb_tm_cnt)(uint8_t intf_lid, uint8_t stream_lid, uint32_t nb_rx,
                              uint32_t nb_missed, uint32_t nb_failed);

/**
 ****************************************************************************************
 * @brief Callback function called when test mode counter have been received
 *
 * @param[in] intf_lid                  Interface local index
 * @param[in] status                    Status
 * @param[in] stream_lid                Stream local index
 * @param[in] tx_unacked_packets        Number of packets transmitted and unacked
 * @param[in] tx_flushed_packets        Number of flushed transmitted packets
 * @param[in] tx_last_subevent_packets  Number of packets transmitted during last subevent
 * @param[in] retransmitted_packets     Number of retransmitted packets
 * @param[in] crc_error_packets         Number of packets received with a CRC error
 * @param[in] rx_unreceived_packets     Number of unreceived packets
 * @param[in] duplicate_packets         Number of duplicate packet received
 ****************************************************************************************
 */
typedef void (*gapi_cb_quality_cmp_evt)(uint8_t intf_lid, uint16_t status, uint8_t stream_lid,
                                       uint32_t tx_unacked_packets, uint32_t tx_flushed_packets,
                                       uint32_t tx_last_subevent_packets, uint32_t retransmitted_packets,
                                       uint32_t crc_error_packets, uint32_t rx_unreceived_packets,
                                       uint32_t duplicate_packets);

/// Set of callback functions structure (Unicast)
typedef struct
{
    /// Callback function called when establishment of an unicast stream is required by a peer device
    gapi_cb_us_establish_req cb_us_establish_req;
    /// Unicast stream enabled indication callback function
    gapi_cb_us_enabled cb_us_enabled;
    /// Unicast stream disabled indication callback function
    gapi_cb_us_disabled cb_us_disabled;
} gapi_cb_ug_t;

/// Set of callback functions structure (Broadcast)
typedef struct
{
    /// Broadcast synchronization status update indication callback function
    gapi_cb_bg_sync_status cb_bg_sync_status;
    /// Broadcast group created indication callback function
    gapi_cb_bg_created cb_bg_created;
} gapi_cb_bg_t;

/// Set of callback functions structure (Common)
typedef struct
{
    /// Command complete event callback function
    gapi_cb_cmp_evt cb_cmp_evt;
    /// Data path update indication callback function
    gapi_cb_dp_update cb_dp_update;
    /// Test mode counter indication callback function\n
    /// Can be NULL
    gapi_cb_tm_cnt cb_tm_cnt;
    /// Get link quality command complete event callback function
    gapi_cb_quality_cmp_evt cb_quality_cmp_evt;
} gapi_cb_common_t;

#if (BLE_ISOOHCI)
/// Callback structure required to handle ISO over HCI events
typedef struct
{
    /**
     ****************************************************************************************
     * @brief Callback executed when an ISO audio packet is received
     *
     * @param[in] intf_lid          Interface local index
     * @param[in] stream_lid        Stream local index
     * @param[in] packet_status     Packet status
     * @param[in] p_timestamp       Pointer to timestamp (can be NULL)
     * @param[in] seq_nb            Sequence number
     * @param[in] sdu_length        SDU length
     * @param[in] p_sdu             Pointer to SDU
     ****************************************************************************************
     */
    void (*cb_dp_data_rx)(uint8_t intf_lid, uint8_t stream_lid, uint8_t packet_status,
                          const uint32_t* p_timestamp, uint8_t seq_nb, uint16_t sdu_length, const uint8_t* p_sdu);
} gapi_cbs_isoohci_t;
#endif // (BLE_ISOOHCI)

/// @} GAPI_CALLBACK

/*
 * API FUNCTIONS DECLARATION
 ****************************************************************************************
 */

/// @addtogroup GAPI_FUNCTION
/// @{

/**
 ****************************************************************************************
 * @brief Register a new interface
 *
 * @param[out] p_intf_lid   Address at which allocated interface index must be returned
 * @param[in] p_cb_ug       Pointer to set of callback functions to be used for this interface (Unicast)\n
 *                          Can be NULL (p_cb_bg cannot be NULL in that case)
 * @param[in] p_cb_bg       Pointer to set of callback functions to be used for this interface (Broadcast)\n
 *                          Can be NULL (p_cb_ug cannot be NULL in that case)
 * @param[in] p_cb_common   Pointer to set of callback functions to be used for this interface (Common)\n
 *                          Cannot be NULL
 *
 * @return An error status
 ****************************************************************************************
 */
uint16_t gapi_intf_register(uint8_t* p_intf_lid, const gapi_cb_ug_t* p_cb_ug, const gapi_cb_bg_t* p_cb_bg,
                            const gapi_cb_common_t* p_cb_common);

/**
 ****************************************************************************************
 * @brief Unregister an interface
 *
 * @param[out] intf_lid   Interface index
 *
 * @return An error status
 ****************************************************************************************
 */
uint16_t gapi_intf_unregister(uint8_t intf_lid);

/**
 ****************************************************************************************
 * @brief Get quality information for an ISO Link
 *
 * @param[in] stream_lid       Stream local index
 *
 * @return An error status
 ****************************************************************************************
 */
uint16_t gapi_gen_get_quality(uint8_t stream_lid);

#if (GAPI_UNICAST_SUPP)
/**
 ****************************************************************************************
 * @brief Add a unicast group
 *
 * @param[in] p_params        Pointer to Unicast Group Parameters structure
 * @param[in] cig_id          CIG ID
 * @param[in] intf_lid        Interface local index for the group
 * @param[out] p_group_lid    Pointer at which allocated group local index is returned
 *
 * @return An handling status
 ****************************************************************************************
 */
uint16_t gapi_ug_add(gapi_ug_param_t* p_params, uint8_t cig_id, uint8_t intf_lid, uint8_t* p_group_lid);

/**
 ****************************************************************************************
 * @brief Add a unicast group for test
 *
 * @param[in] p_params        Pointer to Unicast Group test information structure
 * @param[in] cig_id          CIG ID
 * @param[in] intf_lid        Interface local index for the group
 * @param[out] p_group_lid    Pointer at which allocated group local index is returned
 *
 * @return An handling status
 ****************************************************************************************
 */
uint16_t gapi_ug_test_add(gapi_ug_test_param_t *p_params, uint8_t cig_id, uint8_t intf_lid,
                         uint8_t* p_group_lid);
/**
 ****************************************************************************************
 * @brief Configure a unicast stream
 *
 * @param[in] group_lid       Stream local index
 * @param[in] p_stream_info   Pointer to Unicast Stream information structure
 * @param[in] cis_id          CIS ID
 * @param[out] p_stream_lid   Pointer at which allocated stream local index will be returned
 *
 * @return An handling status
 ****************************************************************************************
 */
uint16_t gapi_us_config(uint8_t group_lid, gapi_us_param_t* p_stream_info, uint8_t cis_id,
                       uint8_t* p_stream_lid);

/**
 ****************************************************************************************
 * @brief Configure a unicast stream for test
 *
 * @param[in] group_lid       Group local index
 * @param[in] p_stream_info   Pointer to Unicast Stream test information structure
 * @param[in] cis_id          CIS ID
 * @param[out] p_stream_lid   Pointer at which allocated stream local index will be returned
 *
 * @return An handling status
 ****************************************************************************************
 */
uint16_t gapi_us_test_config(uint8_t group_lid, gapi_us_test_param_t *p_stream_info, uint8_t cis_id,
                            uint8_t* p_stream_lid);

/**
 ****************************************************************************************
 * @brief Inform GAPI that establishment of an unicast stream can be accepted
 *
 * @param[in] accept            Accept or not stream establishment
 * @param[in] stream_lid        Stream local index
 * @param[in] intf_lid          Interface local index
 ****************************************************************************************
 */
void gapi_us_establish_cfm(bool accept, uint8_t stream_lid, uint8_t intf_lid);

/**
 ****************************************************************************************
 * @brief Bind an unicast stream with a connection
 *
 * @param[in] stream_lid     Stream local index
 * @param[in] conidx        Connection index
 *
 * @return An handling status
 ****************************************************************************************
 */
uint16_t gapi_us_bind(uint8_t stream_lid, uint8_t conidx);

/**
 ****************************************************************************************
 * @brief Update content of an unicast group
 *
 * @param[in] group_lid     Group local index
 *
 * @return An handling status
 ****************************************************************************************
 */
uint16_t gapi_ug_update(uint8_t group_lid);

/**
 ****************************************************************************************
 * @brief Enable bounded stream in a unicast group
 *
 * @param[in] group_lid     Group local index
 *
 * @return An handling status
 ****************************************************************************************
 */
uint16_t gapi_us_enable(uint8_t group_lid);

/**
 ****************************************************************************************
 * @brief Disable an unicast stream
 *
 * @param[in] stream_lid      Stream local index
 *
 * @return An handling status
 ****************************************************************************************
 */
uint16_t gapi_us_disable(uint8_t stream_lid);

/**
 ****************************************************************************************
 * @brief Release an unicast stream
 *
 * @param[in] stream_lid      Stream local index
 *
 * @return An handling status
 ****************************************************************************************
 */
uint16_t gapi_us_release(uint8_t stream_lid);

/**
 ****************************************************************************************
 * @brief Remove a unicast group
 *
 * @param[in] group_lid       Group local index
 *
 * @return An handling status
 ****************************************************************************************
 */
uint16_t gapi_ug_remove(uint8_t group_lid);

/**
 ****************************************************************************************
 * @brief Get unicast group parameters (normal)
 *
 * @param[in] group_lid       Group local index
 *
 * @return Pointer to group parameter structure
 ****************************************************************************************
 */
gapi_ug_param_t* gapi_ug_get_param(uint8_t group_lid);

/**
 ****************************************************************************************
 * @brief Get unicast group parameters (test)
 *
 * @param[in] group_lid       Group local index
 *
 * @return Pointer to group parameter structure
 ****************************************************************************************
 */
gapi_ug_test_param_t* gapi_ug_get_test_param(uint8_t group_lid);

/**
 ****************************************************************************************
 * @brief Get unicast group configuration
 *
 * @param[in] group_lid       Group local index
 *
 * @return Pointer to group configuration structure
 ****************************************************************************************
 */
gapi_ug_config_t* gapi_ug_get_config(uint8_t group_lid);

/**
 ****************************************************************************************
 * @brief Get unicast stream parameter (normal)
 *
 * @param[in] stream_lid       Stream local index
 *
 * @return Pointer to stream parameter structure
 ****************************************************************************************
 */
gapi_us_param_t* gapi_us_get_param(uint8_t stream_lid);

/**
 ****************************************************************************************
 * @brief Get unicast stream parameter (test)
 *
 * @param[in] stream_lid       Stream local index
 *
 * @return Pointer to stream parameter structure
 ****************************************************************************************
 */
gapi_us_test_param_t* gapi_us_get_test_param(uint8_t stream_lid);

/**
 ****************************************************************************************
 * @brief Get unicast stream configuration
 *
 * @param[in] stream_lid       Stream local index
 *
 * @return Pointer to stream configuration structure
 ****************************************************************************************
 */
gapi_us_config_t* gapi_us_get_config(uint8_t stream_lid);
#endif // (GAPI_UNICAST_SUPP)

#if (GAPI_BROADCAST_SUPP)
#if (GAPI_BROADCAST_SOURCE_SUPP)
/**
 ****************************************************************************************
 * @brief Add a Broadcast Group
 *
 * @param[in] p_params          Pointer to Broadcast Group information structure
 * @param[in] big_handle        BIG handle
 * @param[in] nb_streams        Number of streams in the group
 * @param[in] intf_lid          Interface local index for the group
 * @param[out] p_group_lid      Pointer at which allocated group local index will be returned
 * @param[out] p_stream_lids    Pointer at which allocated stream local indexes will be returned
 *
 * @return An handling status
 ****************************************************************************************
 */
uint16_t gapi_bg_add(gapi_bg_param_t* p_params, uint8_t big_handle, uint8_t nb_streams,
                    uint8_t intf_lid, uint8_t* p_group_lid, uint8_t* p_stream_lids);

/**
 ****************************************************************************************
 * @brief Add a Broadcast Group for test
 *
 * @param[in] p_params          Pointer to Broadcast Group information structure
 * @param[in] big_handle        BIG handle
 * @param[in] nb_streams        Number of streams in the group
 * @param[in] intf_lid          Interface local index for the group
 * @param[out] p_group_lid      Pointer at which allocated group local index will be returned
 * @param[out] p_stream_lids    Pointer at which allocated stream local indexes will be returned
 *
 * @return An handling status
 ****************************************************************************************
 */
uint16_t gapi_bg_test_add(gapi_bg_test_param_t* p_params, uint8_t big_handle, uint8_t nb_streams,
                         uint8_t intf_lid, uint8_t* p_group_lid, uint8_t* p_stream_lids);

/**
 ****************************************************************************************
 * @brief Enable a Broadcast Group
 *
 * @param[in] group_lid         Group local index
 * @param[in] adv_actv_lid      Advertising activity local index
 * @param[in] p_broadcast_code  Pointer to 16-byte code used to generate encryption key used to encrypt payloads.
 *                              NULL if stream in the group are not encrypted
 *
 * @return An handling status
 ****************************************************************************************
 */
uint16_t gapi_bg_enable(uint8_t group_lid, uint8_t adv_actv_lid, uint8_t* p_broadcast_code);
#endif // (GAPI_BROADCAST_SOURCE_SUPP)

#if (GAPI_BROADCAST_SINK_SUPP)
/**
 ****************************************************************************************
 * @brief Request to synchronize with a Broadcast Group
 *
 * @param[in] p_broadcast_code  Group local index
 * @param[in] stream_pos_bf     Bit fields indicating streams with which synchronization must be established
 * @param[in] sync_timeout      Synchronization timeout
 * @param[in] big_handle        BIG handle
 * @param[in] sync_actv_lid     Synchronization activity local index
 * @param[in] mse               Maximum number of subevents the controller should use to received
 *                              data payloads in each interval
 * @param[in] intf_lid          Interface local index for the group
 * @param[out] p_group_lid      Pointer at which allocated group local index will be returned
 * @param[out] p_stream_lids    Pointer at which allocated stream local indexes will be returned
 *                              size of array shall be greater or equals number of bit present in stream_pos_bf
 *
 * @return An handling status
 ****************************************************************************************
 */
uint16_t gapi_bg_sync(const uint8_t* p_broadcast_code, uint32_t stream_pos_bf, uint16_t sync_timeout,
                     uint8_t big_handle, uint8_t sync_actv_lid, uint8_t mse, uint8_t intf_lid,
                     uint8_t* p_group_lid, uint8_t* p_stream_lids);
#endif // (GAPI_BROADCAST_SINK_SUPP)

/**
 ****************************************************************************************
 * @brief Disable a Broadcast Group
 *
 * @param[in] group_lid         Group local index
 *
 * @return An handling status
 ****************************************************************************************
 */
uint16_t gapi_bg_disable(uint8_t group_lid);

/**
 ****************************************************************************************
 * @brief Disable a Broadcast Group
 *
 * @param[in] group_lid         Group local index
 *
 * @return An handling status
 ****************************************************************************************
 */
uint16_t gapi_bg_remove(uint8_t group_lid);

/**
 ****************************************************************************************
 * @brief Get broadcast group parameters (normal)
 *
 * @param[in] group_lid       Group local index
 *
 * @return Pointer to group parameter structure
 ****************************************************************************************
 */
gapi_bg_param_t* gapi_bg_get_param(uint8_t group_lid);
#endif // (GAPI_BROADCAST_SUPP)

#if (BLE_ISOOHCI)
/**
 ****************************************************************************************
 * @brief Enable or disable use of ISO over HCI
 *
 * @param[in] intf_lid   Interface index
 * @param[in] p_cbs      Pointer to set of callback functions for reception of ISO over HCI events
 *
 * @return An error status (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t gapi_dp_enable_isoohci(const gapi_cbs_isoohci_t* p_cbs);
#endif // (BLE_ISOOHCI)

/**
 ****************************************************************************************
 * @brief Setup data path on a stream
 *
 * @param[in] stream_lid        Stream local index
 * @param[in] direction         Direction for which data path must be setup
 * @param[in] ctl_delay_us      Controller delay in microseconds
 * @param[in] dp_id             Data Path ID (see #data_path_id enumeration)
 * @param[in] p_codec_cfg       Pointer to Codec configuration structure\n
 *                              NULL if transparent more is used (Codec resides in the Host/Application)
 *
 * @return An handling status
 ****************************************************************************************
 */
uint16_t gapi_dp_setup(uint8_t stream_lid, uint8_t direction, uint32_t ctl_delay_us, uint8_t dp_id,
                       const gapi_dp_codec_cfg_t* p_codec_cfg);

/**
 ****************************************************************************************
 * @brief Remove data path on a stream
 *
 * @param[in] stream_lid    Stream local index
 * @param[in] direction_bf  Bit field indicated directions for which data path must be removed
 *
 * @return An handling status
 ****************************************************************************************
 */
uint16_t gapi_dp_remove(uint8_t stream_lid, uint8_t direction_bf);

#if (GAPI_TEST)
/**
 ****************************************************************************************
 * @brief Start test mode on a stream
 *
 * @param[in] stream_lid    Stream local index
 * @param[in] transmit      Transmit (!=0) or receive (=0) test payloads
 * @param[in] payload_type  Payload type
 *
 * @return An handling status
 ****************************************************************************************
 */
uint16_t gapi_tm_start(uint8_t stream_lid, uint8_t transmit, uint8_t payload_type);

/**
 ****************************************************************************************
 * @brief Get test mode counter for a stream
 *
 * @param[in] stream_lid    Stream local index
 *
 * @return An handling status
 ****************************************************************************************
 */
uint16_t gapi_tm_cnt_get(uint8_t stream_lid);

/**
 ****************************************************************************************
 * @brief Stop test mode on a stream
 *
 * @param[in] stream_lid    Stream local index
 *
 * @return An handling status
 ****************************************************************************************
 */
uint16_t gapi_tm_stop(uint8_t stream_lid);
#endif // (GAPI_TEST)

/**
 ****************************************************************************************
 * @brief Get connection handle for a given stream
 *
 * @param[in] stream_lid    Stream local index
 *
 * @return Connection handle value
 ****************************************************************************************
 */
uint16_t gapi_sm_stream_lid_to_conhdl(uint8_t stream_lid);

/**
 ****************************************************************************************
 * @brief Get stream associated with a given connection handle
 *
 * @param[in]  conhdl        Connection handle
 * @param[out] p_stream_lid  Pointer to Stream local index
 * @param[out] p_group_lid   Pointer to Group local index
 *
 * @return An error status
 ****************************************************************************************
 */
uint16_t gapi_sm_stream_conhdl_to_lid(uint16_t conhdl, uint8_t* p_stream_lid, uint8_t* p_group_lid);

/// @} GAPI_FUNCTION

#endif // GAPI_H_
