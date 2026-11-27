/**
 ****************************************************************************************
 *
 * @file tipc.h
 *
 * @brief Header file - Time Profile Client - Native API
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef _TIPC_H_
#define _TIPC_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup TIPC_API Time Profile Client
 * @ingroup TIP_API
 * @brief Description of API for Time Profile Client
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup TIPC_API_COMMON Common
 * @ingroup TIPC_API
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup TIPC_API_NATIVE Native API
 * @ingroup TIPC_API
 * @brief Description of Native API for Time Profile Client
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "tip_common.h"

/// @addtogroup TIPC_API_COMMON
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Operation codes
enum tipc_op_codes
{
    /// Database Creation Procedure
    TIPC_RESERVED_OP_CODE = 0,
    /// Read char. Operation Code
    TIPC_RD_CHAR_CMD_OP_CODE = 1,
    /// Configure CCC of Current Time Characteristic Operation Code
    TIPC_CT_NTF_CFG_CMD_OP_CODE = 2,
    /// Write the Time Update Control Point Characteristic Value Operation Code
    TIPC_WR_TIME_UPD_CTNL_PT_CMD_OP_CODE = 3,
};

/// Current Time Service characteristics
enum tipc_cts_char
{
    /// Current Time
    TIPC_CHAR_CTS_CURR_TIME,
    /// Local Time Info
    TIPC_CHAR_CTS_LOCAL_TIME_INFO,
    /// Reference Time Info
    TIPC_CHAR_CTS_REF_TIME_INFO,

    /// Number of characteristics
    TIPC_CHAR_CTS_MAX,
};

/// Next DST Change Service characteristics
enum tipc_ndcs_char
{
    /// Time With DST
    TIPC_CHAR_NDCS_TIME_WITH_DST,

    /// Number of characteristics
    TIPC_CHAR_NDCS_MAX,
};

/// Reference Time Update Service characteristics
enum tipc_rtus_char
{
    /// Time Update Control Point
    TIPC_CHAR_RTUS_TIME_UPD_CTNL_PT,
    /// Time Update State
    TIPC_CHAR_RTUS_TIME_UPD_STATE,

    /// Number of characteristics
    TIPC_CHAR_RTUS_MAX,
};

/// Current Time Service descriptors
enum tipc_cts_desc
{
    /// Client Characteristic Configuration descriptor for Current Time characteristic
    TIPC_DESC_CTS_CURR_TIME_CLI_CFG,

    /// Number of descriptors
    TIPC_DESC_CTS_MAX,
};

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// Current Time Service content descriptor structure
typedef struct tipc_cts_content
{
    /// Service information
    prf_svc_t svc;
    /// Characteristics information
    prf_char_t chars[TIPC_CHAR_CTS_MAX];
    /// Descriptors information
    prf_desc_t descs[TIPC_DESC_CTS_MAX];
} tipc_cts_content_t;

/// Next DST Change Service content descriptor structure
typedef struct tipc_ndcs_content
{
    /// Service information
    prf_svc_t svc;
    /// Characteristics information
    prf_char_t chars[TIPC_CHAR_NDCS_MAX];
} tipc_ndcs_content_t;

/// Reference Time Update content descriptor structure
typedef struct tipc_rtus_content
{
    /// Service information
    prf_svc_t svc;
    /// Characteristics information
    prf_char_t chars[TIPC_CHAR_RTUS_MAX];
} tipc_rtus_content_t;

/// @} TIPC_API_COMMON

/// @addtogroup TIPC_API_NATIVE
/// @{

/*
 * NATIVE API CALLBACKS
 ****************************************************************************************
 */

