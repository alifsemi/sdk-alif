/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#ifndef QDEC_EMULATOR_H
#define QDEC_EMULATOR_H

int qenc_emulate_init(void);
void simulate_cw_rotation(int steps);
void simulate_ccw_rotation(int steps);
void simulate_glitch_pulses(int pulses);

#endif /* QDEC_EMULATOR_H */
