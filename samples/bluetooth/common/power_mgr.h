/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#ifndef __POWER_MGR_H__
#define __POWER_MGR_H__

/* TODO/FIXME: this is a temporary workaround to disable sleep
 * during pre-kernel initialization.
 * Remove this when proper PM integration is done.
 */
#define PREKERNEL_DISABLE_SLEEP 1

/**
 * Disable sleep mode.
 */
void power_mgr_disable_sleep(void);

/**
 * Allow sleep mode.
 */
void power_mgr_allow_sleep(void);

#endif /* __POWER_MGR_H__ */
