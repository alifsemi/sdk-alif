/*
 * Copyright (c) 2023 Alif Semiconductor
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "ble_api.h"
#include "co_math.h"
#include "hci_uart.h"
#include "rwip.h"
#include "rwip_config.h"
#include "timer.h"
#include "sync_timer.h"
#include "es0_power_manager.h"
#include "soc_memory_map.h"
#include "alif_ble.h"
#include "gapm.h"
#include "se_service.h"

static k_tid_t ble_tid;

static atomic_t stop_flag = ATOMIC_INIT(0);

#define RWIP_INIT_NO_ERROR   0
#define RESET_MEM_ALLOC_FAIL 0xF2F2F2F2

LOG_MODULE_REGISTER(alif_ble);

/* Heap memory blocks for Alif BLE host stack */
static uint32_t ble_heap_env[RWIP_CALC_HEAP_LEN(RWIP_HEAP_ENV_SIZE) +
			     RWIP_CALC_HEAP_LEN(CONFIG_ALIF_BLE_HOST_ADDL_ENV_HEAPSIZE)] __noinit;
static uint32_t
	ble_heap_profile[RWIP_CALC_HEAP_LEN(RWIP_HEAP_PROFILE_SIZE) +
			 RWIP_CALC_HEAP_LEN(CONFIG_ALIF_BLE_HOST_ADDL_PRF_HEAPSIZE)] __noinit;
static uint32_t ble_heap_msg[RWIP_CALC_HEAP_LEN(RWIP_HEAP_MSG_SIZE)] __noinit;
static uint32_t ble_heap_non_ret[RWIP_CALC_HEAP_LEN(1000)] __noinit;

static uint32_t initialised __noinit;
#define INITIALISED_MAGIC 0x45454545

extern __weak uint32_t __ble_patch_info_start;

static ble_rom_config_t rom_config = {
	.p_ble_heap_env_mem = ble_heap_env,
	.ble_heap_env_mem_size = ARRAY_SIZE(ble_heap_env),
	.p_ble_heap_profile_mem = ble_heap_profile,
	.ble_heap_profile_mem_size = ARRAY_SIZE(ble_heap_profile),
	.p_ble_heap_msg_mem = ble_heap_msg,
	.ble_heap_msg_mem_size = ARRAY_SIZE(ble_heap_msg),
	.p_ble_heap_non_ret_mem = ble_heap_non_ret,
	.ble_heap_non_ret_mem_size = ARRAY_SIZE(ble_heap_non_ret),
	.ble_app_main_task = TASK_APP,
	.patch = &__ble_patch_info_start,
};

static K_THREAD_STACK_DEFINE(ble_stack_area, CONFIG_ALIF_BLE_HOST_THREAD_STACKSIZE);
static struct k_thread ble_thread;

static K_SEM_DEFINE(rwip_schedule_sem, 0, 1);
static K_SEM_DEFINE(rwip_init_sem, 0, 1);
static K_MUTEX_DEFINE(rwip_process_mutex);
static K_SEM_DEFINE(reset_sem, 0, 1);

static int irq_key;

/* Hooks for custom functionality. Can be used e.g. to benchmark BLE task */
void __weak alif_ble_enable_pre_hook(void)
{
}

void __weak alif_ble_enable_post_hook(void)
{
}

void __weak alif_ble_task_post_hook(void)
{
}

static void global_int_start(void)
{
	irq_unlock(irq_key);
}

static void global_int_stop(void)
{
	irq_key = irq_lock();
}

void platform_reset_request(uint32_t const error)
{
	if (error == RESET_MEM_ALLOC_FAIL) {
		__ASSERT(0,
			 "Running out of heap. Please, increase it. Current %u "
			 "(ALIF_BLE_HOST_ADDL_PRF_HEAPSIZE)",
			 CONFIG_ALIF_BLE_HOST_ADDL_PRF_HEAPSIZE);
		return;
	}

	__ASSERT(0, "Platform  reset requested by BLE host stack, err %u", error);
}

static void rtos_evt_post(void)
{
	k_sem_give(&rwip_schedule_sem);
}

void cb_on_stack_initialised(void)
{
	k_sem_give(&rwip_init_sem);
}

