/**
 ****************************************************************************************
 *
 * @file otc.h
 *
 * @brief Object Transfer Client - Definitions
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef OTC_H_
#define OTC_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup OTC Object Transfer Profile Server
 * @ingroup OTP_API
 * @brief Description of Object Transfer Profile Client module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup OTC_ENUM Enumerations
 * @ingroup OTC
 * @brief Enumerations for Object Transfer Profile Client module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup OTC_STRUCT Structures
 * @ingroup OTC
 * @brief Structures for Object Transfer Profile Client module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup OTC_NATIVE_API Native API
 * @ingroup OTC
 * @brief Description of Native API for Object Transfer Profile Client module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup OTC_CALLBACK Callback Functions
 * @ingroup OTC_NATIVE_API
 * @brief Description of callback functions for Object Transfer Profile Client module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup OTC_FUNCTION Functions
 * @ingroup OTC_NATIVE_API
 * @brief Description of functions for Object Transfer Profile Client module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "otp.h"            // Object Transfer Profile Definitions

#if (BLE_OT_CLIENT)

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// @addtogroup OTC_ENUM
/// @{

/// List of OTC_CMD command codes
enum otc_cmd_codes
{
    OTC_DISCOVER = 0x0000,
    OTC_GET = 0x0001,
    OTC_SET_CFG = 0x0003,
    OTC_SET_NAME = 0x0004,
    OTC_SET_TIME = 0x0005,
    OTC_SET_PROPERTIES = 0x0006,
    OTC_OBJECT_CREATE = 0x0007,
    OTC_OBJECT_CONTROL = 0x0008,
    OTC_OBJECT_MANIPULATE = 0x0009,
    OTC_OBJECT_EXECUTE = 0x000A,
    OTC_LIST_CONTROL = 0x000B,
    OTC_LIST_GOTO = 0x000C,
    OTC_FILTER_SET = 0x000D,
    OTC_FILTER_SET_TIME = 0x000E,
    OTC_FILTER_SET_SIZE = 0x000F,
    OTC_FILTER_SET_NAME = 0x0010,
    OTC_FILTER_SET_TYPE = 0x0011,
    OTC_COC_CONNECT = 0x0012,
    OTC_COC_DISCONNECT = 0x0013,
    OTC_COC_SEND = 0x0014,
    OTC_COC_RELEASE = 0x0015,
};

/// Get type values
enum otc_get_type
{
    /// Get value of a single characteristic
    OTC_GET_TYPE_SINGLE = 0,
    /// Get all metadata of the Current Object
    OTC_GET_TYPE_OBJECT,
    /// Get values of all 3 instance of Object List Filter characteristic
    OTC_GET_TYPE_FILTER,
};

/// @} OTC_ENUM

/*
 * TYPES DEFINITION
 ****************************************************************************************
 */

/// @addtogroup OTC_STRUCT
/// @{

/// Content description structure for Object Transfer Service
typedef struct otc_ots_info
{
    /// Service description
    prf_svc_t svc_info;
    /// Characteristics description
    prf_char_t char_info[OTP_CHAR_TYPE_MAX];
    /// Descriptors description
    prf_desc_t desc_info[OTP_DESC_TYPE_MAX];
} otc_ots_info_t;

/// @} OTC_STRUCT

/*
 * CALLBACK FUNCTIONS DEFINITION
 ****************************************************************************************
 */

/// @addtogroup OTC_CALLBACK
/// @{

/**
 ****************************************************************************************
 * @brief Callback function called when an instance of the Object Transfer Service
 * has been found in Server device database
 *
 * @param[in] con_lid       Connection local index
 * @param[in] transfer_lid  Transfer local index
 * @param[in] p_ots_info    Pointer to Content description structure for the service
 ****************************************************************************************
 */
typedef void (*otc_cb_bond_data)(uint8_t con_lid, uint8_t transfer_lid, const otc_ots_info_t* p_ots_info);

/**
 ****************************************************************************************
 * @brief Callback function called when non-empty execution response for current object of
 * an instance of the Object Transfer Service has been received
 *
 * @param[in] con_lid       Connection local index
 * @param[in] transfer_lid  Transfer local index
 * @param[in] rsp_len       Response parameter length
 * @param[in] p_rsp         Pointer to response parameter
 ****************************************************************************************
 */
