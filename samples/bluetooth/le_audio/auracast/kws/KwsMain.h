/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#ifndef KWS_MAIN_H_
#define KWS_MAIN_H_

#if CONFIG_TENSORFLOW_LITE_MICRO
/**
 * @brief Initialize the Keyword Spotting (KWS) system.
 */
int kws_initialize(void);
#else
/* just a dummy init function  */
static inline int kws_initialize(void)
{
	return 0;
};
#endif

#endif /* KWS_MAIN_H_ */
