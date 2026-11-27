/**
 ****************************************************************************************
 *
 * @file ots.h
 *
 * @brief Object Transfer Server - Definitions
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef OTS_H_
#define OTS_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup OTS Object Transfer Profile Server
 * @ingroup OTP_API
 * @brief Description of Object Transfer Profile Server module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup OTS_ENUM Enumerations
 * @ingroup OTS
 * @brief Enumerations for Object Transfer Profile Server module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup OTS_STRUCT Structures
 * @ingroup OTS
 * @brief Structures for Object Transfer Profile Server module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup OTS_NATIVE_API Native API
 * @ingroup OTS
 * @brief Description of Native API for Object Transfer Profile Server module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup OTS_CALLBACK Callback Functions
 * @ingroup OTS_NATIVE_API
 * @brief Description of callback functions for Object Transfer Profile Server module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup OTS_FUNCTION Functions
 * @ingroup OTS_NATIVE_API
 * @brief Description of functions for Object Transfer Profile Server module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "otp.h"            // Object Transfer Profile Definitions

#if (BLE_OT_SERVER)

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// @addtogroup OTS_ENUM
/// @{

/// List of OTS_CMD command codes
enum ots_cmd_codes
{
    OTS_COC_DISCONNECT = 0x0000,
    OTS_COC_SEND = 0x0001,
    OTS_COC_RELEASE = 0x0002,
};

/// List of OTS_REQ_IND request indication codes
enum ots_msg_req_ind_codes
{
    OTS_GET_NAME = 0x0000,
    OTS_SET_NAME = 0x0001,
    OTS_OBJECT_CREATE = 0x0002,
    OTS_OBJECT_EXECUTE = 0x0003,
    OTS_OBJECT_MANIPULATE = 0x0004,
    OTS_OBJECT_CONTROL = 0x0005,
    OTS_FILTER_GET = 0x0006,
    OTS_LIST_CONTROL = 0x0007,
    OTS_LIST_GOTO = 0x0008,
    OTS_FILTER_SET = 0x0009,
    OTS_FILTER_SET_TIME = 0x000A,
    OTS_FILTER_SET_SIZE = 0x000B,
    OTS_FILTER_SET_NAME = 0x000C,
    OTS_FILTER_SET_TYPE = 0x000D,
    OTS_COC_CONNECT = 0x000E,
};

/// Configuration bit field meaning for OTS_ADD request
enum ots_add_cfg_bf
{
    /// Indicate if Write property of Object Name characteristic is supported (=1) or not
    OTS_ADD_CFG_NAME_WRITE_POS = 0,
    OTS_ADD_CFG_NAME_WRITE_BIT = CO_BIT(OTS_ADD_CFG_NAME_WRITE_POS),

    /// Indicate if Object First-Created characteristic is supported (=1) or not
    OTS_ADD_CFG_FIRST_CREATED_POS = 1,
    OTS_ADD_CFG_FIRST_CREATED_BIT = CO_BIT(OTS_ADD_CFG_FIRST_CREATED_POS),

    /// Indicate if Write property of Object First-Created characteristic is supported (=1) or not
    OTS_ADD_CFG_FIRST_CREATED_WRITE_POS = 2,
    OTS_ADD_CFG_FIRST_CREATED_WRITE_BIT = CO_BIT(OTS_ADD_CFG_FIRST_CREATED_WRITE_POS),

    /// Indicate if Object Last-Modified characteristic is supported (=1) or not
    OTS_ADD_CFG_LAST_MODIFIED_POS = 3,
    OTS_ADD_CFG_LAST_MODIFIED_BIT = CO_BIT(OTS_ADD_CFG_LAST_MODIFIED_POS),

    /// Indicate if the Server has access to a real time clock (=1) or not
    OTS_ADD_CFG_ACCESS_CLOCK_POS = 4,
    OTS_ADD_CFG_ACCESS_CLOCK_BIT = CO_BIT(OTS_ADD_CFG_ACCESS_CLOCK_POS),

    /// Indicate if upper layer can store more than one object (=1) or not
    OTS_ADD_CFG_MORE_OBJECTS_POS = 5,
    OTS_ADD_CFG_MORE_OBJECTS_BIT = CO_BIT(OTS_ADD_CFG_MORE_OBJECTS_POS),

    /// Indicate if Write property of Object Properties characteristic is supported (=1) or not
    OTS_ADD_CFG_PROPERTIES_WRITE_POS = 6,
    OTS_ADD_CFG_PROPERTIES_WRITE_BIT = CO_BIT(OTS_ADD_CFG_PROPERTIES_WRITE_POS),

    /// Indicate if Object List Filter characteristic is supported (=1) or not
    OTS_ADD_CFG_LIST_FILTER_POS = 7,
    OTS_ADD_CFG_LIST_FILTER_BIT = CO_BIT(OTS_ADD_CFG_LIST_FILTER_POS),

    /// Indicate if Object Changed characteristic is supported (=1) or not
    OTS_ADD_CFG_CHANGED_POS = 8,
    OTS_ADD_CFG_CHANGED_BIT = CO_BIT(OTS_ADD_CFG_CHANGED_POS),

    /// Indicate if the instance of Object Transfer Service is a secondary one (=1)
    /// or primary one (=0)
    OTS_ADD_CFG_SECONDARY_POS = 9,
    OTS_ADD_CFG_SECONDARY_BIT = CO_BIT(OTS_ADD_CFG_SECONDARY_POS),
};

/// Client configuration bit field meaning
enum ots_cli_cfg_bf
{
    /// Indicate if sending of indications is enabled (= 1)
    /// or disabled for Object Action Control Point characteristic
    OTS_CLI_CFG_OACP_POS = 0,
    OTS_CLI_CFG_OACP_BIT = CO_BIT(OTS_CLI_CFG_OACP_POS),

    /// Indicate if sending of indications is enabled (= 1)
    /// or not for Object List Control Point characteristic
    /// Must be set to 0 if Object List Control Point characteristic is not supported
    OTS_CLI_CFG_OLCP_POS = 1,
    OTS_CLI_CFG_OLCP_BIT = CO_BIT(OTS_CLI_CFG_OLCP_POS),

    /// Indicate if sending of indications is enabled (= 1)
    /// or not for Object Changed characteristic
    /// Must be set to 0 if Object Changed characteristic is not supported
    OTS_CLI_CFG_CHANGED_POS = 2,
    OTS_CLI_CFG_CHANGED_BIT = CO_BIT(OTS_CLI_CFG_CHANGED_POS),
};

/// Object Transfer Service Set type values
enum ots_set_type
{
    /// Set Current Size value
    OTS_SET_TYPE_CURRENT_SIZE = 0,
    /// Set Allocated Size value
    OTS_SET_TYPE_ALLOCATED_SIZE,
    /// Set number of objects
    OTS_SET_TYPE_NUM_OBJECTS,
    /// Set Properties value
    OTS_SET_TYPE_PROPERTIES,
};

/// @} OTS_ENUM

/*
 * TYPES DEFINITION
 ****************************************************************************************
 */

