/*
 * Copyright (c) 2023 Alif Semiconductor
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdbool.h>
#include <stddef.h>
#include "cmsis_compiler.h"
#include "sync_timer.h"
#include <zephyr/irq.h>
#include <zephyr/logging/log.h>
#include "utimer.h"

LOG_MODULE_REGISTER(iso_sync_timer);

#define EVTRTR_BASE     0x400E2000
#define EVTRTR_DMA_CTRL ((uint32_t volatile *)EVTRTR_BASE)

#define EVTRTR_SELECT_GROUP_0 0x0
#define EVTRTR_SELECT_GROUP_1 0x1
#define EVTRTR_SELECT_GROUP_2 0x2
#define EVTRTR_SELECT_GROUP_3 0x3

static void (*sync_timer_cap_cb)(void);
static void (*sync_timer_ovf_cb)(void);

/* UTIMER definitions */
#define UTIMER_BASE    0x48000000u
#define UTIMER_CHAN(n) ((utimer_chan_t *)(UTIMER_BASE + 0x1000u * ((n) + 1u)))

#define UTIMER_SRC_TRIG0_RISING   ((uint32_t)0x00000001u)
#define UTIMER_SRC_TRIG0_FALLING  ((uint32_t)0x00000002u)
#define UTIMER_SRC_TRIG1_RISING   ((uint32_t)0x00000004u)
#define UTIMER_SRC_TRIG1_FALLING  ((uint32_t)0x00000008u)
#define UTIMER_SRC_TRIG2_RISING   ((uint32_t)0x00000010u)
#define UTIMER_SRC_TRIG2_FALLING  ((uint32_t)0x00000020u)
#define UTIMER_SRC_TRIG3_RISING   ((uint32_t)0x00000040u)
#define UTIMER_SRC_TRIG3_FALLING  ((uint32_t)0x00000080u)
#define UTIMER_SRC_TRIG4_RISING   ((uint32_t)0x00000100u)
#define UTIMER_SRC_TRIG4_FALLING  ((uint32_t)0x00000200u)
#define UTIMER_SRC_TRIG5_RISING   ((uint32_t)0x00000400u)
#define UTIMER_SRC_TRIG5_FALLING  ((uint32_t)0x00000800u)
#define UTIMER_SRC_TRIG6_RISING   ((uint32_t)0x00001000u)
#define UTIMER_SRC_TRIG6_FALLING  ((uint32_t)0x00002000u)
#define UTIMER_SRC_TRIG7_RISING   ((uint32_t)0x00004000u)
#define UTIMER_SRC_TRIG7_FALLING  ((uint32_t)0x00008000u)
#define UTIMER_SRC_TRIG8_RISING   ((uint32_t)0x00010000u)
#define UTIMER_SRC_TRIG8_FALLING  ((uint32_t)0x00020000u)
#define UTIMER_SRC_TRIG9_RISING   ((uint32_t)0x00040000u)
#define UTIMER_SRC_TRIG9_FALLING  ((uint32_t)0x00080000u)
#define UTIMER_SRC_TRIG10_RISING  ((uint32_t)0x00100000u)
#define UTIMER_SRC_TRIG10_FALLING ((uint32_t)0x00200000u)
#define UTIMER_SRC_TRIG11_RISING  ((uint32_t)0x00400000u)
#define UTIMER_SRC_TRIG11_FALLING ((uint32_t)0x00800000u)
#define UTIMER_SRC_TRIG12_RISING  ((uint32_t)0x01000000u)
#define UTIMER_SRC_TRIG12_FALLING ((uint32_t)0x02000000u)
#define UTIMER_SRC_TRIG13_RISING  ((uint32_t)0x04000000u)
#define UTIMER_SRC_TRIG13_FALLING ((uint32_t)0x08000000u)
#define UTIMER_SRC_TRIG14_RISING  ((uint32_t)0x10000000u)
#define UTIMER_SRC_TRIG14_FALLING ((uint32_t)0x20000000u)
#define UTIMER_SRC_TRIG15_RISING  ((uint32_t)0x40000000u)
#define UTIMER_SRC_TRIG15_FALLING ((uint32_t)0x80000000u)

