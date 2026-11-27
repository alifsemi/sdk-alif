/**
 ****************************************************************************************
 *
 * @file gatt_cli.h
 *
 * @brief Header file - GATT Client Native API
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef GATT_CLI_H_
#define GATT_CLI_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup GATT_CLIENT_API Client
 * @ingroup GATT_API
 * @brief GATT Client API
 *
 * @defgroup GATT_CLIENT_CB_API Callback API
 * @ingroup GATT_CLIENT_API
 *
 * @defgroup GATT_CLIENT_DISCOVER_API Service / attribute discovery
 * @ingroup GATT_CLIENT_API
 *
 * @defgroup GATT_CLIENT_READ_API Attribute read
 * @ingroup GATT_CLIENT_API
 *
 * @defgroup GATT_CLIENT_WRITE_API Attribute write
 * @ingroup GATT_CLIENT_API
 *
 * @defgroup GATT_CLIENT_EVENT_API Service event reception
 * @ingroup GATT_CLIENT_API
 *
 * @defgroup GATT_CLIENT_MTU_API MTU Exchange
 * @ingroup GATT_CLIENT_API
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

/// @addtogroup GATT_CLIENT_DISCOVER_API
/// @{

/// GATT service discovery information
enum gatt_svc_disc_info
{
    /// Complete service present in structure
    GATT_SVC_CMPLT = 0x00,
    /// First service attribute present in structure
    GATT_SVC_START = 0x01,
    /// Last service attribute present in structure
    GATT_SVC_END = 0x02,
    /// Following service attribute present in structure
    GATT_SVC_CONT = 0x03,
};

/// GATT Service Discovery Type
enum gatt_svc_discovery_type
{
    /// Discover all primary services
    GATT_DISCOVER_SVC_PRIMARY_ALL = 0x00,
    /// Discover primary services using UUID value
    GATT_DISCOVER_SVC_PRIMARY_BY_UUID = 0x01,
    /// Discover all secondary services
    GATT_DISCOVER_SVC_SECONDARY_ALL = 0x02,
    /// Discover secondary services using UUID value
    GATT_DISCOVER_SVC_SECONDARY_BY_UUID = 0x03,
};

/// GATT Characteristic Discovery Type
enum gatt_char_discovery_type
{
    /// Discover all characteristics
    GATT_DISCOVER_CHAR_ALL = 0x00,
    /// Discover characteristics using UUID value
    GATT_DISCOVER_CHAR_BY_UUID = 0x01,
};

/// Service Discovery Attribute type
enum gatt_att_type
{
    /// No Attribute Information
    GATT_ATT_NONE = 0x00,
    /// Primary service attribute
    GATT_ATT_PRIMARY_SVC = 0x01,
    /// Secondary service attribute
    GATT_ATT_SECONDARY_SVC = 0x02,
    /// Included service attribute
    GATT_ATT_INCL_SVC = 0x03,
    /// Characteristic declaration
    GATT_ATT_CHAR = 0x04,
    /// Attribute value
    GATT_ATT_VAL = 0x05,
    /// Attribute descriptor
    GATT_ATT_DESC = 0x06,
};

/// @} GATT_CLIENT_DISCOVER_API

/// @addtogroup GATT_CLIENT_WRITE_API
/// @{

/// Write execution mode
enum gatt_write_mode
{
    /// Perform automatic write execution
    GATT_WRITE_MODE_AUTO_EXECUTE = 0x00,
    /// Force to use prepare write queue. Can be used to write multiple attributes
    GATT_WRITE_MODE_QUEUE = 0x01,
};

/// GATT Write Type
enum gatt_write_type
{
    /// Write attribute
    GATT_WRITE = 0x00,
    /// Write attribute without response
    GATT_WRITE_NO_RESP = 0x01,
    /// Write attribute signed
    GATT_WRITE_SIGNED = 0x02,
};

/// @} GATT_CLIENT_WRITE_API

/*
 * TYPES DEFINITION
 ****************************************************************************************
 */

/// @addtogroup GATT_CLIENT_DISCOVER_API
/// @{

