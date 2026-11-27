/**
 ****************************************************************************************
 *
 * @file gapm_le_per_sync.h
 *
 * @brief Generic Access Profile Manager - Low Energy Periodic Synchronization Activities
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef GAPM_LE_PER_SYNC_H_
#define GAPM_LE_PER_SYNC_H_

#include "rom_build_cfg.h"

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "gapm_le.h"

#if (HL_LE_OBSERVER && BLE_PER_ADV)

/**
 ****************************************************************************************
 * @addtogroup GAPM_LE_PER_SYNC_ACTV_API LE Periodic Synchronization
 *
 * @ingroup GAPM_ACTV_API
 *
 * @brief Create and control LE Periodic Synchronization activity to synchronize to a Periodic Advertiser
 *
 * Even if application can create several periodic synchronization activities, only one can be started at a time.
 * a new one can be initiated once periodic synchronization is established.
 *
 * The application must have a callback structure to handle activities events: #gapm_le_per_sync_cb_actv_t *
 * Two types of synchronization can be performed:
 * - Using a scan activity to search for periodic advertiser
 * - Using a periodic synchronization transfer with an existing LE connection
 *
 * Once activity is created, application can immediately start initiating.
 *
 * @note At least #GAP_ROLE_LE_OBSERVER role is required, for a synchronization transfer,  #GAP_ROLE_LE_PERIPHERAL or
 *       #GAP_ROLE_LE_CENTRAL is required
 *
 * @{
 ****************************************************************************************
 */

/*
 * DEFINES
 ****************************************************************************************
 */

/// Periodic synchronization types
enum gapm_le_per_sync_type
{
    /// Do not use periodic advertiser list for synchronization. Use advertiser information provided
    /// in the GAPM_ACTIVITY_START_CMD.
    GAPM_PER_SYNC_TYPE_GENERAL = 0u,
    /// Use periodic advertiser list for synchronization
    GAPM_PER_SYNC_TYPE_SELECTIVE,
    /// Use Periodic advertising sync transfer information send through connection for synchronization
    GAPM_PER_SYNC_TYPE_PAST,
};

/// Constant Tone Extension sync filtering type
enum gapm_le_per_sync_cte_type
{
    /// Do not sync to packets with an AoA Constant Tone Extension
    GAPM_CTE_NO_SYNC_WITH_AOA = (1u << 0),
    /// Do not sync to packets with an AoD Constant Tone Extension with 1 us slots
    GAPM_CTE_NO_SYNC_WITH_AOD_1US_SLOT = (1u << 1),
    /// Do not sync to packets with an AoD Constant Tone Extension with 2 us slots
    GAPM_CTE_NO_SYNC_WITH_AOD_2US_SLOT = (1u << 2),
    /// Do not sync to packets with a type 3 Constant Tone Extension (currently reserved for future use)
    GAPM_CTE_NO_SYNC_WITH_TYPE_3 = (1u << 3),
    /// Do not sync to packets without a Constant Tone Extension
    GAPM_CTE_NO_SYNC_WITHOUT_CTE = (1u << 4),
};

/// Supported AoA AoD switching sampling rate
enum gapm_le_per_sync_switch_sampling_rate
{
    /// 1 us switching supported for AoD transmission
    GAPM_AOD_TX_1_US = (1u << 0),
    /// 1 us sampling supported for AoD reception
    GAPM_AOD_RX_1_US = (1u << 1),
    /// 1 us switching and sampling supported for AoA reception
    GAPM_AOA_RX_1_US = (1u << 2),
};