#define UTIMER_IRQ_BASE           377u
#define UTIMER_CAPTURE_A_IRQ_BASE (UTIMER_IRQ_BASE + 0)
#define UTIMER_OVERFLOW_IRQ_BASE  (UTIMER_IRQ_BASE + 7)

#define UTIMER_CAPTURE_A_IRQ(chan) (UTIMER_CAPTURE_A_IRQ_BASE + ((chan) * 8u))
#define UTIMER_OVERFLOW_IRQ(chan)  (UTIMER_OVERFLOW_IRQ_BASE + ((chan) * 8u))

/* UTIMER bit mask */
#define UTIMER_CAPTURE_A_BIT_MASK ((uint32_t)0x00000001u)
#define UTIMER_OVERFLOW_BIT_MASK  ((uint32_t)0x00000080u)

/* ISO event configuration */
#define ISO_EVT_EVTRTR_CHAN         8u
#define ISO_EVT_EVTRTR_GROUP        EVTRTR_SELECT_GROUP_2
#define ISO_EVT_UTIMER_CHAN         0u
#define ISO_EVT_UTIMER_TRIG         UTIMER_SRC_TRIG8_RISING
#define ISO_EVT_UTIMER_CAP_A_IRQ    UTIMER_CAPTURE_A_IRQ(ISO_EVT_UTIMER_CHAN)
#define ISO_EVT_UTIMER_OVF_IRQ      UTIMER_OVERFLOW_IRQ(ISO_EVT_UTIMER_CHAN)
#define ISO_EVT_UTIMER_OVF_IRQ_PRIO 3
#define ISO_EVT_UTIMER_CAP_IRQ_PRIO 4

/**
 * UTIMER channel registers.
 */
typedef struct {
	uint32_t volatile cntr_start0_src;     /**< Counter Start Source 0 Register >*/
	uint32_t volatile cntr_start1_src;     /**< Counter Start Source 1 Register >*/
	uint32_t volatile cntr_stop0_src;      /**< Counter Stop Source 0 Register >*/
	uint32_t volatile cntr_stop1_src;      /**< Counter Stop Source 1 Register >*/
	uint32_t volatile cntr_clear0_src;     /**< Counter Clear Source 0 Register >*/
	uint32_t volatile cntr_clear1_src;     /**< Counter Clear Source 1 Register >*/
	uint32_t volatile cntr_up0_src;        /**< Counter Up Source 0 Register >*/
	uint32_t volatile cntr_up1_src;        /**< Counter Up Source 1 Register >*/
	uint32_t volatile cntr_down0_src;      /**< Counter Down Source 0 Register >*/
	uint32_t volatile cntr_down1_src;      /**< Counter Down Source 1 Register >*/
	uint32_t volatile trig_capture_src_a0; /**< Trigger Capture Source A 0 Register >*/
	uint32_t volatile trig_capture_src_a1; /**< Trigger Capture Source A 1 Register >*/
	uint32_t volatile trig_capture_src_b0; /**< Trigger Capture Source B 0 Register >*/
	uint32_t volatile trig_capture_src_b1; /**< Trigger Capture Source B 1 Register >*/
	uint32_t volatile reserved1[18];       /**< Reserved Registers*/
	uint32_t volatile cntr_ctrl;           /**< Counter Control Register >*/
	uint32_t volatile reserved2[2];        /**< Reserved Registers >*/
	uint32_t volatile compare_ctrl_a;      /**< Compare Control A Register >*/
	uint32_t volatile compare_ctrl_b;      /**< Compare Control B Register >*/
	uint32_t volatile buf_op_ctrl;         /**< Buffer Op Control Register >*/
	uint32_t volatile reserved3[2];        /**< Reserved Registers >*/
	uint32_t volatile cntr;                /**< Counter Register >*/
	uint32_t volatile cntr_ptr;            /**< Counter Pointer Register >*/
	uint32_t volatile cntr_ptr_buf1;       /**< Counter Pointer Buffer 1 Register >*/
	uint32_t volatile cntr_ptr_buf2;       /**< Counter Pointer Buffer 2 Register >*/
	uint32_t volatile capture_a;           /**< Capture A Register >*/
	uint32_t volatile capture_a_buf1;      /**< Capture A Buffer 1 Register >*/
	uint32_t volatile capture_a_buf2;      /**< Capture A Buffer 2 Register >*/
	uint32_t volatile reserved4;           /**< Reserved Registers >*/
	uint32_t volatile capture_b;           /**< Capture B Register >*/
	uint32_t volatile capture_b_buf1;      /**< Capture B Buffer 1 Register >*/
	uint32_t volatile capture_b_buf2;      /**< Capture B Buffer 2 Register >*/
	uint32_t volatile reserved5;           /**< Reserved Registers >*/
	uint32_t volatile compare_a;           /**< Compare A Register >*/
	uint32_t volatile compare_a_buf1;      /**< Compare A Buffer 1 Register >*/
	uint32_t volatile compare_a_buf2;      /**< Compare A Buffer 2 Register >*/
	uint32_t volatile reserved6;           /**< Reserved Registers >*/
	uint32_t volatile compare_b;           /**< Compare B Register >*/
	uint32_t volatile compare_b_buf1;      /**< Compare B Buffer 1 Register >*/
	uint32_t volatile compare_b_buf2;      /**< Compare B Buffer 2 Register >*/
	uint32_t volatile reserved7;           /**< Reserved Registers >*/
	uint32_t volatile dead_time_up;        /**< Dead time up Register >*/
	uint32_t volatile dead_time_up_buf;    /**< Dead time up Buffer Register >*/
	uint32_t volatile dead_time_down;      /**< Dead time down Register >*/
	uint32_t volatile dead_time_down_buf;  /**< Dead time up Buffer Register >*/
	uint32_t volatile reserved8[5];        /**< Reserved Registers >*/
	uint32_t volatile chan_status;         /**< Channel Status Register >*/
	uint32_t volatile chan_interrupt;      /**< Channel interrupt Control >*/
	uint32_t volatile chan_interrupt_mask; /**< Channel Interrupt Mask Register >*/
	uint32_t volatile duty_cycle_ctrl;     /**< Duty cycle control register >*/
	uint32_t volatile dead_time_ctrl;      /**< Dead Control Register >*/
	uint32_t volatile reserved9[950];      /**< Reserved Registers >*/
} utimer_chan_t;