/// Attribute info structure for Service Discovery
typedef struct
{
    /// Attribute Type (see #gatt_att_type enumeration)
    uint8_t att_type;
    /// UUID type (see #gatt_uuid_type enumeration)
    uint8_t uuid_type;
    /// UUID - LSB first (0 for GATT_ATT_NONE)
    uint8_t uuid[GATT_UUID_128_LEN];

    /// Information about Service attribute union
    union gatt_info
    {
        /// Service info structure
        struct gatt_svc_info
        {
            /// Service start handle
            uint16_t start_hdl;
            /// Service end handle
            uint16_t end_hdl;
        } svc; //!< Service info (#GATT_ATT_PRIMARY_SVC, #GATT_ATT_SECONDARY_SVC, #GATT_ATT_INCL_SVC)

        /// Characteristic info structure
        struct gatt_char_info
        {
            /// Value handle
            uint16_t val_hdl;
            /// Characteristic properties (see #gatt_att_info_bf enumeration - bits [0-7])
            uint8_t prop;
        } charac;  //!< Characteristic info (#GATT_ATT_CHAR)
    } info; //!< Information about Service attribute
} gatt_svc_att_t;

/// @} GATT_CLIENT_DISCOVER_API

/*
 * CLIENT API - CALLBACKS DEFINITIONS
 ****************************************************************************************
 */

/// @addtogroup GATT_CLIENT_CB_API
/// @{

/// GATT client user callback set
struct gatt_cli_cb
{
    /**
     ****************************************************************************************
     * @brief This function is called when GATT client user discovery procedure is over.
     *
     * @param[in] conidx        Connection index
     * @param[in] user_lid      GATT user local identifier
     * @param[in] metainfo      Metadata information provided by API user (see \glos{METAINFO})
     * @param[in] status        Status of the procedure (see #hl_err enumeration)
     ****************************************************************************************
     */
    void (*cb_discover_cmp) (uint8_t conidx, uint8_t user_lid, uint16_t metainfo, uint16_t status);

    /**
     ****************************************************************************************
     * @brief This function is called when GATT client user read procedure is over.
     *
     * @param[in] conidx        Connection index
     * @param[in] user_lid      GATT user local identifier
     * @param[in] metainfo      Metadata information provided by API user (see \glos{METAINFO})
     * @param[in] status        Status of the procedure (see #hl_err enumeration)
     ****************************************************************************************
     */
    void (*cb_read_cmp) (uint8_t conidx, uint8_t user_lid, uint16_t metainfo, uint16_t status);

    /**
     ****************************************************************************************
     * @brief This function is called when GATT client user write procedure is over.
     *
     * @param[in] conidx        Connection index
     * @param[in] user_lid      GATT user local identifier
     * @param[in] metainfo      Metadata information provided by API user (see \glos{METAINFO})
     * @param[in] status        Status of the procedure (see #hl_err enumeration)
     ****************************************************************************************
     */
    void (*cb_write_cmp) (uint8_t conidx, uint8_t user_lid, uint16_t metainfo, uint16_t status);

    /**
     ****************************************************************************************
     * @brief This function is called when GATT client user has initiated a write procedure.
     *
     *        #gatt_cli_att_val_get_cfm shall be called to provide attribute value.
     *
     * @param[in] conidx        Connection index
     * @param[in] user_lid      GATT user local identifier
     * @param[in] token         Procedure token that must be returned in confirmation function
     * @param[in] metainfo      Metadata information provided by API user (see \glos{METAINFO})
     * @param[in] hdl           Attribute handle
     * @param[in] offset        Value offset
     * @param[in] max_length    Maximum value length to return
     ****************************************************************************************
     */
    void (*cb_att_val_get) (uint8_t conidx, uint8_t user_lid, uint16_t token, uint16_t metainfo,
                            uint16_t hdl, uint16_t offset, uint16_t max_length);

    /**
     ****************************************************************************************
     * @brief This function is called when a full service has been found during a discovery procedure.
     *
     * @param[in] conidx        Connection index
     * @param[in] user_lid      GATT user local identifier
     * @param[in] metainfo      Metadata information provided by API user (see \glos{METAINFO})
     * @param[in] hdl           First handle value of following list
     * @param[in] disc_info     Discovery information (see #gatt_svc_disc_info enumeration)
     * @param[in] nb_att        Number of attributes
     * @param[in] p_atts        Pointer to attribute information present in a service
     ****************************************************************************************
     */
    void (*cb_svc) (uint8_t conidx, uint8_t user_lid, uint16_t metainfo, uint16_t hdl, uint8_t disc_info,
                    uint8_t nb_att, const gatt_svc_att_t* p_atts);

