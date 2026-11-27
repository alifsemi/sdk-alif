/**
 ****************************************************************************************
 *
 * @file co_llcp.h
 *
 * @brief This file contains the LLCP Bluetooth defines, enumerations and structures
 *        definitions for use by all modules in RW stack.
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 ****************************************************************************************
 */

#ifndef CO_LLCP_H_
#define CO_LLCP_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @addtogroup CO_BT
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "co_bt.h"

/*
 * DEFINES
 ****************************************************************************************
 */
#define LLCP_OPCODE_MASK      (0xFF)
#define LLCP_OPCODE_POS       (0)

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// Control packet op_code
/*@TRACE*/
enum co_llcp_op_code
{
    LL_CONNECTION_UPDATE_IND_OPCODE = 0x00,
    LL_CHANNEL_MAP_IND_OPCODE       = 0x01,
    LL_TERMINATE_IND_OPCODE         = 0x02,
    LL_ENC_REQ_OPCODE               = 0x03,
    LL_ENC_RSP_OPCODE               = 0x04,
    LL_START_ENC_REQ_OPCODE         = 0x05,
    LL_START_ENC_RSP_OPCODE         = 0x06,
    LL_UNKNOWN_RSP_OPCODE           = 0x07,
    LL_FEATURE_REQ_OPCODE           = 0x08,
    LL_FEATURE_RSP_OPCODE           = 0x09,
    LL_PAUSE_ENC_REQ_OPCODE         = 0x0A,
    LL_PAUSE_ENC_RSP_OPCODE         = 0x0B,
    LL_VERSION_IND_OPCODE           = 0x0C,
    LL_REJECT_IND_OPCODE            = 0x0D,
    LL_SLAVE_FEATURE_REQ_OPCODE     = 0x0E,
    LL_CONNECTION_PARAM_REQ_OPCODE  = 0x0F,
    LL_CONNECTION_PARAM_RSP_OPCODE  = 0x10,
    LL_REJECT_EXT_IND_OPCODE        = 0x11,
    LL_PING_REQ_OPCODE              = 0x12,
    LL_PING_RSP_OPCODE              = 0x13,
    LL_LENGTH_REQ_OPCODE            = 0x14,
    LL_LENGTH_RSP_OPCODE            = 0x15,
    LL_PHY_REQ_OPCODE               = 0x16,
    LL_PHY_RSP_OPCODE               = 0x17,
    LL_PHY_UPDATE_IND_OPCODE        = 0x18,
    LL_MIN_USED_CHANNELS_IND_OPCODE = 0x19,
    LL_CTE_REQ_OPCODE               = 0x1A,
    LL_CTE_RSP_OPCODE               = 0x1B,
    #if (BLE_PAST)
    LL_PER_SYNC_IND_OPCODE          = 0x1C,
    #endif // (BLE_PAST)
    LL_CLK_ACC_REQ_OPCODE           = 0x1D,
    LL_CLK_ACC_RSP_OPCODE           = 0x1E,

    #if (BLE_CIS)
    /// Request creation of CIS
    LL_CIS_REQ_OPCODE           = 0x1F,
    /// Response to CIS Request with Slave parameters
    LL_CIS_RSP_OPCODE           = 0x20,
    /// Issued by the Master to establish the CIS with a Slave.
    LL_CIS_IND_OPCODE           = 0x21,
    /// Terminate the CIS channel associated with the Stream_ID and Channel_ID fields
    LL_CIS_TERMINATE_IND_OPCODE = 0x22,
    #endif // (BLE_CIS)

    #if BLE_PWR_CTRL
    LL_PWR_CTRL_REQ_OPCODE          = 0x23,
    LL_PWR_CTRL_RSP_OPCODE          = 0x24,
    LL_PWR_CHANGE_IND_OPCODE        = 0x25,
    #endif // BLE_PWR_CTRL

    #if (BLE_SUBRATING)
    LL_SUBRATE_REQ_OPCODE           = 0x26,
    LL_SUBRATE_IND_OPCODE           = 0x27,
    #endif // (BLE_SUBRATING)
    #if (BT_53)
    LL_CH_REP_IND_OPCODE            = 0x28,
    LL_CH_STATUS_IND_OPCODE         = 0x29,
    #endif // (BT_53)

    #if (BLE_PAWR)
    LL_PER_SYNC_WR_IND_OPCODE       = 0x2A,
    #endif // (BLE_PAWR)

    #if (BT_60)
    LL_FEATURE_EXT_REQ_OPCODE       = 0x2B,
    LL_FEATURE_EXT_RSP_OPCODE       = 0x2C,
    #endif // (BT_60)

    #if (BLE_CHSD)
    LL_CS_SEC_RSP_OPCODE            = 0x2D,
    LL_CS_CAP_REQ_OPCODE            = 0x2E,
    LL_CS_CAP_RSP_OPCODE            = 0x2F,
    LL_CS_CONFIG_REQ_OPCODE         = 0x30,
    LL_CS_CONFIG_RSP_OPCODE         = 0x31,
    LL_CS_REQ_OPCODE                = 0x32,
    LL_CS_RSP_OPCODE                = 0x33,
    LL_CS_IND_OPCODE                = 0x34,
    LL_CS_TERMINATE_REQ_OPCODE      = 0x35,
    LL_CS_FAE_REQ_OPCODE            = 0x36,
    LL_CS_FAE_RSP_OPCODE            = 0x37,
    LL_CS_CH_MAP_IND_OPCODE         = 0x38,
    LL_CS_SEC_REQ_OPCODE            = 0x39,
    LL_CS_TERMINATE_RSP_OPCODE      = 0x3A,
    #endif //(BLE_CHSD)

    #if (BLE_FSU)
    LL_FRAME_SPACE_REQ_OPCODE       = 0x3B,
    LL_FRAME_SPACE_RSP_OPCODE       = 0x3C,
    #endif //(BLE_FSU)

    #if (BLE_ENH_TEST_MODE) || (BLE_UPPER_TESTER)
    LL_OTA_UTP_IND_OPCODE           = 0xED, // TBD
    #endif // (BLE_ENH_TEST_MODE) || (BLE_UPPER_TESTER)

    /// Opcode length
    LL_OPCODE_MAX_OPCODE,
    LL_OPCODE_DEBUG = 0xFF,
};

/// LLCP PDU lengths (including op_code)
enum co_llcp_length
{
    LL_CONNECTION_UPDATE_IND_LEN     = 12,
    LL_CHANNEL_MAP_IND_LEN           = 8,
    LL_TERMINATE_IND_LEN             = 2,
    LL_ENC_REQ_LEN                   = 23,
    LL_ENC_RSP_LEN                   = 13,
    LL_START_ENC_REQ_LEN             = 1,
    LL_START_ENC_RSP_LEN             = 1,
    LL_UNKNOWN_RSP_LEN               = 2,
    LL_FEATURE_REQ_LEN               = 9,
    LL_FEATURE_RSP_LEN               = 9,
    LL_PAUSE_ENC_REQ_LEN             = 1,
    LL_PAUSE_ENC_RSP_LEN             = 1,
    LL_VERSION_IND_LEN               = 6,
    LL_REJECT_IND_LEN                = 2,
    LL_SLAVE_FEATURE_REQ_LEN         = 9,
    LL_REJECT_EXT_IND_LEN            = 3,
    LL_CONNECTION_PARAM_REQ_LEN      = 24,
    LL_CONNECTION_PARAM_RSP_LEN      = 24,
    LL_PING_REQ_LEN                  = 1,
    LL_PING_RSP_LEN                  = 1,
    LL_LENGTH_REQ_LEN                = 9,
    LL_LENGTH_RSP_LEN                = 9,
    LL_PHY_REQ_LEN                   = 3,
    LL_PHY_RSP_LEN                   = 3,
    LL_PHY_UPDATE_IND_LEN            = 5,
    LL_MIN_USED_CHANNELS_IND_LEN     = 3,
    LL_CTE_REQ_LEN                   = 2,
    LL_CTE_RSP_LEN                   = 1,
    #if BLE_PAST
    LL_PER_SYNC_IND_LEN              = 35,
    #endif // BLE_PAST
    LL_CLK_ACC_REQ_LEN               = 2,
    LL_CLK_ACC_RSP_LEN               = 2,

