/**
 * @file utmr.h
 *
 */

#ifndef UTMR_H
#define UTMR_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include <stdint.h>

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void utimer_init(void);

void utimer_start(void);
void utimer_stop(void);

uint32_t utimer_get_s(void);
uint32_t utimer_get_ms(void);
uint32_t utimer_get_us(void);
uint64_t utimer_get_ns(void);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*UTMR_H*/
