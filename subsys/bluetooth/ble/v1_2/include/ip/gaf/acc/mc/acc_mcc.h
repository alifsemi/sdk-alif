/**
 ****************************************************************************************
 *
 * @file acc_mcc.h
 *
 * @brief Audio Content Control - Media Control Client - Definitions
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef ACC_MCC_H_
#define ACC_MCC_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup ACC_MCC Media Control Service Client
 * @ingroup ACC_MC
 * @brief Description of Media Control Service Client module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ACC_MCC_ENUM Enumerations
 * @ingroup ACC_MCC
 * @brief Enumerations for Media Control Service Client module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ACC_MCC_STRUCT Structures
 * @ingroup ACC_MCC
 * @brief Structures for Media Control Service Client module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ACC_MCC_NATIVE_API Native API
 * @ingroup ACC_MCC
 * @brief Description of Native API for Media Control Service Client module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ACC_MCC_CALLBACK Callback Functions
 * @ingroup ACC_MCC_NATIVE_API
 * @brief Description of callback functions for Media Control Service Client module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ACC_MCC_FUNCTION Functions
 * @ingroup ACC_MCC_NATIVE_API
 * @brief Description of functions for Media Control Service Client module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "acc_mc.h"            // Audio Content Control - Media Control Definition
#include "gaf.h"               // Generic Audio Framework Definitions

/*
 * DEFINES
 ****************************************************************************************
 */

/// Maximum number of Media Control Service (Generic Media Control Service excluded)
/// that can be handled (limited by Media local index set in metainfo value provided to GATT)
#define ACC_MCC_NB_MCS_MAX          (126)

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// @addtogroup ACC_MCC_ENUM
/// @{

/// List of command type values for Media Control Service Client role
enum acc_mcc_cmd_type
{
    /// Discover
    ACC_MCC_CMD_TYPE_DISCOVER = 0,
    /// Get
    ACC_MCC_CMD_TYPE_GET,
    /// Set Configuration
    ACC_MCC_CMD_TYPE_SET_CFG = 3,
    /// Set
    ACC_MCC_CMD_TYPE_SET,
    /// Set Object ID
    ACC_MCC_CMD_TYPE_SET_OBJECT_ID,
    /// Control
    ACC_MCC_CMD_TYPE_CONTROL,
    /// Search
    ACC_MCC_CMD_TYPE_SEARCH,
};

/// @} ACC_MCC_ENUM

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// @addtogroup ACC_MCC_STRUCT
/// @{

/// Content description structure for Media Control Service
typedef struct acc_mcc_mcs_info
{
    /// Service description
    prf_svc_t svc_info;
    /// UUID
    uint16_t uuid;
    /// Characteristics description
    prf_char_t char_info[ACC_MC_CHAR_TYPE_MAX];
    /// Descriptors description
    prf_desc_t desc_info[ACC_MC_DESC_TYPE_MAX];
} acc_mcc_mcs_info_t;

/// @} ACC_MCC_STRUCT

/*
 * CALLBACK FUNCTIONS DEFINITION
 ****************************************************************************************
 */

/// @addtogroup ACC_MCC_CALLBACK
/// @{

/**
 ****************************************************************************************
 * @brief Callback function called when handling of a command has been completed
 *
 * @param[in] cmd_type      Command type
 * @param[in] status        Status
 * @param[in] con_lid       Connection local index
 * @param[in] media_lid     Media local index
 * @param[in] param         Additional parameter\n
 *                              - Characteristic type\n
 *                              - Characteristic type\n
 *                              - Operation code
 * @param[in] result        Result code
 ****************************************************************************************
 */
typedef void (*acc_mcc_cb_cmp_evt)(uint8_t cmd_type, uint16_t status,
                                   uint8_t con_lid, uint8_t media_lid, uint8_t param, uint8_t result);