    #if (BLE_CIS)
    LL_CIS_REQ_LEN                   = 36,
    LL_CIS_RSP_LEN                   = 9,
    LL_CIS_IND_LEN                   = 16,
    LL_CIS_TERMINATE_IND_LEN         = 4,
    #endif // (BLE_CIS)

    #if BLE_PWR_CTRL
    LL_PWR_CTRL_REQ_LEN              = 4,
    LL_PWR_CTRL_RSP_LEN              = 5,
    LL_PWR_CHANGE_IND_LEN            = 5,
    #endif // BLE_PWR_CTRL

    #if (BLE_SUBRATING)
    LL_SUBRATE_REQ_LEN               = 11,
    LL_SUBRATE_IND_LEN               = 11,
    #endif // (BLE_SUBRATING)
    #if (BT_53)
    LL_CH_REP_IND_LEN                = 4,
    LL_CH_STATUS_IND_LEN             = 11,
    #endif // (BT_53)

    #if (BLE_PAWR)
    LL_PER_SYNC_WR_IND_LEN           = 43,
    #endif // (BLE_PAWR)

    #if (BT_60)
    LL_FEATURE_EXT_REQ_LEN           = 27,
    LL_FEATURE_EXT_RSP_LEN           = 27,
    #endif // (BT_60)

    #if (BLE_CHSD)
    LL_CS_REQ_LEN                    = 29,
    LL_CS_RSP_LEN                    = 22,
    LL_CS_IND_LEN                    = 19,
    LL_CS_CAP_REQ_LEN                = 26,
    LL_CS_CAP_RSP_LEN                = 26,
    LL_CS_CONFIG_REQ_LEN             = 28,
    LL_CS_CONFIG_RSP_LEN             = 2,
    LL_CS_FAE_REQ_LEN                = 1,
    LL_CS_FAE_RSP_LEN                = 73,
    LL_CS_SEC_REQ_LEN                = 21,
    LL_CS_SEC_RSP_LEN                = 21,
    LL_CS_CH_MAP_IND_LEN             = 13,
    LL_CS_TERMINATE_REQ_LEN          = 5,
    LL_CS_TERMINATE_RSP_LEN          = 5,
    #endif //(BLE_CHSD)

    #if (BLE_FSU)
    LL_FRAME_SPACE_REQ_LEN           = 8,
    LL_FRAME_SPACE_RSP_LEN           = 6,
    #endif //(BLE_FSU)

    #if (BLE_CHSD)
    LL_PDU_LENGTH_MAX                = LL_CS_FAE_RSP_LEN,
    #elif (BLE_PAWR)
    LL_PDU_LENGTH_MAX                = LL_PER_SYNC_WR_IND_LEN,
    #elif (BLE_CIS)
    LL_PDU_LENGTH_MAX                = LL_CIS_REQ_LEN,
    #elif (BLE_PAST)
    LL_PDU_LENGTH_MAX                = LL_PER_SYNC_IND_LEN,
    #else // !(BLE_CIS || BLE_PAST)
    LL_PDU_LENGTH_MAX                = LE_MIN_OCTETS,
    #endif // !(BLE_CIS || BLE_PAST)

    #if (BLE_ENH_TEST_MODE) || (BLE_UPPER_TESTER)
    LL_OTA_UTP_IND_LEN               = LL_PDU_LENGTH_MAX, // Variable size, adapts to the maximum LLCP supported
    #endif // (BLE_ENH_TEST_MODE) || (BLE_UPPER_TESTER)
};


/// BIG Control packet op_code
/*@TRACE*/
enum co_big_op_code
{
    /// Channel map update
    BIG_CHANNEL_MAP_IND_OPCODE,
    /// Termination indication
    BIG_TERMINATE_IND_OPCODE,

    /// Opcode length
    BIG_OPCODE_MAX_OPCODE,
};

/// BIG PDU lengths (including op_code)
enum co_bis_length
{
    BIG_CHANNEL_MAP_IND_LEN           = 8,
    BIG_TERMINATE_IND_LEN             = 4,

    BIG_PDU_LENGTH_MAX                = 8
};


/// PDU lengths
enum co_pdu_length
{
    PDU_SCAN_REQ_LEN          = 12,
    PDU_CON_REQ_LEN           = 34,
    PDU_CON_RSP_LEN           = 14,
};

/// Bit field definitions for BN field sent over the air
///    7    6    5    4    3    2    1    0
/// +----+----+----+----+----+----+----+----+
/// |     BN_S2M     |     BN_M2S     |
/// +----+----+----+----+----+----+----+----+
enum co_llcp_cis_bn_field
{
    /// BN Master to Slave
    LLCP_CIS_BN_M2S_LSB     = 0,
    LLCP_CIS_BN_M2S_MASK    = (0x0F),

    /// BN Slave to Master
    LLCP_CIS_BN_S2M_LSB     = 4,
    LLCP_CIS_BN_S2M_MASK    = (0xF0),
};

/// Bit field definitions for combination of advertising Set ID, advertiser address type and sleep clock accuracy in LL_PER_SYNC_IND
///    7    6    5     4     3    2    1    0
/// +----+----+----+-------+----+----+----+----+
/// |      SCA     | AType |        SID        |
/// +----+----+----+-------+----+----+----+----+
enum co_llcp_sid_atype_sca
{
    /// Advertising SID
    LLCP_PER_SYNC_IND_SID_LSB     = 0,
    LLCP_PER_SYNC_IND_SID_MASK    = (0x0F),

    /// Advertiser address type
    LLCP_PER_SYNC_IND_ATYPE_POS   = 4,
    LLCP_PER_SYNC_IND_ATYPE_BIT  = (0x10),

    /// Sleep clock accuracy (@see enum SCA)
    LLCP_PER_SYNC_IND_SCA_LSB     = 5,
    LLCP_PER_SYNC_IND_SCA_MASK    = (0xE0),
};

#if (BLE_CHSD)
/// Bit field definitions for combination of Num Ant and Max Ant Path in LL_CS_CAPABILITIES_REQ/RSP
///    7    6    5     4     3    2    1    0
/// +----+----+----+----+----+----+----+----+
/// |    Max Ant Path   |      Num Ant      |
/// +----+----+----+----+----+----+----+----+
enum co_llcp_chsd_cap_ant
{
    /// Number of antenna elements available for CS tone exchanges
    LLCP_CHSD_CAP_NUM_ANT_LSB       = 0,
    LLCP_CHSD_CAP_NUM_ANT_MASK      = (0x0F),
    /// Maximum number of antenna paths supported for CS tone exchanges
    LLCP_CHSD_CAP_MAX_ANT_PATH_LSB  = (4),
    LLCP_CHSD_CAP_MAX_ANT_PATH_MASK = (0xF0),
};

/// Bit field definitions for role and sub-features in LL_CS_CAPABILITIES_REQ/RSP
///    7    6    5     4     3    2    1    0
/// +----+----+----+----+----+----+----+----+
/// |   RFU   |PCT |#3C |FAE |RFU |  Role   |
/// +----+----+----+----+----+----+----+----+
enum co_llcp_chsd_cap_role_subfeat
{
    /// Supported roles see #chsd_opt_role_bf enumeration
    LLCP_CHSD_CAP_ROLE_LSB          = 0,
    LLCP_CHSD_CAP_ROLE_MASK         = (0x03),
    /// Support of only an FAE of 0 for mode0 transmissions in reflector role
    LLCP_CHSD_CAP_NO_FAE_POS        = 3,
    LLCP_CHSD_CAP_NO_FAE_BIT        = (0x08),
    /// Support channel selection algorithm #3c
    LLCP_CHSD_CAP_CHSEL_3C_POS      = 4,
    LLCP_CHSD_CAP_CHSEL_3C_BIT      = (0x10),
    /// Support of PCT estimates for RTT sounding sequences
    LLCP_CHSD_CAP_PCT_POS           = 5,
    LLCP_CHSD_CAP_PCT_BIT           = (0x20),
};

/// Bit field definitions for TX SNR capabilities in LL_CS_CAPABILITIES_REQ/RSP
///    7    6    5     4     3    2    1    0
/// +----+----+----+----+----+----+----+----+
/// |        TX SNR Capabilities       |RFU |
/// +----+----+----+----+----+----+----+----+
enum co_llcp_chsd_cap_tx_snr
{
    /// Supported optional levels of TX SNR control for modulated transmissions
    LLCP_CHSD_CAP_TX_SNR_LSB        = 1,
    LLCP_CHSD_CAP_TX_SNR_MASK       = (0xFE),
};