    /**
     ****************************************************************************************
     * @brief This function is called when a service has been found during a discovery procedure.
     *
     * @param[in] conidx        Connection index
     * @param[in] user_lid      GATT user local identifier
     * @param[in] metainfo      Metadata information provided by API user (see \glos{METAINFO})
     * @param[in] start_hdl     Service start handle
     * @param[in] end_hdl       Service end handle
     * @param[in] uuid_type     UUID Type (see #gatt_uuid_type enumeration)
     * @param[in] p_uuid        Pointer to service UUID (LSB first)
     ****************************************************************************************
     */
    void (*cb_svc_info) (uint8_t conidx, uint8_t user_lid, uint16_t metainfo, uint16_t start_hdl, uint16_t end_hdl,
                         uint8_t uuid_type, const uint8_t* p_uuid);
    /**
     ****************************************************************************************
     * @brief This function is called when an include service has been found during a discovery procedure.
     *
     * @param[in] conidx        Connection index
     * @param[in] user_lid      GATT user local identifier
     * @param[in] metainfo      Metadata information provided by API user (see \glos{METAINFO})
     * @param[in] inc_svc_hdl   Include service attribute handle
     * @param[in] start_hdl     Service start handle
     * @param[in] end_hdl       Service end handle
     * @param[in] uuid_type     UUID Type (see #gatt_uuid_type enumeration)
     * @param[in] p_uuid        Pointer to service UUID (LSB first)
     ****************************************************************************************
     */
    void (*cb_inc_svc) (uint8_t conidx, uint8_t user_lid, uint16_t metainfo, uint16_t inc_svc_hdl,
                        uint16_t start_hdl, uint16_t end_hdl, uint8_t uuid_type, const uint8_t* p_uuid);
    /**
     ****************************************************************************************
     * @brief This function is called when a characteristic has been found during a discovery procedure.
     *
     * @param[in] conidx        Connection index
     * @param[in] user_lid      GATT user local identifier
     * @param[in] metainfo      Metadata information provided by API user (see \glos{METAINFO})
     * @param[in] char_hdl      Characteristic attribute handle
     * @param[in] val_hdl       Value handle
     * @param[in] prop          Characteristic properties (see #gatt_att_info_bf enumeration - bits [0-7])
     * @param[in] uuid_type     UUID Type (see #gatt_uuid_type enumeration)
     * @param[in] p_uuid        Pointer to characteristic value UUID (LSB first)
     ****************************************************************************************
     */
    void (*cb_char) (uint8_t conidx, uint8_t user_lid, uint16_t metainfo, uint16_t char_hdl, uint16_t val_hdl,
                     uint8_t prop, uint8_t uuid_type, const uint8_t* p_uuid);
    /**
     ****************************************************************************************
     * @brief This function is called when a descriptor has been found during a discovery procedure.
     *
     * @param[in] conidx        Connection index
     * @param[in] user_lid      GATT user local identifier
     * @param[in] metainfo      Metadata information provided by API user (see \glos{METAINFO})
     * @param[in] desc_hdl      Characteristic descriptor attribute handle
     * @param[in] uuid_type     UUID Type (see #gatt_uuid_type enumeration)
     * @param[in] p_uuid        Pointer to attribute UUID (LSB first)
     ****************************************************************************************
     */
    void (*cb_desc) (uint8_t conidx, uint8_t user_lid, uint16_t metainfo, uint16_t desc_hdl,
                     uint8_t uuid_type, const uint8_t* p_uuid);
    /**
     ****************************************************************************************
     * @brief This function is called during a read procedure when attribute value is retrieved
     *        form peer device.
     *
     * @param[in] conidx        Connection index
     * @param[in] user_lid      GATT user local identifier
     * @param[in] metainfo      Metadata information provided by API user (see \glos{METAINFO})
     * @param[in] hdl           Attribute handle
     * @param[in] offset        Value offset
     * @param[in] p_data        Pointer to buffer that contains attribute value starting from offset
     ****************************************************************************************
     */
    void (*cb_att_val) (uint8_t conidx, uint8_t user_lid, uint16_t metainfo, uint16_t hdl, uint16_t offset,
                        co_buf_t* p_data);
    /**
     ****************************************************************************************
     * @brief This function is called when a notification or an indication is received onto
     *        register handle range (#gatt_cli_event_register).
     *
     *        #gatt_cli_att_event_cfm must be called to confirm event reception.
     *
     * @param[in] conidx        Connection index
     * @param[in] user_lid      GATT user local identifier
     * @param[in] token         Procedure token that must be returned in confirmation function
     * @param[in] evt_type      Event type triggered (see #gatt_evt_type enumeration)
     * @param[in] complete      True if event value if complete value has been received
     *                          False if data received is equals to maximum attribute protocol value.
     *                          In such case GATT Client User should perform a read procedure.
     * @param[in] hdl           Attribute handle
     * @param[in] p_data        Pointer to buffer that contains attribute value
     ****************************************************************************************
     */
    void (*cb_att_val_evt) (uint8_t conidx, uint8_t user_lid, uint16_t token, uint8_t evt_type, bool complete,
                            uint16_t hdl, co_buf_t* p_data);

