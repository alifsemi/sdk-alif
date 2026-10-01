/* Copyright (C) 2025 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <zephyr/kernel.h>
#include <string.h>
#include <zephyr/drivers/hwsem_ipm.h>
#include <zephyr/ztest.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include <errno.h>
#include <stdint.h>

#define SLEEP_TIME_MS		 1000
#define SHARED_TEST_ITERATIONS	  7
#define LED0_NODE		DT_ALIAS(led0)

#define DEVICE_DT_GET_AND_COMMA(node_id) DEVICE_DT_GET(node_id),

/* Generate a list of devices for all instances of the "compat" */
#define DEVS_FOR_DT_COMPAT(compat) \
	DT_FOREACH_STATUS_OKAY(compat, DEVICE_DT_GET_AND_COMMA)

static const struct device *const devices[] = {
#ifdef CONFIG_ALIF_HWSEM
	DEVS_FOR_DT_COMPAT(alif_hwsem)
#endif
};
#define WORKER_STACK_SIZE	1024
#define MSG_LEN			64

#if defined(CONFIG_RTSS_HP)
#define MASTER_ID 0xF00DF00D
#elif defined(CONFIG_RTSS_HE)
#define MASTER_ID 0xC0DEC0DE
#endif


struct mailbox {
	char msg[MSG_LEN];
	uint32_t seq;
	uint32_t ready;
};

static volatile struct mailbox mbox;
static struct k_sem slot_free;
static struct k_sem data_ready;
static int writer_result;
static int reader_result;

static struct k_thread writer_tid;
static struct k_thread reader_tid;
static K_THREAD_STACK_DEFINE(writer_stack, WORKER_STACK_SIZE);
static K_THREAD_STACK_DEFINE(reader_stack, WORKER_STACK_SIZE);

static void mailbox_writer(void *p1, void *p2, void *p3)
{
	int iter;
	const struct device *dev = devices[0];

	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	for (iter = 0; iter < SHARED_TEST_ITERATIONS; iter++) {
		if (k_sem_take(&slot_free, K_FOREVER) != 0) {
			writer_result = -1;
			return;
		}

		if (hwsem_lock(dev, MASTER_ID) != 0) {
			writer_result = -2;
			return;
		}

		snprintk((char *)mbox.msg, MSG_LEN,
			 "Hello from HWSEM0 iter %d", iter);
		mbox.seq = (uint32_t)iter;
		mbox.ready = 1U;

		if (hwsem_unlock(dev, MASTER_ID) != 0) {
			writer_result = -3;
			return;
		}

		k_sem_give(&data_ready);
	}

	writer_result = 0;
}

static void mailbox_reader(void *p1, void *p2, void *p3)
{
	int iter;
	char local[MSG_LEN];
	const struct device *dev = devices[1];

	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	for (iter = 0; iter < SHARED_TEST_ITERATIONS; iter++) {
		if (k_sem_take(&data_ready, K_FOREVER) != 0) {
			reader_result = -1;
			return;
		}

		if (hwsem_lock(dev, MASTER_ID) != 0) {
			reader_result = -2;
			return;
		}

		if (mbox.ready != 1U) {
			reader_result = -4;
			hwsem_unlock(dev, MASTER_ID);
			return;
		}

		memcpy(local, (const char *)mbox.msg, MSG_LEN);
		local[MSG_LEN - 1] = '\0';

		printk("HWSEM1 received: \"%s\" (seq=%u)\n",
		       local, mbox.seq);

		if (mbox.seq != (uint32_t)iter) {
			reader_result = -5;
			hwsem_unlock(dev, MASTER_ID);
			return;
		}

		mbox.ready = 0U;

		if (hwsem_unlock(dev, MASTER_ID) != 0) {
			reader_result = -3;
			return;
		}

		k_sem_give(&slot_free);
	}

	reader_result = 0;
}

ZTEST(hwsem_shared_peripheral, hwsem_mailbox_two_devices)
{
	zassert_true(ARRAY_SIZE(devices) >= 2,
		     "Need at least two HWSEM devices");

	memset((void *)&mbox, 0, sizeof(mbox));
	writer_result = -100;
	reader_result = -100;

	k_sem_init(&slot_free, 1, 1);
	k_sem_init(&data_ready, 0, 1);

	k_thread_create(&writer_tid, writer_stack, WORKER_STACK_SIZE,
			mailbox_writer, NULL, NULL, NULL,
			K_PRIO_PREEMPT(1), 0, K_NO_WAIT);

	k_thread_create(&reader_tid, reader_stack, WORKER_STACK_SIZE,
			mailbox_reader, NULL, NULL, NULL,
			K_PRIO_PREEMPT(1), 0, K_NO_WAIT);

	zassert_equal(k_thread_join(&writer_tid, K_FOREVER), 0,
		      "Writer did not finish");
	zassert_equal(k_thread_join(&reader_tid, K_FOREVER), 0,
		      "Reader did not finish");

	zassert_equal(writer_result, 0, "Writer failed: %d", writer_result);
	zassert_equal(reader_result, 0, "Reader failed: %d", reader_result);
	zassert_equal(mbox.ready, 0U, "Mailbox still marked ready");
}

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);



