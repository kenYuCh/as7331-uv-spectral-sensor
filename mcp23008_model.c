

#include <stdbool.h>
#include "mcp23008_model.h"
#include "i2c_master.h"
#include "ticks.h"
#include "as7341_model.h"
#include "normalize.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

void mcp23008_init() {
    I2C_Start();
    I2C_Write(MCP23008_ADDRESS << 1); // write MCP23008 address
    I2C_Write(0x00); // register set to 0x00?GPIO direct register.
    I2C_Write(0x00); // Set all pins as outputs.
    I2C_Stop();
    
};

// Set output pin status to 00000000 (0x00)
void mcp23008_set_gpio_direct(uint8_t gpio_direction) {
    I2C_Start();
    I2C_Write(MCP23008_ADDRESS <<  1);   // write MCP23008 address
    I2C_Write(MCP23008_IODIR_REG);                    // register 0x09?GPIO output register
    I2C_Write(gpio_direction);            // set output state. 
    I2C_Stop();
};

void mcp23008_Write(uint8_t reg, uint8_t data) {
    I2C_Start();
    I2C_Write(MCP23008_ADDRESS << 1);   // write MCP23008 address
    I2C_Write(reg);                    // register 0x09?GPIO output register
    I2C_Write(data);            // set output state. 
    I2C_Stop();
};
char mcp23008_Read(uint8_t reg) {
    char data;
    I2C_Start();
    I2C_Write(MCP23008_ADDRESS << 1); 
    I2C_Write(reg); 
    I2C_Start();
    I2C_Write((MCP23008_ADDRESS << 1) | 1);
    data = I2C_Read(1); 
    I2C_Stop();
    return data;
};

void mcp23008_Write_LED(uint8_t reg, uint8_t data) {
    GIE = 0;
    mcp23008_Write(reg, data);
    GIE = 1;
}

uint8_t mcp23008_Read_LED(uint8_t reg) {
    GIE = 0;
    uint8_t led = mcp23008_Read(reg);
    GIE = 1;
    return led;
}
void enable_LED(LED led, bool state) {
    I2C_Start();
    I2C_Write(MCP23008_ADDRESS << 1);   // write MCP23008 address
    I2C_Write(MCP23008_GPIO_REG);       // register 0x09?GPIO output register
    if(state) {
        I2C_Write(1 << led);            // set output state. 
    } else {
        I2C_Write(0x00);            // set output state. 
    };
    I2C_Stop();
};


static uint32_t currentTicks = 0;
static uint32_t currentTicks_led = 0;
uint8_t led_state = 0;






uint8_t currentState = STATE_IDLE;
uint8_t measure_and_led_process(uint8_t led_gpio) {
    uint8_t trigger = 1;
    uint8_t outputValue = 0;
    uint8_t led = process_led_type(led_gpio);
    
    switch (currentState) {
        case STATE_IDLE:
            if (trigger) {
                currentTicks = ticks;
                currentState = STATE_LED_ON;
            };
            break;
        case STATE_LED_ON:
            if ((ticks - currentTicks) >= 200) {
                currentTicks = ticks;
                printf("STATE_LED_ON Write: %02X \r\n", led);
                mcp23008_Write_LED(0x09, led);
                currentState = STATE_MEAS;
//                uint8_t led_state = mcp23008_Read(0x09);
//                if( led_state == led ) {
//                     printf("STATE_LED_ON: %02X \r\n", led_state);
//                     currentState = STATE_MEAS;
//                }
            };
            break;
//            while(led_st != led) {
//                    if ((ticks - currentTicks_led) >= 50) {
//                        currentTicks_led = ticks;
//                        mcp23008_Read(0x09);
//                        uint8_t st = mcp23008_Read(0x09);
//                        if(st == led) {
//                            break;
//                        };
//                    };
//            };
//            if ((ticks - currentTicks) >= 100) {
//                currentTicks = ticks;
//                printf("STATE_LED_ON \r\n");
//                mcp23008_Write_LED(0x09, led);
//                currentState = STATE_MEAS;
//            }
//            break;
        case STATE_MEAS:
            if ((ticks - currentTicks) >= 500) {
                currentTicks = ticks;
                outputValue = 1;
                printf("STATE_MEAS \r\n");
                start_measure(F1F4ClearNIR);
                currentState = STATE_LED_OFF;
            }
            break;
        
        case STATE_LED_OFF:

//            uint8_t led_st = mcp23008_Read(0x09);
//            while(led_st != 0xFF) {
//                if ((ticks - currentTicks_led) >= 50) {
//                    currentTicks_led = ticks;
//                    mcp23008_Read(0x09);
//                    uint8_t st = mcp23008_Read(0x09);
//                    if(st == 0xFF) {
//                        break;
//                    };
//                };
//            };
            if ((ticks - currentTicks) >= 2000) {
                currentTicks = ticks;
                outputValue = 0;            
                printf("STATE_LED_OFF \r\n");
                mcp23008_Write_LED(0x09, 0xFF);
                currentState = STATE_IDLE;
            };
            break;
    }

    return outputValue;

}