typedef void (*otc_cb_execute_rsp)(uint8_t con_lid, uint8_t transfer_lid, uint16_t rsp_len,
                                   const uint8_t* p_rsp);

/**
 ****************************************************************************************
 * @brief Callback function called when information about current object have been received
 *
 * @param[in] con_lid       Connection local index
 * @param[in] transfer_lid  Transfer local index
 * @param[in] char_type     Characteristic type (see enum #otp_char_type)
 * @param[in] value1        Object Action Control Point features for OTP_CHAR_TYPE_FEATURE characteristic type
*                           Current Size for OTP_CHAR_TYPE_SIZE characteristic type
 *                          Object Properties for OTP_CHAR_TYPE_PROPERTIES characteristic type
 * @param[in] value2        Object List Control Point features for OTP_CHAR_TYPE_FEATURE characteristic type
 *                          Allocated Size for OTP_CHAR_TYPE_SIZE characteristic type
 ****************************************************************************************
 */
typedef void (*otc_cb_value)(uint8_t con_lid, uint8_t transfer_lid, uint8_t char_type,
                             uint32_t value1, uint32_t value2);

/**
 ****************************************************************************************
 * @brief Callback function called when either time of creation or time of last update
 * for current object for an instance of the Object Transfer Service has been received
 *
 * @param[in] con_lid       Connection local index
 * @param[in] transfer_lid  Transfer local index
 * @param[in] char_type     Characteristic type (see enum #otp_char_type)
 * @param[in] p_time        Pointer to First-Created time or Last-Modified time
 ****************************************************************************************
 */
typedef void (*otc_cb_time)(uint8_t con_lid, uint8_t transfer_lid, uint8_t char_type,
                            const prf_date_time_t* p_time);

/**
 ****************************************************************************************
 * @brief Callback function called when Object ID of current object for an instance of
 * the Object Transfer Service has been received
 *
 * @param[in] con_lid       Connection local index
 * @param[in] transfer_lid  Transfer local index
 * @param[in] p_object_id   Pointer to Object ID
 ****************************************************************************************
 */
typedef void (*otc_cb_object_id)(uint8_t con_lid, uint8_t transfer_lid, const ot_object_id_t* p_object_id);

/**
 ****************************************************************************************
 * @brief Callback function called when UUID of current object for an instance of the
 * Object Transfer Service has been received
 *
 * @param[in] con_lid       Connection local index
 * @param[in] transfer_lid  Transfer local index
 * @param[in] uuid_type     Indicate if UUID of the newly created object is 128-bit (!=0) or 16-bit
 * @param[in] p_uuid        Pointer to 128-bit UUID or 16-bit UUID
 ****************************************************************************************
 */
typedef void (*otc_cb_type)(uint8_t con_lid, uint8_t transfer_lid, uint8_t uuid_type, const void* p_uuid);

/**
 ****************************************************************************************
 * @brief Callback function called when name of current object for an instance of the
 * Object Transfer Service has been received
 *
 * @param[in] con_lid       Connection local index
 * @param[in] transfer_lid  Transfer local index
 * @param[in] name_len      Name length
 * @param[in] p_name        Pointer to Name
 ****************************************************************************************
 */
typedef void (*otc_cb_name)(uint8_t con_lid, uint8_t transfer_lid, uint8_t name_len, const uint8_t* p_name);

/**
 ****************************************************************************************
 * @brief Callback function called when a filter rule has been received
 *
 * @param[in] con_lid       Connection local index
 * @param[in] transfer_lid  Transfer local index
 * @param[in] filter_lid    Filter local index
 * @param[in] filter_val    Filter value (see enum #otp_filter_type)
 * @param[in] p_value1      Pointer that can be casted to:
 *                          - Left boundary of the timestamp interval
 *                          - Left boundary of the size interval
 *                          - Name length
 *                          - UUID type
 * @param[in] p_value2      Pointer that can be casted to:
 *                          - Right boundary of the timestamp interval
 *                          - Right boundary of the size interval
 *                          - Name
 *                          - UUID
 ****************************************************************************************
 */
typedef void (*otc_cb_filter)(uint8_t con_lid, uint8_t transfer_lid, uint8_t filter_lid,
                              uint8_t filter_val, const void* p_value1, const void* p_value2);

