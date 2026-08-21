/* Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

#ifndef __DISPLAY_PM_TEST_H__
#define __DISPLAY_PM_TEST_H__

int display_pm_thread_init(void);
int display_pm_thread_start(void);
int display_pm_thread_suspend(void);
int display_pm_thread_resume(void);
int display_pm_wait_streaming_done(void);

#endif /* __DISPLAY_PM_TEST_H__ */