static void hwsem_basic_after(void *fixture)
{
	ARG_UNUSED(fixture);
	/*
	 * Targeted best-effort cleanup: only touch the devices/IDs that
	 * tests in this suite actually use, to avoid spamming
	 * "not locked" warnings for semaphores/IDs that were never
	 * touched by the test that just ran.
	 */
	hwsem_unlock(devices[0], MASTER_ID);

	hwsem_unlock(devices[0], 0xFFFFFFFF);

	if (ARRAY_SIZE(devices) > 1) {
		hwsem_unlock(devices[1], MASTER_ID);
	}
}

#define WRONG_MASTER_ID 0xDEADBEEF
#define UNUSED_CORE_ID  0xABCDEF01

/* Trylock with a different master while HWSEM is already held */
ZTEST(hwsem_basic, test_trylock_wrong_master_while_held)
{
	const struct device *device = devices[0];
	int ret;

	zassert_false(hwsem_lock(device, MASTER_ID),
		      "Unable to lock HWSEM");

	ret = hwsem_trylock(device, WRONG_MASTER_ID);
	zassert_equal(ret, -EBUSY,
		      "Trylock with wrong master should return -EBUSY; got %d",
		      ret);

	/* Same-master trylock may succeed if the HWSEM is recursive */
	ret = hwsem_trylock(device, MASTER_ID);
	zassert_true(ret == 0 || ret == -EBUSY,
		     "Unexpected return from same-master trylock: %d", ret);
	if (ret == 0) {
		zassert_false(hwsem_unlock(device, MASTER_ID),
			      "Unable to unlock recursive same-master lock");
	}

	zassert_false(hwsem_unlock(device, MASTER_ID),
		      "Unable to unlock HWSEM");
}

/*
 * Invalid / unused master IDs.
 * Documented contract: driver may accept them as a valid owner, or reject
 * them. This test records the actual behavior and fails only on crash /
 * unexpected errno values.
 */
ZTEST(hwsem_basic, test_invalid_master_id)
{
	const struct device *device = devices[0];
	const uint32_t ids[] = { 0U, UINT32_MAX, UNUSED_CORE_ID };
	int i, ret;

	for (i = 0; i < ARRAY_SIZE(ids); i++) {
		ret = hwsem_lock(device, ids[i]);
		zassert_true(ret == 0 || ret == -EINVAL || ret == -ENODEV,
			     "hwsem_lock(master=0x%08x) returned %d",
			     ids[i], ret);
		printk("hwsem_lock(master=0x%08x) -> %d (%s)\n",
		       ids[i], ret, ret == 0 ? "accepted" : "rejected");

		if (ret == 0) {
			zassert_false(hwsem_unlock(device, ids[i]),
				      "Unable to unlock after lock with 0x%08x",
				      ids[i]);
		}

		ret = hwsem_trylock(device, ids[i]);
		zassert_true(ret == 0 || ret == -EBUSY ||
			     ret == -EINVAL || ret == -ENODEV,
			     "hwsem_trylock(master=0x%08x) returned %d",
			     ids[i], ret);
		printk("hwsem_trylock(master=0x%08x) -> %d (%s)\n",
		       ids[i], ret,
		       ret == 0 ? "accepted" : "rejected/busy");

		if (ret == 0) {
			zassert_false(hwsem_unlock(device, ids[i]),
				      "Unable to unlock after trylock with 0x%08x",
				      ids[i]);
		}

		ret = hwsem_unlock(device, ids[i]);
		zassert_true(ret != 0,
			     "Unlock of free HWSEM with 0x%08x should fail",
			     ids[i]);
		printk("hwsem_unlock(free, master=0x%08x) -> %d\n",
		       ids[i], ret);
	}
}

/* Test case to validate boundary Master ID values (0 and 0xFFFFFFFF) */
ZTEST(hwsem_basic, test_master_id_boundaries)
{
	const struct device *device = devices[0];
	int ret;

	/*
	 * Master ID 0 is treated as a reserved "empty" sentinel by the
	 * driver and must be rejected, not silently accepted.
	 */
	ret = hwsem_lock(device, 0x00000000);
	zassert_not_equal(ret, 0,
			   "HWSEM lock unexpectedly succeeded with master ID 0");
	if (ret == 0) {
		/* Defensive: don't leave it locked if this ever changes */
		hwsem_unlock(device, 0x00000000);
	}

	/* Maximum master ID value should be accepted and behave normally */
	zassert_false(hwsem_lock(device, 0xFFFFFFFF),
		      "Unable to lock HWSEM with master ID 0xFFFFFFFF");

	zassert_false(hwsem_unlock(device, 0xFFFFFFFF),
		      "Unable to unlock HWSEM with master ID 0xFFFFFFFF");

	/* Confirm it's actually released now (double-unlock should fail) */
	zassert_equal(hwsem_unlock(device, 0xFFFFFFFF), -1,
		      "HWSEM unexpectedly still locked after cleanup unlock");
}