/**
 ****************************************************************************************
 * @brief Callback function called when peer Server notifies that current object for
 * an instance of the Object Transfer Service has been updated
 *
 * @param[in] con_lid       Connection local index
 * @param[in] transfer_lid  Transfer local index
 * @param[in] flags         Flags field (see enum #otp_changed_flags_bf)
 * @param[in] p_object_id   Pointer to Object ID
 ****************************************************************************************
 */
typedef void (*otc_cb_changed)(uint8_t con_lid, uint8_t transfer_lid, uint8_t flags,
                               const ot_object_id_t* p_object_id);

/**
 ****************************************************************************************
 * @brief Callback function called when an LE Credit Based Connection Oriented Link has
 * been established
 *
 * @param[in] con_lid       Connection local index
 * @param[in] peer_max_sdu  Maximum SDU size that the peer on link can receive
 * @param[in] local_max_sdu Maximum SDU size that the local device can receive
 ****************************************************************************************
 */
typedef void (*otc_cb_coc_connected)(uint8_t con_lid, uint16_t peer_max_sdu, uint16_t local_max_sdu);

/**
 ****************************************************************************************
 * @brief Callback function called when an LE Credit Based Connection Oriented Link has
 * been disconnected for an instance of the Object Transfer Service
 *
 * @param[in] con_lid       Connection local index
 * @param[in] reason        Disconnection reason
 ****************************************************************************************
 */
typedef void (*otc_cb_coc_disconnected)(uint8_t con_lid, uint16_t reason);

/**
 ****************************************************************************************
 * @brief Callback function called when data is received through LE Credit Based Connection
 * Oriented Link for an instance of the Object Transfer Service
 *
 * @param[in] con_lid       Connection local index
 * @param[in] length        SDU data length
 * @param[in] p_sdu         Pointer to SDU data
 ****************************************************************************************
 */
typedef void (*otc_cb_coc_data)(uint8_t con_lid, uint16_t length, const uint8_t* p_sdu);

/**
 ****************************************************************************************
 * @brief Callback function called when a command has been handled
 *
 * @param[in] cmd_code      Command code (see enum #otc_cmd_codes)
 * @param[in] status        Status
 * @param[in] con_lid       Connection local index
 * @param[in] transfer_lid  Transfer local index
 * @param[in] value_1       Value that could be:
 *                          Indication-capable characteristic type (see enum #otp_char_type)
 *                          Filter local index
 *                          Operation code (see enum #otp_oacp_opcode, enum otp_olcp_opcode)
 * @param[in] value_2       Value that could be:
 *                          Get type (see enum #otc_get_type)
 *                          Result code (see enum #otp_oacp_result_code, enum otp_olcp_result_code)
 * @param[in] value_3       Value that could be:
 *                          Characteristic type (see enum #otp_char_type)
 *                          Number of objects
 *                          Calculated checksum
 ****************************************************************************************
 */
typedef void (*otc_cb_cmp_evt)(uint16_t cmd_code, uint16_t status, uint8_t con_lid, uint8_t transfer_lid,
                               uint8_t value_1, uint8_t value_2, uint32_t value_3);

/**
 ****************************************************************************************
 * @brief Callback function called when a service changed indication has been received from a Service device
 *
 * @param[in] con_lid       Connection local index
 ****************************************************************************************
 */
typedef void (*otc_cb_svc_changed)(uint8_t con_lid);

/*
 * CALLBACK SET DEFINITION
 ****************************************************************************************
 */