/// @addtogroup OTS_STRUCT
/// @{

/// Configuration structure
typedef struct ots_cfg
{
    /// Number of Object Transfer Services the device may belongs to
    uint8_t nb_transfers;
} ots_cfg_t;

/// Object Changed Information
typedef struct ots_changed_info
{
    /// Object ID
    ot_object_id_t object_id;
    /// Flags field (see enum #otp_changed_flags_bf)
    uint8_t flags;
} ots_changed_info_t;

/// @} OTS_STRUCT

/*
 * CALLBACK FUNCTIONS DEFINITION
 ****************************************************************************************
 */

/// @addtogroup OTS_CALLBACK
/// @{

/**
 ****************************************************************************************
 * @brief Callback function called when client configuration for an instance of the
 *  Object Transfer Service has been updated
 *
 * @param[in] transfer_lid  Transfer local index
 * @param[in] con_lid       Connection local index
 * @param[in] cli_cfg_bf    Client configuration bit field (see enum #ots_cli_cfg_bf)
 ****************************************************************************************
 */
typedef void (*ots_cb_bond_data)(uint8_t transfer_lid, uint8_t con_lid, uint8_t cli_cfg_bf);

/**
 ****************************************************************************************
 * @brief Callback function called when an LE Credit Based Connection Oriented Link has
 * been established
 *
 * @param[in] con_lid       Connection local index
 * @param[in] peer_max_sdu  Maximum SDU size that the peer on the link can receive
 * @param[in] local_max_sdu Maximum SDU size that the local device can receive
 ****************************************************************************************
 */
typedef void (*ots_cb_coc_connected)(uint8_t con_lid, uint16_t peer_max_sdu,
                                     uint16_t local_max_sdu);

