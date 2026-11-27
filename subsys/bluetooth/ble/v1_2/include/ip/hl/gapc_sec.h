/**
 ****************************************************************************************
 *
 * @file gapc_sec.h
 *
 * @brief Generic Access Profile Controller - Security API.
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef GAPC_SEC_H_
#define GAPC_SEC_H_

#include "rom_build_cfg.h"

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "gapc.h"

/**
 ****************************************************************************************
 * @addtogroup GAPC_SEC_API Connection Security (Bond and Encrypt)
 * @ingroup GAPC_API
 * @brief Set of function and callback to handle connection security.
 *
 * Information about handling security is provided in \ref gap_link_sec_section documentation section
 *
 * Example of LE Security interface callback function handling:
 * \snippet{lineno} app_test_le_periph.c APP_LE_SEC_CB
 *
 * \if(btdm)
 * Example of BT-Classic Security interface callback function handling:
 * \snippet{lineno} app_connection.c APP_BT_SEC_CB
 * \endif
 *
 * @{
 * @}
 ****************************************************************************************
 */

/*
 * DEFINES
 ****************************************************************************************
 */

/// @addtogroup GAPC_ENUM_API
/// @{

/// Keypress Notification types
enum gapc_key_ntf_type
{
    /// Passkey entry started
    GAPC_KEY_NTF_ENTRY_STARTED = 0u,
    /// Passkey digit entered
    GAPC_KEY_NTF_DIGIT_ENTERED,
    /// Passkey digit erased
    GAPC_KEY_NTF_DIGIT_ERASED,
    /// Passkey cleared
    GAPC_KEY_NTF_CLEARED,
    /// Passkey entry completed
    GAPC_KEY_NTF_ENTRY_COMPLETED
};

/// Pairing info
enum gapc_expected_info
{
    /// Identity resolving key
    GAPC_INFO_IRK = 0u,
    /// Connection signature resolving key
    GAPC_INFO_CSRK,
    /// Temporary key - OOB data information - Legacy Pairing
    GAPC_INFO_TK_OOB,
    /// Temporary key - Pin code displayed - Legacy Pairing
    GAPC_INFO_TK_DISPLAYED,
    /// Temporary key - Pin code entered - Legacy Pairing
    GAPC_INFO_TK_ENTERED,
    /// OOB data information - Secure Connection
    GAPC_INFO_OOB,
    /// Pass key Displayed- Secure Connection
    GAPC_INFO_PASSKEY_DISPLAYED,
    /// Pass key Entered - Secure Connection
    GAPC_INFO_PASSKEY_ENTERED,
    /// Bluetooth classic IO capabilities
    GAPC_INFO_BT_IOCAP,
    /// Bluetooth classic PIN code value
    GAPC_INFO_BT_PIN_CODE,
    /// Bluetooth classic Passkey value
    GAPC_INFO_BT_PASSKEY,
    GAPC_INFO_MAX,
};

/// Type of combination key used during pairing
enum gapc_key_type
{
    /// Legacy pairing combination key
    GAPC_KEY_LEGACY_COMBINATION = 0x00u,
    /// Secure Connection unauthenticated key (P-256)
    GAPC_KEY_SEC_CON_UNAUTH = 0x07u,
    /// Secure Connection authenticated key (P-256)
    GAPC_KEY_SEC_CON_AUTH = 0x08u,
    /// Key type used during pairing is unknown
    GAPC_KEY_UNKNOWN = 0xFFu,
};

/// @} GAPC_ENUM_API

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// @addtogroup GAPC_STRUCT_API
/// @{

/// Pairing parameters
/*@TRACE*/
typedef struct
{
    /// IO capabilities (see #gap_io_cap enumeration)
    uint8_t iocap;
    /// OOB information (see #gap_le_oob_flag enumeration)
    uint8_t oob;
    /// Authentication (see #gap_auth enumeration)
    /// Note in BT 4.1 the Auth Field is extended to include 'Key Notification' and
    /// and 'Secure Connections'.
    uint8_t auth;
    /// Encryption key size (7 to 16)
    uint8_t key_size;
    ///Initiator key distribution (see #gap_kdist enumeration)
    uint8_t ikey_dist;
    ///Responder key distribution (see #gap_kdist enumeration)
    uint8_t rkey_dist;
} gapc_pairing_t;

#if (BLE_HOST_PRESENT)
/// Long Term Key information
/*@TRACE*/
typedef struct
{
    /// Long Term Key
    gap_sec_key_t key;
    /// Encryption Diversifier
    uint16_t ediv;
    /// Random Number
    gap_le_random_nb_t randnb;
    /// Encryption key size (7 to 16)
    uint8_t key_size;
} gapc_ltk_t;
#endif // (BLE_HOST_PRESENT)

