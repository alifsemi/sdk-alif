/**
 ****************************************************************************************
 *
 * @file acc_mcs.h
 *
 * @brief Audio Content Control - Media Control Server - Definitions
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef ACC_MCS_H_
#define ACC_MCS_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup ACC_MCS Media Control Service Server
 * @ingroup ACC_MC
 * @brief Description of Media Control Service Server module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ACC_MCS_ENUM Enumerations
 * @ingroup ACC_MCS
 * @brief Enumerations for Media Control Service Server module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ACC_MCS_NATIVE_API Native API
 * @ingroup ACC_MCS
 * @brief Description of Native API for Media Control Service Server module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ACC_MCS_CALLBACK Callback Functions
 * @ingroup ACC_MCS_NATIVE_API
 * @brief Description of callback functions for Media Control Service Server module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ACC_MCS_FUNCTION Functions
 * @ingroup ACC_MCS_NATIVE_API
 * @brief Description of functions for Media Control Service Server module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "acc_mc.h"            // Audio Content Control - Media Control Definition
#include "gaf.h"                // Generic Audio Framework Definitions

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// @addtogroup ACC_MCS_ENUM
/// @{

/// Media configuration bit field
enum acc_mcs_cfg_bf
{
    /// Indicate if sending of notifications is supported (= 1) for the Media Player Name
    /// characteristic
    /// Meaningless for Generic Media Control Service as support is mandatory
    ACC_MCS_CFG_PLAYER_NAME_NTF_SUPP_POS = 0,
    ACC_MCS_CFG_PLAYER_NAME_NTF_SUPP_BIT = CO_BIT(ACC_MCS_CFG_PLAYER_NAME_NTF_SUPP_POS),
    /// Indicate if Media Player Icon Object ID characteristic is supported (= 1) or not
    ACC_MCS_CFG_PLAYER_ICON_OBJ_ID_SUPP_POS = 1,
    ACC_MCS_CFG_PLAYER_ICON_OBJ_ID_SUPP_BIT = CO_BIT(ACC_MCS_CFG_PLAYER_ICON_OBJ_ID_SUPP_POS),
    /// Indicate if Media Player Icon URL characteristic is supported
    ACC_MCS_CFG_PLAYER_ICON_URL_SUPP_POS = 2,
    ACC_MCS_CFG_PLAYER_ICON_URL_SUPP_BIT = CO_BIT(ACC_MCS_CFG_PLAYER_ICON_URL_SUPP_POS),
    /// Indicate if sending of notifications is supported (= 1) for the Track Title characteristic
    ACC_MCS_CFG_TRACK_TITLE_NTF_SUPP_POS = 3,
    ACC_MCS_CFG_TRACK_TITLE_NTF_SUPP_BIT = CO_BIT(ACC_MCS_CFG_TRACK_TITLE_NTF_SUPP_POS),
    /// Indicate if sending of notifications is supported (= 1) for the Track Duration characteristic
    ACC_MCS_CFG_TRACK_DURATION_NTF_SUPP_POS = 4,
    ACC_MCS_CFG_TRACK_DURATION_NTF_SUPP_BIT = CO_BIT(ACC_MCS_CFG_TRACK_DURATION_NTF_SUPP_POS),
    /// Indicate if sending of notifications is supported (= 1) for the Track Position characteristic
    ACC_MCS_CFG_TRACK_POSITION_NTF_SUPP_POS = 5,
    ACC_MCS_CFG_TRACK_POSITION_NTF_SUPP_BIT = CO_BIT(ACC_MCS_CFG_TRACK_POSITION_NTF_SUPP_POS),
    /// Indicate if sending of notifications is supported (= 1) for the Playback Speed characteristic
    ACC_MCS_CFG_PLAYBACK_SPEED_NTF_SUPP_POS = 6,
    ACC_MCS_CFG_PLAYBACK_SPEED_NTF_SUPP_BIT = CO_BIT(ACC_MCS_CFG_PLAYBACK_SPEED_NTF_SUPP_POS),
    /// Indicate if sending of notifications is supported (= 1) for the Seeking Speed characteristic
    ACC_MCS_CFG_SEEKING_SPEED_NTF_SUPP_POS = 7,
    ACC_MCS_CFG_SEEKING_SPEED_NTF_SUPP_BIT = CO_BIT(ACC_MCS_CFG_SEEKING_SPEED_NTF_SUPP_POS),
    /// Indicate if Current Track Object ID characteristic is supported (= 1) or not
    /// If supported, following characteristic are also supported:
    ///     - Current Track Segments Object ID characteristic
    ///     - Next Track Object ID characteristic
    ///     - Current Group Object ID characteristic
    ///     - Parent Group Object ID characteristic
    ACC_MCS_CFG_CURR_TRACK_OBJ_ID_SUPP_POS = 8,
    ACC_MCS_CFG_CURR_TRACK_OBJ_ID_SUPP_BIT = CO_BIT(ACC_MCS_CFG_CURR_TRACK_OBJ_ID_SUPP_POS),
    /// Indicate if sending of notifications is supported (= 1) for the Current Track Object ID
    /// characteristic
    ACC_MCS_CFG_CURR_TRACK_NTF_SUPP_POS = 9,
    ACC_MCS_CFG_CURR_TRACK_NTF_SUPP_BIT = CO_BIT(ACC_MCS_CFG_CURR_TRACK_NTF_SUPP_POS),
    /// Indicate if sending of notifications is supported (= 1) for the Next Track Object ID
    /// characteristic
    ACC_MCS_CFG_NEXT_TRACK_NTF_SUPP_POS = 10,
    ACC_MCS_CFG_NEXT_TRACK_NTF_SUPP_BIT = CO_BIT(ACC_MCS_CFG_NEXT_TRACK_NTF_SUPP_POS),
    /// Indicate if sending of notifications is supported (= 1) for the Current Group Object ID
    /// characteristic
    ACC_MCS_CFG_CURR_GROUP_NTF_SUPP_POS = 11,
    ACC_MCS_CFG_CURR_GROUP_NTF_SUPP_BIT = CO_BIT(ACC_MCS_CFG_CURR_GROUP_NTF_SUPP_POS),
    /// Indicate if sending of notifications is supported (= 1) for the Parent Group Object ID
    /// characteristic
    ACC_MCS_CFG_PARENT_GROUP_NTF_SUPP_POS = 12,
    ACC_MCS_CFG_PARENT_GROUP_NTF_SUPP_BIT = CO_BIT(ACC_MCS_CFG_PARENT_GROUP_NTF_SUPP_POS),
    /// Indicate if Player Order characteristic is supported (= 1) or not
    ACC_MCS_CFG_PLAYING_ORDER_SUPP_POS = 13,
    ACC_MCS_CFG_PLAYING_ORDER_SUPP_BIT = CO_BIT(ACC_MCS_CFG_PLAYING_ORDER_SUPP_POS),
    /// Indicate if Player Order Supported characteristic is supported (= 1) or not
    ACC_MCS_CFG_PLAYING_ORDER_SUPP_SUPP_POS = 14,
    ACC_MCS_CFG_PLAYING_ORDER_SUPP_SUPP_BIT = CO_BIT(ACC_MCS_CFG_PLAYING_ORDER_SUPP_SUPP_POS),
    /// Indicate if sending of notifications is supported (= 1) for the Playing Order characteristic
    ACC_MCS_CFG_PLAYING_ORDER_NTF_SUPP_POS = 15,
    ACC_MCS_CFG_PLAYING_ORDER_NTF_SUPP_BIT = CO_BIT(ACC_MCS_CFG_PLAYING_ORDER_NTF_SUPP_POS),
    /// Indicate if Media Control Point characteristic (= 1) is supported or not
    /// If supported, the Media Control Point Opcodes Supported characteristic is also supported
    ACC_MCS_CFG_MEDIA_CP_SUPP_POS = 16,
    ACC_MCS_CFG_MEDIA_CP_SUPP_BIT = CO_BIT(ACC_MCS_CFG_MEDIA_CP_SUPP_POS),
    /// Indicate if sending of notifications is supported (= 1) for the Media Control Point
    /// Opcodes Supported characteristic
    ACC_MCS_CFG_MEDIA_CP_OPCODE_NTF_SUPP_POS = 17,
    ACC_MCS_CFG_MEDIA_CP_OPCODE_NTF_SUPP_BIT = CO_BIT(ACC_MCS_CFG_MEDIA_CP_OPCODE_NTF_SUPP_POS),
    /// Indicate if Search Results Object ID characteristic is supported (= 1) or not
    /// If supported, the Search Control Point characteristic is also supported
    ACC_MCS_CFG_SEARCH_RESULT_OBJ_ID_SUPP_POS = 18,
    ACC_MCS_CFG_SEARCH_RESULT_OBJ_ID_SUPP_BIT = CO_BIT(ACC_MCS_CFG_SEARCH_RESULT_OBJ_ID_SUPP_POS),
    /// Indicate if Seeking Speed characteristic is supported
    ACC_MCS_CFG_SEEKING_SPEED_SUPP_POS = 19,
    ACC_MCS_CFG_SEEKING_SPEED_SUPP_BIT = CO_BIT(ACC_MCS_CFG_SEEKING_SPEED_SUPP_POS),
    /// Indicate if Playback Speed characteristic is supported
    ACC_MCS_CFG_PLAYBACK_SPEED_SUPP_POS = 20,
    ACC_MCS_CFG_PLAYBACK_SPEED_SUPP_BIT = CO_BIT(ACC_MCS_CFG_PLAYBACK_SPEED_SUPP_POS),
};

/// Action type values
enum acc_mcs_action
{
    /// No action
    ACC_MCS_ACTION_NO_ACTION = 0,
    /// Play
    ACC_MCS_ACTION_PLAY,
    /// Pause
    ACC_MCS_ACTION_PAUSE,
    /// Stop
    ACC_MCS_ACTION_STOP,
    /// Fast Forward or Fast Rewind
    ACC_MCS_ACTION_SEEK,
    /// Current Track becomes invalid
    ACC_MCS_ACTION_INACTIVE,
    /// Change track
    ACC_MCS_ACTION_CHANGE_TRACK,

    ACC_MCS_ACTION_MAX
};

/// @} ACC_MCS_ENUM

/*
 * CALLBACK FUNCTIONS DEFINITION
 ****************************************************************************************
 */