/**
 ****************************************************************************************
 * @brief Callback function called when an LE Credit Based Connection Oriented Link has
 * been disconnected
 *
 * @param[in] con_lid       Connection local index
 * @param[in] reason        Disconnection reason
 *****************************************************************************************
 */
typedef void (*ots_cb_coc_disconnected)(uint8_t con_lid, uint16_t reason);

/**
 ****************************************************************************************
 * @brief Callback function called when data is received through LE Credit Based Connection
 * Oriented Link
 *
 * @param[in] con_lid       Connection local index
 * @param[in] length        SDU data length
 * @param[in] p_sdu         Pointer to SDU data
 *****************************************************************************************
 */
typedef void (*ots_cb_coc_data)(uint8_t con_lid, uint16_t length, const uint8_t* p_sdu);

/**
 ****************************************************************************************
 * @brief Callback function called in order to retrieve name of current object for an
 * instance of the Object Transfer Service
 *
 * @param[in] con_lid       Connection local index
 * @param[in] transfer_lid  Transfer local index
 * @param[in] object_lid    Object local index
 * @param[in] token         Token value to return in the confirmation
 * @param[in] offset        Offset
 * @param[in] max_len       Maximum length
 ****************************************************************************************
 */
typedef void (*ots_cb_get_name)(uint8_t con_lid, uint8_t transfer_lid, uint8_t object_lid,
                                uint16_t token, uint16_t offset, uint16_t max_len);

/**
 ****************************************************************************************
 * @brief  Callback function called when a peer Client requests to set the object name
 * for an instance of the Object Transfer Service
 *
 * @param[in] con_lid       Connection local index
 * @param[in] transfer_lid  Transfer local index
 * @param[in] object_lid    Object local index
 * @param[in] token         Token value to return in the confirmation
 * @param[in] name_len      Name length
 * @param[in] p_name        Pointer to name
 ****************************************************************************************
 */
typedef void (*ots_cb_set_name)(uint8_t con_lid, uint8_t transfer_lid, uint8_t object_lid,
                                uint16_t token, uint8_t name_len, const uint8_t* p_name);

/**
 ****************************************************************************************
 * @brief Callback function called when a peer Client requests to create a new object
 * for an instance of the Object Transfer Service
 *
 * @param[in] con_lid       Connection local index
 * @param[in] transfer_lid  Transfer local index
 * @param[in] token         Token value to return in the confirmation
 * @param[in] size          Minimum size of memory allocated for the object
 * @param[in] uuid_type     Indicate if UUID of the newly created object is 128-bit (!=0) or 16-bit
 * @param[in] p_uuid          Pointer to UUID
 ****************************************************************************************
 */
typedef void (*ots_cb_object_create)(uint8_t con_lid, uint8_t transfer_lid, uint16_t token,
                                     uint32_t size, uint8_t uuid_type, const void* p_uuid);

/**
 ****************************************************************************************
 * @brief Callback function called when a peer Client requests execution of
 * current object for an instance of the Object Transfer Service
 *
 * @param[in] con_lid       Connection local index
 * @param[in] transfer_lid  Transfer local index
 * @param[in] object_lid    Object local index
 * @param[in] token         Token value to return in the confirmation
 * @param[in] param_len     Parameter length
 * @param[in] p_param       Pointer to execution parameter
 ****************************************************************************************
 */
typedef void (*ots_cb_object_execute)(uint8_t con_lid, uint8_t transfer_lid, uint8_t object_lid,
                                      uint16_t token, uint16_t param_len, const uint8_t* p_param);

/**
 ****************************************************************************************
 * @brief Callback function called when a peer Client requests to manipulate
 * (read, write, calculate checksum) current object for an instance of the
 * Object Transfer Service
 *
 * @param[in] con_lid       Connection local index
 * @param[in] transfer_lid  Transfer local index
 * @param[in] object_lid    Object local index
 * @param[in] token         Token value to return in the confirmation
 * @param[in] opcode        Operation code value
 * @param[in] offset        Position of the first octet to calculate checksum/read/write/
 *                          on the Current Object
 * @param[in] length        Total number of octets to calculate checksum/read/write on
*                           the Current Object
 * @param[in] mode          Mode bit field
 ****************************************************************************************
 */
typedef void (*ots_cb_object_manipulate)(uint8_t con_lid, uint8_t transfer_lid, uint8_t object_lid,
                                         uint16_t token, uint8_t opcode, uint32_t offset,
                                         uint32_t length, uint8_t mode);