/// Set of callback functions for Object Transfer Client
typedef struct otc_cb
{
    /// Callback function called when an instance of the Object Transfer Service has been discovered
    otc_cb_bond_data cb_bond_data;
    /// Callback function called when non-empty execution response for current object of
    /// an instance of the Object Transfer Service has been receive
    otc_cb_execute_rsp cb_execute_rsp;
    /// Callback function called when information about current object have been received
    otc_cb_value cb_value;
    /// Callback function called when either time of creation or time of last update for
    /// current object for an instance of the Object Transfer Service has been received
    otc_cb_time cb_time;
    /// Callback function called when Object ID of current object for an instance of
    /// the Object Transfer Service has been received
    otc_cb_object_id cb_object_id;
    /// Callback function called when UUID of current object for an instance of
    /// the Object Transfer Service has been received
    otc_cb_type cb_type;
    /// Callback function called when name of current object for an instance of
    /// the Object Transfer Service has been received
    otc_cb_name cb_name;
    /// Callback function called when a filter rule has been received
    otc_cb_filter cb_filter;
    /// Callback function called when peer Server notifies that current object
    /// for an instance of the Object Transfer Service has been updated
    otc_cb_changed cb_changed;
    /// Callback function called when an LE Credit Based Connection Oriented Link has
    /// been established for an instance of the Object Transfer Service
    otc_cb_coc_connected cb_coc_connected;
    /// Callback function called when an LE Credit Based Connection Oriented Link has
    /// been established for an instance of the Object Transfer Service
    otc_cb_coc_disconnected cb_coc_disconnected;
    /// Callback function called when data is received through LE Credit Based Connection
    /// Oriented Link for an instance of the Object Transfer Service
    otc_cb_coc_data cb_coc_data;
    /// Callback function called when a command has been handled
    otc_cb_cmp_evt cb_cmp_evt;
    /// Callback function called when a service changed indication has been received from a Service device
    otc_cb_svc_changed cb_svc_changed;
} otc_cb_t;

/// @} OTC_CALLBACK

/*
 * API FUNCTIONS DECLARATION
 ****************************************************************************************
 */

/// @addtogroup OTC_FUNCTION
/// @{

/**
 ****************************************************************************************
 * @brief Enable use of Object Transfer Service block as Client for a connected device
 * with which no bonding has been established during a previous connection.
 *
 * @param[in] con_lid       Connection local index
 * @param[in] nb_ots_max    Maximum number of Object Transfer Service instances
 *                          that may be found
 * @param[in] svc_type      Service type
 * @param[in] shdl          Start handle (Shall be different from GATT_INVALID_HDL for searching secondary service)
 * @param[in] ehdl          End handle (Shall be different from GATT_INVALID_HDL for searching secondary service)
 * @return An error status
 ****************************************************************************************
 */
uint16_t otc_discover(uint8_t con_lid, uint8_t nb_ots_max, uint8_t svc_type, uint16_t shdl, uint16_t ehdl);

/**
 ****************************************************************************************
 * @brief Get value for one of the following characteristics in an instance of the
 * Object Transfer Service discovered in a Server device by OTP_GET_TYPE_SINGLE get type:
 *     - OTS Feature characteristic
 *     - Object Name characteristic
 *     - Object Type characteristic
 *     - Object Size characteristic
 *     - Object First-Created characteristic (only if supported)
 *     - Object Last-Modified characteristic (only if supported)
 *     - Object ID characteristic (only if supported)
 *     - Object Properties characteristic
 *     - Object List Filter characteristic (only if supported)
 * Get all metadata of the Current Object exposed to the Client device by OTP_GET_TYPE_OBJECT get type.
 * Get values of all 3 instance Object List Filter characteristic by OTP_GET_TYPE_FILTER get type.
 *
 * @param[in] con_lid       Connection local index
 * @param[in] transfer_lid  Transfer local index
 * @param[in] get_type      Get type (see enum #otc_get_type)
 * @param[in] char_type     Characteristic type (see enum #otp_char_type)
 *                          Meaningful only for OTC_GET_TYPE_SINGLE get type
 *
 * @return An error status
 ****************************************************************************************
 */
uint16_t otc_get(uint8_t con_lid, uint8_t transfer_lid, uint8_t get_type, uint8_t char_type);

/**
 ****************************************************************************************
 * @brief Enable or disable sending of indications for one of the following characteristics
 * in an instance of the Object Transfer Service discovered in a Server device database:
 *     - Object Action Control Point characteristic
 *     - Object List Control Point characteristic
 *     - Object Changed characteristic
 *
 * Client configuration for the indicated characteristic cannot be set if configuration
 * for this characteristic is currently being retrieved.
 *
 * @param[in] con_lid       Connection local index
 * @param[in] transfer_lid  Transfer local index
 * @param[in] char_type     Indication-capable characteristic type (see enum #otp_char_type)
 * @param[in] enable        Indicate if sending of indications must be enabled (!=0) or not
 *                          for the indicated characteristic
 *
 * @return An error status
 ****************************************************************************************
 */
