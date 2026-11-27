/**
 ****************************************************************************************
 *
 * @file csis.h
 *
 * @brief Coordinated Set Identification Service - Definitions
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef CSIS_H_
#define CSIS_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup CSIP_API CSIP Coordinated Set Identification Profile
 * @ingroup PROFILE_API
 * @brief Description of Coordinated Set Identification Profile
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup CSIP_COMMON Common
 * @ingroup CSIP_API
 * @brief Description of Common API for Coordinated Set Identification Profile
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup CSIP_DEFINES Definitions
 * @ingroup CSIP_COMMON
 * @brief Defines for Coordinated Set Identification Profile
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup CSIP_STRUCT Structures
 * @ingroup CSIP_COMMON
 * @brief Structures for Coordinated Set Identification Profile
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup CSIP_ENUM Enumerations
 * @ingroup CSIP_COMMON
 * @brief Enumerations for Coordinated Set Identification Profile
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "prf_types.h"              // Profiles Types Definition
#include "gap.h"

/*
 * DEFINES
 ****************************************************************************************
 */

/// @addtogroup CSIP_DEFINES
/// @{

/// Length of RSI value
#define CSIS_RSI_LEN                    (6)
/// Length of Coordinated Set Size characteristic value
#define CSIS_SIZE_LEN                   (1)
/// Length of Set Member Lock characteristic value
#define CSIS_LOCK_LEN                   (1)
/// Length of Set Member Rank characteristic value
#define CSIS_RANK_LEN                   (1)
/// Length of Prand value
#define CSIS_PRAND_LEN                  (3)
/// Length of Hash value
#define CSIS_HASH_LEN                   (3)
/// Position of Hash value in RSI value
#define CSIS_HASH_POS                   (0)
/// Position of Prand value
#define CSIS_PRAND_POS                  (3)
/// Invalid local index
#define CSIS_INVALID_LID                (0xFF)
/// Minimum Size value
#define CSIS_SIZE_MIN                   (1)
/// S1 M value length ("SIRKenc")
#define CSIS_S1_M_LEN                   (7)
/// K1 P value length ("csis")
#define CSIS_K1_P_LEN                   (4)
/// K1 Salt length
#define CSIS_K1_SALT_LEN                (16)
/// K1 length
#define CSIS_K1_LEN                     (16)

/// @} CSIP_DEFINES

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// @addtogroup CSIP_ENUM
/// @{

/// Characteristic type values for Coordinated Set Identification Service
enum csis_char_type
{
    /// Set Identity Resolving Key characteristic
    CSIS_CHAR_TYPE_SIRK = 0,
    /// Coordinated Set Size characteristic
    CSIS_CHAR_TYPE_SIZE,
    /// Set Member Lock characteristic
    CSIS_CHAR_TYPE_LOCK,

    CSIS_CHAR_TYPE_NTF_MAX,

    /// Set Member Rank characteristic
    CSIS_CHAR_TYPE_RANK = CSIS_CHAR_TYPE_NTF_MAX,

    CSIS_CHAR_TYPE_MAX,
};

/// Descriptor type values for Coordinated Set Identification Service
enum csis_desc_type
{
    /// Client Characteristic Configuration descriptor for Set Identity Resolving Key characteristic
    CSIS_DESC_TYPE_CCC_SIRK = 0,
    /// Client Characteristic Configuration descriptor for Coordinated Set Size characteristic
    CSIS_DESC_TYPE_CCC_SIZE,
    /// Client Characteristic Configuration descriptor for Set Member Lock characteristic
    CSIS_DESC_TYPE_CCC_LOCK,

    CSIS_DESC_TYPE_MAX,
};

/// Lock state values
enum csis_lock
{
    CSIS_LOCK_MIN = 1,

    /// Unlocked
    CSIS_LOCK_UNLOCKED = CSIS_LOCK_MIN,
    /// Locked
    CSIS_LOCK_LOCKED,

    CSIS_LOCK_MAX,
};

/// SIRK type values
enum csis_sirk_type
{
    /// Encrypted SIRK
    CSIS_SIRK_TYPE_ENCRYPTED = 0,
    /// Plain text SIRK
    CSIS_SIRK_TYPE_PLAIN = 1,
};

/// Position/length of fields in SIRK Encryption/Decryption buffer
enum csis_sirk_pos_len
{
    /// Position of Type field
    CSIS_SIRK_POS_TYPE = 0,
    /// Position of Value field
    CSIS_SIRK_POS_VALUE = 1,

    /// Length of Type field
    CSIS_SIRK_LEN_TYPE = 1,
    /// Length of VALUE field
    CSIS_SIRK_LEN_VALUE = 16,

    /// Length of SIRK characteristic value
    CSIS_SIRK_LEN = CSIS_SIRK_LEN_TYPE + CSIS_SIRK_LEN_VALUE,

    /// Position of LTK
    CSIS_SIRK_POS_LTK = CSIS_SIRK_LEN,
};

/// Specific error codes for Coordinated Set Identification service
enum csis_err
{
    /// Lock is denied
    CSIS_ERR_LOCK_DENIED = 0x80,
    /// Lock release is not allowed
    CSIS_ERR_LOCK_RELEASE_DISALLOWED,
    /// Invalid Lock value
    CSIS_ERR_INVALID_LOCK_VALUE,
    /// Support OOB SIRK only
    CSIS_ERR_OOB_SIRK_ONLY,
    /// Lock is already granted
    CSIS_ERR_LOCK_ALREADY_GRANTED,

    CSIS_ERR_UNEXPECTED,
};

/// @} CSIP_ENUM

/*
 * VARIABLES
 ****************************************************************************************
 */

/// K1 P value "csis"
extern const uint8_t csis_key_k1_p_val[CSIS_K1_P_LEN];
/// S1("SIRKenc") value
extern const uint8_t csis_key_s1_val[CSIS_K1_SALT_LEN];

/*
 * TYPE DEFINTIONS
 ****************************************************************************************
 */

/// @addtogroup CSIP_STRUCT
/// @{

/// SIRK
typedef struct csis_sirk
{
    /// SIRK
    uint8_t sirk[CSIS_SIRK_LEN_VALUE];
} csis_sirk_t;

/// RSI
typedef struct csis_rsi
{
    /// RSI
    uint8_t rsi[CSIS_RSI_LEN];
} csis_rsi_t;

/// LTK
typedef struct csis_ltk
{
    /// Indicate if LTK is obtained
    bool obtained;
    /// LTK
    uint8_t ltk[GAP_KEY_LEN];
} csis_ltk_t;

/// @} CSIP_STRUCT

/**
 ****************************************************************************************
 * @brief Call back definition of the function that can handle result of an AES based algorithm
 *
 * @param[in] p_aes_res  16 bytes block result
 ****************************************************************************************
 */
typedef void (*csis_sec_aes_res_cb) (const uint8_t* p_aes_res);

#endif // CSIS_H_
