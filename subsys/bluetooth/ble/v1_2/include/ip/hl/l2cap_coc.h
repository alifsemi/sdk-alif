/**
 ****************************************************************************************
 *
 * @file l2cap.h
 *
 * @brief Header file - L2CAP Native API - Connection Oriented Channel.
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 ****************************************************************************************
 */

#ifndef L2CAP_COC_H_
#define L2CAP_COC_H_

#include "rom_build_cfg.h"

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "l2cap.h"

/**
 ****************************************************************************************
 * @defgroup L2CAP_COC_CHANNEL_CALLBACK_API Callback
 * @ingroup L2CAP_COC_CHANNEL_API
 *
 * @defgroup L2CAP_COC_CHANNEL_SPSM_API SPSM Registration
 * @ingroup L2CAP_COC_CHANNEL_API
 * @brief Simplified Protocol/Service Multiplexer registration
 *
 * @defgroup L2CAP_COC_CHANNEL_MGMT_API Management
 * @ingroup L2CAP_COC_CHANNEL_API
 * @brief Establish, terminate or reconfigure L2CAP COC channel
 ****************************************************************************************
 */

/*
 * DEFINES
 ****************************************************************************************
 */

/// @addtogroup L2CAP_COMMON_DEFINE_API
/// @{

/// Minimal L2CAP COC MTU / MPS value
#define L2CAP_COC_MTU_MIN       (64)

/// Parameter to set in nb_chan of #l2cap_coc_connect_cfm to reject connection due to not enough authorization
#define L2CAP_COC_NOT_AUTORIZED (0xFF)

/// @} L2CAP_COMMON_DEFINE_API

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// @addtogroup L2CAP_COMMON_ENUM_API
/// @{

/// L2CAP assigned SPSM
/*@TRACE*/
enum l2cap_spsm_nb
{
    /// Attribute
    L2CAP_SPSM_ATT = 0x0027,
    /// Internet Protocol Support  Profile
    L2CAP_SPSM_IPSP = 0x0023,
    /// Object Transfer Service
    L2CAP_SPSM_OTS = 0x0025,
};

/// L2CAP security level Bit Field
enum l2cap_sec_lvl_bf
{
    /// With encrypted security requirement also requires a 128-bit encryption key size.
    L2CAP_COC_EKS_BIT = 0x01,
    L2CAP_COC_EKS_POS = 0,

    /// Channel minimum required security level (see #gap_sec_lvl enumeration)
    L2CAP_COC_SEC_LVL_MASK = 0x06,
    L2CAP_COC_SEC_LVL_LSB = 1,
};
/// @} L2CAP_COMMON_ENUM_API

/*
 * CALLBACK DEFINITION
 ****************************************************************************************
 */

/// @addtogroup L2CAP_COC_CHANNEL_CALLBACK_API
/// @{