/// @addtogroup ACC_MCS_CALLBACK
/// @{

/**
 ****************************************************************************************
 * @brief Callback function called when Client Characteristic Configuration of a
 * notification-capable characteristic has been updated by a peer client
 *
 * @param[in] media_lid     Media local index
 * @param[in] con_lid       Connection local index
 * @param[in] cli_cfg_bf    Client configuration bit field
 ****************************************************************************************
 */
typedef void (*acc_mcs_cb_bond_data)(uint8_t media_lid, uint8_t con_lid, uint32_t cli_cfg_bf);

/**
 ****************************************************************************************
 * @brief Callback function called when either Current Track Object ID or Next Track Object
 * ID or CUrrent Group Object ID characteristic value has been written by a Client device
 *
 * @param[in] media_lid     Media local index
 * @param[in] con_lid       Connection local index
 * @param[in] char_type     Characteristic type
 * @param[in] p_obj_id      Pointer to Object ID
 ****************************************************************************************
 */
typedef void (*acc_mcs_cb_set_object_id_req)(uint8_t media_lid, uint8_t con_lid, uint8_t char_type,
                                             const acc_mc_object_id_t* p_obj_id);

/**
 ****************************************************************************************
 * @brief Callback function called when a Client device request a control on a Media
 *
 * @param[in] media_lid     Media local index
 * @param[in] con_lid       Connection local index
 * @param[in] opcode        Operation code
 * @param[in] val           Value
 ****************************************************************************************
 */
