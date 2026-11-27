/**
 ****************************************************************************************
 *
 * @file l2cap.h
 *
 * @brief Header file - L2CAP Native API.
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 ****************************************************************************************
 */

#ifndef L2CAP_H_
#define L2CAP_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup L2CAP_API Logical Link Control and Adaptation Protocol (L2CAP)
 * @brief  The L2CAP module is responsible for SDU and PDU exchange over a BLE link,
 *         and Connection oriented channel management.
 *
 * @defgroup L2CAP_COMMON_API Common
 * @ingroup L2CAP_API
 * @brief Common Types, enumerations and functions API
 *
 * @defgroup L2CAP_COMMON_DEFINE_API Defines / Macros
 * @ingroup L2CAP_COMMON_API
 *
 * @defgroup L2CAP_COMMON_ENUM_API Enumerations
 * @ingroup L2CAP_COMMON_API
 *
 * @defgroup L2CAP_CHANNEL_API Channels
 * @ingroup L2CAP_API
 * @brief L2CAP channels management
 *
 * @defgroup L2CAP_CHANNEL_SDU_API SDU Transmission
 * @ingroup L2CAP_CHANNEL_API
 * @brief Functions that handles SDU transmission
 *
 * @defgroup L2CAP_COC_CHANNEL_API Connection Oriented Channel
 * @ingroup L2CAP_CHANNEL_API
 * @brief L2CAP channel with Credit Based Flow control channels
 *
 * @defgroup L2CAP_VS_CHANNEL_API Vendor specific
 * @ingroup L2CAP_CHANNEL_API
 * @brief Manage vendor specific L2CAP channel that convey B-Frames
 *
 * @defgroup L2CAP_VS_CHANNEL_CALLBACK_API Callback
 * @ingroup L2CAP_VS_CHANNEL_API
 *
 * @defgroup L2CAP_VS_CHANNEL_MGMT_API Channel management
 * @ingroup L2CAP_VS_CHANNEL_API
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "compiler.h"
#include <stdbool.h>
#include "co_buf.h"

/*
 * DEFINES
 ****************************************************************************************
 */

/// @addtogroup L2CAP_COMMON_DEFINE_API
/// @{

/// Invalid L2CAP channel local index
#define L2CAP_INVALID_CHAN_LID (0xFF)

/// Buffer Header length that must be reserved for L2CAP processing
#define L2CAP_BUFFER_HEADER_LEN (0)
/// Buffer Tail length that must be reserved for L2CAP processing
#define L2CAP_BUFFER_TAIL_LEN   (1)

/// Minimal LE MTU value
#define L2CAP_LE_MTU_MIN        (23)

/// @} L2CAP_COMMON_DEFINE_API

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// @addtogroup L2CAP_COMMON_ENUM_API
/// @{

/// L2CAP channels id
/*@TRACE*/
enum l2cap_cid
{
    /// Reserved channel id
    L2CAP_CID_RESERVED = 0x0000,
    /// Attribute channel id
    L2CAP_CID_ATTRIBUTE = 0x0004,
    /// Signaling channel id
    L2CAP_CID_LE_SIGNALING = 0x0005,
    /// Security channel id
    L2CAP_CID_SECURITY = 0x0006,
    /// BR/EDR Security channel id
    L2CAP_CID_BR_EDR_SECURITY = 0x0007,
    /// Dynamically allocated minimum range
    L2CAP_CID_DYN_MIN = 0x0040,
    /// Dynamically allocated maximum range
    L2CAP_CID_DYN_MAX = 0x007F,
};

/// L2CAP Channel Type
enum l2cap_chan_type
{
    /// Channel with a dynamically allocated L2CAP channel identifier
     L2CAP_CHAN_DYNAMIC = 0,
     /// Channel with a fixed L2cap channel identifier
     L2CAP_CHAN_FIX = 1,
};

/// L2CAP channel parameter update Bit Field
enum  l2cap_chan_upd_bf
{
    /// Update Channel enable state
    L2CAP_CHAN_UP_EN_STATE_BIT = 0x01,
    L2CAP_CHAN_UP_EN_STATE_POS = 0,