static void *global_to_local_rtss_he(void *global)
{
	uint32_t g_addr = (uint32_t)global;

	if (g_addr >= ITCM_GLOBAL_BASE && g_addr <= ITCM_GLOBAL_BASE + ITCM_SIZE) {
		return (void *)(g_addr - ITCM_GLOBAL_BASE + ITCM_BASE);
	} else if (g_addr >= DTCM_GLOBAL_BASE && g_addr <= DTCM_GLOBAL_BASE + DTCM_SIZE) {
		return (void *)(g_addr - DTCM_GLOBAL_BASE + DTCM_BASE);
	}

	return global;
}

static void temp_memcpy(uint8_t *restrict p_dst, uint8_t *restrict p_src, size_t len)
{
	while (len--) {
		*p_dst++ = *p_src++;
	}
}

int32_t copy_without_dma(void *p_dst, void *p_src, size_t len, void (*cb)(uint32_t err))
{
	__ASSERT_NO_MSG(p_dst);
	__ASSERT_NO_MSG(p_src);

	void *p_dst_loc = global_to_local_rtss_he(p_dst);
	void *p_src_loc = global_to_local_rtss_he(p_src);

	/* BLE RAM is handled as a DEVICE memory which disallows unaligned
	 * accesses so the memcpy is not possible if the SDU size is not properly
	 * aligned (e.g. 155 bytes).
	 * Use custom memcpy for now while the DMA is not implemented.
	 */
	temp_memcpy(p_dst_loc, p_src_loc, len);

	if (cb) {
		cb(0);
	}

	return 0;
}

/* Table of function pointers to be passed to Alif BLE host stack */
static ble_app_hooks_t app_hooks = {.p_global_int_disable = global_int_stop,
				    .p_global_int_restore = global_int_start,
				    .p_hci_itf_read = hci_uart_read,
				    .p_hci_itf_write = hci_uart_write,
				    .p_hci_itf_flow_on = hci_uart_flow_on,
				    .p_hci_itf_flow_off = hci_uart_flow_off,
				    .p_app_init = cb_on_stack_initialised,
				    .p_timer_init = timer_init,
				    .p_timer_get_time = timer_get_time,
				    .p_timer_enable = timer_enable,
				    .p_timer_set_timeout = timer_set_timeout,
				    .p_platform_reset_request = platform_reset_request,
				    .p_rtos_evt_post = rtos_evt_post,
				    .p_dma_copy = copy_without_dma,
				    .p_dma_abort = NULL,
#if IS_ENABLED(CONFIG_ALIF_BLE_ENABLE_AUDIO_SYNC_TIMER)
				    .p_sync_timer_start = sync_timer_start,
				    .p_sync_timer_get_curr_cnt = sync_timer_get_curr_cnt,
				    .p_sync_timer_get_last_capture = sync_timer_get_last_capture,
				    .p_sync_timer_disable_evts = sync_timer_disable_evts,
				    .p_sync_timer_restore_evts = sync_timer_restore_evts
#else
				    .p_sync_timer_start = NULL,
				    .p_sync_timer_get_curr_cnt = NULL,
				    .p_sync_timer_get_last_capture = NULL,
				    .p_sync_timer_disable_evts = NULL,
				    .p_sync_timer_restore_evts = NULL
#endif
				};

int alif_ble_mutex_lock(k_timeout_t timeout)
{
	return k_mutex_lock(&rwip_process_mutex, timeout);
}

void alif_ble_mutex_unlock(void)
{
	k_mutex_unlock(&rwip_process_mutex);
}

static void gapm_reset_cb(uint32_t metainfo, uint16_t status)
{
	LOG_DBG("GAPM RESET status: 0x%02x", status);
	k_sem_give(&reset_sem);
}

/* rwip_init() seeds the host PRNG from the microsecond timer, which reads the same
 * on every cold boot, so random addresses and keys derived from it would repeat
 * across devices.
 */
static void seed_host_prng(void)
{
	uint32_t seed;
	int err = se_service_get_rnd_num((uint8_t *)&seed, sizeof(seed));

	if (err) {
		LOG_WRN("SE RNG unavailable (%d), host PRNG seed is not random", err);
		return;
	}

	co_random_init(seed);
}

