/**
 ****************************************************************************************
 *
 * @file acc_mc.h
 *
 * @brief Audio Content Control - Media Control - Header file
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef ACC_MC_H_
#define ACC_MC_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup ACC_MC Media Control Service
 * @ingroup ACC
 * @brief Description of Media Control Service module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ACC_MC_DEF Definitions
 * @ingroup ACC_MC
 * @brief Definitions for Media Control Service module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ACC_MC_ENUM Enumerations
 * @ingroup ACC_MC
 * @brief Enumerations for Media Control Service module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ACC_MC_STRUCT Structures
 * @ingroup ACC_MC
 * @brief Structures for Media Control Service module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "acc.h"            // Audio Content Control Definitions

/*
 * DEFINES
 ****************************************************************************************
 */

/// @addtogroup ACC_MC_DEF
/// @{

/// Length of Object ID
#define ACC_MC_OBJ_ID_LEN                   (6)
/// Length of Track Changed characteristic value
#define ACC_MC_TRACK_CHANGED_LEN            (0)
/// Length of Media State characteristic value
#define ACC_MC_MEDIA_STATE_LEN              (1)
/// Length of Track Duration characteristic value
#define ACC_MC_TRACK_DURATION_LEN           (4)
/// Length of Track Position characteristic value
#define ACC_MC_TRACK_POSITION_LEN           (4)
/// Length of Playback Speed characteristic value
#define ACC_MC_PLAYBACK_SPEED_LEN           (1)
/// Length of Seeking Speed characteristic value
#define ACC_MC_SEEKING_SPEED_LEN            (1)
/// Length of Playing Order characteristic value
#define ACC_MC_PLAYING_ORDER_LEN            (1)
/// Length of Playing Order Supported characteristic value
#define ACC_MC_PLAYING_ORDER_SUPP_LEN       (2)
/// Length of Media Control Point Opcodes Supported characteristic value
#define ACC_MC_MEDIA_CP_OPCODES_SUPP_LEN    (4)
/// Length of Search Control Point Result Code value
#define ACC_MC_SEARCH_CP_RESULT_LEN         (1)
/// Indicate that Track Duration is unknown
#define ACC_MC_TRACK_DUR_UNKNOWN            (0xFFFFFFFF)
/// Indicate that Track Position is unavailable
#define ACC_MC_TRACK_POS_UNAVAILABLE        (0xFFFFFFFF)

/// @} ACC_MC_DEF

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// @addtogroup ACC_MC_ENUM
/// @{

/// Characteristic type values
/// Values are ordered so that the notification-capable characteristic type values are equal
/// to the characteristic type values
enum acc_mc_char_type
{
    /// Media Player Name characteristic
    ACC_MC_CHAR_TYPE_PLAYER_NAME = 0,
    /// Track Changed characteristic
    ACC_MC_CHAR_TYPE_TRACK_CHANGED,
    /// Track Title characteristic
    ACC_MC_CHAR_TYPE_TRACK_TITLE,
    /// Track Duration characteristic
    ACC_MC_CHAR_TYPE_TRACK_DURATION,
    /// Track Position characteristic
    ACC_MC_CHAR_TYPE_TRACK_POSITION,
    /// Playback Speed characteristic
    ACC_MC_CHAR_TYPE_PLAYBACK_SPEED,
    /// Seeking Speed characteristic
    ACC_MC_CHAR_TYPE_SEEKING_SPEED,
    /// Current Track Object ID characteristic
    ACC_MC_CHAR_TYPE_CUR_TRACK_OBJ_ID,
    /// Next Track Object ID characteristic
    ACC_MC_CHAR_TYPE_NEXT_TRACK_OBJ_ID,
    /// Current Group Object ID characteristic
    ACC_MC_CHAR_TYPE_CUR_GROUP_OBJ_ID,
    /// Parent Group Object ID characteristic
    ACC_MC_CHAR_TYPE_PARENT_GROUP_OBJ_ID,
    /// Playing Order characteristic
    ACC_MC_CHAR_TYPE_PLAYING_ORDER,
    /// Media State characteristic
    ACC_MC_CHAR_TYPE_MEDIA_STATE,
    /// Media Control Point
    ACC_MC_CHAR_TYPE_MEDIA_CP,
    /// Media Control Point Opcodes Supported
    ACC_MC_CHAR_TYPE_MEDIA_CP_OPCODES_SUPP,
    /// Search Results Object ID
    ACC_MC_CHAR_TYPE_SEARCH_RESULTS_OBJ_ID,
    /// Search Control Point
    ACC_MC_CHAR_TYPE_SEARCH_CP,