/**
 ****************************************************************************************
 * @brief Callback function called when a peer Client requests an operation on the
 * current object for an instance of the Object Transfer Service
 *
 * @param[in] con_lid       Connection local index
 * @param[in] transfer_lid  Transfer local index
 * @param[in] object_lid    Object local index
 * @param[in] token         Token value to return in the confirmation
 * @param[in] opcode        Operation code value
 ****************************************************************************************
 */
typedef void (*ots_cb_object_control)(uint8_t con_lid, uint8_t transfer_lid, uint8_t object_lid,
                                      uint16_t token, uint8_t opcode);

/**
 ****************************************************************************************
 * @brief Callback function called when a peer Client requests to get one of the
 * current filter rules for an instance of the Object Transfer Service
 *
 * @param[in] con_lid       Connection local index
 * @param[in] transfer_lid  Transfer local index
 * @param[in] filter_lid    Filter local index
 * @param[in] ots_token     Token value to return in the confirmation
 * @param[in] offset        Offset
 * @param[in] max_len       Maximum length
 ****************************************************************************************
 */
typedef void (*ots_cb_filter_get)(uint8_t con_lid, uint8_t transfer_lid, uint8_t filter_lid,
                                  uint32_t ots_token, uint16_t offset, uint16_t max_len);

/**
 ****************************************************************************************
 * @brief Callback function called when a peer Client requests information about
 * the list of objects or request to change current object for an instance of the
 * Object Transfer Service
 *
 * @param[in] req_ind_code  Request indication code
 * @param[in] con_lid       Connection local index
 * @param[in] transfer_lid  Transfer local index
 * @param[in] token         Token value to return in the confirmation
 * @param[in] opcode        Operation code value
 * @param[in] p_value       Pointer to List Sort Order/Object ID
 ****************************************************************************************
 */
typedef void (*ots_cb_list)(uint16_t req_ind_code, uint8_t con_lid, uint8_t transfer_lid,
                            uint16_t token, uint8_t opcode, const void* p_value);

/**
 ****************************************************************************************
 * @brief Callback function called when a peer Client requests to set one of the
 * current filter rules for an instance of the Object Transfer Service
 *
 * @param[in] req_ind_code  Request indication code
 * @param[in] con_lid       Connection local index
 * @param[in] transfer_lid  Transfer local index
 * @param[in] filter_lid    Filter local index
 * @param[in] token         Token value to return in the confirmation
 * @param[in] filter_val    Filter value
 * @param[in] p_value1      Pointer to start time/minimum size/name length/UUID type
 * @param[in] p_value2      Pointer to end time/maximum size/name/UUID
 ****************************************************************************************
 */
typedef void (*ots_cb_filter_set)(uint16_t req_ind_code, uint8_t con_lid, uint8_t transfer_lid,
                                  uint8_t filter_lid, uint16_t token, uint8_t filter_val,
                                  const void* p_value1, const void* p_value2);

/**
 ****************************************************************************************
 * @brief Callback function called when a peer Client requests to establish a LE Credit
 * Based Connection Oriented Link
 *
 * @param[in] con_lid       Connection local index
 * @param[in] token         Token value to return in the confirmation
 * @param[in] peer_max_sdu  Maximum SDU size that the peer on the link can receive
 ****************************************************************************************
 */
typedef void (*ots_cb_coc_connect)(uint8_t con_lid, uint16_t token, uint16_t peer_max_sdu);

/**
 ****************************************************************************************
 * @brief Callback function called when a command has been handled
 *
 * @param[in] cmd_code      Command code (see enum #otc_cmd_codes)
 * @param[in] status        Status
 * @param[in] con_lid       Connection local index
 ****************************************************************************************
 */
typedef void (*ots_cb_cmp_evt)(uint16_t cmd_code, uint16_t status, uint8_t con_lid);

/*
 * CALLBACK SET DEFINITION
 ****************************************************************************************
 */