    /**
     ****************************************************************************************
     * @brief Event triggered when a service change has been received or if an attribute
     *        transaction triggers an out of sync error.
     *
     * @param[in] conidx        Connection index
     * @param[in] user_lid      GATT user local identifier
     * @param[in] out_of_sync   True if an out of sync error has been received
     * @param[in] start_hdl     Service start handle
     * @param[in] end_hdl       Service end handle
     ****************************************************************************************
     */
    void (*cb_svc_changed) (uint8_t conidx, uint8_t user_lid, bool out_of_sync, uint16_t start_hdl, uint16_t end_hdl);
};

/// @} GATT_CLIENT_CB_API

/*
 * CLIENT API - FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

/// @addtogroup GATT_CLIENT_DISCOVER_API
/// @{

/**
 ****************************************************************************************
 * @brief Function used by a GATT client user to discover primary or secondary services,
 *        exposed by peer device in its attribute database.
 *
 * All services can be discovered or filtering services having a specific UUID.
 * The discovery is done between start handle and end handle range.
 * For a complete discovery start handle must be set to 0x0001 and end handle to
 * 0xFFFF.
 *
 * Wait for #gatt_cli_cb.cb_discover_cmp execution before starting a new procedure
 *
 * @param[in]  conidx       Connection index
 * @param[in]  user_lid     GATT User Local identifier
 * @param[in]  metainfo     Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in]  disc_type    GATT Service discovery type (see #gatt_svc_discovery_type enumeration)
 * @param[in]  full         Perform discovery of all information present in the service
 *                          (True: enable, False: disable)
 * @param[in]  start_hdl    Search start handle
 * @param[in]  end_hdl      Search end handle
 * @param[in]  uuid_type    UUID Type (see #gatt_uuid_type enumeration)
 * @param[in]  p_uuid       Pointer to searched Service UUID (meaningful only for
 *                          discovery by UUID)
 *
 * @return Status of the function execution (see #hl_err enumeration)
 *         Consider status only if an error occurs; else wait for execution completion
 ****************************************************************************************
 */
uint16_t gatt_cli_discover_svc(uint8_t conidx, uint8_t user_lid, uint16_t metainfo, uint8_t disc_type, bool full,
                               uint16_t start_hdl, uint16_t end_hdl, uint8_t uuid_type, const uint8_t* p_uuid);

/**
 ****************************************************************************************
 * @brief Function used by a GATT client user to discover included services, exposed
 *        by peer device in its attribute database.
 *
 *  The discovery is done between start handle and end handle range.
 *  For a complete discovery start handle must be set to 0x0001 and end handle to
 *  0xFFFF.
 *
 *  Wait for #gatt_cli_cb.cb_discover_cmp execution before starting a new procedure
 *
 * @param[in]  conidx       Connection index
 * @param[in]  user_lid     GATT User Local identifier
 * @param[in]  metainfo     Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in]  start_hdl    Search start handle
 * @param[in]  end_hdl      Search end handle
 *
 * @return Status of the function execution (see #hl_err enumeration)
 *         Consider status only if an error occurs; else wait for execution completion
 ****************************************************************************************
 */
uint16_t gatt_cli_discover_inc_svc(uint8_t conidx, uint8_t user_lid, uint16_t metainfo,
                                   uint16_t start_hdl, uint16_t end_hdl);

