/**
 ****************************************************************************************
 *
 * @file mac154_err.h (model: hl_error.h)
 *
 * @brief File that contains all MAC154 error codes.
 *
 * @Note can be use standalone
 *     - if with hl module, this file should be include after.
 *
 * Copyright (C) RivieraWaves 2022-2022
 *
 ****************************************************************************************
 */
#ifndef MAC154_ERROR_H_
#define MAC154_ERROR_H_

/**
 ****************************************************************************************
 * @addtogroup MAC154_APP__ERROR_API Error codes
 * @ingroup MAC154_API
 * @brief High layer error codes
 *
 * This module contains the primitives that allow an application accessing and running the
 * RivieraWaves Host protocol stack
 *
 * @{
 ****************************************************************************************
 */

#ifdef DOCUMENTATION
/* empty fake from BT + BLE + Wifi - there to avoid issue during doxygen generation */
enum hl_err {
	EMPTY_OR_NOT = 0xFF,
};
#endif /* DOCUMENTATION */

/**
 * Mac154 enhancement for HL error codes
 *      extension of hl_err
 *      (but use without hl module)
 * @see enum #hl_err
 */
enum mac154_err {
	/* ---------------------------------------------------------------------------------- */
	/* -------------------- MAC154 Error codes conveyed to upper layer ------------------ */
	/* ---------------------------------------------------------------------------------- */

	/* Mac154 no error */
	MAC154_ERR_NO_ERROR = 0x1500,

	/* Unknown MAC15.4 Command */
	MAC154_ERR_UNKNOWN_COMMAND = 0x1501,
	/* No Answer */
	MAC154_ERR_NO_ANSWER = 0x1502,
	/* Hardware Failure */
	MAC154_ERR_HARDWARE_FAILURE = 0x1503,
	/* Software TX Failure */
	MAC154_ERR_SFTWR_FAILURE_TX = 0x1504,
	/* Software RX Failure */
	MAC154_ERR_SFTWR_FAILURE_RX = 0x1505,
	/* Software ED Failure */
	MAC154_ERR_SFTWR_FAILURE_ED = 0x1506,
	/* Transmission issue */
	MAC154_ERR_RADIO_ISSUE = 0x1507,
	/* TX needed CCA and channel is not free */
	MAC154_ERR_RADIO_CHANNEL_IN_USE = 0x1508,
	/// No more entry available
	MAC154_ERR_LIMIT_EXCEED = 0x1509,
	/// No more entry available
	MAC154_ERR_INV_PARAMS = 0x150A,

	/* reuses of preexisting hl_error... */
#ifndef HL_ERROR_H_
	/* Invalid parameter in request */
	/* PRF_ERR_INVALID_PARAM               =   0x81, */
	/* Out of Range */
	PRF_OUT_OF_RANGE = 0xFF,
#endif /* HL_ERROR_H_ */
};
/* @} MAC154_APP__ERROR_API */

#endif /* MAC154_ERROR_H_ */
