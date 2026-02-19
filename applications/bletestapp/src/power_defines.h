/* Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */
#ifndef __POWER_DEFINES_H__
#define __POWER_DEFINES_H__

#include <zephyr/kernel.h>
#include <aipm.h>

enum lp_counter_source {
	LPRTC,
	LPTIMER,
};

#define SERAM_MEMORY_BLOCKS_IN_USE SERAM_1_MASK | SERAM_2_MASK | SERAM_3_MASK | SERAM_4_MASK

#define APP_RET_MEM_BLOCKS \
	SRAM4_1_MASK | SRAM4_2_MASK | SRAM4_3_MASK | SRAM4_4_MASK | SRAM5_1_MASK | SRAM5_2_MASK |  \
	SRAM5_3_MASK | SRAM5_4_MASK | SRAM5_5_MASK

#if DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(timer0), snps_dw_timers, okay)
#define WAKEUP_SOURCE_LPTIMER	DT_NODELABEL(timer0)
#endif
#if DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(rtc0), snps_dw_apb_rtc, okay)
#define WAKEUP_SOURCE_RTC		DT_NODELABEL(rtc0)
#endif
#if DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(lpgpio), snps_designware_gpio, okay)
#define WAKEUP_SOURCE_LPGPIO	DT_NODELABEL(lpgpio)
#endif

#endif /* __POWER_DEFINES_H__ */
