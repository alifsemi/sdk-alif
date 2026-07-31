/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#ifndef CONTROL_PROFILE_H_
#define CONTROL_PROFILE_H_

#if defined(CONFIG_AURACAST_CONTROL_PROFILE)

/**
 * @brief Register the Auracast control GATT service and start connectable advertising
 *
 * Must be called after gapm_configure() succeeds. Safe to call again after a
 * GAPM reset/reconfigure (re-registers the service and advertising activity).
 *
 * @retval 0 on success
 * @retval Negative error code on failure
 */
int control_profile_start(void);

/**
 * @brief Stop connectable advertising used by the control profile
 *
 * @retval 0 on success
 * @retval Negative error code on failure
 */
int control_profile_stop_adv(void);

/**
 * @brief Restart connectable advertising (e.g. after disconnection)
 *
 * @retval 0 on success
 * @retval Negative error code on failure
 */
int control_profile_restart_adv(void);

/**
 * @brief Notify connected clients of the current mode
 *
 * @retval 0 on success
 * @retval Negative error code on failure
 */
int control_profile_notify_mode(void);

#else /* !CONFIG_AURACAST_CONTROL_PROFILE */

static inline int control_profile_start(void)
{
	return 0;
}

static inline int control_profile_stop_adv(void)
{
	return 0;
}

static inline int control_profile_restart_adv(void)
{
	return 0;
}

static inline int control_profile_notify_mode(void)
{
	return 0;
}

#endif /* CONFIG_AURACAST_CONTROL_PROFILE */

#endif /* CONTROL_PROFILE_H_ */