/**
 ****************************************************************************************
 * @brief Callback function called when value of one of the following characteristic is received:\n
 *      - Media Player Icon Object ID characteristic\n
 *      - Current Track Segments Object ID characteristic\n
 *      - Current Track Object ID characteristic\n
 *      - Next Track Object ID characteristic\n
 *      - Current Group Object ID characteristic\n
 *      - Parent Group Object ID characteristic\n
 *      - Search Results Object ID characteristic\n
 *
 * @param[in] con_lid           Connection local index
 * @param[in] media_lid         Media local index
 * @param[in] char_type         Characteristic type
 * @param[in] p_obj_id          Pointer to Object Id
 ****************************************************************************************
 */
typedef void (*acc_mcc_cb_object_id)(uint8_t con_lid, uint8_t media_lid, uint8_t char_type,
                                     const acc_mc_object_id_t* p_obj_id);

/**
 ****************************************************************************************
 * @brief Callback function called when a notification indicating that current track has changed has been received
 *
 * @param[in] con_lid           Connection local index
 * @param[in] media_lid         Media local index
 ****************************************************************************************
 */
typedef void (*acc_mcc_cb_track_changed)(uint8_t con_lid, uint8_t media_lid);

/**
 ****************************************************************************************
 * @brief Callback function called when value of one of the following characteristic is received:
 *      - Media Player Name characteristic\n
 *      - Media Player Icon URL characteristic\n
 *      - Track Title characteristic
 *
 * @param[in] con_lid           Connection local index
 * @param[in] media_lid         Bearer local index
 * @param[in] char_type         Characteristic type
 * @param[in] val_len           Length of value
 * @param[in] p_val             Pointer to value
 ****************************************************************************************
 */
typedef void (*acc_mcc_cb_value_long)(uint8_t con_lid, uint8_t media_lid, uint8_t char_type,
                                      uint16_t val_len, const char* p_val);

/**
 ****************************************************************************************
 * @brief Callback function called when value of one of the following characteristic is received:\n
 *      - Track Duration characteristic\n
 *      - Track Position characteristic\n
 *      - Playback Speed characteristic\n
 *      - Seeking Speed characteristic\n
 *      - Playing Order characteristic\n
 *      - Playing Order Supported characteristic\n
 *      - Media State characteristic\n
 *      - Media Control Point Opcodes Supported characteristic\n
 *      - Content Control ID characteristic\n
 *
 * @param[in] con_lid           Connection local index
 * @param[in] media_lid         Media local index
 * @param[in] char_type         Characteristic type
 * @param[in] val               Value to be casted as follow:\n
 *                                  - Playback speed (uint8_t)\n
 *                                  - Seeking speed (int8_t)\n
 *                                  - State (uint8_t)\n
 *                                  - Supported operation codes bit field (uint32_t)\n
 *                                  - Playing order (uint8_t)\n
 *                                  - Supported playing orders bit field (uint32_t)\n
 *                                  - Track duration (uint32_t)\n
 *                                  - Track position (uint32_t)\n
 *                                  - CCID (uint8_t)
 ****************************************************************************************
 */
typedef void (*acc_mcc_cb_value)(uint8_t con_lid, uint8_t media_lid, uint8_t char_type, uint32_t val);

/**
 ****************************************************************************************
 * @brief Callback function called when an instance of (Generic) Media Control Control Service has been discovered
 *
 * @param[in] con_lid           Connection local index
 * @param[in] media_lid         Media local index
 * @param[in] p_mcs_info        Pointer to Content description structure
 ****************************************************************************************
 */
typedef void (*acc_mcc_cb_bond_data)(uint8_t con_lid, uint8_t media_lid, const acc_mcc_mcs_info_t* p_mcs_info);

/**
 ****************************************************************************************
 * @brief Callback function called when an included Object Transfer Service instance has been discovered
 *
 * @param[in] con_lid           Connection local index
 * @param[in] media_lid         Media local index
 * @param[in] shdl              Start handle
 * @param[in] ehdl              End handle
 ****************************************************************************************
 */
typedef void (*acc_mcc_cb_included_svc)(uint8_t con_lid, uint8_t media_lid, uint16_t shdl, uint16_t ehdl);

