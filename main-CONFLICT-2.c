#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "mcc_generated_files/mcc.h"
#include "configuration_bits.h"
#include "configuration.h"
#include "tmr0.h"
#include "my_helpers.h"
#include "i2c_master.h"
#include "i2c_slave.h"
#include "eusart1.h"
#include "i2c_master.h"
#include "as7341_model.h"
#include "mcp23008_model.h"
#include "normalize.h"
#include "ticks.h"

#define INTERRUPT_GlobalInterruptEnable() (INTCONbits.GIE = 1)
#define INTERRUPT_GlobalInterruptDisable() (INTCONbits.GIE = 0)
#define INTERRUPT_PeripheralInterruptEnable() (INTCONbits.PEIE = 1)
#define INTERRUPT_PeripheralInterruptDisable() (INTCONbits.PEIE = 0)

#define set_RA5_as_UART_TX()              (RA5PPS = 0x0F)
#define reset_RA5_peripheral_pin_select() (RA5PPS = 0x00)


void __interrupt() INTERRUPT_InterruptManager() {
    // timer0 interrupt
    if (PIR0bits.TMR0IF) {
        PIR0bits.TMR0IF = 0;
        ticks++;

    }
    
    // I2C slave interrupt
    if (PIR3bits.SSP1IF == 1) {
        PIR3bits.SSP1IF = 0;
        i2c_slave_interrupt();
    }
}

typedef enum {
    STATE_IDLE_t,
    STATE_MEASURE_AFTER_t,
    STATE_MEASURE_ING_t,
    STATE_MEASURE_COMPLETE_t,
    STATE_READ_DATA_t
} Spectral_state;


void state_machine(Spectral_state now_state) {
    switch(now_state) {
    
    };
};



