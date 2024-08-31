/* 
 * File:   as7331_model.c
 * Author: ken
 *
 * Created on June 28, 2024, 8:46 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "mcc_generated_files/mcc.h"
#include "configuration_bits.h"
#include "i2c_master.h"
#include "as7331_model.h"

void as7331_init(uint8_t mmode, uint8_t cclk, uint8_t sb, uint8_t breakTime, uint8_t gain, uint8_t time) {
    as7331_i2c_write(AS7331_CREG1,  gain << 4 |  time);
    as7331_i2c_write(AS7331_CREG3,  mmode << 6 | sb << 4 | cclk);
    as7331_i2c_write(AS7331_BREAK,  breakTime);
   
};

uint8_t as7331_get_chip_id() {
    return as7331_i2c_read(AS7331_AGEN);
};

void as7331_reset() {
    uint8_t result = as7331_i2c_read(AS7331_OSR);
    as7331_i2c_write(AS7331_OSR, result | 0x08);
};

void as7331_enable_power(bool state){
    uint8_t result = as7331_i2c_read(AS7331_OSR);
    uint8_t data;
    if(state == true) { // power up
        data = result | 0x40;    //  set bit 6
    } else { // power down
        data = result & ~(0x40); //  clear bit 6
    };
    as7331_i2c_write(AS7331_OSR, data);
};

void as7331_set_measure_mode() {
    uint8_t result = as7331_i2c_read(AS7331_OSR);
    as7331_i2c_write(AS7331_OSR, result | 0x83); // set bit 1 for configuration mode
};

void as7331_set_configuration_mode() {
    uint8_t result = as7331_i2c_read(AS7331_CREG3);
    as7331_i2c_write(AS7331_CREG3, result | 0x02); // set bit 1 for configuration mode
};

void as7331_one_shot() {
    uint8_t result = as7331_i2c_read(AS7331_OSR);
    as7331_i2c_write(AS7331_OSR, result | 0x80); // set bit 7 for forced one-time measurement
}



uint16_t read_temperature() {
    uint16_t data = as7331_i2c_read2Bytes(AS7331_TEMP);
    return data;
};


uint16_t readUVAData() {
  uint16_t data = as7331_i2c_read2Bytes(AS7331_MRES1);
  return data;
 }


uint16_t readUVBData() {
    uint16_t data = as7331_i2c_read2Bytes(AS7331_MRES2);
    return data;
}


uint16_t readUVCData() {
    uint16_t data = as7331_i2c_read2Bytes(AS7331_MRES3);
    return data;
 }

UV_DATA_t read_all_data() {
    UV_DATA_t dest;
    dest.temp =  read_temperature();
    dest.UVA_data  = readUVAData();
    dest.UVB_data  = readUVBData();
    dest.UVC_data  = readUVCData();
//   dest.UVC_data  =  (uint16_t)(((uint16_t)rawData[7]) << 8 | rawData[6]);
   return dest;
};

uint16_t get_status() {
    uint16_t data = as7331_i2c_read2Bytes(AS7331_STATUS);
    uint16_t swapped_data = (data >> 8) | (data << 8);
    return swapped_data;
};

uint8_t as7331_i2c_write(uint8_t reg, uint8_t value) {
    uint8_t data;
    I2C_Start();
    I2C_Write(AS7331_SLAVE_ADDRESS << 1); // slave address + write,
    I2C_Write(reg); 
    data = I2C_Write(value);
    I2C_Stop();
    if(data == true) {
        return 1;
    } else {
        return 0;
    };
    
};

uint8_t as7331_i2c_read(uint8_t reg) {
    uint8_t buf;
    I2C_Start();
    I2C_Write(AS7331_SLAVE_ADDRESS << 1);
    I2C_Write(reg);
    I2C_Start();
    I2C_Write((AS7331_SLAVE_ADDRESS << 1) | 0x01);
    buf = I2C_Read(1); // nack 1, ack 0
    I2C_Stop();
    return buf;
};

uint16_t as7331_i2c_read2Bytes(uint8_t reg) {
    uint8_t data[2];
    uint16_t data_u16;
    I2C_Start();
    I2C_Write(AS7331_SLAVE_ADDRESS << 1);
    I2C_Write(reg);
    I2C_Start();
    I2C_Write((AS7331_SLAVE_ADDRESS << 1) | 0x01);
    data[0] = I2C_Read(0);
    data[1] = I2C_Read(1);
    data_u16 = (data[1] << 8) | data[0];
    I2C_Stop();
    return data_u16;
};
