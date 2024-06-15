/**
 * @file lv_demo_stress.h
 *
 */

#ifndef SIMPLE_DEMO_H
#define SIMPLE_DEMO_H

#ifdef __cplusplus
extern "C" {
#endif


#include "../lvgl.h"

#define LOCATION_ATTRIBUTE(name) __attribute__((section(name))) __attribute__((aligned(4)))

void simple_demo(void);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LV_DEMO_STRESS_H*/