typedef void (*acc_mcs_cb_control_req)(uint8_t media_lid, uint8_t con_lid,
                                       uint8_t opcode, int32_t val);

/**
 ****************************************************************************************
 * @brief Callback function called when a Search request has been received from Client device
 *
 * @param[in] media_lid     Media local index
 * @param[in] con_lid       Connection local index
 * @param[in] param_len     Length of Search Parameters value
 * @param[in] p_param       Pointer to Search Parameters value
 ****************************************************************************************
 */
typedef void (*acc_mcs_cb_search_req)(uint8_t media_lid, uint8_t con_lid,
                                      uint8_t param_len, const uint8_t* p_param);

/**
 ****************************************************************************************
 * @brief Callback function called when a Client device request to set value for either
 * Media Player Name or Media Player Icon Object URL or Track Title characteristic
 *
 * @param[in] media_lid     Media local index
 * @param[in] con_lid       Connection local index
 * @param[in] char_type     Characteristic type
 * @param[in] token         Token
 * @param[in] offset        Offset
 * @param[in] length        Maximum length
 ****************************************************************************************
 */
typedef void (*acc_mcs_cb_get_req)(uint8_t media_lid, uint8_t con_lid, uint8_t char_type,
                                   uint16_t token, uint16_t offset, uint16_t length);

/**
 ****************************************************************************************
 * @brief Callback function called when Client device tries to get the current Track Position
 *
 * @param[in] media_lid     Media local index
 * @param[in] con_lid       Connection local index
 * @param[in] token         Token
 ****************************************************************************************
 */