/// Bit field definitions for combination of Config ID and Action in LL_CS_CONFIG_REQ
///    7    6    5     4     3    2    1    0
/// +----+----+----+----+----+----+----+----+
/// |  Action |          Config ID          |
/// +----+----+----+----+----+----+----+----+
enum co_llcp_chsd_cfg_id_action
{
    /// Config ID (from 0 to 3)
    LLCP_CHSD_CFG_ID_LSB        = 0,
    LLCP_CHSD_CFG_ID_MASK       = (0x3F),

    /// Action (see #chsd_action enumeration)
    LLCP_CHSD_ACTION_LSB        = 6,
    LLCP_CHSD_ACTION_MASK       = (0xC0),
};

/// Bit field definitions for combination of RTT Type and Role in LL_CS_CONFIG_REQ
///    7    6    5     4     3    2    1    0
/// +----+----+----+----+----+----+----+----+
/// |   RFU   |   Role  |      RTT Type     |
/// +----+----+----+----+----+----+----+----+
enum co_llcp_chsd_rtt_type_role
{
    /// RTT Type (see BLE_CHSD_RTT definitions)
    LLCP_CHSD_RTT_TYPE_LSB      = 0,
    LLCP_CHSD_RTT_TYPE_MASK     = (0xF),

    /// Role (see #chsd_role enumeration)
    LLCP_CHSD_ROLE_LSB          = 4,
    LLCP_CHSD_ROLE_MASK         = (0x30),
};

/// Bit field definitions for combination of ChSel and Ch3cShape in LL_CS_CONFIG_REQ
///    7    6    5     4     3    2    1    0
/// +----+----+----+----+----+----+----+----+
/// |      Ch3cShape    |       ChSel       |
/// +----+----+----+----+----+----+----+----+
enum co_llcp_chsd_chsel_shape
{
    /// Channel Selection Algorithm (see BLE_CHSD_CHNL_SELECT definitions)
    LLCP_CHSD_CHSEL_LSB         = 0,
    LLCP_CHSD_CHSEL_MASK        = (0xF),

    /// Ch3c Shape (see BLE_CHSD_CH3C_SHAPE definitions)
    LLCP_CHSD_CH3CSHAPE_LSB     = 4,
    LLCP_CHSD_CH3CSHAPE_MASK    = (0xF0),
};

#endif // (BLE_CHSD)

/*
 * MESSAGES
 ****************************************************************************************
 */

/// LL_DEFAULT structure (for packet with no parameter)
/*@TRACE
 * @NO_PAD
*/
struct  ll_default
{
    /// op_code
    uint8_t             op_code;
};

/// LL_CONNECTION_UPDATE_IND structure.
/*@TRACE
 * @NO_PAD
*/
struct  ll_connection_update_ind
{
    /// op_code
    uint8_t         op_code;
    /// window size (units of 1.25 ms)
    uint8_t         win_size;
    /// window offset (units of 1.25 ms)
    uint16_t        win_off;
    /// interval (units of 1.25 ms)
    uint16_t        interv;
    /// connection latency (unit of connection event)
    uint16_t        latency;
    /// link supervision timeout (unit of 10 ms)
    uint16_t        timeout;
    /// instant (unit of connection event)
    uint16_t        instant;
};

/// LL_CHANNEL_MAP_IND structure.
/*@TRACE
 * @NO_PAD
*/
struct  ll_channel_map_ind
{
    /// op_code
    uint8_t            op_code;
    /// channel mapping
    struct le_ch_map   ch_map;
    /// instant
    uint16_t           instant;
};

/// LL_TERMINATE_IND structure.
/*@TRACE
 * @NO_PAD
*/
struct  ll_terminate_ind
{
    /// op_code
    uint8_t         op_code;
    /// termination code
    uint8_t         err_code;
};

/// LL_ENC_REQ structure.
/*@TRACE
 * @NO_PAD
*/
struct  ll_enc_req
{
    /// op_code
    uint8_t               op_code;
    /// random value
    struct rand_nb        rand_val;
    /// ediv
    uint16_t              ediv;
    /// skdm
    struct sess_k_div_x   skdm;
    /// ivm
    struct init_vect      ivm;
};

/// LL_ENC_RSP structure.
/*@TRACE
 * @NO_PAD
*/
struct  ll_enc_rsp
{
    /// op_code
    uint8_t             op_code;
    /// skds
    struct sess_k_div_x   skds;
    /// ivs
    struct init_vect    ivs;
};

/// LL_START_ENC_REQ structure.
/*@TRACE
 * @NO_PAD
*/
struct  ll_start_enc_req
{
    /// op_code
    uint8_t             op_code;
};

/// LL_START_ENC_RSP structure.
/*@TRACE
 * @NO_PAD
*/
struct  ll_start_enc_rsp
{
    /// op_code
    uint8_t             op_code;
};

/// LL_UNKNOWN_RSP structure.
/*@TRACE
 * @NO_PAD
*/
struct  ll_unknown_rsp
{
    /// op_code
    uint8_t         op_code;
    /// unknown type
    uint8_t         unk_type;
};

/// LL_FEATURE_REQ structure.
/*@TRACE
 * @NO_PAD
*/
struct  ll_feature_req
{
    /// op_code
    uint8_t             op_code;
    /// le features
    le_features_t       feats;
};


/// LL_FEATURE_RSP structure.
/*@TRACE
 * @NO_PAD
*/
struct  ll_feature_rsp
{
    /// op_code
    uint8_t             op_code;
    /// le features
    le_features_t       feats;
};

/// LL_PAUSE_ENC_REQ structure.
/*@TRACE
 * @NO_PAD
*/
struct  ll_pause_enc_req
{
    /// op_code
    uint8_t             op_code;
};

/// LL_PAUSE_ENC_RSP structure.
/*@TRACE
 * @NO_PAD
*/
struct  ll_pause_enc_rsp
{
    /// op_code
    uint8_t             op_code;
};

/// LL_VERSION_IND structure
/*@TRACE
 * @NO_PAD
*/
struct ll_version_ind
{
    /// op_code
    uint8_t     op_code;
    /// version
    uint8_t     vers;
    /// company id
    uint16_t    compid;
    /// sub version
    uint16_t    subvers;
};

/// LL_REJECT_IND structure.
/*@TRACE
 * @NO_PAD
*/
struct  ll_reject_ind
{
    /// op_code
    uint8_t         op_code;
    /// reject reason
    uint8_t         err_code;
};

/// LL_SLAVE_FEATURE_REQ structure.
/*@TRACE
 * @NO_PAD
*/
struct  ll_slave_feature_req
{
    /// op_code
    uint8_t             op_code;
    /// le features
    le_features_t       feats;
};

/// LL_CONNECTION_PARAM_REQ structure.
/*@TRACE
 * @NO_PAD
*/
struct  ll_connection_param_req
{
    /// op_code
    uint8_t         op_code;
    /// minimum value of connInterval (units of 1.25 ms)
    uint16_t        interval_min;
    /// maximum value of connInterval (units of 1.25 ms)
    uint16_t        interval_max;
    /// connSlaveLatency value (unit of connection event)
    uint16_t        latency;
    /// connSupervisionTimeout value (unit of 10 ms)
    uint16_t        timeout;
    /// preferred periodicity (units of 1.25 ms)
    uint8_t         pref_period;
    /// ReferenceConnEventCount (unit of connection event)
    uint16_t        ref_con_event_count;
    /// Offset0 (units of 1.25 ms)
    uint16_t        offset0;
    /// Offset1 (units of 1.25 ms)
    uint16_t        offset1;
    /// Offset2 (units of 1.25 ms)
    uint16_t        offset2;
    /// Offset3 (units of 1.25 ms)
    uint16_t        offset3;
    /// Offset4 (units of 1.25 ms)
    uint16_t        offset4;
    /// Offset5 (units of 1.25 ms)
    uint16_t        offset5;
};

