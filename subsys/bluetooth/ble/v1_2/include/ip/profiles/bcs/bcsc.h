/**
 ****************************************************************************************
 *
 * @file bcsc_msg.h
 *
 * @brief Header file - Body Composition Service Collector/Client Role - Native API
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 * $ Rev $
 *
 ****************************************************************************************
 */

#ifndef _BCSC_H_
#define _BCSC_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @addtogroup BCSC
 * @ingroup Profile
 * @brief  Body Composition Service Collector/Client Role - Message API
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "bcsc_msg.h"


/*
 * DEFINES
 ****************************************************************************************
 */


/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/*
 * NATIVE API CALLBACKS
 ****************************************************************************************
 */

/// Body composition Service client callback set
typedef struct bcsc_cb
{
    /**
     ****************************************************************************************
     * @brief Completion of enable procedure
     *
     * @param[in] conidx        Connection index
     * @param[in] status        Status of the procedure execution (see enum #hl_err)
     * @param[in] p_bas         Pointer to peer database description bond data
     ****************************************************************************************
     */
    void (*cb_enable_cmp)(uint8_t conidx, uint16_t status, const bcsc_bcs_content_t* p_bcs);

    /**
     ****************************************************************************************
     * @brief Inform that read the body composition feature procedure is over
     *
     * @param[in] conidx        Connection index
     * @param[in] status        Status of the procedure execution (see enum #hl_err)
     * @param[in] feature       Body composition feature
     ****************************************************************************************
     */
    void (*cb_read_feature_cmp)(uint8_t conidx, uint16_t status, uint32_t feature);

    /**
     ****************************************************************************************
     * @brief Inform that read indication configuration procedure is over
     *
     * @param[in] conidx        Connection index
     * @param[in] status        Status of the procedure execution (see enum #hl_err)
     * @param[in] bas_instance  Battery Service Instance - From 0 to BASC_NB_BAS_INSTANCES_MAX-1
     * @param[in] ind_cfg       Indication configuration
     ****************************************************************************************
     */
    void (*cb_read_ind_cfg_cmp)(uint8_t conidx, uint16_t status, uint16_t ind_cfg);

    /**
     ****************************************************************************************
     * @brief Inform that indication configuration write procedure is over
     *
     * @param[in] conidx        Connection index
     * @param[in] status        Status of the procedure execution (see enum #hl_err)
     ****************************************************************************************
     */
    void (*cb_write_ind_cfg_cmp)(uint8_t conidx, uint16_t status);

    /**
     ****************************************************************************************
     * @brief Function called when new body configuration measurement information is received
     *
     * @param[in] conidx        Connection index
     * @param[in] p_meas        Pointer to body configuration measurement information
     * @param[in] meas_u        Measurement Units
     * @param[in] mass_resol    Mass Resolution
     * @param[in] hght_resol    Height Resolution
     ****************************************************************************************
     */
    void (*cb_meas)(uint8_t conidx, const bcs_meas_t* p_meas, uint8_t meas_u, uint8_t mass_resol, uint8_t hght_resol);
} bcsc_cb_t;

/*
 * NATIVE API FUNCTIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Restore bond data of a known peer device (at connection establishment)
 *
 * Wait for #cb_enable_cmp execution before starting a new procedure
 *
 * @param[in] conidx        Connection index
 * @param[in] con_type      Connection type
 * @param[in] svc_type      Primary or Secondary Service
 * @param[in] p_bcs         Pointer to peer database description bond data
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t bcsc_enable(uint8_t conidx, uint8_t con_type, uint8_t svc_type, const bcsc_bcs_content_t* p_bcs);

/**
 ****************************************************************************************
 * @brief Perform body composition feature read procedure.
 *
 * Wait for #cb_read_feat_cmp execution before starting a new procedure
 *
 * @param[in] conidx        Connection index
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t bcsc_read_feature(uint8_t conidx);

/**
 ****************************************************************************************
 * @brief Perform indication configuration read procedure
 *
 * Wait for #cb_read_ind_cfg_cmp execution before starting a new procedure
 *
 * @param[in] conidx        Connection index
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t bcsc_read_ind_cfg(uint8_t conidx);

/**
 ****************************************************************************************
 * @brief Perform indication configuration write procedure
 *
 * Wait for #cb_write_ind_cfg_cmp execution before starting a new procedure
 *
 * @param[in] conidx        Connection index
 * @param[in] ind_cfg       Indication Configuration
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t bcsc_write_ind_cfg(uint8_t conidx, uint16_t ind_cfg);


/// @} BCSC

#endif //(_BCSC_H_)