typedef void (*acc_mcs_cb_get_position_req)(uint8_t media_lid, uint8_t con_lid, uint16_t token);

/**
 ****************************************************************************************
 * @brief Callback function called when a Client device request to set value for either
 * Track Position or Playback Speed or Playing Order characteristic
 *
 * @param[in] media_lid     Media local index
 * @param[in] con_lid       Connection local index
 * @param[in] char_type     Characteristic type
 * @param[in] val           Value
 ****************************************************************************************
 */
typedef void (*acc_mcs_cb_set_req)(uint8_t media_lid, uint8_t con_lid, uint8_t char_type,
                                   uint32_t val);

/*
 * CALLBACK SET DEFINITION
 ****************************************************************************************
 */

/// Set of callback functions for Media Control Server
typedef struct acc_mcs_cb
{
    /// Callback function called when Client Characteristic Configuration of a notification-capable
    /// characteristic has been updated by a peer client
    acc_mcs_cb_bond_data cb_bond_data;
    /// Callback function called when either Current Track Object ID or Next Track Object
    /// ID or CUrrent Group Object ID characteristic value has been written by a Client device
    acc_mcs_cb_set_object_id_req cb_set_object_id_req;
    /// Callback function called when a Search request has been received from Client device
    acc_mcs_cb_search_req cb_search_req;
    /// Callback function called when a Client device request a control on a Media
    acc_mcs_cb_control_req cb_control_req;
    /// Callback function called when a Client device request to set value for either
    /// Media Player Name or Media Player Icon Object URL or Track Title characteristic
    acc_mcs_cb_get_req cb_get_req;
    /// Callback function called when Client device tries to get the current Track Position
    acc_mcs_cb_get_position_req cb_get_position_req;
    /// Callback function called when a Client device request to set value for either
    /// Track Position or Playback Speed or Playing Order characteristic
    acc_mcs_cb_set_req cb_set_req;
} acc_mcs_cb_t;

/// @} ACC_MCS_CALLBACK

/*
 * API FUNCTIONS DECLARATION
 ****************************************************************************************
 */

#if (GAF_ACC_MCS)
/// @addtogroup ACC_MCS_FUNCTION
/// @{