/// L2CAP Connection Oriented Channel callback set
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

    /**
     ****************************************************************************************
     * @brief Function called when Connection Oriented Channel creation operation is finished
     *
     * @param[in] conidx    Connection Index
     * @param[in] metainfo  Metadata information provided by API user (see \glos{METAINFO})
     * @param[in] status    Status of the operation (see #hl_err enumeration)
     * @param[in] nb_chan   Number of L2CAP channel created.
     ****************************************************************************************
     */
    void (*cb_coc_create_cmp)(uint8_t conidx, uint16_t metainfo, uint16_t status, uint8_t nb_chan);

    /**
     ****************************************************************************************
     * @brief Function called when a new Connection Oriented Channel is created
     *
     * @param[in] conidx       Connection Index
     * @param[in] metainfo     Metadata information provided by API user (see \glos{METAINFO})
     * @param[in] chan_lid     Connected L2CAP channel local index
     * @param[in] local_rx_mtu Local device Maximum Transmit Unit reception size
     * @param[in] peer_rx_mtu  Peer device Maximum Transmit Unit reception size
     ****************************************************************************************
     */
    void (*cb_coc_created)(uint8_t conidx, uint16_t metainfo, uint8_t chan_lid, uint16_t local_rx_mtu,
                           uint16_t peer_rx_mtu);

    #if (EATT_SUPPORT)
    /**
     ****************************************************************************************
     * @brief Function called when Reconfigure L2CAP channel MTU is terminated
     *
     * @param[in] conidx    Connection Index
     * @param[in] metainfo  Metadata information provided by API user (see \glos{METAINFO})
     * @param[in] status    Status of the operation (see #hl_err enumeration)
     ****************************************************************************************
     */
    void (*cb_coc_reconfigure_cmp)(uint8_t conidx, uint16_t metainfo, uint16_t status);

    /**
     ****************************************************************************************
     * @brief Function called when Local or Peer RX MTU size has been changed onto the L2CAP channel
     *
     * @param[in] conidx       Connection Index
     * @param[in] metainfo     Metadata information provided by API user (see \glos{METAINFO})
     * @param[in] chan_lid     L2CAP channel local index
     * @param[in] local_rx_mtu Local device Maximum Transmit Unit reception size
     * @param[in] peer_rx_mtu  Peer device Maximum Transmit Unit reception size
     ****************************************************************************************
     */
    void (*cb_coc_mtu_changed)(uint8_t conidx, uint16_t metainfo, uint8_t chan_lid, uint16_t local_rx_mtu,
                               uint16_t peer_rx_mtu);
    #endif // (EATT_SUPPORT)

    /**
     ****************************************************************************************
     * @brief Function called when a Connection Oriented Channel is terminated
     *
     * @param[in] conidx    Connection Index
     * @param[in] metainfo  Metadata information provided by API user (see \glos{METAINFO})
     * @param[in] chan_lid  L2CAP channel local index
     * @param[in] reason    Termination Reason (see #hl_err enumeration)
     ****************************************************************************************
     */
    void (*cb_coc_terminated)(uint8_t conidx, uint16_t metainfo, uint8_t chan_lid, uint16_t reason);

    /**
     ****************************************************************************************
     * @brief Function called when Connection Oriented Channel Termination operation is finished
     *
     * @param[in] conidx    Connection Index
     * @param[in] metainfo  Metadata information provided by API user (see \glos{METAINFO})
     * @param[in] chan_lid  Connected L2CAP channel local index
     * @param[in] status    Status of the operation (see #hl_err enumeration)
     ****************************************************************************************
     */
    void (*cb_coc_terminate_cmp)(uint8_t conidx, uint16_t metainfo, uint8_t chan_lid, uint16_t status);
} l2cap_chan_coc_cb_t;


/// @} L2CAP_COC_CHANNEL_CALLBACK_API

/// @addtogroup L2CAP_COC_CHANNEL_SPSM_API
/// @{

/// L2CAP SPSM callback set
typedef struct
{
    /**
     ****************************************************************************************
     * @brief Function call when peer device requests to create several Connection oriented
     *        channels
     *
     *        In response of this function, the upper layer application must call
     *        #l2cap_coc_connect_cfm function
     *
     * @param[in] conidx      Connection Index
     * @param[in] token       Token provided by L2CAP module that must be reused in confirmation function
     * @param[in] nb_chan     Number of L2CAP channel requested to be created in parallel
     * @param[in] spsm        Simplified Protocol/Service Multiplexer
     * @param[in] peer_rx_mtu Peer device Maximum Transmit Unit reception size
     ****************************************************************************************
     */
    void (*cb_coc_connect_req)(uint8_t conidx, uint16_t token, uint8_t nb_chan, uint16_t spsm, uint16_t peer_rx_mtu);
} l2cap_coc_spsm_cb_t;

/// @} L2CAP_COC_CHANNEL_SPSM_API
/*
 * FUCTIONS DEFINITIONS
 ****************************************************************************************
 */

/// @addtogroup L2CAP_COC_CHANNEL_SPSM_API
/// @{

