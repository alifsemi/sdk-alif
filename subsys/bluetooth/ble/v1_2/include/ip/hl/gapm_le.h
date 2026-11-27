
/**
 ****************************************************************************************
 *
 * @file gapm_le.h
 *
 * @brief Generic Access Profile Manager - Low Energy Activities
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef GAPM_LE_H_
#define GAPM_LE_H_

#include "rom_build_cfg.h"

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "gapm.h"
#include "gap_le.h"
#include "gapm_le_list.h"
#include "co_buf.h"
#include "co_bt_defines.h"

/**
 ****************************************************************************************
 * @addtogroup GAPM_LE_API Low Energy
 *
 * @ingroup GAPM_API
 *
 * Set of functions and interfaces required to create and manage Low Energy activities.
 *
 * @{
 * @}
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup GAPM_LE_SEC_API Security toolbox
 * @ingroup GAPM_LE_API
 * @brief OOB data, Random number generation and use ECDH P-256 toolbox
 *
 * @{
 * @}
 ****************************************************************************************
 */

/// @addtogroup GAPM_ENUM_API
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Appearance values
enum gapm_le_appearance
{
    /// Generic Unknown
    GAPM_LE_APPEARANCE_GENERIC_UNKNOWN = 0u,
};

/// Own BD address source of the device
enum gapm_le_own_addr
{
   /// Public or Private Static Address according to device address configuration
   GAPM_STATIC_ADDR = 0u,
   /// Generated resolvable private random address
   GAPM_GEN_RSLV_ADDR,
   /// Generated non-resolvable private random address
   GAPM_GEN_NON_RSLV_ADDR,
};

/// PHY Type
enum gapm_le_phy_type
{
    /// LE 1M
    GAPM_PHY_TYPE_LE_1M = 1u,
    /// LE 2M
    GAPM_PHY_TYPE_LE_2M = 2u,
    /// LE Coded
    GAPM_PHY_TYPE_LE_CODED = 3u,
    /// Number of PHY types
    GAPM_PHY_TYPE_NB = GAPM_PHY_TYPE_LE_CODED,
};

/// Advertising report type
enum gapm_le_adv_report_type
{
    /// Extended advertising report
    GAPM_REPORT_TYPE_ADV_EXT = 0u,
    /// Legacy advertising report
    GAPM_REPORT_TYPE_ADV_LEG,
    /// Extended scan response report
    GAPM_REPORT_TYPE_SCAN_RSP_EXT,
    /// Legacy scan response report
    GAPM_REPORT_TYPE_SCAN_RSP_LEG,
    /// Periodic advertising report
    GAPM_REPORT_TYPE_PER_ADV,
};

/// Advertising report information
enum gapm_le_adv_report_info_bf
{
    /// Report Type
    GAPM_REPORT_INFO_REPORT_TYPE_MASK = 0x07u,
    /// Report is complete
    GAPM_REPORT_INFO_COMPLETE_BIT = (1u << 3),
    /// Connectable advertising
    GAPM_REPORT_INFO_CONN_ADV_BIT = (1u << 4),
    /// Scannable advertising
    GAPM_REPORT_INFO_SCAN_ADV_BIT = (1u << 5),
    /// Directed advertising
    GAPM_REPORT_INFO_DIR_ADV_BIT = (1u << 6),
};

/// @} GAPM_ENUM_API

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// @addtogroup GAPM_STRUCT_API
/// @{

/// Connection parameters
/*@TRACE*/
typedef struct
{
    /// Minimum value for the connection interval (in unit of 1.25ms). Shall be less than or equal to
    /// conn_intv_max value. Allowed range is 7.5ms to 4s.
    uint16_t conn_intv_min;
    /// Maximum value for the connection interval (in unit of 1.25ms). Shall be greater than or equal to
    /// conn_intv_min value. Allowed range is 7.5ms to 4s.
    uint16_t conn_intv_max;
    /// Slave latency. Number of events that can be missed by a connected slave device
    uint16_t conn_latency;
    /// Link supervision timeout (in unit of 10ms). Allowed range is 100ms to 32s
    uint16_t supervision_to;
    /// Recommended minimum duration of connection events (in unit of 625us)
    uint16_t ce_len_min;
    /// Recommended maximum duration of connection events (in unit of 625us)
    uint16_t ce_len_max;
} gapm_le_init_con_param_t;