/// Identity Resolving Key information
/*@TRACE*/
typedef struct
{
    /// Identity Resolving Key
    gap_sec_key_t key;
    /// Device BD Identity Address
    gap_bdaddr_t identity;
} gapc_irk_t;

/// Pairing Key information that can be stored in non-volatile memory
/*@TRACE*/
typedef struct
{
    /// Bit field that describe which key is valid (see #gap_kdist enumeration)
    uint8_t valid_key_bf;
    /// Pairing security level (see #gap_pairing_lvl enumeration)
    uint8_t pairing_lvl;
    #if (BLE_HOST_PRESENT)
    /// Identity resolving key information - for address resolution
    gapc_irk_t irk;
    /// Long term key - for encryption on Low Energy connection
    gapc_ltk_t ltk;
    /// Connection Signature Resolving Key - for attribute packet signature
    gap_sec_key_t csrk;
    #endif // (BLE_HOST_PRESENT)
} gapc_pairing_keys_t;

/// @} GAPC_SEC_API

/*
 * CALLBACK DEFINITIONS
 ****************************************************************************************
 */

/// @addtogroup GAPC_SEC_API
/// @{

/// Callback structure required to handle security events
struct gapc_security_cb
{
    #if (BLE_GAPC)
    /**
     ****************************************************************************************
     * Callback executed when an LE encryption is requested by peer device
     * Request shall be accept using #gapc_le_encrypt_req_reply
     *
     * @note Mandatory callback for peripheral if pairing supported
     *
     * @param[in] conidx        Connection index
     * @param[in] metainfo      Metadata information provided by API user (see \glos{METAINFO})
     * @param[in] ediv          Encryption diversifier
     * @param[in] p_rand        Pointer to random number value
     ****************************************************************************************
     */
    void (*le_encrypt_req)(uint8_t conidx, uint32_t metainfo, uint16_t ediv, const gap_le_random_nb_t* p_rand);
    #endif // (BLE_GAPC)


    /**
     ****************************************************************************************
     * @brief Callback executed when link authentication information available
     *
     * @note Optional callback
     *
     * @param[in] conidx    Connection index
     * @param[in] metainfo  Metadata information provided by API user (see \glos{METAINFO})
     * @param[in] sec_lvl   Link security level (see #gap_sec_lvl enumeration)
     * @param[in] encrypted True if link is encrypted, false otherwise
     * @param[in] key_size  Size of the encryption key (range [7-16]) - 0 if not encrypted
     ****************************************************************************************
     */
    void (*auth_info)(uint8_t conidx, uint32_t metainfo, uint8_t sec_lvl, bool encrypted, uint8_t key_size);

    /**
     ****************************************************************************************
     * Callback executed to inform that an on-going pairing has succeeded
     *
     * @note Mandatory callback
     *
     * @param[in] conidx          Connection index
     * @param[in] metainfo        Metadata information provided by API user (see \glos{METAINFO})
     * @param[in] pairing_level   Pairing security level (see #gap_pairing_lvl enumeration) achieved.
     * @param[in] enc_key_present LTK or link key exchanged during pairing.
     * @param[in] key_type        Type of combination key used during pairing (see #gapc_key_type enumeration).
     ****************************************************************************************
     */
    void (*pairing_succeed)(uint8_t conidx, uint32_t metainfo, uint8_t pairing_level, bool enc_key_present,
                            uint8_t key_type);

    /**
     ****************************************************************************************
     * Callback executed to inform that an on-going pairing has failed
     *
     * @note Mandatory callback
     *
     * @param[in] conidx        Connection index
     * @param[in] metainfo      Metadata information provided by API user (see \glos{METAINFO})
     * @param[in] reason        Failing reason
     ****************************************************************************************
     */
    void (*pairing_failed)(uint8_t conidx, uint32_t metainfo, uint16_t reason);

