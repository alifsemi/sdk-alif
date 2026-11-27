/**
 ****************************************************************************************
 *
 * @file mesh_api_store.h
 *
 * @brief Header file for Mesh Stack Application Programming Interface (Storage part)
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef MESH_API_STORE_
#define MESH_API_STORE_

#include "rom_build_cfg.h"

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "mesh_defines.h"
#include "co_math.h"

/// @addtogroup MESH_API_STORE
/// @{

/*
 * ENUMERATIONS FOR MESH PROFILE
 ****************************************************************************************
 */

/// Configuration bit field meaning
/// 31                    1               0
/// ---------------------------------------
/// | RFU                 | UPDATE IND EN |
/// ---------------------------------------
enum m_store_config_bf
{
    /// Enable/disable sending of update indications - Position
    M_STORE_CONFIG_UPD_IND_EN_POS = 0,
    /// Enable/disable sending of update indications - Bit
    M_STORE_CONFIG_UPD_IND_EN_BIT = CO_BIT(M_STORE_CONFIG_UPD_IND_EN_POS),
};

/// Storage type values
enum m_store_type
{
    /// Miscellaneous states\n
    /// See #m_store_state_t
    M_STORE_TYPE_STATE = 0,
    /// Network key information\n
    /// See #m_store_netkey_t
    M_STORE_TYPE_NET_KEY,
    /// Application key information\n
    /// See #m_store_appkey_t
    M_STORE_TYPE_APP_KEY,
    /// Publication parameters\n
    /// See #m_store_publi_t or #m_store_publi_virt_t
    M_STORE_TYPE_PUBLI_INFO,
    /// Subscription list\n
    /// See #m_store_subs_t
    M_STORE_TYPE_SUBS_LIST,
    /// Binding\n
    /// See #m_store_binding_t
    M_STORE_TYPE_BINDING,
    /// LPN Information\n
    /// See #m_store_lpn_t
    M_STORE_TYPE_LPN,
    /// Friend Information\n
    /// See #m_store_friend_t
    M_STORE_TYPE_FRIEND,
    /// IV/SEQ value\n
    /// See #m_store_iv_seq_t
    M_STORE_TYPE_IV_SEQ,

    /// Maximum value
    M_STORE_TYPE_MAX,
};

/// Storage update type values
enum m_store_upd_type
{
    /// Network key updated
    M_STORE_UPD_TYPE_NET_KEY_UPDATED = 0,
    /// Network key deleted
    M_STORE_UPD_TYPE_NET_KEY_DELETED,
    /// Application key updated
    M_STORE_UPD_TYPE_APP_KEY_UPDATED,
    /// Application key deleted
    M_STORE_UPD_TYPE_APP_KEY_DELETED,
    /// Model publication parameters updated
    M_STORE_UPD_TYPE_PUBLI_PARAM,
    /// Model subscription list updated
    M_STORE_UPD_TYPE_SUBS_LIST,
    /// Model/application key binding updated
    M_STORE_UPD_TYPE_BINDING,
    /// State updated
    M_STORE_UPD_TYPE_STATE,
    /// Friendship with LPN updated
    M_STORE_UPD_TYPE_LPN_UPDATED,
    /// Friendship with LPN lost
    M_STORE_UPD_TYPE_LPN_LOST,
    /// Friendship with Friend updated
    M_STORE_UPD_TYPE_FRIEND_UPDATED,
    /// Friendship with Friend lost
    M_STORE_UPD_TYPE_FRIEND_LOST,
    /// IV / SEQ values updated
    M_STORE_UPD_TYPE_IV_SEQ,

    /// Maximum value
    M_STORE_UPD_TYPE_MAX,
};

