/**
 ****************************************************************************************
 *
 * @file gapm_le_test.h
 *
 * @brief Generic Access Profile Manager - Low Energy Test Mode Activities
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef GAPM_LE_TEST_H_
#define GAPM_LE_TEST_H_

#include "rom_build_cfg.h"

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "gapm_le.h"

/**
 ****************************************************************************************
 * @addtogroup GAPM_LE_TEST_ACTV_API LE Test Mode
 *
 * @ingroup GAPM_ACTV_API
 *
 * @brief Create and control LE TX or RX Test Mode activity
 *
 * Even if application can create several test mode activities, only one can be started at a time.
 *
 * @{
 ****************************************************************************************
 */

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// Debug I/Q sample control
/*@TRACE*/
typedef struct
{
    /// I-sample control
    ///  - 0: up-count from 0x01
    ///  - 1: down-count from 0xFF
    ///  - 2: Fixed value 0x00
    ///  - 3: PRBS pattern
    ///  - 4..0xFF: Fixed value
    uint8_t i;
    /// Q-sample control
    ///  - 0: up-count from 0x01
    ///  - 1: down-count from 0xFF
    ///  - 2: Fixed value 0x00
    ///  - 3: PRBS pattern
    ///  - 4..0xFF: Fixed value
    uint8_t q;
} gapm_le_dbg_iq_ctrl_t;

/// TX Test Mode parameters
typedef struct
{
    /// Tx Channel (Range 0x00 to 0x27)
    uint8_t channel;
    /// Length in bytes of payload data in each packet (orange 0x00-0xFF)
    uint8_t tx_data_length;
    /// Packet Payload type (see #gap_le_packet_payload_type enumeration)
    uint8_t tx_pkt_payload;
    /// Test PHY rate (see #gap_le_phy_val enumeration)
    uint8_t phy;
    /// Transmit power level in dBm (0x7E: minimum | 0x7F: maximum | range: -127 to +20)
    int8_t tx_pwr_lvl;
} gapm_le_test_tx_param_t;

/// RX Test Mode parameters
typedef struct
{
    /// Rx Channel (Range 0x00 to 0x27)
    uint8_t channel;
    /// Test PHY rate (see #gap_le_phy_val enumeration)
    uint8_t phy;
    /// Modulation Index (see #gap_le_modulation_idx enumeration)
    uint8_t modulation_idx;
    /// Slot durations
    uint8_t slot_dur;
} gapm_le_test_rx_param_t;

/// Test Mode CTE parameters
typedef struct
{
    /// CTE length (in 8us unit) (Expected for RX Mode)
    uint8_t cte_len;
    /// CTE type (0: AOA | 1: AOD-1us | 2: AOD-2us) (Expected for TX Mode)
    uint8_t cte_type;
    /// Length of switching pattern (number of antenna IDs in the pattern)
    uint8_t switching_pattern_len;
} gapm_le_test_cte_param_t;

/*
 * INTERFACES
 ****************************************************************************************
 */

/// Callback structure required to create a RX test activity
typedef struct
{
    /// Inherits Activity callback interface
    gapm_actv_cb_t actv;

    /**
     ****************************************************************************************
     * Callback executed when RX Test mode is stopped to provide number of packet received
     *
     * @note Optional callback.
     *
     * @param[in] metainfo      Metadata information provided by API user (see \glos{METAINFO})
     * @param[in] actv_idx      Activity Local identifier
     * @param[in] nb_packet     Number of packet received during test mode
     ****************************************************************************************
     */
    void (*nb_packet_received)(uint32_t metainfo, uint8_t actv_idx, uint16_t nb_packet);

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
} gapm_le_test_rx_cb_actv_t;

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

#if(BLE_HOST_PRESENT && HOST_TEST_MODE)
/**
 ****************************************************************************************
 * @brief Create a RX test mode  activity.
 *
 * @param[in]  metainfo         Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in]  p_cbs            Activity Callback interface
 * @param[out] p_actv_idx       Pointer used to return allocated activity index
 *
 * @return Execution status (see #hl_err enumeration).
 ****************************************************************************************
 */
uint16_t gapm_le_create_test_rx(uint32_t metainfo, const gapm_le_test_rx_cb_actv_t* p_cbs, uint8_t* p_actv_idx);