    /// All characteristics above are notification-capable
    ACC_MC_NTF_CHAR_TYPE_MAX,

    /// Current Track Segments Object ID characteristic
    ACC_MC_CHAR_TYPE_CUR_TRACK_SEG_OBJ_ID = ACC_MC_NTF_CHAR_TYPE_MAX,
    /// Media Player Icon Object ID characteristic
    ACC_MC_CHAR_TYPE_PLAYER_ICON_OBJ_ID,
    /// Media Player Icon URL characteristic
    ACC_MC_CHAR_TYPE_PLAYER_ICON_URL,
    /// Playing Order Supported characteristic
    ACC_MC_CHAR_TYPE_PLAYING_ORDER_SUPP,
    /// Content Control ID
    ACC_MC_CHAR_TYPE_CCID,

    ACC_MC_CHAR_TYPE_MAX,
};

/// Descriptor type values
enum acc_mc_desc_type
{
    /// Client Characteristic Configuration descriptor for Media Player Name characteristic
    ACC_MC_DESC_TYPE_CCC_PLAYER_NAME = 0,
    /// Client Characteristic Configuration descriptor for Track Changed characteristic
    ACC_MC_DESC_TYPE_CCC_TRACK_CHANGED,
    /// Client Characteristic Configuration descriptor for Track Title characteristic
    ACC_MC_DESC_TYPE_CCC_TRACK_TITLE,
    /// Client Characteristic Configuration descriptor for Track Duration characteristic
    ACC_MC_DESC_TYPE_CCC_TRACK_DURATION,
    /// Client Characteristic Configuration descriptor for Track Position characteristic
    ACC_MC_DESC_TYPE_CCC_TRACK_POSITION,
    /// Client Characteristic Configuration descriptor for Playback Speed characteristic
    ACC_MC_DESC_TYPE_CCC_PLAYBACK_SPEED,
    /// Client Characteristic Configuration descriptor for Seeking Speed characteristic
    ACC_MC_DESC_TYPE_CCC_SEEKING_SPEED,
    /// Client Characteristic Configuration descriptor for Current Track Object ID characteristic
    ACC_MC_DESC_TYPE_CCC_CUR_TRACK_OBJ_ID,
    /// Client Characteristic Configuration descriptor for Next Track Object ID characteristic
    ACC_MC_DESC_TYPE_CCC_NEXT_TRACK_OBJ_ID,
    /// Client Characteristic Configuration descriptor for Current Group Object ID characteristic
    ACC_MC_DESC_TYPE_CCC_CUR_GROUP_OBJ_ID,
    /// Client Characteristic Configuration descriptor for Parent Group Object ID characteristic
    ACC_MC_DESC_TYPE_CCC_PARENT_GROUP_OBJ_ID,
    /// Client Characteristic Configuration descriptor for Playing Order characteristic
    ACC_MC_DESC_TYPE_CCC_PLAYING_ORDER,
    /// Client Characteristic Configuration descriptor for Media State characteristic
    ACC_MC_DESC_TYPE_CCC_MEDIA_STATE,
    /// Client Characteristic Configuration descriptor for Media Control Point
    ACC_MC_DESC_TYPE_CCC_MEDIA_CP,
    /// Client Characteristic Configuration descriptor for Media Control Point Opcodes Supported
    ACC_MC_DESC_TYPE_CCC_MEDIA_CP_OPCODES_SUPP,
    /// Client Characteristic Configuration descriptor for Search Results Object ID
    ACC_MC_DESC_TYPE_CCC_SEARCH_RESULTS_OBJ_ID,
    /// Client Characteristic Configuration descriptor for Search Control Point
    ACC_MC_DESC_TYPE_CCC_SEARCH_CP,