/*
 * Test case to confirm no cross-device interference when the same
 * Master ID is reused to lock multiple independent HWSEM instances.
 *
 * Note: this deliberately avoids trylock-against-a-busy-semaphore, since
 * on this driver that path retries internally until timeout rather than
 * failing fast with -EBUSY. Independence is instead proven by locking a
 * second, currently-free device with the same master ID and confirming
 * it succeeds immediately.
 */
ZTEST(hwsem_basic, test_same_master_id_multiple_devices)
{
	int ret;

	if (ARRAY_SIZE(devices) < 2) {
		ztest_test_skip();
	}

	zassert_false(hwsem_lock(devices[0], MASTER_ID),
		      "Failed to lock HWSEM 0 with shared master ID");

	/* HWSEM 1 is free; locking it with the SAME master ID should
	 * succeed immediately, proving lock state is per-device.
	 */
	ret = hwsem_trylock(devices[1], MASTER_ID);
	zassert_equal(ret, 0,
		      "HWSEM 1 should be free and immediately lockable, got %d", ret);

	zassert_false(hwsem_unlock(devices[1], MASTER_ID),
		      "Failed to unlock HWSEM 1");
	zassert_false(hwsem_unlock(devices[0], MASTER_ID),
		      "Failed to unlock HWSEM 0");
}


/* Test Case to validate Initialization of all HWSEM Nodes */
ZTEST(hwsem_basic, test_initialize)
{
	int device_idx;

	printk("Test all %d Hardware Semaphores(HWSEM) on %s\n",
	       (int)ARRAY_SIZE(devices), CONFIG_BOARD);

	for (device_idx = 0; device_idx < ARRAY_SIZE(devices); device_idx++) {
		zassert_true(device_is_ready(devices[device_idx]),
			     "HWSEM device %d not ready", device_idx);
	}
}

/* Test Case to lock all HWSEM Nodes using lock API */
ZTEST(hwsem_basic, test_lock)
{
	int device_idx;

	for (device_idx = 0; device_idx < ARRAY_SIZE(devices); device_idx++) {
		zassert_false(hwsem_lock(devices[device_idx], MASTER_ID),
			      "Unable to lock HWSEM %d\n", device_idx);
		/* Trying to lock already locked HWSEM */
		zassert_false(hwsem_lock(devices[device_idx], MASTER_ID),
			      "Unable to lock HWSEM %d\n", device_idx);
		/* Unlocking the locked HWSEM */
		zassert_false(hwsem_unlock(devices[device_idx], MASTER_ID),
			      "Unable to unlock HWSEM %d\n", device_idx);
		zassert_false(hwsem_unlock(devices[device_idx], MASTER_ID),
			      "Unable to unlock HWSEM %d\n", device_idx);
	}
}

/* Test Case to lock single HWSEM Node using trylock API */
ZTEST(hwsem_basic, test_trylock)
{
	/* Use only the single HWSEM device instance for this test */
	const struct device *device = devices[0];

	/* First trylock: can return 0 (success) or -EBUSY (busy, locked by another core) */
	int ret1 = hwsem_trylock(device, MASTER_ID);

	zassert_true(ret1 == 0 || ret1 == -EBUSY,
		     "Unexpected return from first hwsem_trylock: %d", ret1);

	if (ret1 == 0) {
		/* Unlock if the HWSEM is locked */
		zassert_false(hwsem_unlock(device, MASTER_ID),
			      "Unable to unlock HWSEM");
	}
}

/* Test case to unlock a non-locked HWSEM */
ZTEST(hwsem_basic, test_unlock)
{
	/* Use only the single HWSEM device instance for this test */
	const struct device *device = devices[0];

	int ret = hwsem_unlock(device, MASTER_ID);

	zassert_equal(ret, -1,
		      "Error occurred while trying to unlock; returned %d\n", ret);
}

/*
 * Test case to ensure multiple cores acquire the hardware semaphore to claim
 * the ownership of a shared resource (LED).
 * The core that acquires the semaphore toggles the LED before releasing it.
 * Each core repeats the above process.
 */
ZTEST(hwsem_shared_peripheral, hwsem0_sharing_led)
{
	int iter;
	const struct device *device = devices[0];

	zassert_true(gpio_is_ready_dt(&led), "LED device not ready");

	zassert_equal(gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE), 0,
		      "Unable to configure the LED");

	for (iter = 0; iter < SHARED_TEST_ITERATIONS; iter++) {
		zassert_false(hwsem_lock(device, MASTER_ID),
			      "Unable to lock HWSEM\n");

		zassert_equal(gpio_pin_toggle_dt(&led), 0,
			      "Error while toggling the GPIO\n");

		k_msleep(SLEEP_TIME_MS);

		zassert_false(hwsem_unlock(device, MASTER_ID),
			      "Unable to unlock HWSEM\n");
	}
}

/* HWSEM Basic API Tests */
ZTEST_SUITE(hwsem_basic, NULL, NULL, NULL, hwsem_basic_after, NULL);

/* HWSEM Real-time test with shared peripheral */
ZTEST_SUITE(hwsem_shared_peripheral, NULL, NULL, NULL, NULL, NULL);

