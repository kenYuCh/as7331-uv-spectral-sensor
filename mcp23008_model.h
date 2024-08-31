/* 
 * File:   LED_control.h
 * Author: ken
 *
 * Created on May 23, 2024, 11:56 PM
 * 
 * 
 */

#ifndef LED_CONTROL_H
#define	LED_CONTROL_H

#include <stdbool.h>
#include <stdint.h>

// MCP23008 - CONFIGURATION AND CONTROL REGISTERS
#define MCP23008_SLAVE_I2C_ADDRESS  0x04
#define MCP23008_ADDRESS 0x20       // 
#define MCP23008_IODIR_REG 0x00     // Controls the direction of the data I/O [IO7:IO0]. default is 1111111 (input)
#define MCP23008_IPOL_REG 0x01      // If a bit is set, the corresponding GPIO register bit will reflect the inverted value on the pin.
#define MCP23008_GPINTEN_REG 0x02   // controls the interrupt-onchange feature for each pin.
#define MCP23008_DEFVAL_REG 0x03    // 
#define MCP23008_INTCON_REG 0x04    // 
#define MCP23008_IOCON_REG 0x05     // 
#define MCP23008_GPPU_REG 0x06      // 
#define MCP23008_INTF_REG 0x07      // 
#define MCP23008_INTCAP_REG 0x08    // 
#define MCP23008_GPIO_REG 0x09      // 
#define MCP23008_OLAT 0x0A          // 

//
typedef enum {
    RED = 3,    // GP3
    BLUE = 4,  // GP4
    GREEN = 5  //GP5
} LED;

typedef enum {
    low_level = 0,
    hight_level = 1
} LED_state;

//#define STATE_IDLE 0
//#define STATE_LED_ON 1
//#define STATE_LED_OFF 2
//#define STATE_MEAS 3

typedef enum {
    STATE_IDLE,
    STATE_LED_ON,
    STATE_MEAS,
    STATE_READ_DATA,
    STATE_LED_OFF
} State_t;

//void state_machine(State_t *state);

void mcp23008_init();
void mcp23008_set_gpio_direct(uint8_t gpio_direction);
void enable_LED(LED led, bool state);

void mcp23008_Write(uint8_t reg, uint8_t data);
uint8_t mcp23008_Read(uint8_t reg);

uint8_t measure_and_led_process(uint8_t led_gpio);


uint8_t process_led_type(uint8_t normalize_value);
void mcp23008_Write_LED(uint8_t reg, uint8_t data);
uint8_t mcp23008_Read_LED(uint8_t reg);
#endif	/* LED_CONTROL_H */

