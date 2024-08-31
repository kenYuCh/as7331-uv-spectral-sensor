/* 
 * File:   normalize.h
 * Author: yuhsienchang
 *
 * Created on June 6, 2024, 9:25 AM
 */

#ifndef NORMALIZE_H
#define	NORMALIZE_H

#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>



typedef struct aa {
    uint8_t ADC_CHANNEL_F1_n;/**<F1 diode data>*/
    uint8_t ADC_CHANNEL_F2_n;/**<F2 diode data>*/
    uint8_t ADC_CHANNEL_F3_n;/**<F3 diode data>*/
    uint8_t ADC_CHANNEL_F4_n;/**<F4 diode data>*/
    uint8_t ADC_CHANNEL_CLEAR_n;/**<clear diode data>*/
    uint8_t ADC_CHANNEL_NIR_n;/**<NIR diode data>*/
} Normalize__model_t;


//
//
uint16_t find_min(uint16_t data[], uint16_t length);
uint16_t find_max(uint16_t data[], uint16_t length);

typedef struct nor {
    uint16_t max;
    uint16_t min;
    uint8_t min_index;
} Normalize_info_t;


float normalize(uint16_t value, uint16_t min_val, uint16_t max_val);
//
//void calculate_normalize(uint16_t data[], uint8_t normalized_data[], uint8_t *max_index);
void calculate_normalize(uint16_t data[], uint16_t normalized_data[], uint8_t *max_index);














#endif	/* NORMALIZE_H */