/// Information about received ADV report
typedef struct
{
    /// Bit field providing information about the received report (see #gapm_le_adv_report_info_bf enumeration)
    uint8_t info;
    /// Transmitter device address
    gap_bdaddr_t trans_addr;
    /// Target address (in case of a directed advertising report)
    gap_bdaddr_t target_addr;
    /// TX power (in dBm)
    int8_t tx_pwr;
    /// RSSI (between -127 and +20 dBm)
    int8_t rssi;
    /// Primary PHY on which advertising report has been received
    uint8_t phy_prim;
    /// Secondary PHY on which advertising report has been received
    uint8_t phy_second;
    /// Advertising SID
    /// Valid only for periodic advertising report
    uint8_t adv_sid;
    /// Periodic advertising interval (in unit of 1.25ms, min is 7.5ms)
    /// Valid only for periodic advertising report
    uint16_t period_adv_intv;
} gapm_le_adv_report_info_t;

/// Scan Window operation parameters
/*@TRACE*/
typedef struct
{
    /// Scan interval (N * 0.625 ms)
    uint16_t scan_intv;
    /// Scan window (N * 0.625 ms)
    uint16_t scan_wd;
} gapm_le_scan_wd_op_param_t;

/// information about IQ report
typedef struct
{
    /// Data channel index
    uint8_t channel_idx;
    /// RSSI (in 0.1 dBm)
    int16_t rssi;
    /// RSSI antenna ID
    uint8_t rssi_antenna_id;
    /// CTE type (0: AOA | 1: AOD-1us | 2: AOD-2us) (see #gap_le_cte_type enumeration)
    uint8_t cte_type;
    /// Slot durations (1: 1us | 2: 2us)
    uint8_t slot_dur;
    /// Packet status
    uint8_t pkt_status;
    /// Periodic Adv Event Counter
    uint16_t pa_evt_cnt;
} gapm_le_iq_report_info_t;

/// Controller antenna information
typedef struct
{
    /// Supported switching sampling rates bit field (see #gapm_le_per_sync_switch_sampling_rate enumeration)
    uint8_t supp_switching_sampl_rates;
    /// Number of antennae
    uint8_t antennae_num;
    /// Max length of switching pattern (number of antenna IDs in the pattern)
    uint8_t max_switching_pattern_len;
    /// Max CTE length
    uint8_t max_cte_len;
} gapm_le_antenna_info_t;

/// Suggested default data length
typedef struct
{
    ///Host's suggested value for the Controller's maximum transmitted number of payload octets
    uint16_t suggted_max_tx_octets;
    ///Host's suggested value for the Controller's maximum packet transmission time
    uint16_t suggted_max_tx_time;
} gapm_le_suggested_default_data_len_t;

/// Maximum LE data length
typedef struct
{
    ///Maximum number of payload octets that the local Controller supports for transmission
    uint16_t suppted_max_tx_octets;
    ///Maximum time, in microseconds, that the local Controller supports for transmission
    uint16_t suppted_max_tx_time;
    ///Maximum number of payload octets that the local Controller supports for reception
    uint16_t suppted_max_rx_octets;
    ///Maximum time, in microseconds, that the local Controller supports for reception
    uint16_t suppted_max_rx_time;
} gapm_le_max_data_len_t;

/// Transmit powers range supported by the controller
typedef struct
{
    /// Minimum TX power
    int8_t min_tx_pwr;
    /// Maximum TX power
    int8_t max_tx_pwr;
} gapm_le_tx_power_range_t;

/// TX/RX RF path compensation values
typedef struct
{
    /// RF TX path compensation
    int16_t tx;
    /// RF RX path compensation
    int16_t rx;
} gapm_le_rf_path_compensation_t;

/// @} GAPM_STRUCT_API

/*
 * INTERFACES
 ****************************************************************************************
 */

/// @addtogroup GAPM_ACTV_ITF_API
/// @{

///  Callback structure required to create a \glos{LE}  activity
typedef struct
{
    /// Inherits Activity callback interface
    gapm_actv_cb_t actv;

    /**
     ****************************************************************************************
     * @brief Callback executed when Random (resolvable or non-resolvable) address has been updated by the Host.\n
     * If controller privacy is used, the address may be different from address used over the air.
     *
     * @note Optional callback. Set it to NULL to ignore event reception
     *
     * @param[in] metainfo      Metadata information provided by API user (see \glos{METAINFO})
     * @param[in] actv_idx      Activity local index
     * @param[in] p_addr        Pointer to the updated Private Address (resolvable or non-resolvable)
     ****************************************************************************************
     */
    void (*addr_updated)(uint32_t metainfo, uint8_t actv_idx, const gap_addr_t* p_addr);
} gapm_le_cb_actv_t;

