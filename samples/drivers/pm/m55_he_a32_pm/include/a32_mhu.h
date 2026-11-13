/* Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#ifndef A32_MHU_H
#define A32_MHU_H

/*
 * Ask Linux/TF-A to power down the A32 cluster (PD-9) over MHU0.
 * Returns 0 when SE reports PD2_APPS off, negative on timeout or error.
 */
int a32_mhu_shutdown(void);

#endif /* A32_MHU_H */