    /// Update TX Paused state
    L2CAP_CHAN_UP_TX_PAUSED_STATE_BIT = 0x02,
    L2CAP_CHAN_UP_TX_PAUSED_STATE_POS = 1,

    /// Update Credit Flow control state
    L2CAP_CHAN_UP_CREDIT_FLOW_EN_STATE_BIT = 0x04,
    L2CAP_CHAN_UP_CREDIT_FLOW_EN_STATE_POS = 2,

    /// Update reception MTU
    L2CAP_CHAN_UP_RX_MTU_BIT = 0x10,
    L2CAP_CHAN_UP_RX_MTU_POS = 4,

    /// Update transmission MTU
    L2CAP_CHAN_UP_TX_MTU_BIT = 0x20,
    L2CAP_CHAN_UP_TX_MTU_POS = 5,

    /// Update reception MPS
    L2CAP_CHAN_UP_RX_MPS_BIT = 0x40,
    L2CAP_CHAN_UP_RX_MPS_POS = 6,

    /// Update transmission MPS
    L2CAP_CHAN_UP_TX_MPS_BIT = 0x80,
    L2CAP_CHAN_UP_TX_MPS_POS = 7,
};

/// L2CAP debug segment Bit Field
enum  l2cap_dbg_bf
{
    /// Start segment with L2CAP header present
    /// Can be used for invalid behaviors or to change the L2CAP destination channel identifier
    L2CAP_DBG_SEG_HEADER_PRESENT_BIT = 0x01,
    L2CAP_DBG_SEG_HEADER_PRESENT_POS = 0,

    /// Continuation of segment transmission
    L2CAP_DBG_SEG_CONTINUE_BIT = 0x02,
    L2CAP_DBG_SEG_CONTINUE_POS = 1,
};
/// @} L2CAP_COMMON_ENUM_API

/*
 * CALLBACK DEFINITION
 ****************************************************************************************
 */

/// @addtogroup L2CAP_VS_CHANNEL_CALLBACK_API
/// @{

/// L2CAP Channel callback set
typedef struct
{
    /**
     ****************************************************************************************
     * @brief The received SDU buffer must be acquired by upper application module before
     *        function return.
     *        When SDU process is done, the corresponding SDU buffer must be release to
     *        allocate new reception credits onto a L2CAP dynamic channel.
     *
     * @param[in] conidx    Connection Index
     * @param[in] chan_lid  Connected L2CAP channel local index
     * @param[in] status    Reception status
     * @param[in] p_sdu     Buffer that contains SDU data
     ****************************************************************************************
     */
    void (*cb_sdu_rx)(uint8_t conidx, uint8_t chan_lid, uint16_t status, co_buf_t* p_sdu);

    /**
     ****************************************************************************************
     * @brief Function called when SDU has been transmitted or if an error occurs
     *
     * @param[in] conidx    Connection Index
     * @param[in] metainfo  Metadata information provided by API user (see \glos{METAINFO})
     * @param[in] chan_lid  L2CAP channel local index
     * @param[in] status    Status of the operation (see #hl_err enumeration)
     * @param[in] p_sdu     Pointer to SDU transmitted
     ****************************************************************************************
     */
    void (*cb_sdu_sent)(uint8_t conidx, uint16_t metainfo, uint8_t chan_lid, uint16_t status, co_buf_t* p_sdu);
} l2cap_chan_cb_t;
/// @} L2CAP_VS_CHANNEL_CALLBACK_API

/*
 * FUCTIONS DEFINITIONS
 ****************************************************************************************
 */

/// @addtogroup L2CAP_VS_CHANNEL_MGMT_API
/// @{