/// @} GAPM_ACTV_ITF_API

/// @addtogroup GAPM_INFO_API
/// @{

/**
 ***************************************************************************************
 * @brief Function executed when procedure execution is over.
 *
 * @param[in] metainfo      Metadata information provided by API user (see \glos{METAINFO})
 * @param[in] status        Procedure execution status  (see #hl_err enumeration)
 * @param[in] p_info        Pointer to controller antenna informations
 *                          (NULL if status != GAP_ERR_NO_ERROR)
 ***************************************************************************************
 */
typedef void (*gapm_le_antenna_info_cb)(uint32_t metainfo, uint16_t status, const gapm_le_antenna_info_t *p_info);

/**
 ***************************************************************************************
 * @brief Function executed when procedure execution is over.
 *
 * @param[in] metainfo      Metadata information provided by API user (see \glos{METAINFO})
 * @param[in] status        Procedure execution status  (see #hl_err enumeration)
 * @param[in] p_info        Pointer to controller default data length
 *                          (NULL if status != GAP_ERR_NO_ERROR)
 ***************************************************************************************
 */
typedef void (*gapm_le_suggested_default_data_len_cb)(uint32_t metainfo, uint16_t status,
                                                      const gapm_le_suggested_default_data_len_t *p_info);

/**
 ***************************************************************************************
 * @brief Function executed when procedure execution is over.
 *
 * @param[in] metainfo      Metadata information provided by API user (see \glos{METAINFO})
 * @param[in] status        Procedure execution status  (see #hl_err enumeration)
 * @param[in] p_info        Pointer to controller maximum LE data length information
 *                          (NULL if status != GAP_ERR_NO_ERROR)
 ***************************************************************************************
 */
typedef void (*gapm_le_max_data_len_cb)(uint32_t metainfo, uint16_t status,
                                        const gapm_le_max_data_len_t *p_info);

/**
 ***************************************************************************************
 * @brief Function executed when procedure execution is over.
 *
 * @param[in] metainfo      Metadata information provided by API user (see \glos{METAINFO})
 * @param[in] status        Procedure execution status  (see #hl_err enumeration)
 * @param[in] p_rng         Pointer to TX Power Range value
 *                          (NULL if status != GAP_ERR_NO_ERROR)
 ***************************************************************************************
 */
typedef void (*gapm_le_tx_power_range_cb)(uint32_t metainfo, uint16_t status, const gapm_le_tx_power_range_t* p_rng);

/**
 ***************************************************************************************
 * @brief Function executed when procedure execution is over.
 *
 * @param[in] metainfo      Metadata information provided by API user (see \glos{METAINFO})
 * @param[in] status        Procedure execution status  (see #hl_err enumeration)
 * @param[in] power_lvl     Advertising channel Tx power level
 ***************************************************************************************
 */
typedef void (*gapm_le_adv_cb_tx_power)(uint32_t metainfo, uint16_t status, int8_t power_lvl);

/**
 ***************************************************************************************
 * @brief Function executed when procedure execution is over.
 *
 * @param[in] metainfo        Metadata information provided by API user (see \glos{METAINFO})
 * @param[in] status          Procedure execution status  (see #hl_err enumeration)
 * @param[in] p_rf_path_comp  Pointer to RF Path compensation information
 *                            (NULL if status != GAP_ERR_NO_ERROR)
 ***************************************************************************************
 */
typedef void (*gapm_le_rf_path_compensation_cb)(uint32_t metainfo, uint16_t status,
                                                const gapm_le_rf_path_compensation_t* p_rf_path_comp);

/// @} GAPM_INFO_API

/// @addtogroup GAPM_LE_SEC_API
/// @{

/**
 ***************************************************************************************
 * @brief Function executed when procedure execution is over.
 *
 * @param[in] metainfo        Metadata information provided by API user (see \glos{METAINFO})
 * @param[in] status          Procedure execution status  (see #hl_err enumeration)
 * @param[in] p_pub_key       Pointer to ECDH Public key
 *                            (NULL if status != GAP_ERR_NO_ERROR)
 ***************************************************************************************
 */
