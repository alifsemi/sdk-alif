/*
 * Copyright 2022 Arm Limited and/or its affiliates <open-source-office@arm.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */
/* For the Alif boards, we do not have any DDR at the moment.
 * Add this section to SRAM area and use it for now. */
GROUP_START(DATA_REGION)

	SECTION_DATA_PROLOGUE(tflm_section,,SUBALIGN(16))
	{
		. = ALIGN(16);
		*(tflm_model tflm_arena tflm_input tflm_output)
	}GROUP_DATA_LINK_IN(RAMABLE_REGION, RAMABLE_REGION)

GROUP_END(DATA_REGION)
