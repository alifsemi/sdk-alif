/**
 ****************************************************************************************
 *
 * @file hl_proc.h

 * @brief Procedure Header file
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef HL_PROC_H_
#define HL_PROC_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup HL_PROC_API Procedures
 * @ingroup COMMON_API
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"
#include <stdint.h>
#include "co_list.h"

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Procedure Event type (some additional procedure event can be added per procedure)
enum hl_proc_event
{
    /// Procedure start granted
    HL_PROC_GRANTED = 0u,
    /// Default handler for procedure termination (if procedure is simple)
    HL_PROC_FINISHED,
    /// Simple transition for procedure that requires only 3 transition events (Granted, Continue, Finished)
    HL_PROC_CONTINUE,
    /// First event identifier available for a procedure
    HL_PROC_EVENT_FIRST,

    /// Invalid procedure state transition
    HL_PROC_INVALID = 0xFFu,
};

/*
 * CALLBACK FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

typedef struct hl_proc hl_proc_t;

/**
 ****************************************************************************************
 * @brief Function called when an event is triggered and creates a transition in procedure state machine
 *
 * @param[in] p_proc     Pointer to procedure structure
 * @param[in] event      Event type describing why procedure state transition occurs (see #hl_proc_event)
 * @param[in] status     Status (see #hl_err enumeration)
 *
 * @return True if procedure is finished and can be automatically clean-up, False if procedure continue
 ****************************************************************************************
 */
typedef bool (*hl_proc_cb_transition)(hl_proc_t* p_proc, uint8_t event, uint16_t status);

/**
 ****************************************************************************************
 * @brief Ask procedure to clean up itself
 *
 * @note Shall be called at the end #hl_proc_cleanup ;
 *       it's recommended to point directly to this function
 *
 * @param[in] p_proc     Pointer to procedure structure
 ****************************************************************************************
 */
typedef void (*hl_proc_cb_cleanup)(hl_proc_t* p_proc);

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// Procedure interface
typedef struct
{
    /// Function called when an event is triggered and creates a transition in procedure state machine
    hl_proc_cb_transition cb_transition;
    /// Function called when procedure is cleaned up
    hl_proc_cb_cleanup cb_cleanup;
} hl_proc_itf_t;

/// Basic procedure information structure
struct hl_proc
{
    /// List header allowing to insert the structure in a procedure queue (see #hl_proc_queue_t)
    co_list_hdr_t hdr;
    /// Pointer to procedure interface
    const hl_proc_itf_t* p_itf;
};

/// Procedure queue structure
typedef struct
{
    /// List header, needed to defer start of procedures
    co_list_hdr_t hdr;
    /// Procedure queue (contains #hl_proc elements)
    co_list_t queue;
} hl_proc_queue_t;

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Create a new procedure
 *
 * @param[in] p_proc_queue  Pointer to procedure queue in which create procedure must be inserted
 * @param[in] proc_size     Size of procedure structure to allocate, shall be at least equal to sizeof(hl_proc_t)
 * @param[in] p_itf         Pointer to procedure interface
 * @param[out] pp_proc      Pointer at which pointer to allocated procedure will be returned
 *
 * @return Execution status (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t hl_proc_create(hl_proc_queue_t* p_proc_queue, uint16_t proc_size, const hl_proc_itf_t *p_itf,
                        hl_proc_t** pp_proc);

/**
 ****************************************************************************************
 * @brief Ask a cleanup of procedure memory - shall be called to destroy procedure
 *
 * @param[in] p_proc       Pointer to procedure structure
 ****************************************************************************************
 */
void hl_proc_cleanup(hl_proc_t* p_proc);

/**
 ****************************************************************************************
 * @brief Ask procedure on top of a procedure queue to perform a transition
 *
 * @param[in] p_proc_queue  Pointer to procedure queue
 * @param[in] event         Event type describing why procedure state transition occurs (see #hl_proc_event)
 * @param[in] status        Status (see #hl_err enumeration)
 ****************************************************************************************
 */
void hl_proc_transition(hl_proc_queue_t* p_proc_queue, uint8_t event, uint16_t status);

/**
 ****************************************************************************************
 * @brief Free procedure structure
 *
 * @note Shall not be called if procedure is still in a procedure queue
 *
 * @param[in] p_proc        Pointer to procedure structure
 ****************************************************************************************
 */
void hl_proc_free(hl_proc_t* p_proc);

/**
 ****************************************************************************************
 * @return Pointer to procedure on top of a procedure queue
 *
 * @param[in] p_proc_queue Pointer to procedure queue
 ****************************************************************************************
 */
hl_proc_t* hl_proc_get(hl_proc_queue_t* p_proc_queue);

/**
 ****************************************************************************************
 * @brief Abort and clean all procedures present in a procedure queue.\n
 *        Inform all aborted procedures
 *
 * @param[in] p_proc_queue Pointer to procedure queue
 * @param[in] reason       Queue clean reason (see #hl_err enumeration)
 ****************************************************************************************
 */
void hl_proc_queue_abort(hl_proc_queue_t* p_proc_queue, uint16_t reason);

/**
 ****************************************************************************************
 * @brief Initialize a procedure queue
 *
 * @param[in] p_proc_queue Pointer to procedure queue
 ****************************************************************************************
 */
void hl_proc_queue_initialize(hl_proc_queue_t* p_proc_queue);

/**
 ****************************************************************************************
 * @return True if procedure on top of a procedure queue is waiting to be granted, False otherwise
 *
 * @param[in] p_proc_queue Pointer to procedure queue
 ****************************************************************************************
 */
bool hl_proc_is_waiting_grant(hl_proc_queue_t* p_proc_queue);

/// @} HL_PROC_API

#endif /* HL_PROC_H_ */
