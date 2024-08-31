/* 
 * File:   as7331_model.h
 * Author: ken
 *
 * Created on June 28, 2024, 8:47 PM
 */

#ifndef AS7331_MODEL_H
#define	AS7331_MODEL_H
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>


// Configuration State registers
#define AS7331_OSR                      0x00
#define AS7331_AGEN                     0x02  // should be 0x21
#define AS7331_CREG1                    0x06   
#define AS7331_CREG2                    0x07
#define AS7331_CREG3                    0x08
#define AS7331_BREAK                    0x09
#define AS7331_EDGES                    0x0A
#define AS7331_OPTREG                   0x0B

// Measurement State registers
#define AS7331_STATUS                   0x00
#define AS7331_TEMP                     0x01
#define AS7331_MRES1                    0x02
#define AS7331_MRES2                    0x03
#define AS7331_MRES3                    0x04
#define AS7331_OUTCONVL                 0x05
#define AS7331_OUTCONVH                 0x06


#define AS7331_SLAVE_ADDRESS  0x74  // if A0 = A1 = LOW


typedef enum {
  AS7331_CONT_MODE                = 0x00, // continuous mode
  AS7331_CMD_MODE                 = 0x01, // force mode, one-time measurement
  AS7331_SYNS_MODE                = 0x02,
  AS7331_SYND_MODE                = 0x03
} MMODE;


typedef enum  {
  AS7331_1024           = 0x00, // internal clock frequency, 1.024 MHz, etc
  AS7331_2048           = 0x01,
  AS7331_4096           = 0x02,
  AS7331_8192           = 0x03
} CCLK;

typedef struct uv_data {
    uint16_t UVA_data;
    uint16_t UVB_data;
    uint16_t UVC_data;    
    uint16_t temp;
} UV_DATA_t;


void as7331_init(uint8_t mmode, uint8_t cclk, uint8_t sb, uint8_t breakTime, uint8_t gain, uint8_t time);
uint8_t as7331_get_chip_id();
void as7331_reset();

void as7331_enable_power(bool state);

void as7331_set_measure_mode();
void as7331_set_configuration_mode();
void as7331_start_measure();

uint16_t read_temperature();

UV_DATA_t read_all_data();

void as7331_one_shot();

uint16_t get_status();

uint8_t as7331_i2c_write(uint8_t reg, uint8_t value);
uint8_t as7331_i2c_read(uint8_t reg);

uint16_t as7331_i2c_read2Bytes(uint8_t reg);
#endif	/* AS7331_MODEL_H */