/// Set of callback functions for Object Transfer Server
typedef struct ots_cb
{
    /// Callback function called when client configuration for an instance of
    ///  the Object Transfer Service has been updated
    ots_cb_bond_data cb_bond_data;
    /// Callback function called when an LE Credit Based Connection Oriented Link has
    /// been established for an instance of the Object Transfer Service
    ots_cb_coc_connected cb_coc_connected;
    /// Callback function called when an LE Credit Based Connection Oriented Link has
    /// been disconnected for an instance of the Object Transfer Service
    ots_cb_coc_disconnected cb_coc_disconnected;
    /// Callback function called when data is received through LE Credit Based Connection
    /// Oriented Link for an instance of the Object Transfer Service
    ots_cb_coc_data cb_coc_data;
    /// Callback function called in order to retrieve name of current object
    /// for an instance of the Object Transfer Service
    ots_cb_get_name cb_get_name;
    /// Callback function called when a peer Client requests to set the object name
    /// for an instance of the Object Transfer Service
    ots_cb_set_name cb_set_name;
    /// Callback function called when a peer Client requests to create a new object
    /// for an instance of the Object Transfer Service
    ots_cb_object_create cb_object_create;
    /// Callback function called when a peer Client requests execution of current object
    /// for an instance of the Object Transfer Service
    ots_cb_object_execute cb_object_execute;
    /// Callback function called when a peer Client requests to manipulate
    /// (read, write, calculate checksum) current object for an instance of
    /// the Object Transfer Service
    ots_cb_object_manipulate cb_object_manipulate;
    /// Callback function called when a peer Client requests an operation on the
    /// current object for an instance of the Object Transfer Service
    ots_cb_object_control cb_object_control;
    /// Callback function called when a peer Client requests to get one of the
    /// current filter rules for an instance of the Object Transfer Service
    ots_cb_filter_get cb_filter_get;
    /// Callback function called when a peer Client requests information about
    /// the list of objects or request to change current object for an instance of
    /// the Object Transfer Service
    ots_cb_list cb_list;
    /// Callback function called when a peer Client requests to set one of the
    /// current filter rules for an instance of the Object Transfer Service
    ots_cb_filter_set cb_filter_set;
    /// Callback function called when a peer Client requests to establish a LE Credit
    /// Based Connection Oriented Link for an instance of the Object Transfer Service
    ots_cb_coc_connect cb_coc_connect;
    /// Callback function called when a command has been handled
    ots_cb_cmp_evt cb_cmp_evt;
} ots_cb_t;

/// @} OTS_CALLBACK

/*
 * API FUNCTIONS DECLARATION
 ****************************************************************************************
 */

/// @addtogroup OTS_FUNCTION
/// @{

/**
 ****************************************************************************************
 * @brief Disconnect a LE Credit Based Connection Oriented Link
 *
 * @param[in] con_lid       Connection local index
 *
 * @return An error status
 ****************************************************************************************
 */
uint16_t ots_coc_disconnect(uint8_t con_lid);

/**
 ****************************************************************************************
 * @brief Release buffers which have been consumed by preceding data reception triggered
 * by OTS_DATA indication.
 *
 * @param[in] con_lid       Connection local index
 *
 * @return An error status
 ****************************************************************************************
 */
uint16_t ots_coc_release(uint8_t con_lid);

/**
 ****************************************************************************************
 * @brief Transfer Object content data through a LE Credit Based Connection Oriented Link
 *
 * @param[in] con_lid       Connection local index
 * @param[in] length        SDU length
 * @param[in] p_sdu         Pointer to SDU to be transferred to the peer device
 *
 * @return An error status
 ****************************************************************************************
 */
uint16_t ots_coc_send(uint8_t con_lid, uint16_t length, const uint8_t* p_sdu);

/**
 ****************************************************************************************
 * @brief Add and configure an instance of the Object Transfer Service in the database.
 * A local index is allocated for the added instance and returned in the response message.
 *
 * @param[in] cfg_bf            Configuration bit field (see enum #ots_add_cfg_bf)
 * @param[in] shdl              Required start handle
 *                              If set to GATT_INVALID_HDL, the start handle will be automatically chosen
 * @param[in] oacp_features     Object Action Control Point features (see enum #otp_oacp_feat_bf)
 * @param[in] olcp_features     Object List Control Point features (see enum #otp_olcp_feat_bf)
 * @param[out] p_transfer_lid   Pointer at which allocated transfer local index is returned
 * @param[out] p_start_handle   Pointer at which start handle of added service is returned
 *
 * @return An error status
 ****************************************************************************************
 */
uint16_t ots_add(uint16_t cfg_bf, uint16_t shdl, uint32_t oacp_features, uint32_t olcp_features,
                 uint8_t* p_transfer_lid, uint16_t* p_start_handle);