/**
 ****************************************************************************************
 * @brief Configure use of Media Control Server module as Server
 *
 * @param[in] nb_mcs                 Number of instances of the Media Control Service
 *                                   0 means that only the Generic Media Control Service is
 *                                   supported
 * @param[in] p_cb                   Pointer to set of callback functions for communication with upper
 *                                   layer
 * @param[in] pref_mtu               Preferred MTU
 *                                   Values from 0 to 63 are equivalent to 64
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t acc_mcs_configure(uint8_t nb_mcs, const acc_mcs_cb_t* p_cb, uint16_t pref_mtu);

/**
 ****************************************************************************************
 * @brief Add and configure an instance of the Media Control Service in the database
 *
 * @param[in] cfg_bf                    Configuration bit field
 * @param[in] playing_order_supp_bf     Supported playing order bit field
 * @param[in] ots_shdl                  Start handle of included Object Transfer Service
 * @param[in] ccid                      Content Control ID
 * @param[in] p_icon_obj_id             Pointer to Media Player Icon Object ID
 * @param[in] shdl                      Required start handle
 *                                      If set to GATT_INVALID_HDL, the start handle will be automatically chosen
 * @param[out] p_media_lid              Pointer at which allocated Media local index is returned
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t acc_mcs_add(uint32_t cfg_bf, uint32_t playing_order_supp_bf, uint16_t ots_shdl, uint8_t ccid,
                     const acc_mc_object_id_t* p_icon_obj_id, uint16_t shdl, uint8_t* p_media_lid);

/**
 ****************************************************************************************
 * @brief Set bonding information for either the Generic Media Control Service or an instance
 * of the Media Control Service Service after connection with a Client device with which a
 * bonded relationship had been established during a previous connection.
 *
 * @param[in] media_lid                Media local index
 * @param[in] con_lid                  Connection local index
 * @param[in] cli_cfg_bf               Client configuration bit field
 * @param[in] evt_cfg_bf               Event configuration bit field
 * @param[in] name_len                 Length of Media Player Name value
 * Meaningful only if notification of Media Player Name characteristic is supported
 * @param[in] title_len                Length of Track Title value
 * Meaningful only if notification of Track Title characteristic is supported
 * @param[in] p_name                   Pointer to Media Player Name value
 * @param[in] p_title                  Pointer to Track Title value
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t acc_mcs_restore_bond_data(uint8_t media_lid, uint8_t con_lid, uint32_t cli_cfg_bf, uint32_t evt_cfg_bf,
                                   uint8_t name_len, uint8_t title_len, const uint8_t* p_name, const uint8_t* p_title);

/**
 ****************************************************************************************
 * @brief Set value for either of Track Position characteristic or Playback Speed characteristic
 * or Media Control Point Opcodes Supported characteristic or Playing Order characteristic
 *
 * @param[in] media_lid                Media local index
 * @param[in] char_type                Characteristic type (see #acc_mc_char_type enumeration)
 * @param[in] val                      Value
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t acc_mcs_set(uint8_t media_lid, uint8_t char_type, uint32_t val);

/**
 ****************************************************************************************
 * @brief Set Track Position for a given media
 *
 * @param[in] media_lid                Media local index
 * @param[in] position                 Position
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint16_t acc_mcs_set_position(uint8_t media_lid, uint32_t position)
{
    return (acc_mcs_set(media_lid, ACC_MC_CHAR_TYPE_TRACK_POSITION, position));
}

/**
 ****************************************************************************************
 * @brief Set Playing Order for a given media
 *
 * @param[in] media_lid                Media local index
 * @param[in] playing_order            Playing order (see #acc_mc_play_order enumeration)
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint16_t acc_mcs_set_playing_order(uint8_t media_lid, uint8_t playing_order)
{
    return (acc_mcs_set(media_lid, ACC_MC_CHAR_TYPE_PLAYING_ORDER, playing_order));
}

#if (GAF_ACC_OTS)
/**
 ****************************************************************************************
 * @brief Set value for Next Track Object ID characteristic
 *
 * @param[in] media_lid         Media local index
 * @param[in] p_obj_id          Pointer to Object ID
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t acc_mcs_set_object_id(uint8_t media_lid, const acc_mc_object_id_t* p_obj_id);
#endif //(GAF_ACC_OTS)

/**
 ****************************************************************************************
 * @brief Indicate an update of the Media Player Name value
 *
 * @param[in] media_lid         Media local index
 * @param[in] name_len          Length of Media Player Name value
 * @param[in] p_name            Pointer to Media Player Name value
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t acc_mcs_set_player_name(uint8_t media_lid, uint8_t name_len, const uint8_t* p_name);

/**
 ****************************************************************************************
 * @brief Indicate that an action has been performed on a media (play, pause, stop, ...)
 *
 * @param[in] media_lid         Media local index
 * @param[in] action            Action
 * @param[in] track_pos         Track Position
 * @param[in] seeking_speed     Seeking Speed
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t acc_mcs_action(uint8_t media_lid, uint8_t action, int32_t track_pos, int8_t seeking_speed);

/**
 ****************************************************************************************
 * @brief Play current track on a given media
 *
 * @param[in] media_lid         Media local index
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint16_t acc_mcs_play(uint8_t media_lid)
{
    return (acc_mcs_action(media_lid, ACC_MCS_ACTION_PLAY, 0, 0));
}

/**
 ****************************************************************************************
 * @brief Pause current track on a given media
 *
 * @param[in] media_lid         Media local index
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint16_t acc_mcs_pause(uint8_t media_lid)
{
    return (acc_mcs_action(media_lid, ACC_MCS_ACTION_PAUSE, 0, 0));
}

/**
 ****************************************************************************************
 * @brief Stop current track on a given media
 *
 * @param[in] media_lid         Media local index
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint16_t acc_mcs_stop(uint8_t media_lid)
{
    return (acc_mcs_action(media_lid, ACC_MCS_ACTION_STOP, 0, 0));
}

/**
 ****************************************************************************************
 * @brief Start seeking on a given media
 *
 * @param[in] media_lid         Media local index
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint16_t acc_mcs_seek(uint8_t media_lid)
{
    return (acc_mcs_action(media_lid, ACC_MCS_ACTION_SEEK, 0, 0));
}

/**
 ****************************************************************************************
 * @brief Make current track invalid
 *
 * @param[in] media_lid         Media local index
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint16_t acc_mcs_inactive(uint8_t media_lid)
{
    return (acc_mcs_action(media_lid, ACC_MCS_ACTION_INACTIVE, 0, 0));
}

/**
 ****************************************************************************************
 * @brief Indicate that current track has changed for a media
 *
 * @param[in] media_lid             Media local index
 * @param[in] track_dur             Track Duration
 * @param[in] p_segments_obj_id     Pointer to Current Track Segments Object ID value
 * @param[in] p_current_obj_id      Pointer to Current Track Object ID value
 * @param[in] p_next_obj_id         Pointer to Next Track Object ID value
 * @param[in] p_group_obj_id        Pointer to Current Group Object ID value
 * @param[in] p_parent_obj_id       Pointer to Parent Group Object ID value
 * @param[in] title_len             Length of Track Title value
 * @param[in] p_title               Pointer to Track Title value
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t acc_mcs_track_change(uint8_t media_lid, int32_t track_dur,
                              const acc_mc_object_id_t* p_segments_obj_id, const acc_mc_object_id_t* p_current_obj_id,
                              const acc_mc_object_id_t* p_next_obj_id, const acc_mc_object_id_t* p_group_obj_id,
                              const acc_mc_object_id_t* p_parent_obj_id,
                              uint16_t title_len, const uint8_t* p_title);

/**
 ****************************************************************************************
 * @brief Indicate that current track has changed for a media without providing Object IDs
 *
 * @param[in] media_lid             Media local index
 * @param[in] track_dur             Track Duration
 * @param[in] title_len             Length of Track Title value
 * @param[in] p_title               Pointer to Track Title value
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint16_t acc_mcs_track_change_basic(uint8_t media_lid, int32_t track_dur, uint16_t title_len,
                                             const uint8_t *p_title)
{
    return (acc_mcs_track_change(media_lid, track_dur, NULL, NULL, NULL, NULL, NULL, title_len, p_title));
}

/**
 ****************************************************************************************
 * @brief Confirmation for ACC_MCS_SET_OBJECT_ID or ACC_MCS_SET request indication.
 *
 * @param[in] accept        Indicate if request is accepted or not
 * @param[in] media_lid     Media local index
 * @param[in] val           Track position or Playback Speed value
 ****************************************************************************************
 */