/// Storage state type values
enum m_store_state_type
{
    /// Primary element unicast address
    M_STORE_STATE_TYPE_UNICAST_ADDR = 0,
    /// Default TTL state
    M_STORE_STATE_TYPE_DEFAULT_TTL,
    /// Secure network beacon state
    M_STORE_STATE_TYPE_SEC_BCN,
    /// Network transmit state
    M_STORE_STATE_TYPE_NET_TX,
    /// Relay state
    M_STORE_STATE_TYPE_RELAY,
    /// GATT proxy state
    M_STORE_STATE_TYPE_GATT_PROXY,
    /// Friend state
    M_STORE_STATE_TYPE_FRIEND,
    /// Device key
    M_STORE_STATE_TYPE_DEV_KEY,
    /// IV and SEQ values
    M_STORE_STATE_TYPE_IV_SEQ,

    /// Maximum value
    M_STORE_STATE_TYPE_MAX,
};

/// Network key entry information bitfield
/// 7     4         3     2       0
/// -------------------------------
/// | RFU | PHASE 2 | UPD | FLAGS |
/// -------------------------------
enum m_store_netkey_info
{
    /// Flags
    M_STORE_NETKEY_INFO_FLAGS_LSB  = 0,
    M_STORE_NETKEY_INFO_FLAGS_MASK = 0x03,

    /// Update key present
    M_STORE_NETKEY_INFO_UPD_POS = 2,
    M_STORE_NETKEY_INFO_UPD_BIT = CO_BIT(M_STORE_NETKEY_INFO_UPD_POS),

    /// Phase 2
    M_STORE_NETKEY_INFO_P2_POS = 3,
    M_STORE_NETKEY_INFO_P2_BIT = CO_BIT(M_STORE_NETKEY_INFO_P2_POS),
};

/// Application key entry information bitfield
/// 7     1     0
/// -------------
/// | RFU | UPD |
/// -------------
enum m_store_appkey_info
{
    /// Update key present
    M_STORE_APPKEY_INFO_UPD_POS = 0,
    M_STORE_APPKEY_INFO_UPD_BIT = CO_BIT(M_STORE_APPKEY_INFO_UPD_POS),
};

/// Publication parameters entry information bitfield
/// 7        6         0
/// --------------------
/// |   RFU  |  VENDOR |
/// --------------------
enum m_store_publi_info
{
    /// SIG or vendor model ID
    M_STORE_PUBLI_INFO_VENDOR_POS = 0,
    M_STORE_PUBLI_INFO_VENDOR_BIT = CO_BIT(M_STORE_PUBLI_INFO_VENDOR_POS),
};

/// Subscription list entry information bitfield
/// 7        6         0
/// --------------------
/// | VENDOR | NB_ADDR |
/// --------------------
enum m_store_subs_info
{
    /// Number of addresses in the subscription list
    M_STORE_SUBS_INFO_NB_ADDR_LSB = 0,
    M_STORE_SUBS_INFO_NB_ADDR_MASK = 0x7F,

    /// SIG or vendor model ID
    M_STORE_SUBS_INFO_VENDOR_POS = 7,
    M_STORE_SUBS_INFO_VENDOR_BIT = CO_BIT(M_STORE_SUBS_INFO_VENDOR_POS),
};

/// Model/application key binding entry information bitfield
/// 7        1            0
/// -----------------------
/// | VENDOR | NB_APP_IDS |
/// -----------------------
enum m_store_binding_info
{
    /// Number of application keys bound with the model
    M_STORE_BINDING_INFO_NB_APP_IDS_LSB = 0,
    M_STORE_BINDING_INFO_NB_APP_IDS_MASK = 0x7F,

    /// SIG or vendor model ID
    M_STORE_BINDING_INFO_VENDOR_POS = 7,
    M_STORE_BINDING_INFO_VENDOR_BIT = CO_BIT(M_STORE_BINDING_INFO_VENDOR_POS),
};

/*
 * TYPES DEFINITION FOR MESH PROFILE
 ****************************************************************************************
 */

/// Model Unique Identifier structure
typedef struct m_store_model_uid
{
    /// Element address
    uint16_t element_addr;
    /// Model ID
    uint32_t model_id;
} m_store_model_uid_t;