/// LL_CONNECTION_PARAM_RSP structure.
/*@TRACE
 * @NO_PAD
*/
struct  ll_connection_param_rsp
{
    /// op_code
    uint8_t         op_code;
    /// minimum value of connInterval (units of 1.25 ms)
    uint16_t        interval_min;
    /// maximum value of connInterval (units of 1.25 ms)
    uint16_t        interval_max;
    /// connSlaveLatency value (unit of connection event)
    uint16_t        latency;
    /// connSupervisionTimeout value (unit of 10 ms)
    uint16_t        timeout;
    /// preferred periodicity (units of 1.25 ms)
    uint8_t         pref_period;
    /// ReferenceConnEventCount (unit of connection event)
    uint16_t        ref_con_event_count;
    /// Offset0 (units of 1.25 ms)
    uint16_t        offset0;
    /// Offset1 (units of 1.25 ms)
    uint16_t        offset1;
    /// Offset2 (units of 1.25 ms)
    uint16_t        offset2;
    /// Offset3 (units of 1.25 ms)
    uint16_t        offset3;
    /// Offset4 (units of 1.25 ms)
    uint16_t        offset4;
    /// Offset5 (units of 1.25 ms)
    uint16_t        offset5;
};

/// LL_REJECT_EXT_IND structure.
/*@TRACE
 * @NO_PAD
*/
struct  ll_reject_ext_ind
{
    /// op_code
    uint8_t         op_code;
    /// rejected op_code
    uint8_t         rej_op_code;
    /// reject reason
    uint8_t         err_code;
};

/// LL_PING_REQ structure.
/*@TRACE
 * @NO_PAD
*/
struct  ll_ping_req
{
    /// op_code
    uint8_t         op_code;
};

/// LL_PING_RSP structure.
/*@TRACE
 * @NO_PAD
*/
struct  ll_ping_rsp
{
    /// op_code
    uint8_t         op_code;
};

/// LL_LENGTH_REQ structure.
/*@TRACE
 * @NO_PAD
*/
struct  ll_length_req
{
    /// op_code
    uint8_t     op_code;
    /// The max size in reception (unit of byte)
    uint16_t    max_rx_octets;
    /// The max time in reception (unit of microsecond)
    uint16_t    max_rx_time;
    /// The max size in transmission (unit of byte)
    uint16_t    max_tx_octets;
    /// The max time in transmission (unit of microsecond)
    uint16_t    max_tx_time;
};

/// LL_LENGTH_RSP structure.
/*@TRACE
 * @NO_PAD
*/
struct  ll_length_rsp
{
    /// op_code
    uint8_t     op_code;
    /// The max size in reception (unit of byte)
    uint16_t    max_rx_octets;
    /// The max time in reception (unit of microsecond)
    uint16_t    max_rx_time;
    /// The max size in transmission (unit of byte)
    uint16_t    max_tx_octets;
    /// The max time in transmission (unit of microsecond)
    uint16_t    max_tx_time;
};
/// LL_PHY_REQ structure.
/*@TRACE
 * @NO_PAD
*/
struct  ll_phy_req
{
    /// op_code
    uint8_t    op_code;
    /// Tx phy selection
    uint8_t    tx_phys;
    /// Rx phy selection
    uint8_t    rx_phys;
};

/// LL_PHY_RSP structure.
/*@TRACE
 * @NO_PAD
*/
struct  ll_phy_rsp
{
    /// op_code
    uint8_t    op_code;
    /// Tx phy selection
    uint8_t    tx_phys;
    /// Rx phy selection
    uint8_t    rx_phys;
};

/// LL_PHY_UPDATE_IND structure.
/*@TRACE
 * @NO_PAD
*/
struct  ll_phy_update_ind
{
    /// op_code
    uint8_t    op_code;
    /// master to slave phy selected
    uint8_t    m_to_s_phy;
    /// slave to master phy selected
    uint8_t    s_to_m_phy;
    /// Instant
    uint16_t   instant;
};

/// LL_MIN_USED_CHANNELS_IND structure.
/*@TRACE
 * @NO_PAD
*/
struct  ll_min_used_channels_ind
{
    /// op_code
    uint8_t    op_code;
    /// PHY(s) for which the slave has a minimum number of used channels requirement
    uint8_t    phys;
    /// minimum number of channels to be used on the specified PHY
    uint8_t    min_used_ch;
};

/**
 * CTE length and type bit field definition
 *
 * |     5 bits     |  1b |   2 bits    |
 * |  MinCTELenReq  | RFU | CTETypeReq  |
 *
 *   - MinCTELenReq: minimum CTE length (x 8us)
 *   - CTETypeReq: 0-AoA, 1-AoD_1us, 2-AoD_2us, 3-RFU
 */
enum cte_len_type
{
    CTE_TYPE_REQ_MASK       = 0xC0,
    CTE_TYPE_REQ_LSB        = 6,

    MIN_CTE_LEN_REQ_MASK    = 0x1F,
    MIN_CTE_LEN_REQ_LSB     = 0,
};

/// LL_CTE_REQ structure.
/*@TRACE
 * @NO_PAD
*/
struct ll_cte_req
{
    /// op_code
    uint8_t    op_code;
    /// CTE length and type (@see enum cte_len_type)
    uint8_t    cte_len_type;
};

#if (BLE_PAST)
/// LL_PER_SYNC_IND structure.
/*@TRACE
 * @NO_PAD
*/
struct ll_per_sync_ind
{
    /// op_code
    uint8_t        op_code;
    /// ID (provided by the Host)
    uint16_t       id;
    /// Sync Info (Vol 6, Part B, 2.3.4.6.SyncInfo field)
    uint8_t        sync_info[18];
    /// Connection event counter
    uint16_t       con_evt_cnt;
    /// Last periodic advertising event counter
    uint16_t       last_pa_evt_cnt;
    /// Advertising Set ID, advertiser address type and sleep clock accuracy
    uint8_t        sid_atype_sca;
    /// Periodic advertising PHY (@see enum le_phy_mask)
    uint8_t        phy;
    /// Periodic advertiser address
    struct bd_addr adva;
    /// Sync Connection event counter
    uint16_t       sync_con_evt_cnt;
};
#endif // (BLE_PAST)

#if (BLE_PAWR)
/// LL_PER_SYNC_WR_IND structure.
/*@TRACE
 * @NO_PAD
*/
struct ll_per_sync_wr_ind
{
    /// Periodic Sync common components
    struct ll_per_sync_ind per_sync;
    /// PAwR timing info
    struct pawr_timing_info pawr;
};
#endif // (BLE_PAWR)

/// LL_CLK_ACC_REQ structure.
/*@TRACE
 * @NO_PAD
*/
struct ll_clk_acc_req
{
    /// op_code
    uint8_t    op_code;
    /// SCA (@see enum SCA)
    uint8_t    sca;
};

/// LL_CLK_ACC_RSP structure.
/*@TRACE
 * @NO_PAD
*/
struct ll_clk_acc_rsp
{
    /// op_code
    uint8_t    op_code;
    /// SCA (@see enum SCA)
    uint8_t    sca;
};

#if (BLE_CIS)
/// LL_CIS_REQ structure.
/*@TRACE
 * @NO_PAD
*/
struct  ll_cis_req
{
    /// op_code
    uint8_t         op_code;
    /// Identifier of the Connected Isochronous Group
    uint8_t         cig_id;
    /// Identifier of the Connected Isochronous Stream
    uint8_t         cis_id;
    /// Physical Channel Info Master to Slave PHY
    uint8_t         phy_m2s;
    /// Physical Channel Info Slave to Master PHY
    uint8_t         phy_s2m;
    /// bit[0:11] shall be set to the maximum SDU size in octets from the master Host.
    #if (BLE_EISOAL)
    /// bit[14] Framing_Mode shall be set to 0 if Segmentable mode is being used and to 1 if Unsegmented mode is being used
    #endif // (BLE_EISOAL)
    /// bit[15] Framed shall be 0 if the CIS uses unframed data PDUs and 1 if it uses framed PDUs
    uint16_t        max_sdu_m2s;
    /// bit[0:11] shall be set to the maximum SDU size in octets from the slave Host.
    uint16_t        max_sdu_s2m;
    /// bit[0:19] shall be set to the time in microseconds between two consecutive SDUs from the master Host.
    uint32_t        sdu_interval_m2s;
    /// bit[0:19] Shall be set to the time in microseconds between two consecutive SDUs from the slave Host.
    uint32_t        sdu_interval_s2m;
    /// Max Payload Size: Master to Slave (value between 0 and 251) (in bytes)
    uint16_t         max_pdu_m2s;
    /// Max Payload Size: Slave to Master (value between 0 and 251) (in bytes)
    uint16_t         max_pdu_s2m;
    /// Number of Subevent (value between 1 and 31)
    uint8_t         nse;
    /// bit[0:24] Sub interval in microseconds (Range 400us - ISO Interval)
    uint32_t        sub_interval;
    /// Burst number: Number of new packet per channel interval
    /// bit[0:3] : Master to Slave Burst number The range shall be 0 to 15
    /// bit[4:7] : Slave to Master Burst number The range shall be 0 to 15
    uint8_t         bn;
    /// Master to Slave Flush timeout (value between 1 and 255)
    uint8_t         ft_m2s;
    /// Slave to Master Flush timeout (value between 1 and 255)
    uint8_t         ft_s2m;
    /// Time between two consecutive Connected Isochronous Stream anchor points - 1.25ms step.
    /// The Iso_Interval field shall be between 4 and 3200 (i.e. 5ms to 4s).
    uint16_t        iso_interval;
    /// bit[0:23] Minimum time in microseconds between the start of the connection event with the
    /// connection event counter = connEventCount and the first CIS anchor point.
    /// It shall have a value between 500µs and connInterval
    uint32_t        cis_offset_min;
    /// bit[0:23] Maximum time in microseconds between the start of the connection event with the
    /// connection event counter = connEventCount and the first CIS anchor point.
    /// It shall have a value between CIS_Offset_Min and connInterval.
    uint32_t        cis_offset_max;