typedef void (*gapm_le_public_key_cb)(uint32_t metainfo, uint16_t status, const gap_le_public_key_t* p_pub_key);

/**
 ***************************************************************************************
 * @brief Function executed when procedure execution is over.
 *
 * @param[in] metainfo        Metadata information provided by API user (see \glos{METAINFO})
 * @param[in] status          Procedure execution status  (see #hl_err enumeration)
 * @param[in] p_dh_key        Pointer to computed DH-Key
 *                            (NULL if status != GAP_ERR_NO_ERROR)
 ***************************************************************************************
 */
typedef void (*gapm_le_dh_key_cb)(uint32_t metainfo, uint16_t status, const gap_le_dh_key_t* p_dh_key);

/**
 ***************************************************************************************
 * @brief Function executed when procedure execution is over.
 *
 * @param[in] metainfo        Metadata information provided by API user (see \glos{METAINFO})
 * @param[in] status          Procedure execution status  (see #hl_err enumeration)
 * @param[in] p_data          Pointer to generated LE OOB data
 *                            (NULL if status != GAP_ERR_NO_ERROR)
 ***************************************************************************************
 */
typedef void (*gapm_le_oob_cb)(uint32_t metainfo, uint16_t status, const gap_oob_t* p_data);

/**
 ***************************************************************************************
 * @brief Function executed when procedure execution is over.
 *
 * @param[in] status          Procedure execution status  (see #hl_err enumeration)
 * @param[in] p_addr          Pointer to generated random address
 *                            (NULL if status != GAP_ERR_NO_ERROR)
 ***************************************************************************************
 */
typedef void (*gapm_le_random_addr_cb)(uint16_t status, const gap_addr_t* p_addr);

/**
 ***************************************************************************************
 * @brief Function executed when AES procedure execution is over.
 *
 * @param[in] status          Procedure execution status  (see #hl_err enumeration)
 * @param[in] p_cipher        Pointer to ciphered data
 *                            (NULL if status != GAP_ERR_NO_ERROR)
 ***************************************************************************************
 */
typedef void (*gapm_le_encryption_result_cb)(uint16_t status, const gap_le_aes_result_t* p_cipher);

/**
 ***************************************************************************************
 * @brief Function executed when random number procedure is over.
 *
 * @param[in] status          Procedure execution status  (see #hl_err enumeration)
 * @param[in] p_rand          Pointer to structure that contains 128-bit random number
 *                            (NULL if status != GAP_ERR_NO_ERROR)
 ***************************************************************************************
 */
typedef void (*gapm_le_random_nb_cb)(uint16_t status, const gap_le_aes_random_nb_t* p_rand);

/**
 ***************************************************************************************
 * @brief Function executed when address resolution is over.
 *
 * @param[in] status          Procedure execution status  (see #hl_err enumeration)
 * @param[in] p_addr          Pointer to resolvable private address
 * @param[in] p_irk           Pointer to IRK that correspond to RPA resolution
 *                            (NULL if status != GAP_ERR_NO_ERROR)
 ***************************************************************************************
 */
typedef void (*gapm_le_addr_resolved_ind_cb)(uint16_t status, const gap_addr_t* p_addr, const gap_sec_key_t* p_irk);

/// @} GAPM_LE_SEC_API

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/// @addtogroup GAPM_CONFIG_API
/// @{

#if (BLE_HOST_PRESENT)
/**
 ***************************************************************************************
 * @brief Set device channel map
 *
 * Application should wait #gapm_proc_cmp_cb callback execution before starting a new procedure
 *
 * @param[in] metainfo  Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] p_ch_map   Pointer to new channel map.
 * @param[in] cmp_cb    Function called when procedure is over.
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapm_proc_cmp_cb callback execution
 ***************************************************************************************
 */
uint16_t gapm_le_set_channel_map(uint32_t metainfo, const struct le_ch_map* p_ch_map, gapm_proc_cmp_cb cmp_cb);

#if (BLE_CH_SCAN_SUPPORT)
/**
 ***************************************************************************************
 * @brief Start Channel Scan activity
 *
 * Application should wait #gapm_proc_cmp_cb callback execution before starting a new procedure
 *
 * @param[in] metainfo              Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] scan_win_duration     Window duration of one channel in us
 * @param[in] scan_duration         Channel Scan event in us
 * @param[in] intv                  Channel Scan interval in Time = N*1.25ms
 * @param[in] p_ch_map              Pointer to channel map
 * @param[in] cmp_cb                Function called when procedure is over.
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapm_proc_cmp_cb callback execution
 ***************************************************************************************
 */