/**
 ****************************************************************************************
 * @brief Function used by a GATT client user to discover all or according to a specific
 *        UUID characteristics exposed by peer device in its attribute database.
 *
 * The discovery is done between start handle and end handle range.
 * For a complete discovery start handle must be set to 0x0001 and end handle to
 * 0xFFFF.
 *
 * Wait for #gatt_cli_cb.cb_discover_cmp execution before starting a new procedure
 *
 * @param[in]  conidx       Connection index
 * @param[in]  user_lid     GATT User Local identifier
 * @param[in]  metainfo     Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in]  disc_type    GATT characteristic discovery type (see #gatt_char_discovery_type enumeration)
 * @param[in]  start_hdl    Search start handle
 * @param[in]  end_hdl      Search end handle
 * @param[in]  uuid_type    UUID Type (see #gatt_uuid_type enumeration)
 * @param[in]  p_uuid       Pointer to searched Attribute Value UUID (meaningful only
 *                          for discovery by UUID)
 *
 * @return Status of the function execution (see #hl_err enumeration)
 *         Consider status only if an error occurs; else wait for execution completion
 ****************************************************************************************
 */
uint16_t gatt_cli_discover_char(uint8_t conidx, uint8_t user_lid, uint16_t metainfo, uint8_t disc_type,
                                uint16_t start_hdl, uint16_t end_hdl, uint8_t uuid_type, const uint8_t* p_uuid);

/**
 ****************************************************************************************
 * @brief Function used by a GATT client user to discover characteristic descriptor
 *        exposed by peer device in its attribute database.
 *
 * The discovery is done between start handle and end handle range.
 * For a complete discovery start handle must be set to 0x0001 and end handle to
 * 0xFFFF.
 *
 * Wait for #gatt_cli_cb.cb_discover_cmp execution before starting a new procedure
 *
 * @param[in]  conidx       Connection index
 * @param[in]  user_lid     GATT User Local identifier
 * @param[in]  metainfo     Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in]  start_hdl    Search start handle
 * @param[in]  end_hdl      Search end handle
 *
 * @return Status of the function execution (see #hl_err enumeration)
 *         Consider status only if an error occurs; else wait for execution completion
 ****************************************************************************************
 */
uint16_t gatt_cli_discover_desc(uint8_t conidx, uint8_t user_lid, uint16_t metainfo, uint16_t start_hdl,
                                uint16_t end_hdl);

/**
 ****************************************************************************************
 * @brief Function used by a GATT client user to cancel an on-going discovery procedure.
 *        The metainfo parameter in the request must be equals to metainfo parameter used for
 *        service discovery command.
 *
 *  The discovery is aborted as soon as on-going discovery attribute transaction
 *  is over.
 *
 *  Wait for #gatt_cli_cb.cb_discover_cmp execution before starting a new procedure
 *
 * @param[in]  conidx       Connection index
 * @param[in]  user_lid     GATT User Local identifier
 * @param[in]  metainfo     Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 *
 * @return Status of the function execution (see #hl_err enumeration)
 *         Consider status only if an error occurs; else wait for execution completion
 ****************************************************************************************
 */
uint16_t gatt_cli_discover_cancel(uint8_t conidx, uint8_t user_lid, uint16_t metainfo);

/// @} GATT_CLIENT_DISCOVER_API

/// @addtogroup GATT_CLIENT_READ_API
/// @{

/**
 ****************************************************************************************
 * @brief Function used by a GATT client user to read value of an attribute (identified
 *        by its handle) present in peer database.
 *
 * Wait for #gatt_cli_cb.cb_read_cmp execution before starting a new procedure
 *
 * @param[in]  conidx       Connection index
 * @param[in]  user_lid     GATT User Local identifier
 * @param[in]  metainfo     Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in]  hdl          Attribute handle
 * @param[in]  offset       Value offset
 * @param[in]  length       Value length to read (0 = read all)
 *
 * @return Status of the function execution (see #hl_err enumeration)
 *         Consider status only if an error occurs; else wait for execution completion
 ****************************************************************************************
 */
uint16_t gatt_cli_read(uint8_t conidx, uint8_t user_lid, uint16_t metainfo,
                       uint16_t hdl, uint16_t offset, uint16_t length);

/**
 ****************************************************************************************
 * @brief Function used by a GATT client user to read value of an attribute with a given
 *        UUID in peer database.
 *
 * Wait for #gatt_cli_cb.cb_read_cmp execution before starting a new procedure
 *
 * @param[in]  conidx       Connection index
 * @param[in]  user_lid     GATT User Local identifier
 * @param[in]  metainfo     Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in]  start_hdl    Search start handle
 * @param[in]  end_hdl      Search end handle
 * @param[in]  uuid_type    UUID Type (see #gatt_uuid_type enumeration)
 * @param[in]  p_uuid       Pointer to searched attribute UUID (LSB First)
 *
 * @return Status of the function execution (see #hl_err enumeration)
 *         Consider status only if an error occurs; else wait for execution completion
 ****************************************************************************************
 */
