/**
 ****************************************************************************************
 *
 * @file gapi_isooshm.h
 *
 * @brief ISO data path over shared memory, HL side
 *
 ****************************************************************************************
 */

#ifndef _GAPI_ISOOSHM_H_
#define _GAPI_ISOOSHM_H_

#include "rom_build_cfg.h"

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "isooshm.h"
#include "isooshm_plf.h"
#include "gapi.h"

/** Size of the header added to the SDUs when exchanged over shared memory, in bytes */
#define GAPI_ISOOSHM_SDU_HDR_LEN (ISOOSHM_SDU_HDR_LEN)

typedef isooshm_sdu_buf_t gapi_isooshm_sdu_buf_t;
typedef struct gapi_isooshm_dp gapi_isooshm_dp_t;

/**
 * States of a data path.
 */
typedef enum {
    /** Initialized but not bound to a stream */
    GAPI_ISOOSHM_STATE_INITIALIZED,
    /** Bound to stream, no transfer ongoing or pending */
    GAPI_ISOOSHM_STATE_BOUND,
    /** Bound to a stream, transfer pending */
    GAPI_ISOOSHM_STATE_TRANSFER_PENDING,
    /** Bound to a stream, transfer ongoing */
    GAPI_ISOOSHM_STATE_TRANSFER_ONGOING,
    /** Destroyed, waiting for a call to #gapi_isooshm_unbind */
    GAPI_ISOOSHM_STATE_DESTROYED
} gapi_isooshm_state_t;

/**
 * Status of a received SDU.
 */
typedef enum {
    /** SDU content is valid */
    GAPI_ISOOSHM_SDU_STATUS_VALID = ISOOSHM_SDU_STATUS_VALID,
    /** SDU content might contain errors */
    GAPI_ISOOSHM_SDU_STATUS_ERROR = ISOOSHM_SDU_STATUS_ERROR,
    /** SDU has been lost */
    GAPI_ISOOSHM_SDU_STATUS_LOST = ISOOSHM_SDU_STATUS_LOST
} gapi_isooshm_sdu_status_t;

/**
 * @brief Callback invoked when a new SDU has been exchanged with the controller.
 * @param[in] dp The data path.
 * @param[in] buf The SDU buffer.
 *
 * If the data path is providing SDUs to the controller (GAPI_DP_DIRECTION_INPUT),
 * this callback is invoked when the SDU has been transmitted to the controller
 * and a new SDU can be sent.
 *
 * If the data path is retrieving SDUs from the controller (GAPI_DP_DIRECTION_OUTPUT),
 * this callback is invoked when a new SDU has been received by the controller
 * and copied to the provided buffer.
 *
 * In any case, when this callback is invoked, a new SDU buffer can be provided
 * to the data path using #gapi_isooshm_dp_set_buf.
 *
 * This callback is invoked from ISR. The #gapi_isooshm_dp_set_buf routine can
 * safely be invoked from this callback.
 */
typedef void (*gapi_isooshm_cb_t)(gapi_isooshm_dp_t* dp, gapi_isooshm_sdu_buf_t* buf);

/**
 * Timing information about the last SDU processed by the controller, useful for
 * synchronization purposes.
 */
typedef struct {
    /** Timestamp, in µs, of the SDU Synchronization Reference */
    uint32_t sdu_ref;
    /** Timestamp, in µs, of the Reference Anchor point of the SDU */
    uint32_t sdu_anchor;
    /** SDU sequence number */
    uint16_t seq_num;
} gapi_isooshm_sdu_sync_t;

/**
 * ISO over shared memory data path.
 */
typedef struct gapi_isooshm_dp {
    /** Stream local ID */
    uint8_t stream_lid;
    /** Link ID */
    uint8_t link_id;
    /** Data path direction (@see #gapi_dp_direction) */
    uint8_t dir;
    /** Data path state (@see #gapi_isooshm_state_t) */
    uint8_t state;
    /** Group ID of BIG/CIG assigned by host*/
    uint8_t grp_id;
    /** Reserved for future use */
    uint8_t reserved[3];
    /** Queue in shared memory used to exchange the SDUs with the controller */
    ipc_queue_t* sdu_queue;
    /** Buffer used to exchange the next SDU with the controller */
    gapi_isooshm_sdu_buf_t* buf;
    /** Callback to invoked when an SDU has been exchanged with the controller */
    gapi_isooshm_cb_t cb;
    /** Ongoing data transfer */
    isooshm_plf_transfer_t transfer;
} gapi_isooshm_dp_t;

