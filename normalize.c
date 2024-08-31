
#include "normalize.h"
#include <stdint.h>



uint16_t find_min(uint16_t data[], uint16_t length) {
    uint16_t min_val = data[0];
    for (uint16_t i = 1; i < length; i++) {
        if (data[i] < min_val) {
            min_val = data[i];
        }
    }
    return min_val;
}


uint16_t find_max(uint16_t data[], uint16_t length) {
    uint16_t max_val = data[0];
    for (uint16_t i = 1; i < length; i++) {
        if (data[i] > max_val) {
            max_val = data[i];
            
        }
    }
    return max_val;
}

float normalize(uint16_t value, uint16_t min_val, uint16_t max_val) {
    if (max_val == min_val) return 0; 
    return (float)(value - min_val) / (max_val - min_val);
}
//
//void calculate_normalize(uint16_t data[], uint8_t normalized_data[], uint8_t *max_index){
//    *max_index = 0;
//    uint16_t min = find_min(data, 4);
//    printf("find_min: %u \r\n", min);
//    uint16_t max = find_max(data, 4);
//    printf("find_max: %u \r\n", max);
//    
//    uint8_t max_normalized_value = 0;
//    
//    for(uint8_t i=0 ; i<4 ; i++) {
//        normalized_data[i] = (uint8_t)(normalize(data[i], min, max)*10);
//        if (normalized_data[i] > max_normalized_value) {
//           max_normalized_value = normalized_data[i];
//           *max_index = i;
//        }
//    };
//    
////    nor_model->ADC_CHANNEL_F1_n = normalized_data[0];
////    nor_model->ADC_CHANNEL_F2_n = normalized_data[1];
////    nor_model->ADC_CHANNEL_F3_n = normalized_data[2];
////    nor_model->ADC_CHANNEL_F4_n = normalized_data[3];
//
//};


void calculate_normalize(uint16_t data[], uint16_t normalized_data[], uint8_t *max_index){
    *max_index = 0;
    uint16_t min = find_min(data, 4);
    printf("find_min: %u \r\n", min);
    uint16_t max = find_max(data, 4);
    printf("find_max: %u \r\n", max);
    
    uint8_t max_normalized_value = 0;
    
    for(uint8_t i=0 ; i<4 ; i++) {
        normalized_data[i] = data[i];
        if (data[i] > max_normalized_value) {
           max_normalized_value = normalized_data[i];
           *max_index = i;
        }
    };
    
//    nor_model->ADC_CHANNEL_F1_n = normalized_data[0];
//    nor_model->ADC_CHANNEL_F2_n = normalized_data[1];
//    nor_model->ADC_CHANNEL_F3_n = normalized_data[2];
//    nor_model->ADC_CHANNEL_F4_n = normalized_data[3];

};