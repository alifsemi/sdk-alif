/**
 ****************************************************************************************
 *
 * @file gatt_srv.h
 *
 * @brief Header file - GATT Server Native API
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef GATT_SRV_H_
#define GATT_SRV_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup GATT_SERVER_API Server
 * @ingroup GATT_API
 * @brief GATT Server API
 *
 * @defgroup GATT_SERVER_CB_API Callback API
 * @ingroup GATT_SERVER_API
 *
 * @defgroup GATT_SERVER_EVENT_API Service event transmission
 * @ingroup GATT_SERVER_API
 * @brief Send notifications or indications
 *
 * @defgroup GATT_SERVER_CLI_API Answer client request
 * @ingroup GATT_SERVER_API
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "gatt.h"

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// @addtogroup GATT_SERVER_EVENT_API
/// @{

/// GATT Event Type
enum gatt_evt_type
{
    /// Server initiated notification
    GATT_NOTIFY = 0x00,
    /// Server initiated indication
    GATT_INDICATE = 0x01,
};

/// @} GATT_SERVER_EVENT_API

/*
 * SERVICE API - CALLBACKS
 ****************************************************************************************
 */

/// @addtogroup GATT_SERVER_CB_API
/// @{

/// GATT server user callback set
struct gatt_srv_cb
{
    /**
     ****************************************************************************************
     * @brief This function is called when GATT server user has initiated event send to peer
     *        device or if an error occurs.
     *
     * @note Mandatory callback if send event supported
     *
     * @param[in] conidx        Connection index
     * @param[in] user_lid      GATT user local identifier
     * @param[in] metainfo      Metadata information provided by API user (see \glos{METAINFO})
     * @param[in] status        Status of the procedure (see #hl_err enumeration)
     ****************************************************************************************
     */
    void (*cb_event_sent)(uint8_t conidx, uint8_t user_lid, uint16_t metainfo, uint16_t status);

    /**
     ****************************************************************************************
     * @brief This function is called when peer want to read local attribute database value.
     *
     * #gatt_srv_att_read_get_cfm shall be called to provide attribute value
     *
     * @note Mandatory callback
     *
     * @param[in] conidx        Connection index
     * @param[in] user_lid      GATT user local identifier
     * @param[in] token         Procedure token that must be returned in confirmation function
     * @param[in] hdl           Attribute handle
     * @param[in] offset        Value offset
     * @param[in] max_length    Maximum value length to return
     ****************************************************************************************
     */
    void (*cb_att_read_get)(uint8_t conidx, uint8_t user_lid, uint16_t token, uint16_t hdl, uint16_t offset,
                            uint16_t max_length);

    /**
     ****************************************************************************************
     * @brief This function is called when GATT server user has initiated event send procedure,
     *
     * #gatt_srv_att_event_get_cfm shall be called to provide attribute value
     *
     * @note Mandatory callback if reliable send event supported
     *
     * @param[in] conidx        Connection index
     * @param[in] user_lid      GATT user local identifier
     * @param[in] token         Procedure token that must be returned in confirmation function
     * @param[in] metainfo      Metadata information provided by API user (see \glos{METAINFO})
     * @param[in] hdl           Attribute handle
     * @param[in] max_length    Maximum value length to return
     ****************************************************************************************
     */
    void (*cb_att_event_get)(uint8_t conidx, uint8_t user_lid, uint16_t token, uint16_t metainfo, uint16_t hdl,
                             uint16_t max_length);

    /**
     ****************************************************************************************
     * @brief This function is called during a write procedure to get information about a
     *        specific attribute handle.
     *
     * #gatt_srv_att_info_get_cfm shall be called to provide attribute information
     *
     * @param[in] conidx        Connection index
     * @param[in] user_lid      GATT user local identifier
     * @param[in] token         Procedure token that must be returned in confirmation function
     * @param[in] hdl           Attribute handle
     ****************************************************************************************
     */
    void (*cb_att_info_get)(uint8_t conidx, uint8_t user_lid, uint16_t token, uint16_t hdl);