/**
 ****************************************************************************************
 * @brief Start RX test activity.
 *
 * @param[in] actv_idx          Activity local index
 * @param[in] p_param           RX Test mode parameters
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapm_actv_cb_t.proc_cmp callback execution
 ****************************************************************************************
 */
uint16_t gapm_le_start_test_rx(uint8_t actv_idx, const gapm_le_test_rx_param_t* p_param);

/**
 ****************************************************************************************
 * @brief Start RX test activity.
 *
 * @param[in] actv_idx          Activity local index
 * @param[in] p_param           RX Test mode parameters
 * @param[in] p_cte_param       Pointer to CTE parameters
 * @param[in] p_antenna_id      Pointer to antenna id array (size p_cte_param.switching_pattern_len)
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapm_actv_cb_t.proc_cmp callback execution
 ****************************************************************************************
 */
uint16_t gapm_le_start_test_rx_with_cte(uint8_t actv_idx, const gapm_le_test_rx_param_t* p_param,
                                        const gapm_le_test_cte_param_t* p_cte_param, const uint8_t* p_antenna_id);
/**
 ****************************************************************************************
 * @brief Create a TX test mode  activity.
 *
 * @param[in]  metainfo         Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in]  p_cbs            Activity Callback interface
 * @param[out] p_actv_idx       Pointer used to return allocated activity index
 *
 * @return Execution status (see #hl_err enumeration).
 ****************************************************************************************
 */
uint16_t gapm_le_create_test_tx(uint32_t metainfo, const gapm_actv_cb_t* p_cbs, uint8_t* p_actv_idx);

/**
 ****************************************************************************************
 * @brief Start TX test activity.
 *
 * @param[in] actv_idx          Activity local index
 * @param[in] p_param           TX Test mode parameters
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapm_actv_cb_t.proc_cmp callback execution
 ****************************************************************************************
 */
uint16_t gapm_le_start_test_tx(uint8_t actv_idx, const gapm_le_test_tx_param_t* p_param);

/**
 ****************************************************************************************
 * @brief Start TX test activity.
 *
 * @param[in] actv_idx          Activity local index
 * @param[in] p_param           TX Test mode parameters
 * @param[in] p_cte_param       Pointer to CTE parameters
 * @param[in] p_antenna_id      Pointer to antenna id array (size p_cte_param.switching_pattern_len)
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapm_actv_cb_t.proc_cmp callback execution
 ****************************************************************************************
 */
uint16_t gapm_le_start_test_tx_with_cte(uint8_t actv_idx, const gapm_le_test_tx_param_t* p_param,
                                        const gapm_le_test_cte_param_t* p_cte_param, const uint8_t* p_antenna_id);
#endif // (BLE_HOST_PRESENT && HOST_TEST_MODE)

#if (BLE_HOST_PRESENT && BLE_HOST_IQ_GEN)
/**
 ***************************************************************************************
 * @brief Configure the Debug Platform I&Q Sampling generator
 *
 * Application should wait #gapm_proc_cmp_cb callback execution before starting a new procedure
 *
 * @param[in] metainfo   Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] mode       Antenna switch/sample control
 *                        - bit[0]: 0: up-sweep; 1: up-down sweep (internal switching mode)
 *                        - bit[1]: 0: 1us intervals; 1: 2us intervals (internal switching mode)
 *                        - bit[2]: 0: internal switching mode; 1: baseband switching mode
 * @param[in] nb_antenna Number of antenna patterns
 * @param[in] p_iq_ctrl  Pointer to I/Q sample control array.
 * @param[in] cmp_cb     Function called when procedure is over.
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapm_proc_cmp_cb callback execution
 ***************************************************************************************
 */
uint16_t gapm_le_dbg_configure_iqgen(uint32_t metainfo, uint8_t mode, uint8_t nb_antenna,
                                     const gapm_le_dbg_iq_ctrl_t* p_iq_ctrl, gapm_proc_cmp_cb cmp_cb);
#endif // (BLE_HOST_PRESENT && BLE_HOST_IQ_GEN)

/// @} GAPM_LE_TEST_ACTV_API

#endif /* GAPM_LE_TEST_H_ */