    ACC_MC_DESC_TYPE_MAX,
};

/// Operation code values
enum acc_mc_opcode
{
    /// Start playing the current track
    ACC_MC_OPCODE_PLAY = 1,
    /// Pause playing the current track
    ACC_MC_OPCODE_PAUSE,
    /// Fast rewind the current track
    ACC_MC_OPCODE_FAST_RW,
    /// Fast forward the current track
    ACC_MC_OPCODE_FAST_FW,
    /// Stop current activity and return to inactive state
    ACC_MC_OPCODE_STOP,
    /// Set the current position relative to the current position
    ACC_MC_OPCODE_MOVE_RELATIVE = 16,
    /// Set the current position to the previous segment of the current track
    ACC_MC_OPCODE_PREV_SEG = 32,
    /// Set the current position to the next segment of the current track
    ACC_MC_OPCODE_NEXT_SEG,
    /// Set the current position to the first segment of the current track
    ACC_MC_OPCODE_FIRST_SEG,
    /// Set the current position to the last segment of the current track
    ACC_MC_OPCODE_LAST_SEG,
    /// Set the current position to the nth segment of the current track
    ACC_MC_OPCODE_GOTO_SEG,
    /// Set the current track to the previous track in the current group playing order
    ACC_MC_OPCODE_PREV_TRACK = 48,
    /// Set the current track to the next track in the current group playing order
    ACC_MC_OPCODE_NEXT_TRACK,
    /// Set the current track to the first track in the current group playing order
    ACC_MC_OPCODE_FIRST_TRACK,
    /// Set the current track to the last track in the current group playing order
    ACC_MC_OPCODE_LAST_TRACK,
    /// Set the current track to the nth track in the current group playing order
    ACC_MC_OPCODE_GOTO_TRACK,
    /// Set the current group to the previous group in the sequence of groups
    ACC_MC_OPCODE_PREV_GROUP = 64,
    /// Set the current group to the next group in the sequence of groups
    ACC_MC_OPCODE_NEXT_GROUP,
    /// Set the current group to the first group in the sequence of groups
    ACC_MC_OPCODE_FIRST_GROUP,
    /// Set the current group to the next group in the sequence of groups
    ACC_MC_OPCODE_LAST_GROUP,
    /// Set the current group to the nth group in the sequence of groups
    ACC_MC_OPCODE_GOTO_GROUP,
};

/// Operation code supported bit field meaning
enum acc_mc_opcode_supp
{
    /// Start playing the current track
    ACC_MC_OPCODE_SUPP_PLAY_BIT = CO_BIT(0),
    /// Pause playing the current track
    ACC_MC_OPCODE_SUPP_PAUSE_BIT = CO_BIT(1),
    /// Fast rewind the current track
    ACC_MC_OPCODE_SUPP_FAST_RW_BIT = CO_BIT(2),
    /// Fast forward the current track
    ACC_MC_OPCODE_SUPP_FAST_FW_BIT = CO_BIT(3),
    /// Stop current activity and return to inactive state
    ACC_MC_OPCODE_SUPP_STOP_BIT = CO_BIT(4),
    /// Set the current position relative to the current position
    ACC_MC_OPCODE_SUPP_MOVE_RELATIVE_BIT = CO_BIT(5),
    /// Set the current position to the previous segment of the current track
    ACC_MC_OPCODE_SUPP_PREV_SEG_BIT = CO_BIT(6),
    /// Set the current position to the next segment of the current track
    ACC_MC_OPCODE_SUPP_NEXT_SEG_BIT = CO_BIT(7),
    /// Set the current position to the first segment of the current track
    ACC_MC_OPCODE_SUPP_FIRST_SEG_BIT = CO_BIT(8),
    /// Set the current position to the last segment of the current track
    ACC_MC_OPCODE_SUPP_LAST_SEG_BIT = CO_BIT(9),
    /// Set the current position to the nth segment of the current track
    ACC_MC_OPCODE_SUPP_GOTO_SEG_BIT = CO_BIT(10),
    /// Set the current track to the previous track in the current group playing order
    ACC_MC_OPCODE_SUPP_PREV_TRACK_BIT = CO_BIT(11),
    /// Set the current track to the next track in the current group playing order
    ACC_MC_OPCODE_SUPP_NEXT_TRACK_BIT = CO_BIT(12),
    /// Set the current track to the first track in the current group playing order
    ACC_MC_OPCODE_SUPP_FIRST_TRACK_BIT = CO_BIT(13),
    /// Set the current track to the last track in the current group playing order
    ACC_MC_OPCODE_SUPP_LAST_TRACK_BIT = CO_BIT(14),
    /// Set the current track to the nth track in the current group playing order
    ACC_MC_OPCODE_SUPP_GOTO_TRACK_BIT = CO_BIT(15),
    /// Set the current group to the previous group in the sequence of groups
    ACC_MC_OPCODE_SUPP_PREV_GROUP_BIT = CO_BIT(16),
    /// Set the current group to the next group in the sequence of groups
    ACC_MC_OPCODE_SUPP_NEXT_GROUP_BIT = CO_BIT(17),
    /// Set the current group to the first group in the sequence of groups
    ACC_MC_OPCODE_SUPP_FIRST_GROUP_BIT = CO_BIT(18),
    /// Set the current group to the next group in the sequence of groups
    ACC_MC_OPCODE_SUPP_LAST_GROUP_BIT = CO_BIT(19),
    /// Set the current group to the nth group in the sequence of groups
    ACC_MC_OPCODE_SUPP_GOTO_GROUP_BIT = CO_BIT(20),