/**
 ****************************************************************************************
 * @brief Set bonding information for instances of the Object Transfer Service after
 * connection with a Client device with which a bonded relationship had been established
 * during a previous connection.
 *
 * @param[in] con_lid           Connection local index
 * @param[in] transfer_lid      Transfer local index
 * @param[in] cli_cfg_bf        Client configuration bit field (see enum #ots_cli_cfg_bf)
 * @param[in] evt_cfg_bf        Event configuration bit field (see enum #ots_cli_cfg_bf)
 * @param[in] nb_changes        Number of Object Changed indications to send
 * @param[in] p_changed_info    Pointer to Object Changed information to inform
 *
 * @return An error status
 ****************************************************************************************
 */
uint16_t ots_restore_bond_data(uint8_t con_lid, uint8_t transfer_lid, uint8_t cli_cfg_bf, uint8_t evt_cfg_bf,
                               uint8_t nb_changes, const ots_changed_info_t *p_changed_info);

/**
 ****************************************************************************************
 * @brief Set bonding information for instances of the Object Transfer Service after
 * connection with a Client device with which a bonded relationship had been established
 * during a previous connection.
 *
 * @param[in] p_object_id           Pointer to Object ID
 * @param[in] current_size          Current Size field represents the actual number of
*                                   octets of the object
 * @param[in] allocated_size        Allocated Size field represents the number of octets
 *                                  allocated for the object
 * @param[in] p_first_created_time  Pointer to First-Created time
 * @param[in] p_last_modified_time  Pointer to Last-Modified time
 * @param[in] properties            Object Properties field
 * @param[in] uuid_type             Indicate if UUID of the newly created object is
 *                                  128-bit (!=0) or 16-bit
 * @param[in] p_uuid                Pointer to UUID
 * @param[out] p_object_lid         Pointer at which allocated object local index is returned

 * @return An error status
 ****************************************************************************************
 */
uint16_t ots_object_add(const ot_object_id_t* p_object_id, uint32_t current_size, uint32_t allocated_size,
                        const prf_date_time_t* p_first_created_time, const prf_date_time_t* p_last_modified_time,
                        uint32_t properties, uint8_t uuid_type, const void* p_uuid, uint8_t* p_object_lid);

/**
 ****************************************************************************************
 * @brief Remove an object from the object list of Server database.
 *
 * @param[in] object_lid        Object local index
 *
 * @return An error status
 ****************************************************************************************
 */
uint16_t ots_object_remove(uint8_t object_lid);

/**
 ****************************************************************************************
 * @brief Update the object currently exposed to a Client device for a given
 * Object Transfer Service instance.
 *
 * @param[in] con_lid           Connection local index
 * @param[in] transfer_lid      Transfer local index
 * @param[in] object_lid        Object local index
 *
 * @return An error status
 ****************************************************************************************
 */
uint16_t ots_object_change(uint8_t con_lid, uint8_t transfer_lid, uint8_t object_lid);

/**
 ****************************************************************************************
 * @brief Indicate that an object that has not been added by OTS_OBJECT_CHANGE request
 * has been updated by the Server device.
 *
 * @param[in] flags             Flags field (see enum #otp_changed_flags_bf)
 * @param[in] p_object_id       Pointer to Object ID
 *
 * @return An error status
 ****************************************************************************************
 */
uint16_t ots_object_changed(uint8_t flags, const ot_object_id_t* p_object_id);

/**
 ****************************************************************************************
 * @brief Update number of object in the list or Current Size/Properties of the
 * currently exposed object.
 *
 * @param[in] object_lid        Object local index
 * @param[in] set_type          Set type (see enum #ots_set_type)
 * @param[in] value             Value that could be:
 *                              Current Size field represents the actual number of octets of the object
 *                              Object Properties field (see enum #otp_prop_bf)
 *                              Number of objects in the list
 *
 * @return An error status
 ****************************************************************************************
 */
uint16_t ots_set(uint8_t object_lid, uint8_t set_type, uint32_t value);

/**
 ****************************************************************************************
 * @brief Update time at which the currently exposed object has been modified.
 *
 * @param[in] object_lid        Object local index
 * @param[in] p_time            Pointer to Last-Modified time
 *
 * @return An error status
 ****************************************************************************************
 */
uint16_t ots_set_time(uint8_t object_lid, const prf_date_time_t* p_time);