typedef struct {
	volatile uint32_t glb_cntr_start;   /**< Global Counter Start Register >*/
	volatile uint32_t glb_cntr_stop;    /**< Global Counter Stop Register >*/
	volatile uint32_t glb_cntr_clear;   /**< Global Counter Clear Register >*/
	volatile uint32_t glb_cntr_running; /**< Global Counter Running Status Register >*/
	volatile uint32_t glb_driver_oen;   /**< Channel Driver oen Register >*/
} TIMER_RegInfo;

static void overflow_irq_handler(const void *context)
{
	(void)context;

	/* LOG_DBG("ISO sync timer overflow IRQ handler"); */

	/* Clear OVERFLOW IRQ */
	UTIMER_CHAN(ISO_EVT_UTIMER_CHAN)->chan_interrupt |= UTIMER_OVERFLOW_BIT_MASK;
	(void)UTIMER_CHAN(ISO_EVT_UTIMER_CHAN)->chan_interrupt;

	if (sync_timer_ovf_cb) {
		sync_timer_ovf_cb();
	}
}

static void capture_irq_handler(const void *context)
{
	(void)context;

	/* LOG_DBG("ISO sync timer capture IRQ handler"); */

	/* Clear CAPTURE A IRQ */
	UTIMER_CHAN(ISO_EVT_UTIMER_CHAN)->chan_interrupt |= UTIMER_CAPTURE_A_BIT_MASK;
	(void)UTIMER_CHAN(ISO_EVT_UTIMER_CHAN)->chan_interrupt;

	if (sync_timer_cap_cb) {
		sync_timer_cap_cb();
	}
}