    /**
     ****************************************************************************************
     * Callback executed when an information is required by pairing algorithm.\n
     *
     * Request shall be be answered with:
     * - #gapc_le_pairing_provide_irk or #gapc_le_pairing_provide_irk_with_identity if exp_info = #GAPC_INFO_IRK
     * - #gapc_pairing_provide_csrk if exp_info = #GAPC_INFO_CSRK
     * - #gapc_le_pairing_provide_tk if exp_info = #GAPC_INFO_TK_OOB or #GAPC_INFO_TK_DISPLAYED or #GAPC_INFO_TK_ENTERED
     * - #gapc_pairing_provide_oob_data if exp_info = #GAPC_INFO_OOB
     * - #gapc_pairing_provide_passkey if exp_info = #GAPC_INFO_PASSKEY_DISPLAYED or #GAPC_INFO_PASSKEY_ENTERED
     *
     * \if (btdm)
     * BT Classic:
     * - #gapc_pairing_provide_passkey if exp_info = #GAPC_INFO_BT_PASSKEY
     * - #gapc_bt_pairing_provide_iocap if exp_info = #GAPC_INFO_BT_IOCAP
     * - #gapc_bt_pairing_provide_pin_code if exp_info = #GAPC_INFO_BT_PIN_CODE
     * \endif
     *
     * @note Mandatory callback
     *
     * @param[in] conidx        Connection index
     * @param[in] metainfo      Metadata information provided by API user (see \glos{METAINFO})
     * @param[in] exp_info      Expected information (see #gapc_expected_info enumeration)
     ****************************************************************************************
     */
    void (*info_req)(uint8_t conidx, uint32_t metainfo, uint8_t exp_info);

    /**
     ****************************************************************************************
     * Callback executed to inform that peer LE peripheral is asking for a pairing / encryption
     * with a specific authentication level.\n
     *
     * Starting pairing / encryption is optional on central side.
     *
     * @note Optional callback
     *
     * @param[in] conidx        Connection index
     * @param[in] metainfo      Metadata information provided by API user (see \glos{METAINFO})
     * @param[in] auth_level    Authentication level (@ref gap_auth)
     ****************************************************************************************
     */
    void (*auth_req)(uint8_t conidx, uint32_t metainfo, uint8_t auth_level);

    /**
     ****************************************************************************************
     * Callback executed to inform that a LE pairing is initiated by peer central.
     * Request shall be accept using #gapc_le_pairing_accept
     *
     * @note Mandatory callback for peripheral if pairing supported
     *
     * @param[in] conidx        Connection index
     * @param[in] metainfo      Metadata information provided by API user (see \glos{METAINFO})
     * @param[in] auth_level    Authentication level (@ref gap_auth)
     ****************************************************************************************
     */
    void (*pairing_req)(uint8_t conidx, uint32_t metainfo, uint8_t auth_level);


    /**
     ****************************************************************************************
     * Callback request some keys that must be exchange during pairing procedure\n
     * Request shall be be answered with: #gapc_pairing_numeric_compare_rsp
     *
     * @note Mandatory callback for LE secure connection or BT Classic pairing
     *
     * @param[in] conidx        Connection index
     * @param[in] metainfo      Metadata information provided by API user (see \glos{METAINFO})
     * @param[in] numeric_value Numeric value to display
     ****************************************************************************************
     */
    void (*numeric_compare_req)(uint8_t conidx, uint32_t metainfo, uint32_t numeric_value);


    /**
     ****************************************************************************************
     * Callback executed when peer key pressed is received
     *
     * @note Optional callback.
     *
     * @param[in] conidx            Connection index
     * @param[in] metainfo          Metadata information provided by API user (see \glos{METAINFO})
     * @param[in] notification_type Key pressed information (see #gapc_key_ntf_type enumeration)
     ****************************************************************************************
     */
    void (*key_pressed)(uint8_t conidx, uint32_t metainfo, uint8_t notification_type);


    #if (BLE_GAPC)
    /**
     ****************************************************************************************
     * Callback executed when an information is required by pairing algorithm.\n
     *
     * Request shall be be answered with #gapc_le_pairing_provide_ltk
     *
     * @note Required callback if legacy pairing supported.
     *
     * @param[in] conidx        Connection index
     * @param[in] metainfo      Metadata information provided by API user (see \glos{METAINFO})
     * @param[in] key_size      Size of the LTK to provide (range [7-16])
     ****************************************************************************************
     */
    void (*ltk_req)(uint8_t conidx, uint32_t metainfo, uint8_t key_size);
    #endif // (BLE_GAPC)

    /**
     ****************************************************************************************
     * Callback used to indicate pairing keys that must be stored on a non-volatile memory.
     *
     * @note Mandatory callback
     *
     * @param[in] conidx        Connection index
     * @param[in] metainfo      Metadata information provided by API user (see \glos{METAINFO})
     * @param[in] p_keys        Pointer to structure that contains keys received / computed
     ****************************************************************************************
     */
    void (*key_received)(uint8_t conidx, uint32_t metainfo, const gapc_pairing_keys_t* p_keys);