void main() {


    SYSTEM_Initialize();
    tmr0_init();
    EUSART1_Initialize();
    __delay_ms(500);

    reset_RA5_peripheral_pin_select();
    set_RA5_as_UART_TX();

    INTERRUPT_GlobalInterruptEnable();
    INTERRUPT_PeripheralInterruptEnable();

    i2c_master_init();
    i2c_slave_init();
    
    printf("Hello Spectral-Light-Sensor\r\n");
    printf("sn: %s\r\n", DEVICE_SERIAL);
    setting_mcp23008_parameter();
    as7341_begin(SPM);

    
    uint8_t led_state = 0;
    uint8_t measuring = 0;
    uint8_t dark = 0;
    
    Spectral_state spectral_state = STATE_IDLE_t;
    F1F4_CLEAR_NIR_DATA_t data1;
    while (1){
        
        uint32_t now = get_ticks_with_lock();
        
        static uint32_t lastI2cTicksLed = 0;
        if(calculate_diff(now, lastI2cTicksLed) >= 300){
            lastI2cTicksLed = now;
            state_machine(spectral_state);
            
        };
        static uint32_t lastI2cTicks = 0;
        if(calculate_diff(now, lastI2cTicks) >= 3000){
            lastI2cTicks = now;
            
                if (dark == 1) {
                    if(led_state !=0xFB) {
                        uint8_t led_type = process_led_type(4); //red 0xFB
                        mcp23008_Write_LED(0x09, led_type);
                    };
                } else {
                    if(led_state != 0xFE) {
                        uint8_t led_type = process_led_type(3); //red 0xFB
                        mcp23008_Write_LED(0x09, led_type);
                    };
                };
            
            
            
            spectral_state = STATE_MEASURE_ING_t;
            __delay_ms(100);
            start_measure(F1F4ClearNIR);
            __delay_ms(200);
            data1 = read_spectral_data_one();
            __delay_ms(100);
            led_state = mcp23008_Read_LED(0x09);
            if (data1.ADC_CHANNEL_CLEAR <= 100) {
                dark = 1;
            } else {
                dark = 0;
            };
            spectral_state = STATE_MEASURE_COMPLETE_t;
            
        }

        static uint32_t prev_ticks1 = 0;
        if(calculate_diff(now, prev_ticks1) >= 4000) {
            prev_ticks1 = now;
            printf("Channel-F1(405-425nm): %u \r\n"
                   "Channel-F2(435-455nm): %u \r\n"
                   "Channel-F3(470-490nm): %u \r\n"
                   "Channel-F4(505-525nm): %u \r\n"
                   "Channel-CLEAR: %u \r\n",
//                   "Channel-NIR: %u \r\n",
                   data1.ADC_CHANNEL_F1,
                   data1.ADC_CHANNEL_F2,
                   data1.ADC_CHANNEL_F3,
                   data1.ADC_CHANNEL_F4,
                   data1.ADC_CHANNEL_CLEAR
//                   data1.ADC_CHANNEL_NIR
            );
            
//            __delay_ms(100);
//            printf("Normalization: \r\n");
//            uint16_t nor_data_array[4];
//            nor_data_array[0] = data1.ADC_CHANNEL_F1;
//            nor_data_array[1] = data1.ADC_CHANNEL_F2;
//            nor_data_array[2] = data1.ADC_CHANNEL_F3;
//            nor_data_array[3] = data1.ADC_CHANNEL_F4;
            
//            __delay_ms(100);
//            uint8_t normalized_data[4];
//            uint8_t normalized_data_max_index;
//            calculate_normalize(nor_data_array, normalized_data, &normalized_data_max_index);
//            
//            LED_type = normalized_data_max_index+1;
//            printf("Index of the maximum normalized value: %d\n", normalized_data_max_index+1);
//            printf("normalized_data - F%i: %d.%d \r\n", 1, normalized_data[0]/10, normalized_data[0]%10);
//            printf("normalized_data - F%i: %d.%d \r\n", 2, normalized_data[1]/10, normalized_data[1]%10);
//            printf("normalized_data - F%i: %d.%d \r\n", 3, normalized_data[2]/10, normalized_data[2]%10);
//            printf("normalized_data - F%i: %d.%d \r\n", 4, normalized_data[3]/10, normalized_data[3]%10);
//            
////            
            
            // ---------------------------------------------------
            INTERRUPT_GlobalInterruptDisable();
            i2c_slave_set_register(0, data1.ADC_CHANNEL_F1 >> 8);
            i2c_slave_set_register(1, data1.ADC_CHANNEL_F1 & 0xFF);
            i2c_slave_set_register(2, data1.ADC_CHANNEL_F2 >> 8);
            i2c_slave_set_register(3, data1.ADC_CHANNEL_F2 & 0xFF);
            i2c_slave_set_register(4, data1.ADC_CHANNEL_F3 >> 8);
            i2c_slave_set_register(5, data1.ADC_CHANNEL_F3 & 0xFF);
            i2c_slave_set_register(6, data1.ADC_CHANNEL_F4 >> 8);
            i2c_slave_set_register(7, data1.ADC_CHANNEL_F4 & 0xFF);
            i2c_slave_set_register(8, data1.ADC_CHANNEL_CLEAR >> 8);
            i2c_slave_set_register(9, data1.ADC_CHANNEL_CLEAR & 0xFF);
            
            for (uint8_t i = 0; i < 5; i++) {
                char buffer[3];
                buffer[0] = DEVICE_SERIAL[i * 2];
                buffer[1] = DEVICE_SERIAL[i * 2 + 1];
                buffer[2] = '\0';
                uint8_t value = strtol(buffer, NULL, 16);
                i2c_slave_set_register(57u + i, value);
            };
            INTERRUPT_GlobalInterruptEnable();
            //dump_memory_map();  
            
        };
        static uint32_t prev_ticks_restart = 0;
        if(calculate_diff(now, prev_ticks_restart) >= 1800000) { //3600000  every 1 hour then pic restart
            prev_ticks_restart = now;
            break;
        };
    };
}


/*
 * Ken Note
 */

// 
