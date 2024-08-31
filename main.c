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
#include "as7331_model.h"
#include "mcp23008_model.h"
#include "normalize.h"
#include "ticks.h"
#include <time.h>

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
    None,
    STATE_IDLE_t,
    STATE_MEASURE_AFTER_t,
    STATE_MEASURE_ING_t,
    STATE_MEASURE_COMPLETE_t,
    STATE_READ_DATA_t,
    STATE_DARK_t,
    STATE_LIGHT_t
} Spectral_state;

Spectral_state spectral_state = STATE_IDLE_t;
static uint32_t lastI2cTicksStatemachine = 0;

uint8_t running_complete = 0;


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
    
    
    F1F4_CLEAR_NIR_DATA_t data1;
    F5F8_CLEAR_NIR_DATA_t data2;
    UV_DATA_t uv_data;
    
    printf("Hello Spectral-Light-Sensor\r\n");
    printf("sn: %s\r\n", DEVICE_SERIAL);
//    setting_mcp23008_parameter();
//    as7341_begin(SPM);

    
    int8_t state = rand() % 250 + 1;
    
    // AS7331 setting
    MMODE mmode = AS7331_CONT_MODE;
    CCLK  cclk  = AS7331_1024;
    uint8_t sb    = 0x01;
    uint8_t breakTime = 40;  
    uint8_t gain = 8;
    uint8_t time = 9;
    bool AS7331_Ready_flag = false;
    uint16_t status = 0;
    volatile bool alarmFlag = false;
    
    as7331_enable_power(true);
    as7331_reset();
    __delay_ms(100); 
    
    uint8_t AS7331_ID = as7331_get_chip_id();
    printf("AS7331_ID: 0x%02x \r\n", AS7331_ID);
    
    if(AS7331_ID == 0x21) {
        printf("AS7331 is online \r\n");
        as7331_set_configuration_mode();
        as7331_init(mmode, cclk, sb, breakTime, gain, time);
        as7331_set_measure_mode();
    } else {
        if(AS7331_ID != 0x21) printf("AS7331 not functioning! \r\n");
        while(1){}; // wait here forever for a power cycle
    }
    status = get_status();
    printf("AS7331 is status: 0x%x \r\n" ,status);
    
    
    while (1){
        uint32_t now = get_ticks_with_lock();
        static uint32_t lastI2cTicksLed = 0;
        if(calculate_diff(now, lastI2cTicksLed) >= 1000){
            lastI2cTicksLed = now;
            status = get_status();
            printf("AS7331 is status: 0x%x \r\n" ,status);
            // 
            // Error handling
            if(status & 0x0080) printf("overflow of internal time reference! \r\n");
            if(status & 0x0040) printf("overflow of measurement register(s)!");
            if(status & 0x0020) printf("overflow of internal conversion channel(s)! \r\n");
            if(status & 0x0010) printf("measurement results overwritten! \r\n");
            if(status & 0x0004) printf("measurement in progress! \r\n");
            if(status & 0x0008) {
                uv_data = read_all_data();
            }
            

//            state_machine(spectral_state); // unuse
//            start_measure(F1F4ClearNIR);
//            __delay_ms(100);
//            data1 = read_spectral_data_one();
//            __delay_ms(100);
//            start_measure(F5F8ClearNIR);
//             __delay_ms(100);
//            data2 = read_spectral_data_two();
//            __delay_ms(100);
        };
        
        static uint32_t prev_ticks4 = 0;
        if(calculate_diff(now, prev_ticks4) >= 3000) {
            prev_ticks4 = now;
            state = rand() % 250 + 1;
            INTERRUPT_GlobalInterruptDisable();
            i2c_slave_set_register(0, data1.ADC_CHANNEL_F1 >> 8);
            i2c_slave_set_register(1, data1.ADC_CHANNEL_F1 & 0xFF);
            i2c_slave_set_register(2, data1.ADC_CHANNEL_F2 >> 8);
            i2c_slave_set_register(3, data1.ADC_CHANNEL_F2 & 0xFF);
            i2c_slave_set_register(4, data1.ADC_CHANNEL_F3 >> 8);
            i2c_slave_set_register(5, data1.ADC_CHANNEL_F3 & 0xFF);
            i2c_slave_set_register(6, data1.ADC_CHANNEL_F4 >> 8);
            i2c_slave_set_register(7, data1.ADC_CHANNEL_F4 & 0xFF);
            i2c_slave_set_register(8, data2.ADC_CHANNEL_F5 >> 8);
            i2c_slave_set_register(9, data2.ADC_CHANNEL_F5 & 0xFF);
            i2c_slave_set_register(10, data2.ADC_CHANNEL_F6 >> 8);
            i2c_slave_set_register(11, data2.ADC_CHANNEL_F6 & 0xFF);
            i2c_slave_set_register(12, data2.ADC_CHANNEL_F7 >> 8);
            i2c_slave_set_register(13, data2.ADC_CHANNEL_F7 & 0xFF);
            i2c_slave_set_register(14, data2.ADC_CHANNEL_F8 >> 8);
            i2c_slave_set_register(15, data2.ADC_CHANNEL_F8 & 0xFF);
            i2c_slave_set_register(16, data2.ADC_CHANNEL_CLEAR >> 8);
            i2c_slave_set_register(17, data2.ADC_CHANNEL_CLEAR & 0xFF);
            i2c_slave_set_register(18, data2.ADC_CHANNEL_NIR >> 8);
            i2c_slave_set_register(19, data2.ADC_CHANNEL_NIR & 0xFF);
            //
            i2c_slave_set_register(20, uv_data.UVA_data >> 8);
            i2c_slave_set_register(21, uv_data.UVA_data & 0xFF);
            i2c_slave_set_register(22, uv_data.UVB_data >> 8);
            i2c_slave_set_register(23, uv_data.UVB_data & 0xFF);
            i2c_slave_set_register(24, uv_data.UVC_data >> 8);
            i2c_slave_set_register(25, uv_data.UVC_data & 0xFF);
            
        
            i2c_slave_set_register(26, state);

            for (uint8_t i = 0; i < 5; i++) {
                char buffer[3];
                buffer[0] = DEVICE_SERIAL[i * 2];
                buffer[1] = DEVICE_SERIAL[i * 2 + 1];
                buffer[2] = '\0';
                uint8_t value = strtol(buffer, NULL, 16);
                i2c_slave_set_register(57u + i, value);
            };
            INTERRUPT_GlobalInterruptEnable();
            dump_memory_map();  
        }
        
        static uint32_t prev_ticks1 = 0;
        if(calculate_diff(now, prev_ticks1) >= 5000) {
            prev_ticks1 = now;
            printf("UVA:            %5d \r\n"
                   "UVB:            %5d \r\n"
                   "UVC:            %5d \r\n",
                    uv_data.UVA_data,
                    uv_data.UVB_data,
                    uv_data.UVC_data
            );
            printf("Channe-F1(415nm - Violet):          %5d \r\n"
                   "Channe-F2(445nm - Blue):            %5d \r\n"
                   "Channe-F3(480nm - Cyan):            %5d \r\n"
                   "Channe-F4(515nm - Green):           %5d \r\n",
                   data1.ADC_CHANNEL_F1,
                   data1.ADC_CHANNEL_F2,
                   data1.ADC_CHANNEL_F3,
                   data1.ADC_CHANNEL_F4
            );
            printf("Channe-F5(555nm - Yellow Green):    %5d \r\n"
                   "Channe-F6(590nm - Yellow):          %5d \r\n"
                   "Channe-F7(630nm - Orange):          %5d \r\n"
                   "Channe-F8(680nm - Red):             %5d \r\n" 
                   "Channe-CLEAR (FULL SPECTRUM WAVE):  %5d \r\n"
                   "Channe-NIR(910nm - Near IR):        %5d \r\n",

               data2.ADC_CHANNEL_F5,
               data2.ADC_CHANNEL_F6,
               data2.ADC_CHANNEL_F7,
               data2.ADC_CHANNEL_F8,
               data2.ADC_CHANNEL_CLEAR,
               data2.ADC_CHANNEL_NIR
            );
            
        };

        
//        static uint32_t prev_ticks_restart = 0;
//        if(calculate_diff(now, prev_ticks_restart) >= 10000) { //3600000  every 1 hour then pic restart
//            prev_ticks_restart = now;
//            if(running_complete == 1) {
//                break;
//            };
//        };
    };
    
    
}


/*
 * Ken Note
 */

// 