    /**
     ****************************************************************************************
     * Callback executed to inform that a pairing repeated attempt problem is detected.
     * Peer device is supposed to wait before initiating a new pairing
     *
     * @note Optional callback
     *
     * @param[in] conidx         Connection index
     * @param[in] metainfo       Metadata information provided by API user (see \glos{METAINFO})
     *
     ****************************************************************************************
     */
    void (*repeated_attempt)(uint8_t conidx, uint32_t metainfo);
};

/// @} GAPC_SEC_API

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/// @addtogroup GAPC_SEC_API Connection Security (Bond and Encrypt)
/// @{
// ---------------------------- SECURITY API ----------------------------------------------

#if (BLE_HOST_PRESENT)
#if (HL_LE_CENTRAL)
/**
 ****************************************************************************************
 * @brief On LE connection, encrypt an exiting BLE connection
 *
 * @note Can be initiated only by Central of the connection.
 * @note Once link is encrypted, #gapc_security_cb.auth_info is called
 *
 * @param[in] conidx     Connection index
 * @param[in] metainfo   Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] p_ltk_info Pointer to LTK information
 * @param[in] cmp_cb     Function called when procedure is over.
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapc_proc_cmp_cb callback execution
 ****************************************************************************************
 */
uint16_t gapc_le_encrypt(uint8_t conidx, uint32_t metainfo, const gapc_ltk_t* p_ltk_info, gapc_proc_cmp_cb cmp_cb);
#endif // (HL_LE_CENTRAL)

#if (HL_LE_PERIPHERAL)
/**
 ****************************************************************************************
 * @brief On LE connection, this function shall be used to accept or reject encryption request from peer device
 *
 * @note Once link is encrypted, #gapc_security_cb.auth_info is called
 *
 * @param[in] conidx     Connection index
 * @param[in] accept     True to accept, False to reject
 * @param[in] p_ltk      Pointer to LTK value
 * @param[in] key_size   Size of LTK key (range [7:16])
 *
 * @return Execution status (see #hl_err enumeration).
 ****************************************************************************************
 */
uint16_t gapc_le_encrypt_req_reply(uint8_t conidx, bool accept, const gap_sec_key_t* p_ltk, uint8_t key_size);
#endif // (HL_LE_PERIPHERAL)
#endif // (BLE_HOST_PRESENT)


#if (BLE_HOST_PRESENT)
#if (HL_LE_PERIPHERAL)
/**
 ****************************************************************************************
 * @brief On LE connection, peripheral side, ask central to secure BLE connection with a certain authentication level
 *
 * @param[in] conidx   Connection Index
 * @param[in] auth     Authentication (see #gap_auth enumeration)
 *
 * @return Execution status (see #hl_err enumeration).
 ****************************************************************************************
 */
uint16_t gapc_le_request_security(uint8_t conidx, uint8_t auth);
#endif // (HL_LE_PERIPHERAL)

#if (HL_LE_CENTRAL)
/**
 ****************************************************************************************
 * @brief On LE connection, this function is used to initiate a pairing
 *
 * @note Can be initiated only by Central of the connection.
 *
 * @param[in] conidx         Connection index
 * @param[in] p_pairing_info Pointer to pairing information
 * @param[in] rfu            Deprecated parameter reserved for future use.
 *                           To configure required security level, use #gapm_le_configure_security_level.
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapc_proc_cmp_cb callback execution
 ****************************************************************************************
 */
uint16_t gapc_le_bond(uint8_t conidx, const gapc_pairing_t* p_pairing_info, uint8_t rfu);
#endif // (HL_LE_CENTRAL)

#if (HL_LE_PERIPHERAL)
/**
 ****************************************************************************************
 * @brief On LE connection, this function shall be used to accept or reject pairing request from peer device
 *
 * @param[in] conidx         Connection index
 * @param[in] accept         True to accept, false to reject.
 * @param[in] p_pairing_info Pointer to local pairing requirement info
 * @param[in] rfu            Deprecated parameter reserved for future use
 *                           To configure required security level, use #gapm_le_configure_security_level.
 *
 * @return Execution status (see #hl_err enumeration).
 ****************************************************************************************
 */
uint16_t gapc_le_pairing_accept(uint8_t conidx, bool accept, const gapc_pairing_t* p_pairing_info,
                                uint8_t rfu);
#endif // (HL_LE_PERIPHERAL)

/**
 ****************************************************************************************
 * @brief On LE connection, this function shall be used to provide requested entered or displayed pin code
 *        or OOB Data for legacy pairing
 *
 * @param[in] conidx     Connection index
 * @param[in] accept     True to accept, false to reject.
 * @param[in] p_tk       Pointer to Temporary key.
 *
 * @return Execution status (see #hl_err enumeration).
 ****************************************************************************************
 */