uint16_t otc_set_cfg(uint8_t con_lid, uint8_t transfer_lid, uint8_t char_type, uint8_t enable);

/**
 ****************************************************************************************
 * @brief Set name value for Object Name characteristic in an instance of the Object
 * Transfer Service discovered in a Server device:
 *
 * @param[in] con_lid       Connection local index
 * @param[in] transfer_lid  Transfer local index
 * @param[in] name_len      Name length
 * @param[in] p_name        Pointer to name
 *
 * @return An error status
 ****************************************************************************************
 */
uint16_t otc_set_name(uint8_t con_lid, uint8_t transfer_lid, uint8_t name_len, const uint8_t* p_name);

/**
 ****************************************************************************************
 * @brief Set time value for Object First-Created characteristic or Object Last-Modified
 * characteristic in an instance of the Object Transfer Service discovered in a
 * Server device
 *
 * @param[in] con_lid       Connection local index
 * @param[in] transfer_lid  Transfer local index
 * @param[in] char_type     Characteristic type (see enum #otp_char_type)
 * @param[in] p_time        Pointer to First-Created time or Last-Modified time
 *
 * @return An error status
 ****************************************************************************************
 */
uint16_t otc_set_time(uint8_t con_lid, uint8_t transfer_lid, uint8_t char_type, const prf_date_time_t* p_time);

/**
 ****************************************************************************************
 * @brief Set properties value for Object Properties characteristic in an instance of
 * the Object Transfer Service discovered in a Server device
 *
 * @param[in] con_lid       Connection local index
 * @param[in] transfer_lid  Transfer local index
 * @param[in] properties    Properties (see enum #otp_prop_bf)
 *
 * @return An error status
 ****************************************************************************************
 */
uint16_t otc_set_properties(uint8_t con_lid, uint8_t transfer_lid, uint32_t properties);

/**
 ****************************************************************************************
 * @brief Require Server device to create a new, empty object on one of its exposed
 * Object Transfer Service (by setting Object Action Control Point characteristic
 * value on the indicated service).
 *
 * @param[in] con_lid       Connection local index
 * @param[in] transfer_lid  Transfer local index
 * @param[in] size          Minimum size of memory allocated for the object
 * @param[in] uuid_type     Indicate if UUID of the newly created object is 128-bit (!=0) or 16-bit
 * @param[in] p_uuid        Pointer to UUID
 *
 * @return An error status
 ****************************************************************************************
 */
uint16_t otc_object_create(uint8_t con_lid, uint8_t transfer_lid, uint32_t size, uint8_t uuid_type,
                           const void* p_uuid);

/**
 ****************************************************************************************
 * @brief Require Server device to do following procedures on one of its exposed
 * Object Transfer Service (by setting Object Action Control Point characteristic
 * value on the indicated service):
 *     - Delete procedure
 *     - Abort procedure
 *
 * @param[in] con_lid       Connection local index
 * @param[in] transfer_lid  Transfer local index
 * @param[in] opcode        Operation code value (see enum #otp_oacp_opcode)
 *
 * @return An error status
 ****************************************************************************************
 */
uint16_t otc_object_control(uint8_t con_lid, uint8_t transfer_lid, uint8_t opcode);

/**
 ****************************************************************************************
 * @brief Require Server device to do following procedures on one of its exposed
 * Object Transfer Service (by setting Object Action Control Point characteristic
 * value on the indicated service):
 *     - Calculate Checksum procedure
 *     - Read procedure
 *     - Write procedure
 *
 * @param[in] con_lid       Connection local index
 * @param[in] transfer_lid  Transfer local index
 * @param[in] opcode        Operation code value (see enum #otp_oacp_opcode)
 * @param[in] offset        Position of the first octet to calculate checksum/read/write/
 *                          on the Current Object
 * @param[in] length        Total number of octets to calculate checksum/read/write
 *                          on the Current Object
 * @param[in] mode          Mode bit field, only for write operation (see enum #otp_oacp_mode_bf)
 *
 * @return An error status
 ****************************************************************************************
 */
uint16_t otc_object_manipulate(uint8_t con_lid, uint8_t transfer_lid, uint8_t opcode,
                               uint32_t offset, uint32_t length, uint8_t mode);