uint16_t gapm_le_start_ch_scan(uint32_t metainfo, uint32_t scan_win_duration, const uint32_t scan_duration,
                                       const uint16_t intv, struct le_ch_map * p_ch_map, gapm_proc_cmp_cb cmp_cb);
/**
 ***************************************************************************************
 * @brief Stop Channel Scan activity
 *
 * Application should wait #gapm_proc_cmp_cb callback execution before starting a new procedure
 *
 * @param[in] metainfo              Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] cmp_cb    Function called when procedure is over.
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapm_proc_cmp_cb callback execution
 ***************************************************************************************
 */
uint16_t gapm_le_stop_ch_scan(uint32_t metainfo, gapm_proc_cmp_cb cmp_cb);
#endif // (BLE_CH_SCAN_SUPPORT)
#endif // (BLE_HOST_PRESENT)

/**
 ***************************************************************************************
 * @brief Set device IRK used for resolvable random BD address generation
 *
 * @param[in] p_irk     Pointer to device irk (LSB first).
 *
 * @return Execution status (see #hl_err enumeration).
 ***************************************************************************************
 */
uint16_t gapm_le_set_irk(const gap_sec_key_t* p_irk);

#if (BLE_SUBRATING)
/**
 ***************************************************************************************
 * @brief Set the initial values for the acceptable parameters for subrating requests for all future ACL connections
 * where the Controller is the Central.
 *
 * @param[in] metainfo          Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] p_subrate_req     Pointer to subrating requirements
 * @param[in] cmp_cb            Function called when procedure is over
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapm_proc_cmp_cb callback execution
 ***************************************************************************************
 */
uint16_t gapm_le_set_default_subrate(uint32_t metainfo, const gap_le_subrate_req_t* p_subrate_req,
                                     gapm_proc_cmp_cb cmp_cb);
#endif // (BLE_SUBRATING)

/**
 ***************************************************************************************
 * @brief Set Appearance
 * Once set to a value different than 0, application is no more requested to provide appearance value when Appearance
 * characteristic is read by a peer device\n
 * Set value can be retrieved using #gapm_le_get_appearance function
 *
 * @param[in] appearance     Appearance value
 *
 * @return Execution status (see #hl_err enumeration).
 ***************************************************************************************
 */
uint16_t gapm_le_set_appearance(uint16_t appearance);

/**
 ***************************************************************************************
 * @return Appearance value
 ***************************************************************************************
 */
uint16_t gapm_le_get_appearance(void);

/// @} GAPM_CONFIG_API

/// @addtogroup GAPM_INFO_API
/// @{

#if (BLE_AOA || BLE_AOD)
/**
 ***************************************************************************************
 * @brief Get controller antenna information. Information returned in res_cb function
 *
 * @param[in] metainfo  Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] res_cb    Function called when procedure is over
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapm_le_antenna_info_cb callback execution
 ***************************************************************************************
 */
uint16_t gapm_le_get_antenna_info(uint32_t metainfo, gapm_le_antenna_info_cb res_cb);
#endif // (BLE_AOA || BLE_AOD)

/**
 ***************************************************************************************
 * @brief Get information about suggested default data length. Information returned in res_cb function
 *
 * @param[in] metainfo  Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] res_cb    Function called when procedure is over
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapm_le_suggested_default_data_len_cb callback execution
 ***************************************************************************************
 */
uint16_t gapm_le_get_suggested_default_data_len(uint32_t metainfo, gapm_le_suggested_default_data_len_cb res_cb);

/**
 ***************************************************************************************
 * @brief Get information about maximum LE data length. Information returned in res_cb function
 *
 * @param[in] metainfo  Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] res_cb    Function called when procedure is over
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapm_le_max_data_len_cb callback execution
 ***************************************************************************************
 */
uint16_t gapm_le_get_max_data_len(uint32_t metainfo, gapm_le_max_data_len_cb res_cb);

/**
 ***************************************************************************************
 * @brief Get TX Power range value. Information returned in res_cb function
 *
 * @param[in] metainfo  Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] res_cb    Function called when procedure is over
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapm_le_tx_power_range_cb callback execution
 ***************************************************************************************
 */
uint16_t gapm_le_get_tx_power(uint32_t metainfo, gapm_le_tx_power_range_cb res_cb);