uint16_t gapc_le_pairing_provide_tk(uint8_t conidx, bool accept, const gap_sec_key_t* p_tk);
#endif // (BLE_HOST_PRESENT)

/**
 ****************************************************************************************
 * @brief On LE connection, this function shall be used to provide requested OOB data - Secure connection
 *
 * @param[in] conidx     Connection index
 * @param[in] accept     True to accept, false to reject.
 * @param[in] p_data     Pointer to OOB data
 *
 * @return Execution status (see #hl_err enumeration).
 ****************************************************************************************
 */
uint16_t gapc_pairing_provide_oob_data(uint8_t conidx, bool accept, const gap_oob_t* p_data);

/**
 ****************************************************************************************
 * @brief On LE connection, this function shall be used to provide requested entered or displayed pass key
 *        - Secure connection
 *
 * @param[in] conidx     Connection index
 * @param[in] accept     True to accept, false to reject.
 * @param[in] passkey    Pass key value (range [0, 999999])
 *
 * @return Execution status (see #hl_err enumeration).
 ****************************************************************************************
 */
uint16_t gapc_pairing_provide_passkey(uint8_t conidx, bool accept, uint32_t passkey);

#if (BLE_HOST_PRESENT)
/**
 ****************************************************************************************
 * @brief On LE connection, this function shall be used to provide requested LTK - legacy pairing only
 *
 * @param[in] conidx     Connection index
 * @param[in] p_ltk      Pointer to LTK information. If NULL consider that application reject because negotiated
 *                       key size is insufficient
 *
 * @return Execution status (see #hl_err enumeration).
 ****************************************************************************************
 */
uint16_t gapc_le_pairing_provide_ltk(uint8_t conidx, const gapc_ltk_t* p_ltk);

/**
 ****************************************************************************************
 * @brief This function is recommended to be used to provide requested IRK
 *
 * @param[in] conidx     Connection index
 * @param[in] p_irk      Pointer to IRK information
 *
 * @return Execution status (see #hl_err enumeration).
 ****************************************************************************************
 */
uint16_t gapc_le_pairing_provide_irk(uint8_t conidx, const gap_sec_key_t* p_irk);

/**
 ****************************************************************************************
 * @brief This function could be used to provide requested IRK and overwrite device identity address.
 *
 * This function could be used to manage multiple identity for a single device.
 * On LE link which use RPA for connection establishment, another identity can be provided to peer device.
 * For a reconnection with same device, IRK exchanged must be used to connect using privacy (RPA).
 * The exchanged identity address cannot be present in non-connected activities (advertising, scan, initiating).
 *
 * @note Using this function is at risk since identity isn't managed by host stack
 *
 * @param[in] conidx     Connection index
 * @param[in] p_irk      Pointer to IRK information
 * @param[in] p_identity Device identity
 *
 * @return Execution status (see #hl_err enumeration).
 ****************************************************************************************
 */
uint16_t gapc_le_pairing_provide_irk_with_identity(uint8_t conidx, const gap_sec_key_t* p_irk,
                                                   const gap_bdaddr_t* p_identity);

/**
 ****************************************************************************************
 * @brief This function shall be used to provide requested CSRK.
 *
 * @param[in] conidx     Connection index
 * @param[in] p_csrk     Pointer to CSRK information
 *
 * @return Execution status (see #hl_err enumeration).
 ****************************************************************************************
 */
uint16_t gapc_pairing_provide_csrk(uint8_t conidx, const gap_sec_key_t* p_csrk);

/**
 ****************************************************************************************
 * @brief On LE connection, during a passkey entry pairing, this function informs peer device about user actions.
 *
 * @param[in] conidx            Connection index
 * @param[in] notification_type Key notification type (see #gapc_key_ntf_type enumeration)
 *
 * @return Execution status (see #hl_err enumeration).
 ****************************************************************************************
 */
uint16_t gapc_le_notify_key_pressed(uint8_t conidx, uint8_t notification_type);
#endif // (BLE_HOST_PRESENT)

/**
 ****************************************************************************************
 * @brief This function shall be used to accept or reject the requested numeric comparison
 *
 * @param[in] conidx     Connection index
 * @param[in] accept     True to accept, false to reject.
 *
 * @return Execution status (see #hl_err enumeration).
 ****************************************************************************************
 */
uint16_t gapc_pairing_numeric_compare_rsp(uint8_t conidx, bool accept);


/// @} GAPC_SEC_API

#endif /* GAPC_SEC_H_ */
