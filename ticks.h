/* 
 * File:   ticks.h
 * Author: yuhsienchang
 *
 * Created on June 6, 2024, 1:59 PM
 */

#ifndef TICKS_H
#define	TICKS_H

#include <stdint.h>
volatile uint32_t ticks = 0;

uint32_t get_ticks_with_lock();
uint32_t calculate_diff(uint32_t now, uint32_t prev_ticks);

#endif	/* TICKS_H */