    /**
     ****************************************************************************************
     * @brief This function is called during a write procedure to modify attribute handle.
     *
     * #gatt_srv_att_val_set_cfm shall be called to accept or reject attribute update.
     *
     * @note Mandatory callback
     *
     * @param[in] conidx        Connection index
     * @param[in] user_lid      GATT user local identifier
     * @param[in] token         Procedure token that must be returned in confirmation function
     * @param[in] hdl           Attribute handle
     * @param[in] offset        Value offset
     * @param[in] p_data        Pointer to buffer that contains data to write starting from offset
     ****************************************************************************************
     */
    void (*cb_att_val_set)(uint8_t conidx, uint8_t user_lid, uint16_t token, uint16_t hdl, uint16_t offset,
                           co_buf_t* p_data);
};

/// @} GATT_SERVER_CB_API

/*
 * SERVICE API - FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

/// @addtogroup GATT_SERVER_EVENT_API
/// @{

/**
 ****************************************************************************************
 * @brief Function used by a GATT server user to send notifications or indications for
 *        some attributes values to peer device.
 *        Number of attributes must be set to one for GATT_INDICATE event type.
 *
 * This function is consider reliable because GATT user is aware of maximum packet
 * size that can be transmitted over the air.
 *
 * Attribute value will be requested by GATT using #gatt_srv_cb.cb_att_event_get function
 * Wait for #gatt_srv_cb.cb_event_sent execution before starting a new procedure
 *
 * @param[in]  conidx       Connection index
 * @param[in]  user_lid     GATT User Local identifier
 * @param[in]  metainfo     Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in]  evt_type     Event type to trigger (see #gatt_evt_type enumeration)
 * @param[in]  nb_att       Number of attribute
 * @param[in]  p_atts       Pointer to List of attribute
 *
 * @return Status of the function execution (see #hl_err enumeration)
 *         Consider status only if an error occurs; else wait for execution completion
 ****************************************************************************************
 */
uint16_t gatt_srv_event_reliable_send(uint8_t conidx, uint8_t user_lid, uint16_t metainfo, uint8_t evt_type,
                                      uint8_t nb_att, const gatt_att_t* p_atts);

/**
 ****************************************************************************************
 * @brief Provide attribute value requested by GATT Layer for an event procedure.
 *        If rejected, value is not used.
 *
 * @note Minimum buffer head: #GATT_BUFFER_HEADER_LEN,
 *       and minimum buffer tail: #GATT_BUFFER_TAIL_LEN
 *
 * @param[in]  conidx       Connection index
 * @param[in]  user_lid     GATT User Local identifier
 * @param[in]  token        Procedure token provided in corresponding callback
 * @param[in]  status       Status of attribute value get (see #hl_err enumeration)
 * @param[in]  att_length   Complete Length of the attribute value
 * @param[in]  p_data       Pointer to buffer that contains attribute data
 *                          (data size does not exceed maximum size provided)
 *
 * @return Status of the function execution (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t gatt_srv_att_event_get_cfm(uint8_t conidx, uint8_t user_lid, uint16_t token, uint16_t status,
                                    uint16_t att_length, co_buf_t* p_data);

/**
 ****************************************************************************************
 * @brief Function used by a GATT server user to send notifications or indications.
 *
 * Since user is not aware of MTU size of the bearer used for attribute
 * transmission it cannot be considered reliable. If size of the data buffer is too
 * big, data is truncated to max supported length.
 *
 * Wait for #gatt_srv_cb.cb_event_sent execution before starting a new procedure
 *
 * @note Minimum buffer head: #GATT_BUFFER_HEADER_LEN,
 *       and minimum buffer tail: #GATT_BUFFER_TAIL_LEN
 *
 * @param[in]  conidx       Connection index
 * @param[in]  user_lid     GATT User Local identifier
 * @param[in]  metainfo     Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in]  evt_type     Event type to trigger (see #gatt_evt_type enumeration)
 * @param[in]  hdl          Attribute handle
 * @param[in]  p_data       Data buffer that must be transmitted
 *
 * @return Status of the function execution (see #hl_err enumeration)
 *         Consider status only if an error occurs; else wait for execution completion
 ****************************************************************************************
 */
uint16_t gatt_srv_event_send(uint8_t conidx, uint8_t user_lid, uint16_t metainfo, uint8_t evt_type, uint16_t hdl,
                             co_buf_t* p_data);