/**
 * @brief Initializes an ISO over shared memory data path.
 * @param[in, out] dp The data path.
 * @param[in] cb The callback to invoke when the transfer of an SDU is complete.
 * @return 0 on success, an error code otherwise (see #hl_err).
 */
uint16_t gapi_isooshm_dp_init(gapi_isooshm_dp_t* dp, gapi_isooshm_cb_t cb);

/**
 * @brief Binds a data path to an existing ISO stream.
 * @param[in, out] dp The data path.
 * @param[in] stream_lid The stream local ID.
 * @param[in] dir The data path direction.
 * @return 0 on success, an error code otherwise (see #hl_err).
 */
uint16_t gapi_isooshm_dp_bind(gapi_isooshm_dp_t* dp, uint8_t stream_lid, enum gapi_dp_direction dir);

/**
 * @brief Unbinds a data path from an ISO stream.
 * @param[in, out] dp The data path.
 * @param[out] pending_buf If not NULL, set to the current data path SDU buffer, if any.
 * @return 0 on success, an error code otherwise (see #hl_err).
 */
uint16_t gapi_isooshm_dp_unbind(gapi_isooshm_dp_t* dp, gapi_isooshm_sdu_buf_t** pending_buf);

/**
 * @brief Provides an SDU buffer to the data path.
 * @param[in] dp The data path.
 * @param[in] buf The buffer.
 * @return 0 on success, an error code otherwise (see #hl_err).
 *
 * The provided buffer must 4-byte aligned and will be transfered using DMA. If
 * the buffer is located in a cacheable memory area, the caller is responsible
 * for any required cache maintenance operation.
 */
uint16_t gapi_isooshm_dp_set_buf(gapi_isooshm_dp_t* dp, gapi_isooshm_sdu_buf_t* buf);

/**
 * @brief Retrives timing information of the last SDU processed by the controller.
 * @param[in] dp The data path.
 * @param[out] sync The timing information.
 * @return 0 on success, an error code otherwise (see #hl_err).
 *
 * Timing information is only available for Tx data paths (transferring SDUs from host to
 * controller).
 */
uint16_t gapi_isooshm_dp_get_sync(gapi_isooshm_dp_t const* dp, gapi_isooshm_sdu_sync_t* sync);

/**
 * @brief  Provides local controller time which is in sync with
 *         host timer. Valid only when one or more ISO datapath
 *         has been started
 *
 * @return current controller time(in microseconds)
 *
 */
uint32_t gapi_isooshm_dp_get_local_time(void);

/**
 * @brief  Provides the drift between peer(master) and local clock
 *         when an ISO stream is established. In an ideal case the
 *         time between two consecutive ISO events should be equal
 *         to the ISO interval set for the link. In this case the
 *         peer drift is 0. If time difference between two ISO events
 *         do not exactly match ISO interval due to drift in clocks
 *         of peer and local controller then the drift is measured
 *         on the controller and shared with host as peer drift.
 *         The SDU presentation time or generation time can be
 *         adjusted by applying this peer drift.
 *
 * @param[in] dp The data path.
 *
 * @return current peer controller drift(in microseconds)
 *
 */
uint32_t gapi_isooshm_dp_get_peer_drift(gapi_isooshm_dp_t* dp);

/**
 * @brief  Applies peer controller drift correction
 * @param[in] dp       The data path.
 * @param[in] ref_time Reference time for which drift correction
 *                     needs to be applied
 *
 * @return Drift corrected time(in microseconds)
 */
uint32_t gapi_isooshm_dp_apply_drift_correction(gapi_isooshm_dp_t* dp, uint32_t ref_time);

#endif // _GAPI_ISOOSHM_H_