/// Set of callback functions for communication with upper layer
typedef struct tipc_cb
{
    /**
     ****************************************************************************************
     * @brief Completion of enable procedure
     *
     * @param[in] conidx        Connection index
     * @param[in] status        Status of the procedure execution (see #hl_err enumeration)
     * @param[in] p_cts         Pointer to peer CTS database description bond data
     * @param[in] p_ndcs        Pointer to peer NDCS database description bond data
     * @param[in] p_rtus        Pointer to peer RTUS database description bond data
     ****************************************************************************************
     */
    void (*cb_enable_cmp)(uint8_t conidx, uint16_t status, const tipc_cts_content_t* p_cts,
                          const tipc_ndcs_content_t* p_ndcs, const tipc_rtus_content_t* p_rtus);

    /**
     ****************************************************************************************
     * @brief Completion of read alert status procedure.
     *
     * @param[in] conidx        Connection index
     * @param[in] status        Status of the procedure execution (see #hl_err enumeration)
     * @param[in] val_id        Value identifier (see #tip_value_id enumeration)
     * @param[in] p_value       Pointer to time value information
     ****************************************************************************************
     */
    void (*cb_read_cmp)(uint8_t conidx, uint16_t status, uint8_t val_id, const union tip_value* p_value);

    /**
     ****************************************************************************************
     * @brief Completion of write Characteristic Configuration procedure.
     *
     * @param[in] conidx        Connection index
     * @param[in] status        Status of the procedure execution (see #hl_err enumeration)
     ****************************************************************************************
     */
    void (*cb_write_cfg_cmp)(uint8_t conidx, uint16_t status);

    /**
     ****************************************************************************************
     * @brief Function called when current time update is received
     *
     * @param[in] conidx         Connection index
     * @param[in] p_current_time Pointer to current time update value
     ****************************************************************************************
     */
    void (*cb_curr_time)(uint8_t conidx, const tip_curr_time_t* p_current_time);

    /**
     ****************************************************************************************
     * @brief Completion of control point request procedure
     *
     * @param[in] conidx        Connection index
     * @param[in] status        Status of the Request Send (see #hl_err enumeration)
     ****************************************************************************************
     */
    void (*cb_ctnl_pt_req_cmp)(uint8_t conidx, uint16_t status);
} tipc_cb_t;

/*
 * NATIVE API FUNCTIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Add support of Current Time Service, Next Change Service and Reference Time Update service as Client
 *
 * @param[in] p_cbs             Pointer to set of callback functions\n
 *                              Shall not be NULL
 *
 * @return Status of the function execution (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t tipc_add(const tipc_cb_t* p_cbs);

/**
 ****************************************************************************************
 * @brief Restore bond data of a known peer device (at connection establishment)
 * Wait for #tipc_cb_t.cb_enable_cmp execution before starting a new procedure
 *
 * @param[in] conidx        Connection index
 * @param[in] con_type      Connection type
 * @param[in] p_cts         Pointer to peer CTS database description bond data
 * @param[in] p_ndcs        Pointer to peer NDCS database description bond data
 * @param[in] p_rtus        Pointer to peer RTUS database description bond data
 *
 * @return Status of the function execution (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t tipc_enable(uint8_t conidx, uint8_t con_type, const tipc_cts_content_t* p_cts,
                     const tipc_ndcs_content_t* p_ndcs, const tipc_rtus_content_t* p_rtus);

/**
 ****************************************************************************************
 * @brief Perform a read procedure
 * Wait for #tipc_cb_t.cb_read_cmp execution before starting a new procedure
 *
 * @param[in] conidx        Connection index
 * @param[in] val_id        Value identifier (see #tip_value_id enumeration)
 *
 * @return Status of the function execution (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t tipc_read(uint8_t conidx, uint8_t val_id);

/**
 ****************************************************************************************
 * @brief Perform a write Characteristic Configuration procedure.
 * Wait for #tipc_cb_t.cb_write_cfg_cmp execution before starting a new procedure
 *
 * @param[in] conidx        Connection index
 * @param[in] cfg_val       Configuration value
 *
 * @return Status of the function execution (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t tipc_write_cfg(uint8_t conidx, uint16_t cfg_val);

/**
 ****************************************************************************************
 * @brief Function called to send a control point request
 * Wait for #tipc_cb_t.cb_ctnl_pt_req_cmp execution before starting a new procedure
 *
 * @param[in] conidx         Connection index
 * @param[in] value          Control Point value
 *
 * @return Status of the function execution (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t tipc_ctnl_pt_req(uint8_t conidx, tip_time_upd_contr_pt_t value);

/// @} TIPS_API_NATIVE

#endif // _TIPC_H_