/**
 ****************************************************************************************
 * @brief Internal function used to register a fixed L2CAP channel such as ATT, Signaling or SMP.
 *        This channel does not support  segmentation and reassembly mechanism.
 *
 * @param[in]  conidx       Connection Index
 * @param[in]  cid          L2CAP Channel Identifier for both reception and transmission
 * @param[in]  mtu          Maximum Transmit Unit Size for both reception and transmission
 * @param[in]  p_cb         Pointer to set of callback functions to be used for communication with the Upper Layer.
 * @param[out] p_chan_lid   Pointer to L2CAP Channel local index
 *
 * @return Status of the function execution (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t l2cap_chan_fix_register(uint8_t conidx, uint16_t cid, uint16_t mtu, const l2cap_chan_cb_t* p_cb,
                                 uint8_t* p_chan_lid);

/**
 ****************************************************************************************
 * @brief Internal function used to unregister a dynamic or a fixed channel.
 *        After sending this command, it's no more possible to receive or send SDUs onto
 *        the channel.
 *
 * @param[in]  conidx       Connection Index
 * @param[in]  chan_lid     L2CAP Channel local index
 *
 * @return Status of the function execution (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t l2cap_chan_unregister(uint8_t conidx, uint8_t chan_lid);

/**
 ****************************************************************************************
 * @brief Internal function used to update the L2CAP Fix Channel MTU
 *
 * @param[in]  conidx       Connection Index
 * @param[in]  chan_lid     Channel Local identifier
 * @param[in]  mtu          Maximum Transmit Unit Size for both reception and transmission
 *
 * @return Status of the function execution (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t l2cap_chan_fix_mtu_update(uint8_t conidx, uint8_t chan_lid, uint16_t mtu);
/// @} L2CAP_VS_CHANNEL_MGMT_API

/// @addtogroup L2CAP_CHANNEL_SDU_API
/// @{

/**
 ****************************************************************************************
 * @brief Function used to send a SDU onto a specific L2CAP fixed or dynamic channel.
 *
 * @note Upper layer shall wait for #l2cap_chan_coc_cb_t.cb_sdu_sent callback before sending other
 *       message to prevent buffer overflow on the device.
 *
 * @note Minimum buffer head: #L2CAP_BUFFER_HEADER_LEN,
 *       and minimum buffer tail: #L2CAP_BUFFER_TAIL_LEN
 *
 * In debug mode, only L2CAP segment can be transmitted. This can be start segment or continuation segment.
 * L2CAP length and channel ID (CID) fields must be present in SDU data.
 *
 * @param[in] conidx        Connection Index
 * @param[in] metainfo      Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] chan_lid      L2CAP Channel local index
 * @param[in] p_sdu         Buffer that contains SDU data
 *
 * @return Status of the function execution (see #hl_err enumeration)
 *         Consider status only if an error occurs; else wait for execution completion
 ****************************************************************************************
 */
uint16_t l2cap_chan_sdu_send(uint8_t conidx, uint16_t metainfo, uint8_t chan_lid, co_buf_t* p_sdu);

#if (RW_DEBUG)
/**
 ****************************************************************************************
 * @brief DEBUG Function: used to send any kind of ACL PDU
 *        Only L2CAP segment can be transmitted. This can be start segment or continuation segment.
 *        L2CAP length and channel ID (CID) fields must be present in SDU data.
 *
 * @note Upper layer shall wait for #l2cap_chan_coc_cb_t.cb_sdu_sent callback before sending other
 *       message to prevent buffer overflow on the device.
 *
 * @note Minimum buffer head: #L2CAP_BUFFER_HEADER_LEN,
 *       and minimum buffer tail: #L2CAP_BUFFER_TAIL_LEN
 *
 * @param[in] conidx        Connection Index
 * @param[in] metainfo      Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] chan_lid      L2CAP Channel local index
 * @param[in] dbg_bf        Debug bit field (see #l2cap_dbg_bf enumeration)
 * @param[in] p_sdu         Buffer that contains SDU data
 *
 * @return Status of the function execution (see #hl_err enumeration)
 *         Consider status only if an error occurs; else wait for execution completion
 ****************************************************************************************
 */
uint16_t l2cap_chan_debug_sdu_send(uint8_t conidx, uint16_t metainfo, uint8_t chan_lid, uint8_t dbg_bf,
                                   co_buf_t* p_sdu);
#endif // (RW_DEBUG)

/// @} L2CAP_CHANNEL_SDU_API

#endif // L2CAP_H_