/**
 ****************************************************************************************
 * @brief Command used to register a Simplified Protocol/Service Multiplexer in order to accept
 *        L2CAP connection oriented channel (COC) from a peer device. Security level
 *        parameter ensures that a certain level of security is met before requesting
 *        application to accept or reject channel establishment.
 *
 * @param[in] spsm       Simplified Protocol/Service Multiplexer
 * @param[in] sec_lvl_bf Security level bit field (see #l2cap_sec_lvl_bf enumeration)
 * @param[in] p_cb       Pointer to set of callback functions to be used for communication
 *                       with the Upper Layer
 *
 * @return Status of the function execution (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t l2cap_coc_spsm_add(uint16_t spsm, uint8_t sec_lvl_bf, const l2cap_coc_spsm_cb_t* p_cb);

/**
 ****************************************************************************************
 * @brief Command used to unregister a Simplified Protocol/Service Multiplexer.
 *
 * @param[in] spsm     Simplified Protocol/Service Multiplexer
 *
 * @return Status of the function execution (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t l2cap_coc_spsm_remove(uint16_t spsm);

/// @} L2CAP_COC_CHANNEL_SPSM_API

/// @addtogroup L2CAP_COC_CHANNEL_MGMT_API
/// @{

/**
 ****************************************************************************************
 * @brief Command use to initiate creation of several L2CAP connection oriented channels.
 *        The operation completes when all requested L2CAP channels are created or when
 *        an error is detected.
 *        If all channels cannot be created, reason is provided in error code.
 *
 * @param[in] conidx        Connection Index
 * @param[in] metainfo      Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] spsm          Simplified Protocol/Service Multiplexer
 * @param[in] nb_chan       Number of L2CAP channel to create in parallel
 * @param[in] local_rx_mtu  Local reception Maximum Transmit Unit Size
 * @param[in] p_cb          Pointer to set of callback functions to be used for
 *                          communication with the Upper Layer.
 *
 * @return Status of the function execution (see #hl_err enumeration)
 *         Consider status only if an error occurs; else wait for execution completion
 ****************************************************************************************
 */
uint16_t l2cap_coc_create(uint8_t conidx, uint16_t metainfo, uint16_t spsm, uint8_t nb_chan, uint16_t local_rx_mtu,
                          const l2cap_chan_coc_cb_t* p_cb);

/**
 ****************************************************************************************
 * @brief Command use to accept or reject connection oriented channel establishment
 *
 * @param[in] conidx        Connection Index
 * @param[in] token         Value provided by L2CAP layer that must be reused in confirmation function
 * @param[in] nb_chan       Number of L2CAP channel local device accepts to create in parallel
 *                          0: reject all channel creation
 * @param[in] local_rx_mtu  Local reception Maximum Transmit Unit Size
 * @param[in] p_cb          Pointer to set of callback functions to be used for
 *                          communication with the Upper Layer.
 *
 * @return Status of the function execution (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t l2cap_coc_connect_cfm(uint8_t conidx, uint16_t token, uint8_t nb_chan, uint16_t local_rx_mtu,
                              const l2cap_chan_coc_cb_t* p_cb);

#if (EATT_SUPPORT)
/**
 ****************************************************************************************
 * @brief Command use to reconfigure a L2CAP connection oriented channel RX MTU
 *
 * @param[in] conidx        Connection Index
 * @param[in] metainfo      Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] local_rx_mtu  New Local reception Maximum Transmit Unit Size
 * @param[in] nb_chan       Number of L2CAP Channel local index in provided array
 * @param[in] p_chan_lid    Pointer to an array of L2CAP Channel local index
 *
 *
 * @return Status of the function execution (see #hl_err enumeration)
 *         Consider status only if an error occurs; else wait for execution completion
 ****************************************************************************************
 */
uint16_t l2cap_coc_reconfigure(uint8_t conidx, uint16_t metainfo, uint16_t local_rx_mtu, uint8_t nb_chan,
                               uint8_t* p_chan_lid);

#endif // (EATT_SUPPORT)

/**
 ****************************************************************************************
 * @brief Command use to initiate disconnection of a L2CAP connection oriented channel.
 *
 * @param[in] conidx        Connection Index
 * @param[in] metainfo      Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] chan_lid      L2CAP Channel local index
 *
 * @return Status of the function execution (see #hl_err enumeration)
 *         Consider status only if an error occurs; else wait for execution completion
 ****************************************************************************************
 */
uint16_t l2cap_coc_terminate(uint8_t conidx, uint16_t metainfo, uint8_t chan_lid);

/// @} L2CAP_COC_CHANNEL_MGMT_API

#endif // L2CAP_H_