/**
 ****************************************************************************************
 * @brief Function used by a GATT server user to send notifications or indications of an
 *        attribute to multiple connected devices.
 *
 * Since user is not aware of MTU size of the bearer used for attribute
 * transmission it cannot be considered reliable. If size of the data buffer is too
 * big, data is truncated to max supported length.
 *
 * Event are sent sequentially over each connection and can take some time.
 * The cb_event_sent callback is called once when procedure is completed.
 *
 * It is possible to cancel an on-going multi-point procedure using
 * #gatt_srv_event_mtp_cancel function. In such case GATT user must ensure
 * that the couple user_lid + metainfo parameters are unique for GATT module.
 *
 * Wait for #gatt_srv_cb.cb_event_sent execution before starting a new procedure
 *
 * @note Minimum buffer head: #GATT_BUFFER_HEADER_LEN,
 *       and minimum buffer tail: #GATT_BUFFER_TAIL_LEN
 *
 * @param[in]  conidx_bf    Connection index bit field
 * @param[in]  user_lid     GATT User Local identifier
 * @param[in]  metainfo     Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in]  evt_type     Event type to trigger (see #gatt_evt_type enumeration)
 * @param[in]  hdl          Attribute handle
 * @param[in]  p_data       Data buffer that must be transmitted
 * @param[in]  filter       True to filter intermediate command completed event triggered
 *                          when event is sent onto a specific connection.
 *
 * @return Status of the function execution (see #hl_err enumeration)
 *         Consider status only if an error occurs; else wait for execution completion
 ****************************************************************************************
 */
uint16_t gatt_srv_event_mtp_send(uint32_t conidx_bf, uint8_t user_lid, uint16_t metainfo, uint8_t evt_type,
                                 uint16_t hdl, co_buf_t* p_data, bool filter);

/**
 ****************************************************************************************
 * @brief Command used by a GATT server user to cancel a multi connection event transmission
 *
 * @note Once procedure is done, #gatt_srv_cb.cb_event_sent function is called.
 *
 * @param[in]  user_lid     GATT User Local identifier used in #gatt_srv_event_mtp_send
 * @param[in]  metainfo     Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 *
 * @return Status of the function execution (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t gatt_srv_event_mtp_cancel(uint8_t user_lid, uint16_t metainfo);

/// @} GATT_SERVER_EVENT_API

/// @addtogroup GATT_SERVER_CLI_API
/// @{

/**
 ****************************************************************************************
 * @brief Provide attribute value requested by GATT Layer for a read procedure
 *        If rejected, value is not used.
 *
 * @note Minimum buffer head: #GATT_BUFFER_HEADER_LEN,
 *       and minimum buffer tail: #GATT_BUFFER_TAIL_LEN
 *
 * @param[in]  conidx       Connection index
 * @param[in]  user_lid     GATT User Local identifier
 * @param[in]  token        Procedure token provided in corresponding callback
 * @param[in]  status       Status of attribute value get (see #hl_err enumeration)
 * @param[in]  att_length   Complete Length of the attribute value
 * @param[in]  p_data       Pointer to buffer that contains attribute data
 *                          (starting from offset and does not exceed maximum size provided)
 *
 * @return Status of the function execution (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t gatt_srv_att_read_get_cfm(uint8_t conidx, uint8_t user_lid, uint16_t token, uint16_t status,
                                   uint16_t att_length, co_buf_t* p_data);

/**
 ****************************************************************************************
 * @brief Provide information about attribute requested by GATT Layer.
 *
 * @param[in]  conidx       Connection index
 * @param[in]  user_lid     GATT User Local identifier
 * @param[in]  token        Procedure token provided in corresponding callback
 * @param[in]  status       Status of attribute info get (see #hl_err enumeration)
 * @param[in]  att_length   Attribute value length
 *
 * @return Status of the function execution (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t gatt_srv_att_info_get_cfm(uint8_t conidx, uint8_t user_lid, uint16_t token, uint16_t status,
                                   uint16_t att_length);

/**
 ****************************************************************************************
 * @brief Provide status of attribute value modification by GATT server user.
 *
 * @param[in]  conidx       Connection index
 * @param[in]  user_lid     GATT User Local identifier
 * @param[in]  token        Procedure token provided in corresponding callback
 * @param[in]  status       Status of attribute value set (see #hl_err enumeration)
 *
 * @return Status of the function execution (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t gatt_srv_att_val_set_cfm(uint8_t conidx, uint8_t user_lid, uint16_t token, uint16_t status);

/// @} GATT_SERVER_CLI_API

#endif /* GATT_SRV_H_ */