/**
 ****************************************************************************************
 * @brief Callback function called when a service changed indication has been received from a Server device
 *
 * @param[in] con_lid           Connection local index
 ****************************************************************************************
 */
typedef void (*acc_mcc_cb_svc_changed)(uint8_t con_lid);

/*
 * CALLBACK SET DEFINITION
 ****************************************************************************************
 */

/// Set of callback functions for Media Control Client
typedef struct acc_mcc_cb
{
    /// Callback function called when handling of a command has been completed
    acc_mcc_cb_cmp_evt cb_cmp_evt;
    /// Callback function called when value of one of the following characteristic is received:\n
    ///       - Media Player Icon Object ID characteristic\n
    ///       - Current Track Segments Object ID characteristic\n
    ///       - Current Track Object ID characteristic\n
    ///       - Next Track Object ID characteristic\n
    ///       - Current Group Object ID characteristic\n
    ///       - Parent Group Object ID characteristic\n
    ///       - Search Results Object ID characteristic\n
    acc_mcc_cb_object_id cb_object_id;
    /// Callback function called when a notification indicating that current track has changed has been received
    acc_mcc_cb_track_changed cb_track_changed;
    /// Callback function called when value of one of the following characteristic is received:\n
    ///     - Media Player Name characteristic\n
    ///     - Media Player Icon URL characteristic\n
    ///     - Track Title characteristic
    acc_mcc_cb_value_long cb_value_long;
    /// Callback function called when value of one of the following characteristic is received:\n
    ///     - Track Duration characteristic\n
    ///     - Track Position characteristic\n
    ///     - Playback Speed characteristic\n
    ///     - Seeking Speed characteristic\n
    ///     - Playing Order characteristic\n
    ///     - Playing Order Supported characteristic\n
    ///     - Media State characteristic\n
    ///     - Media Control Point Opcodes Supported characteristic\n
    ///     - Content Control ID characteristic
    acc_mcc_cb_value cb_value;
    /// Callback function called when an instance of (Generic) Media Control Control Service has been discovered
    acc_mcc_cb_bond_data cb_bond_data;
    /// Callback function called when an included Object Transfer Service instance has been discovered
    acc_mcc_cb_included_svc cb_included_svc;
    /// Callback function called when a service changed indication has been received from a Server device
    acc_mcc_cb_svc_changed cb_svc_changed;
} acc_mcc_cb_t;

/// @} ACC_MCC_CALLBACK

/*
 * API FUNCTIONS DECLARATION
 ****************************************************************************************
 */

#if (GAF_ACC_MCC)
/// @addtogroup ACC_MCC_FUNCTION
/// @{