uint16_t gatt_cli_read_by_uuid(uint8_t conidx, uint8_t user_lid, uint16_t metainfo,
                               uint16_t start_hdl, uint16_t end_hdl, uint8_t uuid_type, const uint8_t* p_uuid);

/**
 ****************************************************************************************
 * @brief Function used by a GATT client user to read multiple attribute at the same time.
 *        If one of attribute length is unknown, the read multiple variable length
 *        procedure is used.
 *
 * Wait for #gatt_cli_cb.cb_read_cmp execution before starting a new procedure
 *
 * @param[in]  conidx       Connection index
 * @param[in]  user_lid     GATT User Local identifier
 * @param[in]  metainfo     Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in]  nb_att       Number of attribute
 * @param[in]  p_atts       Pointer to list of attribute
 *                          If Attribute length is zero (consider length unknown):
  *                            - Attribute protocol read multiple variable length
  *                              procedure used
 *
 * @return Status of the function execution (see #hl_err enumeration)
 *         Consider status only if an error occurs; else wait for execution completion
 ****************************************************************************************
 */
uint16_t gatt_cli_read_multiple(uint8_t conidx, uint8_t user_lid, uint16_t metainfo,
                                uint8_t nb_att, const gatt_att_t* p_atts);


/// @} GATT_CLIENT_READ_API

/// @addtogroup GATT_CLIENT_WRITE_API
/// @{

/**
 ****************************************************************************************
 * @brief Function used by a GATT client user to request to write value of an attribute
 *        in peer database.
 *
 * This function is consider reliable because GATT user is aware of maximum packet
 * size that can be transmitted over the air.
 *
 * Attribute value will be requested by GATT using #gatt_cli_cb.cb_att_val_get function
 *
 * Wait for #gatt_cli_cb.cb_write_cmp execution before starting a new procedure
 *
 * @param[in]  conidx       Connection index
 * @param[in]  user_lid     GATT User Local identifier
 * @param[in]  metainfo     Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in]  write_type   GATT write type (see #gatt_write_type enumeration)
 * @param[in]  write_mode   Write execution mode (see #gatt_write_mode enumeration).
 *                          Valid only for GATT_WRITE.
 * @param[in]  hdl          Attribute handle
 * @param[in]  offset       Value offset, valid only for GATT_WRITE
 * @param[in]  length       Value length to write
 *
 * @return Status of the function execution (see #hl_err enumeration)
 *         Consider status only if an error occurs; else wait for execution completion
 ****************************************************************************************
 */
uint16_t gatt_cli_write_reliable(uint8_t conidx, uint8_t user_lid, uint16_t metainfo, uint8_t write_type,
                                 uint8_t write_mode, uint16_t hdl, uint16_t offset, uint16_t length);

/**
 ****************************************************************************************
 * @brief Function used by a GATT client user to request to write value of an attribute
 *        in peer database.
 *
 * Since user is not aware of MTU size of the bearer used for attribute
 * transmission it cannot be considered reliable.
 *
 * For a GATT_WRITE_NO_RESP if attribute bearer max transmission size isn't sufficient,
 * a GATT_WRITE (with response) procedure will be used.
 *
 * For a GATT_WRITE_SIGNED, if attribute bearer max transmission size isn't sufficient,
 * the procedure is aborted with L2CAP_ERR_INVALID_MTU error code.
 *
 * Wait for #gatt_cli_cb.cb_write_cmp execution before starting a new procedure
 *
 *
 * @note Minimum buffer head: #GATT_BUFFER_HEADER_LEN,
 *       and minimum buffer tail for write signed: #GATT_BUFFER_SIGN_TAIL_LEN otherwise  #GATT_BUFFER_TAIL_LEN
 *
 * @param[in]  conidx       Connection index
 * @param[in]  user_lid     GATT User Local identifier
 * @param[in]  metainfo     Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in]  write_type   GATT write type (see #gatt_write_type enumeration)
 * @param[in]  hdl          Attribute handle
 * @param[in]  offset       Value offset, valid only for GATT_WRITE
 * @param[in]  p_data       Data buffer that must be transmitted
 *
 * @return Status of the function execution (see #hl_err enumeration)
 *         Consider status only if an error occurs; else wait for execution completion
 ****************************************************************************************
 */