/**
 ****************************************************************************************
 * @brief Require Server device to perform an Execute procedure on one of its exposed
 * Object Transfer Service (by setting Object Action Control Point characteristic
 * value on the indicated service).
 *
 * @param[in] con_lid       Connection local index
 * @param[in] transfer_lid  Transfer local index
 * @param[in] param_len     Parameter length
 * @param[in] p_param       Pointer to parameter
 *
 * @return An error status
 ****************************************************************************************
 */
uint16_t otc_object_execute(uint8_t con_lid, uint8_t transfer_lid, uint16_t param_len, const uint8_t* p_param);

/**
 ****************************************************************************************
 * @brief Require Server device to do following procedures on one of its exposed
 * Object Transfer Service (by setting Object List Control Point characteristic
 * value on the indicated service):
 *     - Order procedure (only if supported)
 *     - Request Number of Objects procedure (only if supported)
 *     - Clear Marking procedure (only if supported)
 *
 * @param[in] con_lid       Connection local index
 * @param[in] transfer_lid  Transfer local index
 * @param[in] opcode        Operation code value (see enum #otp_olcp_opcode)
 * @param[in] order         List Sort Order (see enum #otp_olcp_order)
 *                          Meaningful only for Order operation code
 *
 * @return An error status
 ****************************************************************************************
 */
uint16_t otc_list_control(uint8_t con_lid, uint8_t transfer_lid, uint8_t opcode, uint8_t order);

/**
 ****************************************************************************************
 * @brief Require Server device to do following procedures on one of its exposed
 * Object Transfer Service (by setting Object List Control Point characteristic
 * value on the indicated service).
 *     - First procedure
 *     - Last procedure
 *     - Previous procedure
 *     - Next procedure
 *     - Go To procedure (only if supported)
 *
 * @param[in] con_lid       Connection local index
 * @param[in] transfer_lid  Transfer local index
 * @param[in] opcode        Operation code value (see enum #otp_olcp_opcode)
 * @param[in] p_object_id   Pointer to Object ID
 *                          Meaningful only for Go To operation code
 *
 * @return An error status
 ****************************************************************************************
 */
uint16_t otc_list_goto(uint8_t con_lid, uint8_t transfer_lid, uint8_t opcode, const ot_object_id_t* p_object_id);

/**
 ****************************************************************************************
 * @brief Set one of the following filter values for an instance of Object List Filter
 * characteristic in an instance of the Object Transfer Service discovered in a Server device:
 *     - No Filter (0x00)
 *     - Marked Objects (0x0A)
 *
 * @param[in] con_lid       Connection local index
 * @param[in] transfer_lid  Transfer local index
 * @param[in] filter_lid    Filter local index
 * @param[in] filter_val    Filter value (see enum #otp_filter_type)
 *
 * @return An error status
 ****************************************************************************************
 */
uint16_t otc_filter_set(uint8_t con_lid, uint8_t transfer_lid, uint8_t filter_lid, uint8_t filter_val);

/**
 ****************************************************************************************
 * @brief Set one of the following filter values for an instance of Object List Filter
 * characteristic in an instance of the Object Transfer Service discovered in a Server device:
 *     - Created between (0x06)
 *     - Modified between (0x07)
 *
 * @param[in] con_lid       Connection local index
 * @param[in] transfer_lid  Transfer local index
 * @param[in] filter_lid    Filter local index
 * @param[in] filter_val    Filter value (see enum #otp_filter_type)
 * @param[in] p_time_start  Pointer to left boundary of the timestamp interval
 * @param[in] p_time_end    Pointer to right boundary of the timestamp interval
 *
 * @return An error status
 ****************************************************************************************
 */
uint16_t otc_filter_set_time(uint8_t con_lid, uint8_t transfer_lid, uint8_t filter_lid, uint8_t filter_val,
                             const prf_date_time_t* p_time_start, const prf_date_time_t* p_time_end);

/**
 ****************************************************************************************
 * @brief Set one of the following filter values for an instance of Object List Filter
 * characteristic in an instance of the Object Transfer Service discovered in a Server device:
 *     - Current Size between (0x08)
 *     - Allocated Size between (0x09)
 *
 * @param[in] con_lid       Connection local index
 * @param[in] transfer_lid  Transfer local index
 * @param[in] filter_lid    Filter local index
 * @param[in] filter_val    Filter value (see enum #otp_filter_type)
 * @param[in] size_min      Left boundary of the size interval
 * @param[in] size_max      Right boundary of the size interval
 *
 * @return An error status
 ****************************************************************************************
 */