/**
 ****************************************************************************************
 * @brief Configure use of Media Control Client module as Client
 *
 * @param[in] p_cb          Pointer to set of callback functions for communication with upper layers
 * @param[in] pref_mtu      Preferred MTU. Values from 0 to 63 are equivalent to 64
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t acc_mcc_configure(const acc_mcc_cb_t* p_cb, uint16_t pref_mtu);

/**
 ****************************************************************************************
 * @brief Enable use of Media Control Service block as Client for a Server device with which a bonding has been
 * established during a previous connection.
 *
 * @param[in] con_lid       Connection local index
 * @param[in] nb_media      Number of instances of the (Generic) Media Control Service
 * @param[in] p_mcs_info    Pointer to Content description structures
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t acc_mcc_restore_bond_data(uint8_t con_lid, uint8_t nb_media, const acc_mcc_mcs_info_t* p_mcs_info);

/**
 ****************************************************************************************
 * @brief Enable use of Media Control Service block as Client for a connected device with which no bonding has been
 * established during a previous connection.
 *
 * @param[in] con_lid       Connection local index
 * @param[in] nb_mcs_max    Maximum number of Media Control Service instances that can be found
 *                          in peer device database additionally to the Generic Media Control Service
 * @param[in] shdl          Start handle for the discovery. Set GATT_INVALID_HDL if not provided
 * @param[in] ehdl          End handle for the discovery. Set GATT_INVALID_HDL if not provided
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t acc_mcc_discover(uint8_t con_lid, uint8_t nb_mcs_max, uint16_t shdl, uint16_t ehdl);

#if (GAF_DBG)
/**
 ****************************************************************************************
 * @brief Get value for one of the following characteristics for a Media exposed by a Server device:\n
 *      - Media Player Name characteristic\n
 *      - Media Player Icon Object ID if characteristic is supported for the indicated Media\n
 *      - Media Player Icon URL characteristic if characteristic is supported for the indicated Media\n
 *      - Track Title characteristic\n
 *      - Track Duration characteristic\n
 *      - Track Position characteristic\n
 *      - Playback Speed characteristic if characteristic is supported for the indicated Media\n
 *      - Seeking Speed characteristic if characteristic is supported for the indicated Media\n
 *      - Current Track Segments Object ID characteristic if characteristic is supported for the indicated Media\n
 *      - Current Track Object ID characteristic if characteristic is supported for the indicated Media\n
 *      - Next Track Object ID characteristic if characteristic is supported for the indicated Media\n
 *      - Current Group Object ID characteristic if characteristic is supported for the indicated Media\n
 *      - Parent Group Object ID characteristic if characteristic is supported for the indicated Media\n
 *      - Playing Order characteristic if characteristic is supported for the indicated Media\n
 *      - Playing Order Supported characteristic if characteristic is supported for the indicated Media\n
 *      - Media State characteristic\n
 *      - Media Control Point Opcodes Supported characteristic if characteristic is supported for the indicated Media\n
 *      - Search Results Object ID characteristic if characteristic is supported for the indicated Media\n
 *      - Content Control ID characteristic\n
 *
 * @param[in] con_lid       Connection local index
 * @param[in] media_lid     Media local index
 * @param[in] char_type     Characteristic type
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t acc_mcc_get(uint8_t con_lid, uint8_t media_lid, uint8_t char_type);

/**
 ****************************************************************************************
 * @brief Enable or disable sending of notifications for one of the following characteristics for a Media exposed
 * by a Server device:\n
 *      - Media Player Name characteristic if sending of notifications is supported for the characteristic\n
 *      - Track Changed characteristic\n
 *      - Track Title characteristic if sending of notifications is supported for the characteristic\n
 *      - Track Duration characteristic if sending of notifications is supported for the characteristic\n
 *      - Track Position characteristic if sending of notifications is supported for the characteristic\n
 *      - Playback Speed characteristic if characteristic is supported for the indicated Media and if
 *      sending of notifications if supported for the characteristic\n
 *      - Seeking Speed characteristic if characteristic is supported for the indicated Media and if
 *      sending of notifications if supported for the characteristic\n
 *      - Current Track Object ID characteristic if characteristic is supported for the indicated Media
 *      and if sending of notifications if supported for the characteristic\n
 *      - Next Track Object ID characteristic if characteristic is supported for the indicated Media and
 *      if sending of notifications if supported for the characteristic\n
 *      - Current Group Object ID characteristic if characteristic is supported for the indicated Media
 *      and if sending of notifications if supported for the characteristic\n
 *      - Parent Group Object ID characteristic if characteristic is supported for the indicated Media and
 *       if sending of notifications if supported for the characteristic\n
 *      - Playing Order characteristic if characteristic is supported for the indicated Media and if
 *      sending of notifications if supported for the characteristic\n
 *      - Media State characteristic\n
 *      - Media Control Point characteristic if characteristic is supported for the indicated Media\n
 *      - Media Control Point Opcodes Supported characteristic if characteristic is supported for the
 *      indicated Media and if sending of notifications if supported for the characteristic\n
 *      - Search Results Object ID characteristic if characteristic is supported for the indicated Media
 *
 * @param[in] con_lid           Connection local index
 * @param[in] media_lid         Media local index
 * @param[in] char_type         Characteristic type
 * @param[in] enable            Indicate if sending of notifications must be enabled (!= 0) or disabled for the
 * indicated characteristic
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t acc_mcc_set_cfg(uint8_t con_lid, uint8_t media_lid, uint8_t char_type, uint8_t enable);
#endif //(GAF_DBG)

/**
 ****************************************************************************************
 * @brief Set value of one of the following characteristics for a Media exposed by a Server device:
 *      - Track Position characteristic
 *      - Playback Speed characteristic if characteristic is supported for the indicated Media
 *      - Playing Order characteristic if characteristic is supported for the indicated Media
 *
 * @param[in] con_lid               Connection local index
 * @param[in] media_lid             Media local index
 * @param[in] char_type             Characteristic type
 * @param[in] reliable              Indicate if feedback from Server device is required (!= 0)
 * or not
 * @param[in] val                   Value to be casted as follow:
 *                                      - Track position (int32_t) - acc_mcc_set_track_position function can be used
 *                                      - Playback speed (int8_t) - acc_mcc_set_playback_speed function can be used
 *                                      - Playing order (uint8_t) - acc_mcc_set_playing order function can be used
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t acc_mcc_set(uint8_t con_lid, uint8_t media_lid, uint8_t char_type, uint8_t reliable, uint32_t val);

/**
 ****************************************************************************************
 * @brief Set Track Position\n
 * If the value is zero or greater, then the current playing position shall be set to the offset from
 * the start of the track\n
 * If the value is less than zero, then the current playing position shall be set to the offset from the end
 * of the track
 *
 * @param[in] con_lid               Connection local index
 * @param[in] media_lid             Media local index
 * @param[in] reliable              Indicate if feedback from Server device is required (!= 0) or not
 * @param[in] track_position        Track position (int32_t)
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint16_t acc_mcc_set_track_position(uint8_t con_lid, uint8_t media_lid, uint8_t reliable,
                                             int32_t track_position)
{
    return (acc_mcc_set(con_lid, media_lid, ACC_MC_CHAR_TYPE_TRACK_POSITION, reliable, track_position));
}

/**
 ****************************************************************************************
 * @brief Set Playback Speed
 *
 * @param[in] con_lid               Connection local index
 * @param[in] media_lid             Media local index
 * @param[in] reliable              Indicate if feedback from Server device is required (!= 0) or not
 * @param[in] playback_speed        Playback Speed (int8_t)
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint16_t acc_mcc_set_playback_speed(uint8_t con_lid, uint8_t media_lid, uint8_t reliable,
                                             int8_t playback_speed)
{
    return (acc_mcc_set(con_lid, media_lid, ACC_MC_CHAR_TYPE_PLAYBACK_SPEED, reliable, playback_speed));
}

/**
 ****************************************************************************************
 * @brief Set Playing Order
 *
 * @param[in] con_lid               Connection local index
 * @param[in] media_lid             Media local index
 * @param[in] reliable              Indicate if feedback from Server device is required (!= 0) or not
 * @param[in] playing_order         Playing Order (uint8_t)
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint16_t acc_mcc_set_playing_order(uint8_t con_lid, uint8_t media_lid, uint8_t reliable,
                                            uint8_t playing_order)
{
    return (acc_mcc_set(con_lid, media_lid, ACC_MC_CHAR_TYPE_PLAYING_ORDER, reliable, playing_order));
}

#if (GAF_ACC_OTC)
/**
 ****************************************************************************************
 * @brief Set value of one of the following characteristics for a Media exposed by a Server device:
 *      - Current Track Segments Object ID characteristic if characteristic is supported
 *      for the indicated Media
 *      - Next Track Object ID characteristic if characteristic is supported for the
 *      indicated Media
 *      - Current Group Object ID characteristic if characteristic is supported for the
 *      indicated Media
 *
 * @param[in] con_lid               Connection local index
 * @param[in] media_lid             Media local index
 * @param[in] char_type             Characteristic type
 * @param[in] reliable              Indicate if feedback from Server device is required (!= 0)
 * or not
 * @param[in] p_obj_id              Pointer to Object ID
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t acc_mcc_set_object_id(uint8_t con_lid, uint8_t media_lid, uint8_t char_type,
                               uint8_t reliable, const acc_mc_object_id_t* p_obj_id);
#endif //(GAF_ACC_OTC)

/**
 ****************************************************************************************
 * @brief Control behavior of a Media exposed by a Server device if Media Control Point
 * characteristic is supported for this Media.
 *
 * @param[in] con_lid           Connection local index
 * @param[in] media_lid         Media local index
 * @param[in] opcode            Operation code
 * @param[in] reliable          Indicate if feedback from Server device is required (!= 0)
 * or not
 * @param[in] val               Value (for Goto Segment or Move Relative or Goto Track or
 * Goto Group)
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t acc_mcc_control(uint8_t con_lid, uint8_t media_lid, uint8_t opcode, uint8_t reliable, int32_t val);

/**
 ****************************************************************************************
 * @brief Start playing of the current track
 *
 * @param[in] con_lid           Connection local index
 * @param[in] media_lid         Media local index
 * @param[in] reliable          Indicate if feedback from Server device is required (!= 0) or not
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint16_t acc_mcc_play(uint8_t con_lid, uint8_t media_lid, uint8_t reliable)
{
    return (acc_mcc_control(con_lid, media_lid, ACC_MC_OPCODE_PLAY, reliable, 0));
}

/**
 ****************************************************************************************
 * @brief Pause playing the current track
 *
 * @param[in] con_lid           Connection local index
 * @param[in] media_lid         Media local index
 * @param[in] reliable          Indicate if feedback from Server device is required (!= 0) or not
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint16_t acc_mcc_pause(uint8_t con_lid, uint8_t media_lid, uint8_t reliable)
{
    return (acc_mcc_control(con_lid, media_lid, ACC_MC_OPCODE_PAUSE, reliable, 0));
}

/**
 ****************************************************************************************
 * @brief Stop current activity, return to the paused state and set the current track position to the start of
 * the current track
 *
 * @param[in] con_lid           Connection local index
 * @param[in] media_lid         Media local index
 * @param[in] reliable          Indicate if feedback from Server device is required (!= 0) or not
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint16_t acc_mcc_stop(uint8_t con_lid, uint8_t media_lid, uint8_t reliable)
{
    return (acc_mcc_control(con_lid, media_lid, ACC_MC_OPCODE_STOP, reliable, 0));
}

/**
 ****************************************************************************************
 * @brief Go to previous track on a given media
 *
 * @param[in] con_lid           Connection local index
 * @param[in] media_lid         Media local index
 * @param[in] reliable          Indicate if feedback from Server device is required (!= 0) or not
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint16_t acc_mcc_previous(uint8_t con_lid, uint8_t media_lid, uint8_t reliable)
{
    return (acc_mcc_control(con_lid, media_lid, ACC_MC_OPCODE_PREV_TRACK, reliable, 0));
}

/**
 ****************************************************************************************
 * @brief Go to next track on a given media
 *
 * @param[in] con_lid           Connection local index
 * @param[in] media_lid         Media local index
 * @param[in] reliable          Indicate if feedback from Server device is required (!= 0) or not
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint16_t acc_mcc_next(uint8_t con_lid, uint8_t media_lid, uint8_t reliable)
{
    return (acc_mcc_control(con_lid, media_lid, ACC_MC_OPCODE_NEXT_TRACK, reliable, 0));
}

#if (GAF_ACC_OTC)
/**
 ****************************************************************************************
 * @brief Request from a Service device to perform a search of all the media for one of its
 * exposed Media.
 *
 * @param[in] con_lid           Connection local index
 * @param[in] media_lid         Media local index
 * @param[in] reliable          Indicate if feedback from Server device is required (!= 0)
 * or not
 * @param[in] param_len         Length of parameter value
 * @param[in] p_param           Pointer to parameter value
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t acc_mcc_search(uint8_t con_lid, uint8_t media_lid, uint8_t reliable,
                        uint8_t param_len, const uint8_t* p_param);
#endif //(GAF_ACC_OTC)

/// @} ACC_MCC_FUNCTION
#endif //(GAF_ACC_MCC)

#endif // ACC_MCC_H_