uint16_t gatt_cli_write(uint8_t conidx, uint8_t user_lid, uint16_t metainfo, uint8_t write_type,
                        uint16_t hdl, uint16_t offset, co_buf_t* p_data);

/**
 ****************************************************************************************
 * @brief Provide attribute value requested by GATT Layer, length shall be
 *        set to zero if request is rejected.
 *
 * @note Minimum buffer head: #GATT_BUFFER_HEADER_LEN,
 *       and minimum buffer tail for write signed: #GATT_BUFFER_SIGN_TAIL_LEN otherwise  #GATT_BUFFER_TAIL_LEN
 *
 * @param[in]  conidx       Connection index
 * @param[in]  user_lid     GATT User Local identifier
 * @param[in]  token        Procedure token provided in corresponding callback
 * @param[in]  status       Status of attribute value get (see #hl_err enumeration)
 * @param[in]  p_data       Pointer to buffer that contains data to write (starting
 *                          from offset)
 *
 * @return Status of the function execution (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t gatt_cli_att_val_get_cfm(uint8_t conidx, uint8_t user_lid, uint16_t token, uint16_t status, co_buf_t* p_data);

/**
 ****************************************************************************************
 * @brief Function used by a GATT client user to request peer server to execute prepare
 *        write queue.
 *
 * Wait for #gatt_cli_cb.cb_write_cmp execution before starting a new procedure
 *
 * @param[in]  conidx       Connection index
 * @param[in]  user_lid     GATT User Local identifier
 * @param[in]  metainfo     Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in]  execute      True: Perform pending write operations
 *                          False: Cancel pending write operations
 *
 * @return Status of the function execution (see #hl_err enumeration)
 *         Consider status only if an error occurs; else wait for execution completion
 ****************************************************************************************
 */
uint16_t gatt_cli_write_exe(uint8_t conidx, uint8_t user_lid, uint16_t metainfo, bool execute);

/// @} GATT_CLIENT_WRITE_API

/// @addtogroup GATT_CLIENT_EVENT_API
/// @{

/**
 ****************************************************************************************
 * @brief Function used by a GATT client user to register for reception of events
 *        (notification / indication) for a given handle range.
 *
 * @note If an indication or notification is received within registered handle range,
 *       #gatt_cli_cb.cb_att_val_evt callback function of client interface is executed.
 *
 * @param[in]  conidx       Connection index
 * @param[in]  user_lid     GATT User Local identifier
 * @param[in]  start_hdl    Attribute start handle
 * @param[in]  end_hdl      Attribute end handle
 *
 * @return Status of the function execution (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t gatt_cli_event_register(uint8_t conidx, uint8_t user_lid, uint16_t start_hdl, uint16_t end_hdl);

/**
 ****************************************************************************************
 * @brief Function used by a GATT client user to stop reception of events (notification /
 *        indication) onto a specific handle range.
 *
 * @param[in]  conidx       Connection index
 * @param[in]  user_lid     GATT User Local identifier
 * @param[in]  start_hdl    Attribute start handle
 * @param[in]  end_hdl      Attribute end handle
 *
 * @return Status of the function execution (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t gatt_cli_event_unregister(uint8_t conidx, uint8_t user_lid, uint16_t start_hdl, uint16_t end_hdl);

/**
 ****************************************************************************************
 * @brief Provide status of attribute event handled by GATT client user.
 *
 * @param[in]  conidx       Connection index
 * @param[in]  user_lid     GATT User Local identifier
 * @param[in]  token        Procedure token provided in corresponding callback
 *
 * @return Status of the function execution (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t gatt_cli_att_event_cfm(uint8_t conidx, uint8_t user_lid, uint16_t token);

/// @} GATT_CLIENT_EVENT_API

/// @addtogroup GATT_CLIENT_MTU_API
/// @{

/**
 ****************************************************************************************
 * @brief Request a MTU exchange on legacy attribute bearer.
 *        There is no callback executed when the procedure is over.
 *
 * @param[in]  conidx       Connection index
 * @param[in]  user_lid     GATT User Local identifier
 *
 * @return Status of the function execution (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t gatt_cli_mtu_exch(uint8_t conidx, uint8_t user_lid);

/// @} GATT_CLIENT_MTU_API

#endif /* GATT_CLI_H_ */