uint16_t otc_filter_set_size(uint8_t con_lid, uint8_t transfer_lid, uint8_t filter_lid, uint8_t filter_val,
                             uint32_t size_min, uint32_t size_max);

/**
 ****************************************************************************************
 * @brief Set one of the following filter values for an instance of Object List Filter
 * characteristic in an instance of the Object Transfer Service discovered in a Server device:
 *     - Name Starts With (0x01)
 *     - Name Ends With (0x02)
 *     - Name Contains (0x03)
 *     - Name is Exactly (0x04)
 *
 * @param[in] con_lid       Connection local index
 * @param[in] transfer_lid  Transfer local index
 * @param[in] filter_lid    Filter local index
 * @param[in] filter_val    Filter value (see enum #otp_filter_type)
 * @param[in] name_len      Name length
 * @param[in] p_name        Pointer to name
 *
 * @return An error status
 ****************************************************************************************
 */
uint16_t otc_filter_set_name(uint8_t con_lid, uint8_t transfer_lid, uint8_t filter_lid, uint8_t filter_val,
                             uint8_t name_len, const uint8_t* p_name);

/**
 ****************************************************************************************
 * @brief Set Object Type filter value (0x05) for an instance of Object List Filter
 * characteristic in an instance of the Object Transfer Service discovered in a Server device
 *
 * @param[in] con_lid       Connection local index
 * @param[in] transfer_lid  Transfer local index
 * @param[in] filter_lid    Filter local index
 * @param[in] uuid_type     Indicate if UUID of the newly created object is 128-bit (!=0) or 16-bit
 * @param[in] p_uuid        Pointer to UUID
 *
 * @return An error status
 ****************************************************************************************
 */
uint16_t otc_filter_set_type(uint8_t con_lid, uint8_t transfer_lid, uint8_t filter_lid,
                             uint8_t uuid_type, const void* p_uuid);

/**
 ****************************************************************************************
 * @brief Establish a LE Credit Based Connection Oriented Link
 *
 * @param[in] con_lid       Connection local index
 * @param[in] local_max_sdu Maximum SDU size that the local device can receive
 *
 * @return An error status
 ****************************************************************************************
 */
uint16_t otc_coc_connect(uint8_t con_lid, uint16_t local_max_sdu);

/**
 ****************************************************************************************
 * @brief Disconnect a LE Credit Based Connection Oriented Link
 *
 * @param[in] con_lid       Connection local index
 *
 * @return An error status
 ****************************************************************************************
 */
uint16_t otc_coc_disconnect(uint8_t con_lid);

/**
 ****************************************************************************************
 * @brief Transfer Object content data through a LE Credit Based Connection Oriented Link
 *
 * @param[in] con_lid       Connection local index
 * @param[in] length        SDU length
 * @param[in] p_sdu         Pointer to SDU to be transferred to the peer device
 *
 * @return An error status
 * ****************************************************************************************
 */
uint16_t otc_coc_send(uint8_t con_lid, uint16_t length, const uint8_t* p_sdu);

/**
 ****************************************************************************************
 * @brief Release buffers which have been consumed by preceding data reception triggered
 * by OTC_DATA indication
 *
 * @param[in] con_lid       Connection local index
 *
 * @return An error status
 ****************************************************************************************
 */
uint16_t otc_coc_release(uint8_t con_lid);

/**
 ****************************************************************************************
 * @brief Enable use of Object Transfer Profile block as Client for a connected device
 * with which a bonding has been established during a previous connection.
 *
 * @param[in] con_lid       Connection local index
 * @param[in] nb_ots        Number of Object Transfer Service instance discovered in peer device database
 * @param[in] p_ots_info    Pointer to content description of each Object Transfer Service instance
 *
 * @return An error status
 ****************************************************************************************
 */
uint16_t otc_restore_bond_data(uint8_t con_lid, uint8_t nb_ots, const otc_ots_info_t* p_ots_info);

/// @} OTC_FUNCTION

#endif //(BLE_OT_CLIENT)

#endif // OTC_H_