void acc_mcs_cfm_set(bool accept, uint8_t media_lid, uint32_t val);

/**
 ****************************************************************************************
 * @brief Confirmation for ACC_MCS_CONTROL request indication.
 *
 * @param[in] result            Result
 * @param[in] media_lid         Media local index
 * @param[in] action            Action
 * @param[in] track_pos         Track Position
 * @param[in] seeking_speed     Seeking Speed
 ****************************************************************************************
 */
void acc_mcs_cfm_control(uint8_t result, uint8_t media_lid,
                         uint8_t action, int32_t track_pos, int8_t seeking_speed);

#if (GAF_ACC_OTS)
/**
 ****************************************************************************************
 * @brief Confirmation for ACC_MCS_SEARCH request indication.
 *
 * @param[in] accept        Indicate if request is accepted or not
 * @param[in] media_lid     Media local index
 * @param[in] p_obj_id      Pointer to Search Result Object ID value
 ****************************************************************************************
 */
void acc_mcs_cfm_search(bool accept, uint8_t media_lid, const acc_mc_object_id_t* p_obj_id);
#endif //(GAF_ACC_OTS)

/**
 ****************************************************************************************
 * @brief Confirmation for ACC_MCS_GET request indication.
 *
 * @param[in] accept        Indicate if request is accepted or not
 * @param[in] media_lid     Media local index
 * @param[in] con_lid       Connection local index
 * @param[in] char_type     Characteristic type
 * @param[in] token         Token
 * @param[in] length        Length
 * @param[in] p_val         Pointer to value
 ****************************************************************************************
 */
void acc_mcs_cfm_get(bool accept, uint8_t media_lid, uint8_t con_lid, uint8_t char_type,
                     uint16_t token, uint16_t length, const uint8_t* p_val);

/**
 ****************************************************************************************
 * @brief Confirmation for ACC_MCS_GET_POSITION request indication.
 *
 * @param[in] accept        Indicate if request is accepted or not
 * @param[in] media_lid     Media local index
 * @param[in] con_lid       Connection local index
 * @param[in] token         Token
 * @param[in] track_pos     Track Position
 ****************************************************************************************
 */
void acc_mcs_cfm_get_position(bool accept, uint8_t media_lid, uint8_t con_lid, uint16_t token,
                              int32_t track_pos);

/// @} ACC_MCS_FUNCTION
#endif //(GAF_ACC_MCS)

#endif // ACC_MCS_H_