    /// Reference point where the Offset_Min and Offset_Max are applied to determine the first CIS anchor point.
    /// It be set to a connection event counter value that meets the requirement
    /// currEvent - 2^14 < connEventCount < currEvent + 2^14 (modulo 65536), where currEvent is the counter value for
    /// the connection event where the LL_CIS_REQ PDU is being transmitted (or retransmitted).
    uint16_t        conn_event_cnt;
};

/// CIS Request Bit Field parameters
enum ll_cis_req_bf
{
    #if (BLE_EISOAL)
    /// bit[14] - Framing mode in max_sdu_m2s parameter
    BLE_CIS_FRAMING_MODE_BIT  = 0x4000,
    BLE_CIS_FRAMING_MODE_POS  = 14,
    #endif // (BLE_EISOAL)

    /// bit[15] - Frame mode in max_sdu_m2s parameter
    BLE_CIS_FRAME_MODE_BIT    = 0x8000,
    BLE_CIS_FRAME_MODE_POS    = 15,

    /// bit[0:11] - Maximum SDU size
    BLE_CIS_MAX_SDU_MASK      = 0x0FFF,
    BLE_CIS_MAX_SDU_LSB       = 0,

    /// bit[0:19] - SDU Interval
    BLE_CIS_SDU_INTERVAL_MASK = 0x0FFFFF,
    BLE_CIS_SDU_INTERVAL_LSB  = 0,
};




/// LL_CIS_RSP structure.
/*@TRACE
 * @NO_PAD
*/
struct  ll_cis_rsp
{
    /// op_code
    uint8_t         op_code;
    /// bit[0:23] Minimum time in microseconds between the start of the connection event with the
    /// connection event counter = connEventCount and the first CIS anchor point.
    /// It shall have a value between 400µs and connInterval
    uint32_t        cis_offset_min;
    /// bit[0:23] Maximum time in microseconds between the start of the connection event with the
    /// connection event counter = connEventCount and the first CIS anchor point.
    /// It shall have a value between CIS_Offset_Min and connInterval.
    uint32_t        cis_offset_max;
    /// Event counter of the ACL event from which the Ch_Offset_Min and the Ch_Offset_Max are referenced
    uint16_t        conn_event_cnt;
};

/// LL_CIS_UPDATE_IND structure.
/*@TRACE
 * @NO_PAD
*/
struct  ll_cis_ind
{
    /// op_code
    uint8_t         op_code;
    /// Access Address field is the number used to identify the CIS channel.
    uint32_t        aa;
    /// bit[0:23] Time in microseconds from the start of the ACL connection event referenced by the connEventCount to the first
    /// CIS anchor point.
    uint32_t        cis_offset;
    /// bit[0:23] Time in microseconds from the first anchor point of the Stream being set up to the CIG Synchronization point
    /// in the same isochronous interval. The CIS_Synchronization_Delay is calculated by the master for each CIS
    /// separately, and sent to the slave in the LL_CIS_IND when establishing the CIS.
    /// The slave uses the CIS_Synchronization_Delay parameter to determine the CIG Synchronization point.
    uint32_t        cig_sync_delay;
    /// bit[0:23] Time reference when all CIS events are completed. The CIG Synchronization point is at the end of the last
    /// subevent of the last CIS in the CIG in an Iso_Interval. The CIG_Synchronization_Delay is the time in
    /// microseconds between the start of the first subevent of the first CIS in the CIG and the
    /// CIG Synchronization point
    uint32_t        cis_sync_delay;
    /// Reference point where the CIS_Offset is applied to determine the first CIS anchor point.
    /// The connEventCount shall be set to a connection event counter value that meets the requirement
    /// currEvent 2^14 < connEventCount < currEvent + 2^14 (modulo 65536), where currEvent is the counter value
    /// for the connection event where the LL_CIS_IND PDU is being transmitted (or retransmitted).
    uint16_t        conn_event_cnt;
};

/// LL_CIS_TERMINATE_IND structure.
/*@TRACE
 * @NO_PAD
*/
struct  ll_cis_terminate_ind
{
    /// op_code
    uint8_t         op_code;
    /// Identifier of the Connected Isochronous Group
    uint8_t         cig_id;
    /// Identifier of the Connected Isochronous Stream
    uint8_t         cis_id;
    /// Reason for CIS termination
    uint8_t         err_code;
};
#endif // (BLE_CIS)

#if BLE_PWR_CTRL
/// LL_PWR_CTRL_REQ structure.
/*@TRACE
* @NO_PAD
*/
struct ll_pwr_ctrl_req
{
    /// op_code
    uint8_t         op_code;
    /// PHY mask for which the request is being made
    uint8_t         phy_mask;
    ///The requested change in the recipiant's transmit power level (in dBs)
    int8_t          delta;
    /// The senders transmit power level for the PHY indicated (in dBs)
    int8_t          tx_pwr;
};

/// LL_PWR_CTRL_RSP structure.
/*@TRACE
* @NO_PAD
*/
struct ll_pwr_ctrl_rsp
{
    /// op_code
    uint8_t         op_code;
    /// bf (@see enum pwr_ctrl_flags)
    uint8_t         flags;
    /// The actual change in the sender's transmit power level (in dBs)
    int8_t          delta;
    /// The senders transmit power level for the PHY indicated (in dBs)
    int8_t          tx_pwr;
    /// The maximum acceptable power reduction of peer device
    uint8_t         apr;
};

/// LL_PWR_CHANGE_IND structure.
/*@TRACE
* @NO_PAD
*/
struct ll_pwr_change_ind
{
    /// op_code
    uint8_t         op_code;
    /// PHY(s) mask for which the power level has changed
    uint8_t         phy_mask;
    /// bf (@see enum pwr_ctrl_flags)
    uint8_t         flags;
    /// The change in the sender's transmit power level (in dBs)
    int8_t          delta;
    /// The senders transmit power level for the PHY(s) indicated (in dBs)
    int8_t         tx_pwr;
};
#endif // BLE_PWR_CTRL

#if (BLE_SUBRATING)
/// LL_SUBRATE_REQ structure.
/*@TRACE
* @NO_PAD
*/
struct ll_subrate_req
{
    /// op_code
    uint8_t     op_code;
    /// Subrate Factor Min (unit of connection event)
    uint16_t    subrate_min;
    /// Subrate Factor Max (unit of connection event)
    uint16_t    subrate_max;
    /// Max Latency  (unit of connection event)
    uint16_t    max_latency;
    /// Continuation Number (unit of connection event)
    uint16_t    cont_num;
    /// Timeout (unit of 10ms)
    uint16_t    timeout;
};

/// LL_SUBRATE_IND structure.
/*@TRACE
* @NO_PAD
*/
struct ll_subrate_ind
{
    /// op_code
    uint8_t     op_code;
    /// Subrate Factor (unit of connection event)
    uint16_t    subrate;
    /// Subrate Base Event (unit of connection event)
    uint16_t    base_evt;
    /// Latency  (unit of connection event)
    uint16_t    latency;
    /// Continuation Number (unit of connection event)
    uint16_t    cont_num;
    /// Timeout (unit of 10ms)
    uint16_t    timeout;
};
#endif // (BLE_SUBRATING)
#if (BT_53)
/// LL_CH_REP_IND structure.
/*@TRACE
* @NO_PAD
*/
struct ll_ch_rep_ind
{
    /// op_code
    uint8_t         op_code;
    /// Enable
    uint8_t         en;
    /// Min_Spacing in units of 200 ms
    uint8_t         min_spacing;
    /// Max_Delay in units of 200 ms
    uint8_t         max_delay;
};

