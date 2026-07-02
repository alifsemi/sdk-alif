/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 * Common definitions shared by the MHUv2 sample test cases.
 *
 * Each test case keeps its own callbacks, message values and main loop (those
 * differ per case), but the scaffolding below is identical across them and is
 * collected here to avoid duplicating it in every source file.
 */

#ifndef MHUV2_COMMON_H
#define MHUV2_COMMON_H

#include <zephyr/device.h>
#include <zephyr/sys/printk.h>

/* Number of message exchanges each test case performs. */
#define ITERATIONS 10

/* Check that an MHU device is ready; returns 0 on success, -1 on failure. */
static inline int mhu_check_ready(const struct device *dev, const char *name)
{
	if (!device_is_ready(dev)) {
		printk("MHU device %s not ready\n", name);
		return -1;
	}
	return 0;
}

/* Fetch an MHU device from its devicetree alias and verify it is ready.
 * Returns 0 on success, -1 on failure.  The caller must check the return
 * value.  'dev' is the device-pointer variable to assign; 'alias' is the
 * devicetree alias token (e.g. rtssmhu0r).
 */
#define MHU_GET_READY(dev, alias)                                              \
	({                                                                     \
		(dev) = DEVICE_DT_GET(DT_ALIAS(alias));                        \
		mhu_check_ready(dev, #dev);                                    \
	})

#endif /* MHUV2_COMMON_H */