static void ble_task(void *dummy1, void *dummy2, void *dummy3)
{
	int ret = 0;

	alif_ble_enable_pre_hook();

	ret = hci_uart_init();
	__ASSERT(0 == ret, "Failed to initialise HCI UART");

#if IS_ENABLED(CONFIG_ALIF_BLE_ENABLE_AUDIO_SYNC_TIMER)
	ret = sync_timer_init();
	__ASSERT(0 == ret, "Failed to initialise sync timer");
#endif

	if (initialised != INITIALISED_MAGIC) {
		LOG_DBG("Cold start");

		/* hci_open calls this so should not be called here */
		if (0 != take_es0_into_use()) {
			__ASSERT(0, "Failed to boot ESO");
		}

		ble_init_err_code_t ble_success = ble_stack_init(&app_hooks, &rom_config);

		/* NOTE: BLE_INIT_ERR_PATCH will not be treated as an error case */
		if (ble_success == BLE_INIT_ERR_INVALID_ARGS) {
			LOG_ERR("Failed to initialise BLE stack: %u", ble_success);
			__ASSERT(false, "Failed to initialise BLE stack");
			return;
		}

		rwip_init(RWIP_INIT_NO_ERROR);
		seed_host_prng();
		initialised = INITIALISED_MAGIC;
	} else {
		/* Everything is already initialised as we are in warm restart case */
		LOG_DBG("Already initialised");
		app_hooks.p_app_init();
		k_sem_give(&rwip_schedule_sem);
	}

	LOG_DBG("task starting event loop");

	alif_ble_enable_post_hook();

	while (!atomic_get(&stop_flag)) {
		k_sem_take(&rwip_schedule_sem, K_FOREVER);

		alif_ble_mutex_lock(K_FOREVER);
		rwip_process();
		alif_ble_mutex_unlock();
	}

	LOG_DBG("Ending BLE thread");

	alif_ble_task_post_hook();
}

int alif_ble_enable(void (*cb)(void))
{
	/** If a user callback is provided, pass this into host stack so it will be called when the
	 * stack is initialised.Otherwise pass in our own callback which will post a semaphore,
	 * and block this function until the semaphore is posted to.
	 */
	int ret = (initialised == INITIALISED_MAGIC) ? -EALREADY : 0;

	/* Clear the stop_flag to allow entering the ble_thread loop */
	atomic_clear(&stop_flag);

	if (cb != NULL) {
		app_hooks.p_app_init = cb;
	} else {
		app_hooks.p_app_init = cb_on_stack_initialised;
	}

	ble_tid = k_thread_create(&ble_thread, ble_stack_area,
			K_THREAD_STACK_SIZEOF(ble_stack_area),
			ble_task, NULL, NULL, NULL, CONFIG_ALIF_BLE_HOST_THREAD_PRIORITY, 0,
			K_FOREVER);
	k_thread_start(&ble_thread);

	LOG_DBG("Waiting for ble_task to complete initialisation");

	/* Only block on the semaphore if a user callback was not provided */
	if (cb == NULL) {
		k_sem_take(&rwip_init_sem, K_FOREVER);
	}

	return ret;
}
int alif_ble_disable(void)
{
	int err;

	LOG_DBG("Stopping BLE");

	/* Clear initialised thread indicator */
	initialised = 0;

	/* Reset stack to clean processes and queues*/
	gapm_reset(0, gapm_reset_cb);

	k_sem_take(&reset_sem, K_FOREVER);

	/* Signal the BLE thread to exit the loop */
	atomic_set(&stop_flag, 1);

	/* Pass the semaphore to exit the BLE thread loop */
	k_sem_give(&rwip_schedule_sem);

	/* Call k_thread_join() to ensure the ble_thread is fully terminated */
	err = k_thread_join(&ble_thread, K_MSEC(500));

	if (err) {
		LOG_ERR("Terminating BLE thread error 0x%02x", err);

		LOG_WRN("Aborting BLE thread");

		k_thread_abort(ble_tid);

		err = k_thread_join(&ble_thread, K_FOREVER);
		if (err) {
			LOG_ERR("Error aborting BLE thread");
			return err;
		}
	}
	/* Request es0 stop */
	err = (int)stop_using_es0();
	if (err) {
		LOG_ERR("Error stopping es0: %02x", err);
		return err;
	}
	return 0;
}