    ACC_MC_MEDIA_CP_OPCODES_SUPP_MASK = 0x001FFFFF,
};

/// Search type values
enum acc_mc_search_type
{
    ACC_MC_SEARCH_TYPE_MIN = 1,

    /// Track Name
    ACC_MC_SEARCH_TRACK_NAME = ACC_MC_SEARCH_TYPE_MIN,
    /// Artist Name
    ACC_MC_SEARCH_ARTIST_NAME,
    /// Album Name
    ACC_MC_SEARCH_ALBUM_NAME,
    /// Group Name
    ACC_MC_SEARCH_GROUP_NAME,
    /// Earliest Year
    ACC_MC_SEARCH_EARLIEST_YEAR,
    /// Latest Year
    ACC_MC_SEARCH_LATEST_YEAR,
    /// Genre
    ACC_MC_SEARCH_GENRE,
    /// Only Tracks
    ACC_MC_SEARCH_ONLY_TRACKS,
    /// Only Groups
    ACC_MC_SEARCH_ONLY_GROUPS,

    ACC_MC_SEARCH_TYPE_MAX,
};

/// Media State values
enum acc_mc_media_state
{
    /// Inactive
    ACC_MC_MEDIA_STATE_INACTIVE = 0,
    /// Playing
    ACC_MC_MEDIA_STATE_PLAYING,
    /// Paused
    ACC_MC_MEDIA_STATE_PAUSED,
    /// Seeking
    ACC_MC_MEDIA_STATE_SEEKING,

    ACC_MC_MEDIA_STATE_MAX,
};

/// Playing Order values
enum acc_mc_play_order
{
    ACC_MC_PLAY_ORDER_MIN = 1,

    /// A single track is played once there is no next track
    ACC_MC_PLAY_ORDER_SINGLE_ONCE = ACC_MC_PLAY_ORDER_MIN,
    /// A single track is played repeatedly; the next track is the current track
    ACC_MC_PLAY_ORDER_SINGLE_REPEAT,
    /// The tracks within a group are played once in track order
    ACC_MC_PLAY_ORDER_ORDER_ONCE,
    /// The tracks within a group are played in track order repeatedly
    ACC_MC_PLAY_ORDER_ORDER_REPEAT,
    /// The tracks within a group are played once only from the oldest first
    ACC_MC_PLAY_ORDER_OLDEST_ONCE,
    /// The tracks within a group are played from the oldest first repeatedly
    ACC_MC_PLAY_ORDER_OLDEST_REPEAT,
    /// The tracks within a group are played once only from the newest first
    ACC_MC_PLAY_ORDER_NEWEST_ONCE,
    /// The tracks within a group are played from the newest first repeatedly
    ACC_MC_PLAY_ORDER_NEWEST_REPEAT,
    /// The tracks within a group are played in random order once
    ACC_MC_PLAY_ORDER_SHUFFLE_ONCE,
    /// The tracks within a group are played in random order repeatedly
    ACC_MC_PLAY_ORDER_SHUFFLE_REPEAT,