/// LL_CH_STATUS_IND structure.
/*@TRACE
* @NO_PAD
*/
struct ll_ch_status_ind
{
    /// op_code
    uint8_t         op_code;
    /// Channel classification map
    uint8_t         ch_class_map[10];
};
#endif // (BT_53)

#if (BT_60)
/// LL_FEATURE_EXT_REQ structure.
/*@TRACE
* @NO_PAD
*/
typedef struct ll_feat_ext_req
{
    /// op_code
    uint8_t     op_code;
    /// Max page
    uint8_t     max_page;
    /// Page number
    uint8_t     page_nb;
    /// Feature page
    le_ext_features_t feats;
} ll_feat_ext_req_t;

/// LL_FEATURE_EXT_RSP structure.
/*@TRACE
* @NO_PAD
*/
typedef ll_feat_ext_req_t ll_feat_ext_rsp_t;
#endif // (BT_60)

#if (BLE_CHSD)
/// LL_CS_REQ structure.
/*@TRACE
* @NO_PAD
*/
typedef struct
{
    /// op_code
    uint8_t     op_code;
    /// Configuration ID
    uint8_t     config_id;
    /// Connection event count
    uint16_t    con_evt_cnt;
    /// Offset minimum (in us)
    uint32_t    offset_min;
    /// Offset maximum (in us)
    uint32_t    offset_max;
    /// Max procedure length (unit of 625 us)
    uint16_t    max_proc_len;
    /// Event interval (unit of connection intervals)
    uint16_t    evt_intv;
    /// Subevents per event
    uint8_t     subevt_per_evt;
    /// Subevent interval (unit of 625 us)
    uint16_t    subevt_intv;
    /// Subevent length (in us)
    uint32_t    subevt_len;
    /// Procedure interval (unit of connection intervals)
    uint16_t    proc_intv;
    /// Procedure count
    uint16_t    proc_cnt;
    /// Preferred ACI (antenna configuration index 0-7)
    uint8_t     aci;
    /// Preferred peer antenna (bit0-bit3 for ant0-ant3)
    uint8_t     pref_peer_ant_bf;
    /// PHY (see #le_phy_pwr_mask enumeration)
    uint8_t     phy;
    /// Power Delta (in dB)
    int8_t      pwr_delta;
    /// TX SNR (index_i 0-4, index_r 0-4)
    uint8_t     tx_snr_i_r;
} ll_cs_req_t;

/// LL_CS_RSP structure.
/*@TRACE
* @NO_PAD
*/
typedef struct
{
    /// op_code
    uint8_t     op_code;
    /// Configuration ID
    uint8_t     config_id;
    /// Connection event count
    uint16_t    con_evt_cnt;
    /// Offset minimum (in us)
    uint32_t    offset_min;
    /// Offset maximum (in us)
    uint32_t    offset_max;
    /// Event interval (unit of connection intervals)
    uint16_t    evt_intv;
    /// Subevents per event
    uint8_t     subevt_per_evt;
    /// Subevent interval (unit of 625 us)
    uint16_t    subevt_intv;
    /// Subevent length (in us)
    uint32_t    subevt_len;
    /// Preferred ACI (antenna configuration index 0-7)
    uint8_t     aci;
    /// PHY (see #le_phy_pwr_mask enumeration)
    uint8_t     phy;
    /// Power Delta (unit of dB)
    int8_t      pwr_delta;
    /// RFU
    uint8_t     rfu;
} ll_cs_rsp_t;

/// LL_CS_IND structure.
/*@TRACE
* @NO_PAD
*/
typedef struct
{
    /// op_code
    uint8_t     op_code;
    /// Configuration ID
    uint8_t     config_id;
    /// Connection event count
    uint16_t    con_evt_cnt;
    /// Offset  (in us)
    uint32_t    offset;
    /// Event interval (unit of connection intervals)
    uint16_t    evt_intv;
    /// Subevents per event
    uint8_t     subevt_per_evt;
    /// Subevent interval (unit of 625 us)
    uint16_t    subevt_intv;
    /// Subevent length (in us)
    uint32_t    subevt_len;
    /// ACI (antenna configuration index 0-7)
    uint8_t     aci;
    /// PHY (see #le_phy_pwr_mask enumeration)
    uint8_t     phy;
    /// Power delta (unit of dB)
    int8_t      pwr_delta;
    /// RFU
    uint8_t     rfu;
} ll_cs_ind_t;

/// LL_CS_CAP_REQ structure.
/*@TRACE
* @NO_PAD
*/
typedef struct
{
    /// op_code
    uint8_t     op_code;
    /// Optional CS mode types supported (see #chsd_opt_mode_bf enumeration)
    uint8_t     mode_types_bf;
    /// RTT capability (see #chsd_opt_rtt_precision_bf enumeration)
    uint8_t     rtt_capa_bf;
    /// RTT AA only N
    uint8_t     rtt_aa_only_n;
    /// RTT sounding N
    uint8_t     rtt_sounding_n;
    /// RTT random sequence N
    uint8_t     rtt_random_sequence_n;
    /// NADM sounding capability (see #chsd_opt_nadm_sseq_cap_bf enumeration)
    uint16_t    nadm_sounding;
    /// NADM random sequence capability (see #chsd_opt_nadm_rand_cap_bf enumeration)
    uint16_t    nadm_random_sequence;
    /// CS SYNC PHY capability (see #chsd_opt_phy_cap_bf)
    uint8_t     cs_sync_phy;
    /// Number of antenna and maximum antenna paths fields (see #co_llcp_chsd_cap_ant enumeration)
    uint8_t     ant;
    /// Role and sub-feature field (see #co_llcp_chsd_cap_role_subfeat enumeration)
    uint8_t     role_subfeat;
    /// Number of configurations supported (value 1-4)
    uint8_t     num_config;
    /// Max procedures supported (max procedure repetitions, or 0 for no limit)
    uint16_t    max_procedures;
    /// T_SW capability (in us)
    uint8_t     t_sw_us;
    /// T_IP1 capability (see #chsd_opt_t_ip_bf enumeration)
    uint16_t    t_ip1_times_bf;
    /// T_IP2 capability (see #chsd_opt_t_ip_bf enumeration)
    uint16_t    t_ip2_times_bf;
    /// T_FCS capability (see #chsd_opt_t_fcs_bf enumeration)
    uint16_t    t_fcs_times_bf;
    /// T_PM capability (see #chsd_opt_t_pm_bf enumeration)
    uint16_t    t_pm_times_bf;
    /// TX SNR capability (see #co_llcp_chsd_cap_tx_snr enumeration)
    uint8_t     tx_snr_bf;
} ll_cs_cap_req_t;

/// LL_CS_CAP_RSP structure.
/*@TRACE
* @NO_PAD
*/
typedef ll_cs_cap_req_t ll_cs_cap_rsp_t;

