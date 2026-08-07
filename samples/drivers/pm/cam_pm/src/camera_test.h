/* Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

#ifndef __CAMERA_PM_TEST_INC__
#define __CAMERA_PM_TEST_INC__

int camera_pm_thread_init(void);
int camera_pm_thread_start(void);
int camera_pm_thread_suspend(void);
int camera_pm_thread_resume(void);
int camera_pm_thread_stop(void);
int camera_pm_wait_capture_done(void);

#endif /* __CAMERA_PM_TEST_INC__ */