/**
 ****************************************************************************************
 * @brief Confirmation for OTS_GET_NAME request indication.
 *
 * @param[in] accept            Indicate if request is accepted or not
 * @param[in] con_lid           Connection local index
 * @param[in] token             Token value to return in the confirmation
 * @param[in] name_len          Name length
 * @param[in] p_name            Pointer to name
 ****************************************************************************************
 */
void ots_cfm_get_name(bool accept, uint8_t con_lid, uint16_t token, uint8_t name_len, const uint8_t* p_name);

/**
 ****************************************************************************************
 * @brief Confirmation for OTS_SET_NAME request indication.
 *
 * @param[in] accept            Indicate if request is accepted or not
 * @param[in] con_lid           Connection local index
 * @param[in] token             Token value to return in the confirmation
 ****************************************************************************************
 */
void ots_cfm_set_name(bool accept, uint8_t con_lid, uint16_t token);

/**
 ****************************************************************************************
 * @brief Confirmation for OTS_OBJECT_CREATE request indication or OTS_OBJECT_MANIPULATE
 * request indication or OTS_OBJECT_CONTROL request indication.
 *
 * @param[in] result_code       Result code (see enum #otp_oacp_result_code)
 * @param[in] con_lid           Connection local index
 * @param[in] transfer_lid      Transfer local index
 * @param[in] token             Token value to return in the confirmation
 * @param[in] checksum          Calculated checksum value included in the response
 *                              if operation code is OTP_OACP_OPCODE_CALCULATE_CHECKSUM
 *                              and result code is OTP_OACP_RESULT_SUCCESS
 ****************************************************************************************
 */
void ots_cfm_object_control(uint8_t result_code, uint8_t con_lid, uint8_t transfer_lid,
                            uint16_t token, uint32_t checksum);

/**
 ****************************************************************************************
 * @brief Confirmation for OTS_OBJECT_EXECUTE request indication.
 *
 * @param[in] result_code       Result code (see enum #otp_oacp_result_code)
 * @param[in] con_lid           Connection local index
 * @param[in] transfer_lid      Transfer local index
 * @param[in] token             Token value to return in the confirmation
 * @param[in] rsp_len           Response parameter length
 * @param[in] p_rsp             Pointer to response
 ****************************************************************************************
 */
void ots_cfm_object_execute(uint8_t result_code, uint8_t con_lid, uint8_t transfer_lid, uint16_t token,
                            uint16_t rsp_len, const uint8_t* p_rsp);

/**
 ****************************************************************************************
 * @brief Confirmation for OTS_FILTER_GET request indication for OTP_FILTER_TYPE_NO_FILTER
 * and OTP_FILTER_TYPE_MARKED_OBJECTS filter values.
 *
 * @param[in] accept            Indicate if request is accepted or not
 * @param[in] con_lid           Connection local index
 * @param[in] transfer_lid      Transfer local index
 * @param[in] ots_token         Token value to return in the confirmation
 * @param[in] filter_val        Filter value (see enum #otp_filter_type)
 ****************************************************************************************
 */
void ots_cfm_filter_get(bool accept, uint8_t con_lid, uint8_t transfer_lid,
                        uint32_t ots_token, uint8_t filter_val);

/**
 ****************************************************************************************
 * @brief Confirmation for OTS_FILTER_GET request indication for OTP_FILTER_TYPE_CREATED_BETW
 * and OTP_FILTER_TYPE_MODIFIED_BETW filter values.
 *
 * @param[in] accept            Indicate if request is accepted or not
 * @param[in] con_lid           Connection local index
 * @param[in] transfer_lid      Transfer local index
 * @param[in] ots_token         Token value to return in the confirmation
 * @param[in] filter_val        Filter value (see enum #otp_filter_type)
 * @param[in] p_time_start      Pointer to left boundary of the timestamp interval
 * @param[in] p_time_end        Pointer to right boundary of the timestamp interval
 ****************************************************************************************
 */
void ots_cfm_filter_get_time(bool accept, uint8_t con_lid, uint8_t transfer_lid, uint32_t ots_token,
                             uint8_t filter_val, const prf_date_time_t* p_time_start,
                             const prf_date_time_t* p_time_end);

