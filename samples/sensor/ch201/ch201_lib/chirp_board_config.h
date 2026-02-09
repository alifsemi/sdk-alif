/*
 * chirp_board_config.h
 *
 * This file defines default values for the required symbols used to build an
 * application with the Chirp SonicLib API and driver. These symbols are used
 * for static array allocations and counters in SonicLib (and often
 * applications), and are based on the number of specific resources on the
 * target board.
 *
 * This file is derived from invn/soniclib/details/chirp_board_config.h and
 * includes board-specific changes to the macro definitions.
 */

#ifndef CHIRP_BOARD_CONFIG_H_
#define CHIRP_BOARD_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

#define INCLUDE_WHITNEY_SUPPORT 1

/* Settings for the Chirp driver test setup */
#define CHIRP_MAX_NUM_SENSORS   CONFIG_CH201_MAX_NUM_SENSORS   /* max possible num of sensor dev */

#define CHIRP_NUM_BUSES         CONFIG_CH201_NUM_BUSES	/* number of I2C buses used by sensors */

#define CHIRP_RTC_CAL_PULSE_MS	CONFIG_CH201_RTC_CAL_PULSE_MS  /* Len of RTC calibration pulse */

#define CHIRP_SENSOR_INT_PIN    CONFIG_CH201_SENSOR_NUM_INT_PIN   /* Number of INT pins */
#define CHIRP_SENSOR_TRIG_PIN   CONFIG_CH201_SENSOR_NUM_TRIG_PIN  /* Number of TRIG pins */

#define CHIRP_I2C_SPEED_HZ      400000				  /* I2C speed in hertz */

#ifdef __cplusplus
}
#endif

#endif /* CHIRP_BOARD_CONFIG_H_ */