/// Header structure
typedef struct m_store_hdr
{
    /// Type (see #m_store_type enumeration)
    uint8_t type;
} m_store_hdr_t;

/// Header structure for key-related information
typedef struct m_store_hdr_key
{
    /// Header
    m_store_hdr_t hdr;
    /// Key ID
    uint16_t key_id;
} m_store_hdr_key_t;

/// Header structure for model-related information
typedef struct m_store_hdr_model
{
    /// Header
    m_store_hdr_t hdr;
    /// Model Unique Identifier
    m_store_model_uid_t model_uid;
} m_store_hdr_model_t;

/// Header structure for device-related information
typedef struct m_store_hdr_device
{
    /// Header
    m_store_hdr_t hdr;
    /// Address
    uint16_t addr;
} m_store_hdr_device_t;

/// State structure
typedef struct m_store_state
{
    /// Header
    m_store_hdr_t hdr;
    /// State that has been updated
    /// Meaningless for a load
    uint8_t state_type;
    /// Unicast address
    uint16_t unicast_addr;
    /// Relay state
    uint8_t relay_state;
    /// Relay retransmit state
    uint8_t relay_retx;
    /// Default TTL
    uint8_t default_ttl;
    /// Secure Network Beacon state
    uint8_t beaconing_state;
    /// Network TX state
    uint8_t net_tx_state;
    /// GATT Proxy state
    uint8_t gatt_proxy_state;
    /// Friend state
    uint8_t friend_state;
    /// Device Key
    uint8_t device_key[MESH_KEY_LEN];
} m_store_state_t;

/// IV / SEQ values structure
typedef struct m_store_iv_seq
{
    /// Header
    m_store_hdr_t hdr;
    /// Last IV update time (wrap)
    uint16_t nb_wrap;
    /// Last IV update time (clock)
    uint32_t clock_ms;
    /// IV
    uint32_t iv;
    /// SEQ
    uint32_t seq;
    /// Information
    uint8_t info;
} m_store_iv_seq_t;

/// Network key information structure if network key is not being updated
typedef struct m_store_netkey
{
    /// Header
    m_store_hdr_key_t hdr;
    /// Information (see #m_store_netkey_info enumeration)
    uint8_t info;
    /// Network Key
    uint8_t key[MESH_KEY_LEN];
} m_store_netkey_t;

/// Network key information structure if network key is being updated
typedef struct m_store_netkey_upd
{
    /// Current key information
    m_store_netkey_t key_info;
    /// New network key
    uint8_t new_key[MESH_KEY_LEN];
} m_store_netkey_upd_t;

/// Application key information structure if application key is not being updated
typedef struct m_store_appkey
{
    /// Header
    m_store_hdr_key_t hdr;
    /// Information (see #m_store_appkey_info enumeration)
    uint8_t info;
    /// NetKey ID
    uint16_t netkey_id;
    /// Application Key
    uint8_t key[MESH_KEY_LEN];
} m_store_appkey_t;

/// Application key information structure if application key is being updated
typedef struct m_store_appkey_upd
{
    /// Current key information
    m_store_appkey_t key_info;
    /// New application key
    uint8_t new_key[MESH_KEY_LEN];
} m_store_appkey_upd_t;

/// Model publication parameter structure if publication address is not a virtual address
typedef struct m_store_publi
{
    /// Header
    m_store_hdr_model_t hdr;
    /// Information (see #m_store_publi_info enumeration)
    uint8_t info;
    /// Publication address
    uint16_t addr;
    /// AppKey ID
    uint16_t appkey_id;
    /// TTL
    uint8_t ttl;
    /// Period
    uint8_t period;
    /// Retransmission parameters
    uint8_t retx_params;
    /// Friend credentials
    uint8_t friend_cred;
} m_store_publi_t;