uint8_t process_led_type(uint8_t normalize_index){
    
    uint8_t output_value = 0;
    
    switch (normalize_index) {
        case 1: // PURPLE
            output_value = 0xFE;
            break;
        case 2: // 
            output_value = 0xFE;
            break;
        case 3: // GREEN test
            output_value = 0xFE;
            break;
        case 4: // RED 
            output_value = 0xFB; // or 
            break;
        case 5: // CLEAR 
            output_value = 0xE7; // or 
            break;
        case 6: // CLEAR 
            output_value = 0xFF; // or 
            break;    
    };
    return output_value;
};

/*
 
uint8_t measure_and_led_process(uint8_t led_gpio) {
    uint8_t trigger = 1;
    uint8_t outputValue = 0;
    uint8_t led = process_led_type(led_gpio);
    
    switch (currentState) {
        case STATE_IDLE: {
                currentTicks = ticks;
                currentState = STATE_LED_ON;
            }
            break;
        case STATE_LED_ON: {
                uint8_t led_st = mcp23008_Read(0x09);
                while(led_st != led) {
                    if ((ticks - currentTicks_led) >= 50) {
                        currentTicks_led = ticks;
                        mcp23008_Write_LED(0x09, led);
                        uint8_t st = mcp23008_Read(0x09);
                        if(st == led) {
                            currentState = STATE_MEAS; 
                            break;
                        };
                    };
                };
         
            }
            break;
//            if ((ticks - currentTicks) >= 100) {
//                uint8_t led_st = mcp23008_Read(0x09);
//                currentTicks = ticks;
//                printf("STATE_LED_ON \r\n");
//                mcp23008_Write_LED(0x09, led);
//                currentState = STATE_MEAS;
//            }
            
        case STATE_MEAS:
            if ((ticks - currentTicks) >= 200) {
                currentTicks = ticks;
                outputValue = 1;
                printf("STATE_MEAS \r\n");
                start_measure(F1F4ClearNIR);
                currentState = STATE_LED_OFF;
            }
            break;
        case STATE_LED_OFF: {
            uint8_t led_st = mcp23008_Read(0x09);
            while(led_st != 0xFF) {
                if ((ticks - currentTicks_led) >= 50) {
                    currentTicks_led = ticks;
                    uint8_t st = mcp23008_Read(0x09);
                    if(st == 0xFF) {
                        currentState = STATE_IDLE;
                        break;
                    };
                };
            };
            break;
        }

            
//            if ((ticks - currentTicks) >= 100) {
//                currentTicks = ticks;
//                outputValue = 0;            
//                printf("STATE_LED_OFF \r\n");
//                mcp23008_Write_LED(0x09, 0xFF);
//                currentState = STATE_IDLE;
//            };
           
    }

    return outputValue;

}
 
 */