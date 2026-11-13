/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#ifndef A32_GOING_OFF_H
#define A32_GOING_OFF_H

#include <stdint.h>

/*
 * Keep in sync with:
 *   TF-A   include/plat/alif/common/alif_mhu.h
 *   Linux  arch/arm/mach-ensemble/a32_m55_shutdown.c
 *
 * One 32-bit value on MHU0 channel 0 (same transport as the Linux/Zephyr
 * MHU0 samples).
 *
 * Scenario A (Linux poweroff):
 *   TF-A -> A32_GOING_OFF, M55 -> A32_GOING_OFF_ACK
 *
 * Scenario B (M55 master-initiated):
 *   M55 -> M55_PERIPH_OFF_REQ, Linux IRQ -> M55_PERIPH_OFF_REQ_ACK
 *   then the same A32_GOING_OFF handshake as Scenario A.
 */
#define A32_GOING_OFF			0xA320FF01U
#define A32_GOING_OFF_ACK		0xA320FFA1U
#define M55_PERIPH_OFF_REQ		0xA320FF10U
#define M55_PERIPH_OFF_REQ_ACK		0xA320FF11U
#define A32_GOING_OFF_CHANNEL		0U

enum a32_cluster_state {
	A32_STATE_ON = 0,
	A32_STATE_OFF_REQ_SENT,
	A32_STATE_SHUTDOWN,
	A32_STATE_OFF,
	A32_STATE_FAIL,
};

#endif /* A32_GOING_OFF_H */