/**
 ***************************************************************************************
 * @brief Get RF Path compensation values. Information returned in res_cb function
 *
 * @param[in] metainfo  Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] res_cb    Function called when procedure is over
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapm_le_rf_path_compensation_cb callback execution
 ***************************************************************************************
 */
uint16_t gapm_le_get_rx_path_compensation(uint32_t metainfo, gapm_le_rf_path_compensation_cb res_cb);

/// @} GAPM_INFO_API

/// @addtogroup GAPM_LE_SEC_API
/// @{

/**
 ***************************************************************************************
 * @brief Generate a random number.
 *
 * @param[in] res_cb    Function called when procedure is over
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapm_le_random_addr_cb callback execution
 ***************************************************************************************
 */
uint16_t gapm_le_generate_random_nb(gapm_le_random_nb_cb res_cb);

/**
 ***************************************************************************************
 * @brief Cipher 128-bit data using AES
 *
 * @param[in] p_key     Pointer to 128-bit key used for ciphering
 * @param[in] p_data    Pointer to 128-bit data to cipher
 * @param[in] res_cb    Function called when procedure is over
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapm_le_random_nb_cb callback execution
 ***************************************************************************************
 */
uint16_t gapm_le_aes_cipher(const uint8_t* p_key, const uint8_t* p_data, gapm_le_encryption_result_cb res_cb);

/**
 ***************************************************************************************
 * @brief De-Cipher 128-bit data using AES - Shall be supported by HW
 *
 * @param[in] p_key     Pointer to 128-bit key used for deciphering
 * @param[in] p_data    Pointer to 128-bit data to decipher
 * @param[in] res_cb    Function called when procedure is over
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapm_le_random_nb_cb callback execution
 ***************************************************************************************
 */
uint16_t gapm_le_aes_decipher(const uint8_t* p_key, const uint8_t* p_data, gapm_le_encryption_result_cb res_cb);

/**
 ***************************************************************************************
 * @brief Generate a random address.
 *
 * @param[in] rnd_type  Random address type see #gap_le_random_addr_type enumeration
 * @param[in] res_cb    Function called when procedure is over
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapm_le_random_addr_cb callback execution
 ***************************************************************************************
 */
uint16_t gapm_le_generate_random_addr(uint8_t rnd_type, gapm_le_random_addr_cb res_cb);

/**
 ***************************************************************************************
 * @brief Resolve a resolvable private address
 *
 * @param[in] p_addr    Pointer to random private address
 * @param[in] nb_irk    Number of IRKs
 * @param[in] p_irk     Pointer to array that contain IRK(s)
 * @param[in] res_cb    Function called when procedure is over
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapm_le_addr_resolved_ind_cb callback execution
 ***************************************************************************************
 */
uint16_t gapm_le_resolve_address(const gap_addr_t* p_addr, uint8_t nb_irk, const gap_sec_key_t* p_irk,
                                 gapm_le_addr_resolved_ind_cb res_cb);

/**
 ***************************************************************************************
 * @brief Get ECDH public key value (New key pair generated each time this function is called)
 *
 * @param[in] metainfo  Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] res_cb    Function called when procedure is over
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapm_le_public_key_cb callback execution
 ***************************************************************************************
 */
uint16_t gapm_le_get_public_key(uint32_t metainfo, gapm_le_public_key_cb res_cb);

/**
 ***************************************************************************************
 * @brief Comnpute DH-Key using own ECDH private key and a given public key
 *
 * @param[in] metainfo  Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] p_pub_key Public key from a peer device
 * @param[in] res_cb    Function called when procedure is over
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapm_le_dh_key_cb callback execution
 ***************************************************************************************
 */
uint16_t gapm_le_compute_dh_key(uint32_t metainfo, const gap_le_public_key_t* p_pub_key, gapm_le_dh_key_cb res_cb);

/**
 ***************************************************************************************
 * @brief Generate LE OOB data using ECDH (New ECDH key pair generated each time this function is called)
 *        OOB data shall be used onto following pairing onto a LE connection
 *
 *        OOB data must be conveyed to peer device through an out of band method.
 *
 * @param[in] metainfo  Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] res_cb    Function called when procedure is over
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapm_le_oob_cb callback execution
 ***************************************************************************************
 */
uint16_t gapm_le_generate_oob_data(uint32_t metainfo, gapm_le_oob_cb res_cb);

/// @} GAPM_LE_SEC_API

#endif /* GAPM_LE_H_ */