/// Bit field of enabled advertising reports
enum gapm_le_per_sync_report_en_bf
{
    /// Periodic advertising reports reception enabled
    GAPM_REPORT_ADV_EN_BIT = 0x01u,
    GAPM_REPORT_ADV_EN_POS = 0u,
    /// BIG Info advertising reports reception enabled
    GAPM_REPORT_BIGINFO_EN_BIT = 0x02u,
    GAPM_REPORT_BIGINFO_EN_POS = 1u,
    /// Duplicate filtering enabled
    GAPM_REPORT_DUPLICATE_FILTER_EN_BIT = 0x04u,
    GAPM_REPORT_DUPLICATE_FILTER_EN_POS = 2u,
};

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// Periodic synchronization parameters
/*@TRACE*/
typedef struct
{
    /// Number of periodic advertising that can be skipped after a successful reception Maximum authorized
    /// value is 499
    uint16_t skip;
    /// Synchronization timeout for the periodic advertising (in unit of 10ms between 100ms and 163.84s)
    uint16_t sync_to;
    /// Periodic synchronization type (see #gapm_le_per_sync_type enumeration)
    uint8_t type;
    /// Connection index used for periodic sync info reception (only valid for GAPM_PER_SYNC_TYPE_PAST)
    uint8_t conidx;
    /// Address of advertiser with which synchronization has to be established (used only if use_pal is false)
    gapm_le_per_adv_bdaddr_t adv_addr;
    /// bit field that contains list of reports that are enabled or not (see #gapm_le_per_sync_report_en_bf enumeration)
    uint8_t report_en_bf;
    /// Type of Constant Tone Extension device should sync on (see #gapm_le_per_sync_cte_type enumeration).
    uint8_t cte_type;
} gapm_le_per_sync_param_t;

/// Periodic Sync information
typedef struct
{
    /// Advertiser address
    gap_bdaddr_t addr;
    /// PHY on which synchronization has been established (#gap_le_phy_val)
    uint8_t phy;
    /// Periodic advertising interval (in unit of 1.25ms, min is 7.5ms)
    uint16_t interval;
    /// Advertising SID
    uint8_t adv_sid;
    /// Advertiser clock accuracy (see #gapm_clock_accuracy enumeration)
    uint8_t clk_acc;
    /// Only valid for a Periodic Advertising Sync Transfer, else ignore
    uint16_t serv_data;
} gapm_le_per_sync_info_t;

#if (BLE_BIS)
/// BIG Info Report
/*@TRACE*/
typedef struct
{
    /// Value of the SDU interval in microseconds (Range 0x0000FF-0x0FFFFF)
    uint32_t sdu_interval;
    /// Value of the ISO Interval (1.25 ms unit)
    uint16_t iso_interval;
    /// Value of the maximum PDU size (Range 0x0000-0x00FB)
    uint16_t max_pdu;
    /// VValue of the maximum SDU size (Range 0x0000-0x0FFF)
    uint16_t max_sdu;
    /// Number of BIS present in the group (Range 0x01-0x1F)
    uint8_t num_bis;
    /// Number of subevents (Range 0x01-0x1F)
    uint8_t nse;
    /// Burst number (Range 0x01-0x07)
    uint8_t bn;
    /// Pre-transmit offset (Range 0x00-0x0F)
    uint8_t pto;
    /// Initial retransmission count (Range 0x01-0x0F)
    uint8_t irc;
    /// PHY used for transmission (0x01: 1M, 0x02: 2M, 0x03: Coded, All other values: RFU)
    uint8_t phy;
    /// Framing mode (0x00: Unframed, 0x01: Framed, All other values: RFU)
    uint8_t framing;
    /// True if broadcast isochronous group is encrypted, False otherwise
    bool encrypted;
} gapm_le_big_info_t;
#endif // (BLE_BIS)

/*
 * INTERFACES
 ****************************************************************************************
 */

