/**
 ****************************************************************************************
 *
 * @file lanc.h
 *
 * @brief Header file - Location and Navigation Profile Collector Role - Native API.
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef _LANC_H_
#define _LANC_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @addtogroup LANC
 * @ingroup Profile
 * @brief Location and Navigation Profile Collector - Native API
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "lanc_msg.h"

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/*
 * NATIVE API CALLBACKS
 ****************************************************************************************
 */


/// location and navigation service client callback set
typedef struct lanc_cb
{
    /**
     ****************************************************************************************
     * @brief Completion of enable procedure
     *
     * @param[in] conidx        Connection index
     * @param[in] status        Status of the procedure execution (see enum #hl_err)
     * @param[in] p_lns         Pointer to peer database description bond data
     ****************************************************************************************
     */
    void (*cb_enable_cmp)(uint8_t conidx, uint16_t status, const lanc_lns_content_t* p_lns);

    /**
     ****************************************************************************************
     * @brief Completion of read features procedure.
     *
     * @param[in] conidx        Connection index
     * @param[in] status        Status of the procedure execution (see enum #hl_err)
     * @param[in] sensor_feat   CP sensor feature
     *
     ****************************************************************************************
     */
    void (*cb_read_features_cmp)(uint8_t conidx, uint16_t status, uint32_t features);

    /**
     ****************************************************************************************
     * @brief Completion of read position quality procedure.
     *
     * @param[in] conidx        Connection index
     * @param[in] status        Status of the procedure execution (see enum #hl_err)
     * @param[in] p_pos_q       Pointer to position quality informations
     *
     ****************************************************************************************
     */
    void (*cb_read_pos_q_cmp)(uint8_t conidx, uint16_t status, const lanp_posq_t* p_pos_q);

    /**
     ****************************************************************************************
     * @brief Completion of read Characteristic Configuration procedure.
     *
     * @param[in] conidx        Connection index
     * @param[in] status        Status of the procedure execution (see enum #hl_err)
     * @param[in] desc_code     Descriptor Code (see enum #lanc_code)
     *                              - LANC_RD_WR_LOC_SPEED_CL_CFG: Location and Speed Client Char. Configuration Descriptor
     *                              - LANC_RD_WR_LN_CTNL_PT_CFG: Control Point Client Char. Configuration Descriptor
     *                              - LANC_RD_WR_NAVIGATION_CFG: Navigation Client Char. Configuration Descriptor
     * @param[in] cfg_val       Configuration value
     *
     ****************************************************************************************
     */
    void (*cb_read_cfg_cmp)(uint8_t conidx, uint16_t status, uint8_t desc_code, uint16_t cfg_val);

    /**
     ****************************************************************************************
     * @brief Completion of write Characteristic Configuration procedure.
     *
     * @param[in] conidx        Connection index
     * @param[in] status        Status of the procedure execution (see enum #hl_err)
     * @param[in] desc_code     Descriptor Code (see enum #lanc_code)
     *                              - LANC_RD_WR_LOC_SPEED_CL_CFG: Location and Speed Client Char. Configuration Descriptor
     *                              - LANC_RD_WR_LN_CTNL_PT_CFG: Control Point Client Char. Configuration Descriptor
     *                              - LANC_RD_WR_NAVIGATION_CFG: Navigation Client Char. Configuration Descriptor
     *
     ****************************************************************************************
     */
    void (*cb_write_cfg_cmp)(uint8_t conidx, uint16_t status, uint8_t desc_code);

    /**
     ****************************************************************************************
     * @brief Function called when location and speed information is received
     *
     * @param[in] conidx         Connection index
     * @param[in] p_loc_speed    Pointer to data information
     ****************************************************************************************
     */
    void (*cb_loc_speed)(uint8_t conidx, const lanp_loc_speed_t* p_loc_speed);

    /**
     ****************************************************************************************
     * @brief Function called when CP vector information is received
     *
     * @param[in] conidx         Connection index
     * @param[in] p_vector       Pointer to navigation information
     ****************************************************************************************
     */
    void (*cb_navigation)(uint8_t conidx, const lanp_navigation_t* p_nav);

    /**
     ****************************************************************************************
     * @brief Completion of control point request procedure
     *
     * @param[in] conidx           Connection index
     * @param[in] status           Status of the Request Send (see enum #hl_err)
     * @param[in] req_op_code      Requested Operation Code see enum #lanp_ln_ctnl_pt_code
     * @param[in] resp_value       Response Value see enum #lanp_ctnl_pt_resp_val
     * @param[in] number_of_routes Number of route (valid only if req_op_code = LANP_LN_CTNL_PT_REQ_NUMBER_OF_ROUTES)
     * @param[in] p_name           Route name (valid only if req_op_code = LANP_LN_CTNL_PT_REQ_NAME_OF_ROUTE)
     ****************************************************************************************
     */
    void (*cb_ctnl_pt_req_cmp)(uint8_t conidx, uint16_t status, uint8_t req_op_code, uint8_t resp_value,
                               uint16_t number_of_routes, co_buf_t* p_name);
} lanc_cb_t;

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
 * @param[in] p_lns         Pointer to peer database description bond data
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t lanc_enable(uint8_t conidx, uint8_t con_type, const lanc_lns_content_t* p_lns);