/// LL_CS_CONFIG_REQ structure.
/*@TRACE
* @NO_PAD
*/
typedef struct
{
    /// op_code
    uint8_t     op_code;
    /// Configuration ID and Action fields (see #co_llcp_chd_cfg_id_action enumeration)
    uint8_t     config_id_action;
    /// Channel classification map
    uint8_t     ch_class_map[LE_CH_CLASS_MAP_LEN];
    /// Channel map repetition (number of repetitions, from 1 to 0xff)
    uint8_t     channel_map_repetition;
    /// Main mode (see BLE_CHSD_MODE1/2/3 definitions)
    uint8_t     main_mode;
    /// Sub mode (see BLE_CHSD_MODE1/2/3/UNUSED definitions)
    uint8_t     sub_mode;
    /// Main mode min steps (number of steps, from 1 to main_mode_max_steps)
    uint8_t     main_mode_min_steps;
    /// Main mode max steps (number of steps, from main_mode_min_steps to N_STEPS_MAX-1)
    uint8_t     main_mode_max_steps;
    /// Main mode repetition (number of steps, from 0 to 3)
    uint8_t     main_mode_repetition;
    /// Mode0 steps (number of steps, from 1 to 3)
    uint8_t     mode0_steps;
    /// CS SYNC PHY bit field (see #le_phy_mask enumeration)
    uint8_t     cs_sync_phy_bf;
    /// RTT Type and Role (see #co_llcp_chsd_rtt_type_role enumeration)
    uint8_t     rtt_type_and_role;
    /// ChSel, CH3cShape (see #co_llcp_chsd_chsel_shape enumeration)
    uint8_t     chsel_shape;
    /// CH3cJump value (number of channels skipped in each rising and falling sequence, from 2 to 8)
    uint8_t     ch3cjump;
    /// T_IP1 index, of duration between the RTT packets (see #llm_chsd_t_ip1_times definition)
    uint8_t     t_ip1_idx;
    /// T_IP2 index, of duration between the CS tones (see #llm_chsd_t_ip2_times definition)
    uint8_t     t_ip2_idx;
    /// T_FCS index, of duration for frequency changes (see #llm_chsd_t_fcs_times definition)
    uint8_t     t_fcs_idx;
    /// T_PM index, of duration for the phase measurement period of CS tones (see #llm_chsd_t_pm_times definition)
    uint8_t     t_pm_idx;
    /// RFU
    uint8_t     rfu;
} ll_cs_config_req_t;

/// LL_CS_CONFIG_RSP structure.
/*@TRACE
* @NO_PAD
*/
typedef struct
{
    /// op_code
    uint8_t     op_code;
    /// Configuration ID and RFU field (see #co_llcp_chd_cfg_id_action enumeration)
    uint8_t     config_id;
} ll_cs_config_rsp_t;

/// LL_CS_FAE_REQ structure.
/*@TRACE
* @NO_PAD
*/
typedef struct
{
    /// op_code
    uint8_t     op_code;
} ll_cs_fae_req_t;

/// LL_CS_FAE_RSP structure.
/*@TRACE
* @NO_PAD
*/
typedef struct
{
    /// op_code
    uint8_t     op_code;
    /// Per-channel mode-0 FAE table
    int8_t      ch_fae[BLE_CHSD_NB_CHANNEL_MAX];
} ll_cs_fae_rsp_t;

/// LL_CS_SEC_REQ structure.
/*@TRACE
* @NO_PAD
*/
typedef struct
{
    /// op_code
    uint8_t     op_code;
    /// CS_IV_C - Central's portion of initialization vector
    uint8_t     cs_iv_c[BLE_CHSD_IV_C_LEN];
    /// CS_IN_C - Central's portion of initialization nonce
    uint8_t     cs_in_c[BLE_CHSD_IN_C_LEN];
    /// CS_PV_C - Central's portion of personalization vector
    uint8_t     cs_pv_c[BLE_CHSD_PV_C_LEN];
} ll_cs_sec_req_t;

/// LL_CS_SEC_RSP structure.
/*@TRACE
* @NO_PAD
*/
typedef struct
{
    /// op_code
    uint8_t     op_code;
    /// CS_IV_C - Peripheral's portion of initialization vector
    uint8_t     cs_iv_p[BLE_CHSD_IV_P_LEN];
    /// CS_IN_C - Peripheral's portion of initialization nonce
    uint8_t     cs_in_p[BLE_CHSD_IN_P_LEN];
    /// CS_PV_C - Peripheral's portion of personalization vector
    uint8_t     cs_pv_p[BLE_CHSD_PV_P_LEN];
} ll_cs_sec_rsp_t;

/// LL_CS_CH_MAP_IND structure.
/*@TRACE
* @NO_PAD
*/
typedef struct
{
    /// op_code
    uint8_t     op_code;
    /// Channel classification map
    uint8_t     ch_map[LE_CH_CLASS_MAP_LEN];
    /// Instant (unit of connection event)
    uint16_t    instant;
} ll_cs_ch_map_ind_t;

/// LL_CS_TERMINATE_REQ structure.
/*@TRACE
* @NO_PAD
*/
typedef struct
{
    /// op_code
    uint8_t     op_code;
    /// Configuration ID and RFU field (see #co_llcp_chd_cfg_id_action enumeration)
    uint8_t     config_id;
    /// Procedure count
    uint16_t    proc_cnt;
    /// Error code (@see enum co_error)
    uint8_t     err_code;
} ll_cs_terminate_req_t;

/// LL_CS_TERMINATE_RSP structure.
/*@TRACE
* @NO_PAD
*/
typedef struct
{
    /// op_code
    uint8_t     op_code;
    /// Configuration ID and RFU field (see #co_llcp_chd_cfg_id_action enumeration)
    uint8_t     config_id;
    /// Procedure count
    uint16_t    proc_cnt;
    /// Error code (@see enum co_error)
    uint8_t     err_code;
} ll_cs_terminate_rsp_t;

#endif //(BLE_CHSD)

#if (BLE_FSU)
/// LL_FRAME_SPACE_REQ structure.
/*@TRACE
* @NO_PAD
*/
typedef struct
{
    /// op_code
    uint8_t     op_code;
    /// Frame Space Min  (in us, range: 0x0000 to 0x2710)
    uint16_t    fs_min;
    /// Frame Space Max  (in us, range: 0x0000 to 0x2710)
    uint16_t    fs_max;
    /// PHYS (bit field, see enum le_phy_mask)
    uint8_t     phys;
    /// Spacing_Types (bit field, see enum fsu_spacing_types)
    uint16_t    spacing_types;
} ll_frame_space_req_t;

/// LL_FRAME_SPACE_RSP structure.
/*@TRACE
* @NO_PAD
*/
typedef struct
{
    /// op_code
    uint8_t     op_code;
    /// Frame Space  (in us, range: 0x0000 to 0x2710)
    uint16_t    fs;
    /// PHYS (bit field, see enum le_phy_mask)
    uint8_t     phys;
    /// Spacing_Types (bit field, see enum fsu_spacing_types)
    uint16_t    spacing_types;
} ll_frame_space_rsp_t;
#endif //(BLE_FSU)

#if (BLE_ENH_TEST_MODE) || (BLE_UPPER_TESTER)
/// LL_OTA_UTP_IND structure.
/*@TRACE
* @NO_PAD
*/
struct ll_ota_utp_ind
{
    /// op_code
    uint8_t         op_code;

    /// Length of the data (in bytes) (note: not present in the actual PDU data)
    uint8_t         length;

    /// Data
    uint8_t         data[LL_PDU_LENGTH_MAX];
};
#endif // (BLE_ENH_TEST_MODE) || (BLE_UPPER_TESTER)


/// LLCP pdu format
/*@TRACE
 @trc_ref co_llcp_op_code
 */
union llcp_pdu
{
    /// op_code
    uint8_t  op_code;

    //@trc_union op_code == LL_CONNECTION_UPDATE_IND_OPCODE
    struct ll_connection_update_ind     con_update_ind;

    //@trc_union op_code == LL_CHANNEL_MAP_IND_OPCODE
    struct ll_channel_map_ind           channel_map_ind;

    //@trc_union op_code == LL_TERMINATE_IND_OPCODE
    struct ll_terminate_ind             terminate_ind;

    //@trc_union op_code == LL_ENC_REQ_OPCODE
    struct ll_enc_req                   enc_req;

    //@trc_union op_code == LL_ENC_RSP_OPCODE
    struct ll_enc_rsp                   enc_rsp;

    //@trc_union op_code == LL_START_ENC_REQ_OPCODE
    struct ll_start_enc_req             start_enc_req;

    //@trc_union op_code == LL_START_ENC_RSP_OPCODE
    struct ll_start_enc_rsp             start_enc_rsp;

    //@trc_union op_code == LL_UNKNOWN_RSP_OPCODE
    struct ll_unknown_rsp               unknown_rsp;

    //@trc_union op_code == LL_FEATURE_REQ_OPCODE
    struct ll_feature_req               feats_req;

    //@trc_union op_code == LL_FEATURE_RSP_OPCODE
    struct ll_feature_rsp               feats_rsp;

    //@trc_union op_code == LL_PAUSE_ENC_REQ_OPCODE
    struct ll_pause_enc_req             pause_enc_req;

    //@trc_union op_code == LL_PAUSE_ENC_RSP_OPCODE
    struct ll_pause_enc_rsp             pause_enc_rsp;

    //@trc_union op_code == LL_VERSION_IND_OPCODE
    struct ll_version_ind               vers_ind;