/// Callback structure required to create a Periodic Sync activity
typedef struct
{
    /// Inherits Activity callback interface
    gapm_actv_cb_t actv;

    /**
     ****************************************************************************************
     * Callback executed when a valid advertising report is received
     *
     * @note Mandatory callback. Shall be set to a valid callback
     *
     * @param[in] metainfo      Metadata information provided by API user (see \glos{METAINFO})
     * @param[in] actv_idx      Activity Local identifier
     * @param[in] p_info        Pointer to advertising report information (also present in report metadata)
     * @param[in] p_report      Pointer to buffer that contains advertising data
     ****************************************************************************************
     */
    void (*report_received)(uint32_t metainfo, uint8_t actv_idx, const gapm_le_adv_report_info_t* p_info,
                            co_buf_t* p_report);

    /**
     ****************************************************************************************
     * Callback executed when periodic sync is established
     *
     * @note Mandatory callback. Shall be set to a valid callback
     *
     * @param[in] metainfo      Metadata information provided by API user (see \glos{METAINFO})
     * @param[in] actv_idx      Activity Local identifier
     * @param[in] p_info        Pointer to periodic sync information
     ****************************************************************************************
     */
    void (*established)(uint32_t metainfo, uint8_t actv_idx, const gapm_le_per_sync_info_t* p_info);

    #if (BLE_CONLESS_CTE_RX)
    /**
     ****************************************************************************************
     * Callback executed when an IQ report is received
     *
     * @note Optional callback.
     *
     * @param[in] metainfo      Metadata information provided by API user (see \glos{METAINFO})
     * @param[in] actv_idx      Activity Local identifier
     * @param[in] p_info        Pointer to IQ report information
     * @param[in] nb_sample     Number of IQ samples
     * @param[in] p_samples     Pointer to array of samples
     ****************************************************************************************
     */
    void (*iq_report_received)(uint32_t metainfo, uint8_t actv_idx, const gapm_le_iq_report_info_t* p_info,
                               uint8_t nb_sample, const gap_le_iq_sample_t* p_samples);
    #endif // (BLE_CONLESS_CTE_RX)

    #if (BLE_BIS)
    /**
     ****************************************************************************************
     * Callback executed when a BIG Info report is received
     *
     * @note Optional callback.
     *
     * @param[in] metainfo      Metadata information provided by API user (see \glos{METAINFO})
     * @param[in] actv_idx      Activity Local identifier
     * @param[in] p_report      Pointer to BIG Info report
     ****************************************************************************************
     */
    void (*big_info_report_received)(uint32_t metainfo, uint8_t actv_idx, const gapm_le_big_info_t* p_report);
    #endif // (BLE_BIS)
} gapm_le_per_sync_cb_actv_t;

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Create a periodic sync activity.
 *
 * @param[in]  metainfo         Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in]  p_cbs            Activity Callback interface
 * @param[out] p_actv_idx       Pointer used to return allocated activity index
 *
 * @return Execution status (see #hl_err enumeration).
 ****************************************************************************************
 */
uint16_t gapm_le_create_per_sync(uint32_t metainfo, const gapm_le_per_sync_cb_actv_t* p_cbs, uint8_t* p_actv_idx);

/**
 ****************************************************************************************
 * @brief Start periodic sync activity.
 *
 * @param[in] actv_idx          Activity local index
 * @param[in] p_param           Periodic Sync parameters
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapm_actv_cb_t.proc_cmp callback execution
 ****************************************************************************************
 */
uint16_t gapm_le_start_per_sync(uint8_t actv_idx, const gapm_le_per_sync_param_t* p_param);

/**
 ****************************************************************************************
 * @brief Control reports that can be received over a periodic sync activity
 *
 * @param[in] actv_idx          Activity local index
 * @param[in] report_en_bf      Bit field that contains list of reports that are enabled or not
 * (see #gapm_le_per_sync_report_en_bf enumeration)
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapm_actv_cb_t.proc_cmp callback execution
 ****************************************************************************************
 */
uint16_t gapm_le_control_per_sync_report(uint8_t actv_idx, uint8_t report_en_bf);

#if (BLE_CONLESS_CTE_RX)
/**
 ****************************************************************************************
 * @brief Control capturing IQ samples from the Constant Tone Extension of periodic advertising packets
 *
 * @param[in] actv_idx                Activity local index
 * @param[in] enable                  True to enable IQ sampling, false to disable
 * @param[in] slot_dur                Slot durations (1: 1us | 2: 2us)
 * @param[in] max_sampl_cte           Max sampled CTEs
 * @param[in] switching_pattern_len   Length of switching pattern
 * @param[in] p_antenna_id            Pointer to Antenna IDs
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapm_actv_cb_t.proc_cmp callback execution
 ****************************************************************************************
 */
uint16_t gapm_le_control_per_sync_iq_report(uint8_t actv_idx, uint8_t enable, uint8_t slot_dur,
                                            uint8_t max_sampl_cte, uint8_t switching_pattern_len,
                                            const uint8_t* p_antenna_id);
#endif // (BLE_CONLESS_CTE_RX)

/**
 ***************************************************************************************
 * @brief Get periodic synchronization handle
 *
 * @param[in] actv_idx   Activity index
 *
 * @return Required periodic synchronization handle
 ***************************************************************************************
 */
uint16_t gapm_le_get_per_sync_hdl(uint8_t actv_idx);
#endif // (HL_LE_OBSERVER && BLE_PER_ADV)
/// @} GAPM_LE_PER_SYNC_ACTV_API

#endif /* GAPM_LE_PER_SYNC_H_ */