/// Model publication parameter structure if publication address is a virtual address
typedef struct m_store_publi_virt
{
    /// Publication parameters
    m_store_publi_t publi_params;
    /// Label UUID
    uint8_t label_uuid[M_LABEL_UUID_LEN];
} m_store_publi_virt_t;

/// Model subscription list structure
typedef struct m_store_subs
{
    /// Header
    m_store_hdr_model_t hdr;
    /// Information (see #m_store_subs_info enumeration)
    uint8_t info;
    /// Length of address list
    uint16_t length;
    /// Address list\n
    /// Each virtual address is followed by its associated Label UUID
    uint8_t list[__ARRAY_EMPTY];
} m_store_subs_t;

/// Model/Application key binding structure
typedef struct m_store_binding
{
    /// Header
    m_store_hdr_model_t hdr;
    /// Information (see #m_store_binding_info enumeration)
    uint8_t info;
    /// List of AppKey IDs
    uint16_t appkey_ids[__ARRAY_EMPTY];
} m_store_binding_t;

/// Friend structure (for a lost friendship)
typedef struct m_store_friend_lost
{
    /// Header
    m_store_hdr_device_t hdr;
} m_store_friend_lost_t;

/// Friend structure (empty subscription list)
typedef struct m_store_friend_empty
{
    /// Header
    m_store_hdr_device_t hdr;
    /// Low Power Node receive delay in milliseconds
    uint8_t rx_delay_ms;
    /// Friend Sequence Number
    uint8_t fsn;
    /// Number of elements in the Low Power Node
    uint8_t nb_elements;
    /// Current number of addresses in the subscription list (valid once friendship has
    /// been established
    uint8_t nb_addr;
    /// Poll Timeout in milliseconds
    uint32_t poll_timeout_ms;
    /// Last poll reception time
    uint32_t poll_rx_time_ms;
    uint16_t poll_rx_nb_wrap;
    /// Low Power Node counter
    uint16_t lpn_cnt;
    /// Friend counter for transmission of Friend Offer message
    uint16_t friend_cnt;
    /// Subscription list
    uint16_t subs_list[__ARRAY_EMPTY];
} m_store_friend_empty_t;

/// Friend structure
typedef struct m_store_friend
{
    /// Header
    m_store_hdr_device_t hdr;
    /// Low Power Node receive delay in milliseconds
    uint8_t rx_delay_ms;
    /// Friend Sequence Number
    uint8_t fsn;
    /// Number of elements in the Low Power Node
    uint8_t nb_elements;
    /// Current number of addresses in the subscription list (valid once friendship has
    /// been established
    uint8_t nb_addr;
    /// Poll Timeout in milliseconds
    uint32_t poll_timeout_ms;
    /// Last poll reception time
    uint32_t poll_rx_time_ms;
    uint16_t poll_rx_nb_wrap;
    /// Low Power Node counter
    uint16_t lpn_cnt;
    /// Friend counter for transmission of Friend Offer message
    uint16_t friend_cnt;
    /// Subscription list
    uint16_t subs_list[__ARRAY_EMPTY];
} m_store_friend_t;

/// LPN structure
typedef struct m_store_lpn
{
    /// Header
    m_store_hdr_device_t hdr;
    /// Poll interval in milliseconds
    uint32_t poll_intv_ms;
    /// Friend Counter
    uint16_t friend_cnt;
    /// LPN Counter
    uint16_t lpn_cnt;
    /// Receive delay in milliseconds
    uint8_t rx_delay_ms;
    /// Receive window in milliseconds
    uint8_t rx_window_ms;
    /// Friend subscription list size
    uint8_t subs_list_size;
    /// Friend Sequence Number
    uint8_t fsn;
} m_store_lpn_t;

/// LPN structure (for a lost friendship)
typedef struct m_store_lpn_lost
{
    /// Header
    m_store_hdr_device_t hdr;
} m_store_lpn_lost_t;


/// @} MESH_API_STORE

#endif /* MESH_API_STORE_ */