int32_t sync_timer_init(void)
{
	/* Enable clock for DMA2 and EVTRTR2 */
	mem_addr_t reg = 0x43007010;
	uint32_t orig = sys_read32(reg);
	uint32_t new = orig | 0x10;

	sys_write32(new, reg);

	/*
	 * Set up event router to generate a global event on the rising edge of the
	 * ISO GPIO 0 signal indicating an event occurs on an ISO over shared memory
	 * data path.
	 */
	EVTRTR_DMA_CTRL[ISO_EVT_EVTRTR_CHAN] = ISO_EVT_EVTRTR_GROUP;

	/*
	 * There is no interrupt on the M55 directly associated with the ISO GPIO 0
	 * so we use instead the ISO GPIO 0 event to indirectly raise an interrupt
	 * by triggering a capture on a dedicated UTIMER channel.
	 */
	sys_set_bit(UTIMER_GLB_CLOCK_ENABLE(0x48000000), ISO_EVT_UTIMER_CHAN);

	utimer_chan_t *utimer_chan = UTIMER_CHAN(ISO_EVT_UTIMER_CHAN);

	/* Capture timer value when GPIO 0 is triggered on CAPTURE_A */
	utimer_chan->trig_capture_src_a0 = ISO_EVT_UTIMER_TRIG;
	utimer_chan->trig_capture_src_a1 = 0x00000000u;
	utimer_chan->chan_interrupt_mask = ~(UTIMER_CAPTURE_A_BIT_MASK);

	/* Power on the counter */
	utimer_chan->cntr_start1_src = 0x80000000u; /* global programmatic start enabled */
	utimer_chan->cntr_stop1_src = 0x80000000u;  /* global programmatic stop enabled */
	utimer_chan->cntr_clear1_src = 0x80000000u; /* global programmatic clear enabled */

	/* UP counter configuration */
	utimer_chan->cntr_ptr = UINT32_MAX;   /* Max count value */
	utimer_chan->cntr = 0x00000000u;      /* Start count value */
	utimer_chan->cntr_ctrl = 0x00000001u; /* Start up counter */

	/* Enable overflow interrupt */
	utimer_chan->chan_interrupt_mask &= ~(UTIMER_OVERFLOW_BIT_MASK);

	/* Connect IRQs */
	IRQ_CONNECT(ISO_EVT_UTIMER_OVF_IRQ, ISO_EVT_UTIMER_OVF_IRQ_PRIO, overflow_irq_handler, 0,
		    0);
	IRQ_CONNECT(ISO_EVT_UTIMER_CAP_A_IRQ, ISO_EVT_UTIMER_CAP_IRQ_PRIO, capture_irq_handler, 0,
		    0);

	/* LOG_DBG("ISO sync timer initialised"); */

	return 0;
}

uint32_t sync_timer_start(void (*sync_timer_capture_evt_cb)(void),
			  void (*sync_timer_overflow_evt_cb)(void))
{
	if (sync_timer_capture_evt_cb) {
		sync_timer_cap_cb = sync_timer_capture_evt_cb;
	}
	if (sync_timer_overflow_evt_cb) {
		sync_timer_ovf_cb = sync_timer_overflow_evt_cb;
	}

	/* Enable IRQs */
	sync_timer_restore_evts();

	/* Global timer channel enable */
	((TIMER_RegInfo *)(UTIMER_BASE))->glb_cntr_start |= (1 << ISO_EVT_UTIMER_CHAN);

	/* LOG_DBG("ISO sync timer started"); */

	return CONFIG_SYS_CLOCK_HW_CYCLES_PER_SEC;
}

uint32_t sync_timer_get_curr_cnt(void)
{
	return UTIMER_CHAN(ISO_EVT_UTIMER_CHAN)->cntr;
}

uint32_t sync_timer_get_last_capture(void)
{
	return UTIMER_CHAN(ISO_EVT_UTIMER_CHAN)->capture_a;
}

void sync_timer_disable_evts(void)
{
	irq_disable(ISO_EVT_UTIMER_OVF_IRQ);
	irq_disable(ISO_EVT_UTIMER_CAP_A_IRQ);
	NVIC_ClearPendingIRQ(ISO_EVT_UTIMER_OVF_IRQ);
	NVIC_ClearPendingIRQ(ISO_EVT_UTIMER_CAP_A_IRQ);
}

void sync_timer_restore_evts(void)
{
	irq_enable(ISO_EVT_UTIMER_OVF_IRQ);
	irq_enable(ISO_EVT_UTIMER_CAP_A_IRQ);
}