/**
 ****************************************************************************************
 * @brief Confirmation for OTS_FILTER_GET request indication for OTP_FILTER_TYPE_CURRENT_SIZE_BETW
 * and OTP_FILTER_TYPE_ALLOCATED_SIZE_BETW filter values.
 *
 * @param[in] accept            Indicate if request is accepted or not
 * @param[in] con_lid           Connection local index
 * @param[in] transfer_lid      Transfer local index
 * @param[in] ots_token         Token value to return in the confirmation
 * @param[in] filter_val        Filter value (see enum #otp_filter_type)
 * @param[in] size_min          Left boundary of the size interval
 * @param[in] size_max          Right boundary of the size interval
 ****************************************************************************************
 */
void ots_cfm_filter_get_size(bool accept, uint8_t con_lid, uint8_t transfer_lid, uint32_t ots_token,
                             uint8_t filter_val, uint32_t size_min, uint32_t size_max);

/**
 ****************************************************************************************
 * @brief Confirmation for OTS_FILTER_GET request indication for
 * OTP_FILTER_TYPE_NAME_STARTS_WITH, OTP_FILTER_TYPE_NAME_ENDS_WITH
 * OTP_FILTER_TYPE_NAME_CONTAINS and OTP_FILTER_TYPE_NAME_IS_EXACTLY filter values.
 *
 * @param[in] accept            Indicate if request is accepted or not
 * @param[in] con_lid           Connection local index
 * @param[in] transfer_lid      Transfer local index
 * @param[in] ots_token         Token value to return in the confirmation
 * @param[in] filter_val        Filter value (see enum #otp_filter_type)
 * @param[in] name_len          Name length
 * @param[in] p_name            Pointer to name
 ****************************************************************************************
 */
void ots_cfm_filter_get_name(bool accept, uint8_t con_lid, uint8_t transfer_lid, uint32_t ots_token,
                             uint8_t filter_val, uint8_t name_len, const uint8_t* p_name);

/**
 ****************************************************************************************
 * @brief Confirmation for OTS_FILTER_GET request indication for
 * OTP_FILTER_TYPE_OBJECT_TYPE filter value.
 *
 * @param[in] accept            Indicate if request is accepted or not
 * @param[in] con_lid           Connection local index
 * @param[in] transfer_lid      Transfer local index
 * @param[in] ots_token         Token value to return in the confirmation
 * @param[in] uuid_type         Indicate if UUID of the newly created object is 128-bit (!=0) or 16-bit
 * @param[in] p_uuid            Pointer to UUID
 ****************************************************************************************
 */
void ots_cfm_filter_get_type(bool accept, uint8_t con_lid, uint8_t transfer_lid,
                             uint32_t ots_token, uint8_t uuid_type, const void* p_uuid);

/**
 ****************************************************************************************
 * @brief Confirmation for OTS_LIST_CONTROL request indication or OTS_LIST_GOTO request indication.
 *
 * @param[in] result_code       Result code (see enum #otp_olcp_result_code)
 * @param[in] con_lid           Connection local index
 * @param[in] transfer_lid      Transfer local index
 * @param[in] token             Token value to return in the confirmation
 * @param[in] nb_object         Number of objects
 ****************************************************************************************
 */
void ots_cfm_list_control(uint8_t result_code, uint8_t con_lid, uint8_t transfer_lid,
                          uint16_t token, uint32_t nb_object);

/**
 ****************************************************************************************
 * @brief Confirmation for OTS_FILTER_SET request indication or
 * OTS_FILTER_SET_TIME request indication or OTS_FILTER_SET_SIZE request indication or
 *  OTS_FILTER_SET_NAME request indication or OTS_FILTER_SET_TYPE request indication.
 *
 * @param[in] accept            Indicate if request is accepted or not
 * @param[in] con_lid           Connection local index
 * @param[in] transfer_lid      Transfer local index
 * @param[in] token             Token value to return in the confirmation
 ****************************************************************************************
 */
void ots_cfm_filter_set(bool accept, uint8_t con_lid, uint8_t transfer_lid, uint16_t token);

/**
 ****************************************************************************************
 * @brief Confirmation for OTS_COC_CONNECT request indication
 *
 * @param[in] accept            Indicate if request is accepted or not
 * @param[in] con_lid           Connection local index
 * @param[in] token             Token value to return in the confirmation
 * @param[in] local_max_sdu     Maximum SDU size that the local device can receive
 ****************************************************************************************
 */
void ots_cfm_coc_connect(bool accept, uint8_t con_lid,
                         uint16_t token, uint16_t local_max_sdu);

/// @} OTS_FUNCTION

#endif //(BLE_OT_SERVER)

#endif // OTS_H_