/**
 ****************************************************************************************
 * @brief Perform a read features procedure.
 *
 * Wait for #cb_read_features_cmp execution before starting a new procedure
 *
 * @param[in] conidx        Connection index
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t lanc_read_features(uint8_t conidx);

/**
 ****************************************************************************************
 * @brief Perform a read position quality procedure.
 *
 * Wait for #cb_read_pos_q_cmp execution before starting a new procedure
 *
 * @param[in] conidx        Connection index
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t lanc_read_pos_q(uint8_t conidx);

/**
 ****************************************************************************************
 * @brief Perform a read Characteristic Configuration procedure.
 *
 * Wait for #cb_read_cfg_cmp execution before starting a new procedure
 *
 * @param[in] conidx        Connection index
 * @param[in] desc_code     Descriptor Code (see enum #lanc_code)
 *                              - LANC_RD_WR_LOC_SPEED_CL_CFG: Location and Speed Client Char. Configuration Descriptor
 *                              - LANC_RD_WR_LN_CTNL_PT_CFG: Control Point Client Char. Configuration Descriptor
 *                              - LANC_RD_WR_NAVIGATION_CFG: Navigation Client Char. Configuration Descriptor
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t lanc_read_cfg(uint8_t conidx, uint8_t desc_code);

/**
 ****************************************************************************************
 * @brief Perform a write Characteristic Configuration procedure.
 *
 * Wait for #cb_write_cfg_cmp execution before starting a new procedure
 *
 * @param[in] conidx        Connection index
 * @param[in] desc_code     Descriptor Code (see enum #lanc_code)
 *                              - LANC_RD_WR_LOC_SPEED_CL_CFG: Location and Speed Client Char. Configuration Descriptor
 *                              - LANC_RD_WR_LN_CTNL_PT_CFG: Control Point Client Char. Configuration Descriptor
 *                              - LANC_RD_WR_NAVIGATION_CFG: Navigation Client Char. Configuration Descriptor
 * @param[in] cfg_val       Configuration value
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t lanc_write_cfg(uint8_t conidx, uint8_t desc_code, uint16_t cfg_val);

/**
 ****************************************************************************************
 * @brief Function called to send a control point request
 *
 * Wait for #cb_ctnl_pt_req_cmp execution before starting a new procedure
 *
 * @param[in] conidx         Connection index
 * @param[in] req_op_code    Requested Operation Code see enum #lanp_ln_ctnl_pt_code
 * @param[in] p_value        Pointer to request data content
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t lanc_ctnl_pt_req(uint8_t conidx, uint8_t req_op_code, const union lanp_ln_ctnl_pt_req_val* p_value);

/// @} LANC

#endif //(_LANC_H_)