    ACC_MC_PLAY_ORDER_MAX,

    /// Mask for Playing Order Supported characteristic value
    ACC_MC_PLAY_ORDER_SUPP_MASK = 0x03FF,
};

/// Result code values of Media Control Point Notificaiton
enum acc_mc_media_cp_result
{
    ACC_MC_MEDIA_CP_RESULT_MIN = 1,

    /// Success
    ACC_MC_MEDIA_CP_RESULT_SUCCESS = ACC_MC_MEDIA_CP_RESULT_MIN,
    /// Operation code not supported
    ACC_MC_MEDIA_CP_RESULT_NOT_SUPPORTED,
    /// Media player inactive
    ACC_MC_MEDIA_CP_RESULT_PLAYER_INACTIVE,
    /// Command cannot be completed
    ACC_MC_MEDIA_CP_RESULT_CANNOT_COMPLETE,

    ACC_MC_MEDIA_CP_RESULT_MAX,
};

/// Result code values of Search Control Point Notification
enum acc_mc_search_cp_result
{
    ACC_MC_SEARCH_CP_RESULT_MIN = 1,

    /// Success
    ACC_MC_SEARCH_CP_RESULT_SUCCESS = ACC_MC_SEARCH_CP_RESULT_MIN,
    /// Failure
    ACC_MC_SEARCH_CP_RESULT_FAILURE,

    ACC_MC_SEARCH_CP_RESULT_MAX,
};

/// Media Control Point characteristic value position when written
enum acc_mc_cp_pos_wr
{
    /// Operation code
    ACC_MC_CP_POS_WR_OPCODE_POS = 0,
    /// Parameter
    ACC_MC_CP_POS_WR_PARAM_POS,

    /// Minimum length
    ACC_MC_CP_POS_WR_LEN_MIN = 1,
    /// Maximum length
    ACC_MC_CP_POS_WR_LEN_MAX = 5,
};

/// Media Control Point characteristic value position when notified
enum acc_mc_cp_pos_ntf
{
    /// Operation code
    ACC_MC_CP_POS_NTF_OPCODE_POS = 0,
    /// Result code
    ACC_MC_CP_POS_NTF_RESULT_POS,

    /// Length
    ACC_MC_CP_POS_NTF_LEN,
};

/// Search Control Point characteristic value position
enum acc_mc_search_cp_pos
{
    /// Item length
    ACC_MC_SEARCH_CP_ITEM_LEN_POS = 0,
    /// Type
    ACC_MC_SEARCH_CP_TYPE_POS,

    /// Minimum length
    ACC_MC_SEARCH_CP_LEN_MIN,
    /// Minimum item length
    ACC_MC_SEARCH_CP_ITEM_LEN_MIN = 1,

    /// Parameter
    ACC_MC_SEARCH_CP_PARAM_POS = ACC_MC_SEARCH_CP_LEN_MIN,

    /// Maximum length
    ACC_MC_SEARCH_CP_LEN_MAX = 64,
};

/// Application error codes for Media Control Service
enum acc_mc_err
{
    /// Value changed during Read Long
    ACC_MC_ERR_VALUE_CHANGED = 0x80,

    ACC_MC_ERR_UNEXPECTED,
};

/// @} ACC_MC_ENUM

/*
 * TYPES DEFINITION
 ****************************************************************************************
 */

/// @addtogroup ACC_MC_STRUCT
/// @{

/// Object ID
typedef struct acc_mc_object_id
{
    /// Object ID
    uint8_t obj_id[ACC_MC_OBJ_ID_LEN];
} acc_mc_object_id_t;

/// @} ACC_MC_STRUCT

#endif // ACC_MC_H_