    //@trc_union op_code == LL_REJECT_IND_OPCODE
    struct ll_reject_ind                reject_ind;

    //@trc_union op_code == LL_SLAVE_FEATURE_REQ_OPCODE
    struct ll_slave_feature_req         slave_feature_req;

    //@trc_union op_code == LL_CONNECTION_PARAM_REQ_OPCODE
    struct ll_connection_param_req      con_param_req;

    //@trc_union op_code == LL_CONNECTION_PARAM_RSP_OPCODE
    struct ll_connection_param_rsp      con_param_rsp;

    //@trc_union op_code == LL_REJECT_EXT_IND_OPCODE
    struct ll_reject_ext_ind            reject_ind_ext;

    //@trc_union op_code == LL_PING_REQ_OPCODE
    struct ll_ping_req                  ping_req;

    //@trc_union op_code == LL_PING_RSP_OPCODE
    struct ll_ping_rsp                  ping_rsp;

    //@trc_union op_code == LL_LENGTH_REQ_OPCODE
    struct ll_length_req                length_req;

    //@trc_union op_code == LL_LENGTH_RSP_OPCODE
    struct ll_length_rsp                length_rsp;

    //@trc_union op_code == LL_PHY_REQ_OPCODE
    struct ll_phy_req                   phy_req;

    //@trc_union op_code == LL_PHY_RSP_OPCODE
    struct ll_phy_rsp                   phy_rsp;

    //@trc_union op_code == LL_PHY_UPDATE_IND_OPCODE
    struct ll_phy_update_ind            phy_upd_ind;

    //@trc_union op_code == LL_MIN_USED_CHANNELS_IND_OPCODE
    struct ll_min_used_channels_ind     min_used_channels_ind;

    //@trc_union op_code == LL_CLK_ACC_REQ_OPCODE
    struct ll_clk_acc_req               clk_acc_req;

    //@trc_union op_code == LL_CLK_ACC_RSP_OPCODE
    struct ll_clk_acc_rsp               clk_acc_rsp;

    #if (BLE_PAST)
    //@trc_union op_code == LL_PER_SYNC_IND_OPCODE
    struct ll_per_sync_ind              per_sync_ind;
    #endif // (BLE_PAST)

    #if (BLE_CIS)
    //@trc_union op_code == LL_CIS_REQ_OPCODE
    struct  ll_cis_req                  cis_req;

    //@trc_union op_code == LL_CIS_RSP_OPCODE
    struct  ll_cis_rsp                  cis_rsp;

    //@trc_union op_code == LL_CIS_IND_OPCODE
    struct  ll_cis_ind                  cis_ind;

    //@trc_union op_code == LL_CIS_TERMINATE_IND_OPCODE
    struct  ll_cis_terminate_ind        cis_terminate_ind;
    #endif // (BLE_CIS)

    #if BLE_PWR_CTRL
    //@trc_union op_code == LL_PWR_CTRL_REQ_OPCODE
    struct ll_pwr_ctrl_req              pwr_ctrl_req;

    //@trc_union op_code == LL_PWR_CTRL_RSP_OPCODE
    struct ll_pwr_ctrl_rsp              pwr_ctrl_rsp;

    //@trc_union op_code == LL_PWR_CHANGE_IND_OPCODE
    struct ll_pwr_change_ind            pwr_change_ind;
    #endif // BLE_PWR_CTRL

    #if (BLE_SUBRATING)
    //@trc_union op_code == LL_SUBRATE_REQ_OPCODE
    struct ll_subrate_req                subrate_req;

    //@trc_union op_code == LL_SUBRATE_IND_OPCODE
    struct ll_subrate_ind                subrate_ind;
    #endif // (BLE_SUBRATING)
    #if (BT_53)
    //@trc_union op_code == LL_CH_REP_IND_OPCODE
    struct ll_ch_rep_ind                ch_rep_ind;

    //@trc_union op_code == LL_CH_STATUS_IND_OPCODE
    struct ll_ch_status_ind             ch_status_ind;
    #endif // (BT_53)

    #if (BLE_PAWR)
    //@trc_union op_code == LL_PER_SYNC_WR_IND_OPCODE
    struct ll_per_sync_wr_ind           per_sync_wr_ind;
    #endif // (BLE_PAWR)

    #if (BT_60)
    //@trc_union op_code == LL_FEATURE_EXT_REQ_OPCODE
    ll_feat_ext_req_t                   feat_ext_req;

    //@trc_union op_code == LL_FEATURE_EXT_RSP_OPCODE
    ll_feat_ext_rsp_t                   feat_ext_rsp;
    #endif // (BT_60)

    #if (BLE_CHSD)
    //@trc_union op_code == LL_CS_REQ_OPCODE
    ll_cs_req_t                         cs_res;

    //@trc_union op_code == LL_CS_RSP_OPCODE
    ll_cs_rsp_t                         cs_rsp;

    //@trc_union op_code == LL_CS_IND_OPCODE
    ll_cs_ind_t                         cs_ind;

    //@trc_union op_code == LL_CS_CAP_REQ_OPCODE
    ll_cs_cap_req_t                     cs_cap_req;

    //@trc_union op_code == LL_CS_CAP_RSP_OPCODE
    ll_cs_cap_rsp_t                     cs_cap_rsp;

    //@trc_union op_code == LL_CS_CONFIG_REQ_OPCODE
    ll_cs_config_req_t                  cs_config_req;

    //@trc_union op_code == LL_CS_CONFIG_RSP_OPCODE
    ll_cs_config_rsp_t                  cs_config_rsp;

    //@trc_union op_code == LL_CS_FAE_REQ_OPCODE
    ll_cs_fae_req_t                     cs_fae_req;

    //@trc_union op_code == LL_CS_FAE_REQ_OPCODE
    ll_cs_fae_rsp_t                     cs_fae_rsp;

    //@trc_union op_code == LL_CS_SEC_REQ_OPCODE
    ll_cs_sec_req_t                     cs_sec_req;

    //@trc_union op_code == LL_CS_SEC_RSP_OPCODE
    ll_cs_sec_rsp_t                     cs_sec_rsp;

    //@trc_union op_code == LL_CS_CH_MAP_IND_OPCODE
    ll_cs_ch_map_ind_t                  cs_ch_map_ind;

    //@trc_union op_code == LL_CS_TERMINATE_REQ_OPCODE
    ll_cs_terminate_req_t               cs_terminate_req;

    //@trc_union op_code == LL_CS_TERMINATE_RSP_OPCODE
    ll_cs_terminate_rsp_t               cs_terminate_rsp;
    #endif //(BLE_CHSD)

    #if (BLE_FSU)
    //@trc_union op_code == LL_FRAME_SPACE_REQ_OPCODE
    ll_frame_space_req_t                frame_space_req;

    //@trc_union op_code == LL_FRAME_SPACE_RSP_OPCODE
    ll_frame_space_rsp_t                frame_space_rsp;
    #endif //(BLE_FSU)

    #if (BLE_ENH_TEST_MODE) || (BLE_UPPER_TESTER)
    //@trc_union op_code == LL_OTA_UTP_IND_OPCODE
    struct ll_ota_utp_ind               ota_utp_ind;
    #endif // (BLE_ENH_TEST_MODE) || (BLE_UPPER_TESTER)
};


/// BIG_CHANNEL_MAP_IND structure.
/*@TRACE
 * @NO_PAD
*/
struct  big_channel_map_ind
{
    /// op_code
    uint8_t            op_code;
    /// channel mapping
    struct le_ch_map   ch_map;
    /// BIG Event Count (Instant)
    uint16_t           evt_cnt;
};

/// BIG_TERMINATE_IND structure.
/*@TRACE
 * @NO_PAD
*/
struct  big_terminate_ind
{
    /// op_code
    uint8_t         op_code;
    /// termination code
    uint8_t         err_code;
    /// BIG Event Count (Instant)
    uint16_t        evt_cnt;
};


/// BIS PDU format
/*@TRACE
 @trc_ref co_big_op_code
 */
union big_pdu
{
    /// op_code
    uint8_t  op_code;

    //@trc_union op_code == BIG_CHANNEL_MAP_IND_OPCODE
    struct big_channel_map_ind channel_map_ind;

    //@trc_union op_code == BIG_TERMINATE_IND_OPCODE
    struct big_terminate_ind   terminate_ind;
};

/// @} CO_BT
#endif // CO_LLCP_H_
