/*
 * Copyright 2025 Alif Semiconductor
 * Copyright 2022 Arm Limited and/or its affiliates <open-source-office@arm.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Expected output data for Keyword Spotting model
 * Output shape: [1, 12] - 12 keyword classes:
 *   0: silence, 1: unknown, 2: yes, 3: no, 4: up, 5: down,
 *   6: left, 7: right, 8: on, 9: off, 10: stop, 11: go
 *
 * Note: The expected output values depend on the specific model used.
 * For untrained models, verification is skipped (shape check only).
 * Set SKIP_OUTPUT_VERIFICATION=0 and update values below for trained models.
 */

#ifndef KWS_OUTPUT_H
#define KWS_OUTPUT_H

#include <stdint.h>

#define SKIP_OUTPUT_VERIFICATION 1

#define KWS_NUM_OUTPUT_CLASSES 12

__aligned(4) static const uint8_t kws_expected_output[KWS_NUM_OUTPUT_CLASSES]
__attribute__((section(".rodata.kws_output"))) = {
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80
};

#endif /* KWS_OUTPUT_H */
