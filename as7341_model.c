
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "configuration_bits.h"
#include "i2c_master.h"
#include "as7341_model.h"
#include "mcp23008_model.h"
#include "mcc_generated_files/mcc.h"

as7341_gpio_mode_t gpio_mode;

uint8_t as7341_begin(eMode_t mode) {
    printf("AS734 initializing...\r\n");
    if(read_ID() == ERR_DATA_BUS){
        return ERR_DATA_BUS;
    };
    __delay_ms(200);
    printf("AS734 successfully initialized !  \r\n");
    enableAS7341(true); // Power
    __delay_ms(50);
    measurement_mode = mode;
    setting_as734_parameter();
    return ERR_OK;
};

void setting_mcp23008_parameter() {
    mcp23008_init();
    mcp23008_set_gpio_direct(0x00);
    mcp23008_Write_LED(0x09, 0xFF);
    printf("MCP23008 Setting ... successful! \r\n");
};
/* Ken Note
 If you have set all the gold parameters correctly but there is no data when you start measuring, 
 it may be due to sensitivity issues. Try adjusting the ATIME, ASTEP, AGAIN parameters.
*/
void setting_as734_parameter() {
//    enable_wait(true);
//    set_WTIME(50);
    //Integration time = (ATIME + 1) x (ASTEP + 1) x 2.78µs
    //Set the value of register ATIME, through which the value of Integration time can be calculated. The value represents the time that must be spent during data reading.
    // Longer integration times provide higher spectral resolution but may reduce sensor response.
    set_ATIME(50); // 100
    //Set the value of register ASTEP, through which the value of Integration time can be calculated. The value represents the time that must be spent during data reading.
    // The integration step determines how long the sensor takes during data reading. This value affects the frequency and accuracy of data collection in the sensor.
    set_ASTEP(599);   // 599
    //Set gain value(0~10 corresponds to X0.5,X1,X2,X4,X8,X16,X32,X64,X128,X256,X512)
    // The gain setting affects the sensor's sensitivity and measurement range.
    set_AGAIN(AS7341_GAIN_64X); // 7 spectral sensitivity.
    set_threshold(0, 10000);
    enable_smux_interrupt(true);
    enable__sys_int(true);
//    enable_flicker_detection(true);

//    set_gpio(true);
    
    //Enable LED
//    enable_led(true);
    // Set pin current to control brightness (1~20 corresponds to current 4mA,6mA,8mA,10mA,12mA,......,42mA)
//    control_led(10);
};

/* REG_AS7341_ID(0x92) */
uint8_t read_ID() {
    uint8_t as7341_id = as7341_i2c_read(AS7341_ID);
    printf("AS7341 Modules default 0x00, But Simugro AS7341 Read ID: %u !\r\n", as7341_id);
    if(as7341_id == 0x00) {
        printf("Could not find AS7341, Try to find again !\r\n");
        return ERR_DATA_BUS;
    };
    printf("Yes, AS7341 was successfully found! \r\n");
    return ERR_OK;
};

/* chip Power ON.
 * 0: AS7341 disabled.
 * 1: AS7341 enabled.
 * Note: When bit is set, internal oscillator is activated,
 *       allowing timers and ADC channels to operate..
 */
void enableAS7341(bool state) {
    uint8_t data = as7341_i2c_read(AS7341_ENABLE);
    if(state == true) {
        data = data | (1<<0);
    } else {
//        data = data & (~1);
        data = data & (~(1 << 0));
    };
    as7341_i2c_write(AS7341_ENABLE, data);
    
};


/* SP_EN, bit[1]
 * Spectral Measurement Enable.
 * 0: Spectral Measurement Enabled.
 * 1: Spectral Measurement Disabled.
 */
void enable_spetral_measurement(bool state) {
    uint8_t data = as7341_i2c_read(AS7341_ENABLE);
    if(state == true) {
        data = data | (1<<1);
    } else {
        data = data & (~(1<<1));
    };
    as7341_i2c_write(AS7341_ENABLE, data);
};


/* enable WAIAT, bit[3]
 * 0: Wait time between two consecutive spectral.
 * measurements disabled.
 * 1: Wait time between two consecutive spectral.
 * measurements enabled.
 */
void enable_wait(bool state) {
    uint8_t data = as7341_i2c_read(AS7341_ENABLE);
    if(state == true){
      data = data | (1<<3);
    } else {
      data = data & (~(1<<3));
    };
    as7341_i2c_write(AS7341_ENABLE, data);
};

/* 1: Starts SMUX command bit[4]
 * Note: this bit gets cleared automatically as soon as SMUX operation is finished.
 */
void enable_smux(bool state) {
    uint8_t data = as7341_i2c_read(AS7341_ENABLE);
//    printf("enable_smux_byte: %x \r\n", data);
    if(state == true){
        data = data | (1<<4);
    } else {
      data = data & (~(1<<4));
    };
    as7341_i2c_write(AS7341_ENABLE, data); 
};

bool get_enable_smux() {
    bool isEnabled = false;
    uint8_t data = as7341_i2c_read(AS7341_ENABLE);
//    data = (data & 0x10) >> 4;
    if ( (data & 0x10) == 0x10) { // byte[5] = smux.
      return isEnabled = true;
    } else {
      return isEnabled = false;
    };
};

void enable_flicker_detection(bool state) {
    uint8_t data = as7341_i2c_read(AS7341_ENABLE);
    if(state == true){
        data = data | (1<<6);
    } else {
        data = data & (~(1<<6));
    };
    as7341_i2c_write(AS7341_ENABLE, data);
};




/*
INT_MODE (Integration mode): 0x70 
 * 0x00: SPM mode
 * 0x01: SYNS mode
 * 0x03: SYND mode
*/
void config(eMode_t mode) {
    set_bank(1);
    uint8_t data = as7341_i2c_read(AS7341_CONFIG);
    switch(mode){
        case SPM : {
            data = (data & (~(1<<3))) | SPM;
          };
          break;
          case SYNS : {
            data = (data & (~(1<<3))) | SYNS;
          };
          break;
          case SYND : {
            data = (data & (~(1<<3))) | SYND;
          };
          break;
          default : break;
        };
    as7341_i2c_write(AS7341_CONFIG, data);
//    uint8_t data = as7341_i2c_read(AS7341_CONFIG);
//    printf("MEAS_MODE: %u \r\n", data);
    set_bank(0);
};


void set_F1F4_Clear_NIR() {
    // first  zero may be clean all adc
    uint8_t data[20] ={
        0x30,0x01,0x00,0x00,0x00,
        0x42,0x00,0x00,0x50,0x00,
        0x00,0x00,0x20,0x04,0x00,
        0x30,0x01,0x50,0x00,0x06
    };
    for(uint8_t i = 0; i<20; i++) {
        as7341_i2c_write(i, data[i]);
        __delay_ms(1);
    };
};

void set_F5F8_Clear_NIR() {
    uint8_t data[20] ={
        0x00,0x00,0x00,0x40,0x02,
        0x00,0x10,0x03,0x50,0x10,
        0x03,0x00,0x00,0x00,0x24,
        0x00,0x00,0x50,0x00,0x06
    };
    for(uint8_t i = 0; i<20; i++) {
        as7341_i2c_write(i, data[i]);
        __delay_ms(1);
    };
};

void set_FDConfig() {
    uint8_t data[20] ={
        0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,
        0x03,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00
    };
    for(uint8_t i = 0; i<20; i++) {
        as7341_i2c_write(i, data[i]);
        __delay_ms(1);
    };
};

uint8_t start_measure(channel_t channel) {
    bool isEnabled = true;
    bool isDataReady = false;
    
//    set_bank(0);
    enable_spetral_measurement(false);

    if(measurement_mode == SYNS){
        printf("measurement_mode: SYNS \r\n");
        config(SYNS);
    } else if(measurement_mode == SPM){
        printf("measurement_mode: SPM \r\n");
        config(SPM);
    };

    set_smux_command(AS7341_SMUX_CMD_WRITE);
    
//    set_smux_command(AS7341_SMUX_CMD_WRITE); // 0xAF 0x10
    if(channel  == F1F4ClearNIR) {
        printf("measurement_channel: F1F4ClearNIR \r\n");
        set_F1F4_Clear_NIR(); //
    } else if(channel == F5F8ClearNIR) { 
        printf("measurement_channel: F5F8ClearNIR \r\n");
        set_F5F8_Clear_NIR(); //
    };
    
    enable_smux(true);
    
    while (isEnabled) {
        isEnabled = get_enable_smux();
        __delay_ms(50);
//        printf("get_enable_smux \r\n");
    };

    enable_spetral_measurement(true);
    
    while (!(isDataReady)) {
        isDataReady = getIsDataReady();
        __delay_ms(50);
//        printf("getIsDataReady \r\n");
    };
    
    clear_interrupt();
    return 0;
};

void auto_GAIN() {
    uint8_t regVal = as7341_i2c_read(0xB1);
    regVal = regVal & 0xFB;
    regVal = regVal | 0x04;
    as7341_i2c_write(0xB1, regVal);
};

uint8_t read_flicker_data() {
    uint8_t flicker;
    enable_spetral_measurement(false);
    set_smux_command(AS7341_SMUX_CMD_WRITE);
    set_FDConfig();
    enable_smux(true);
    enable_spetral_measurement(true);
    uint8_t retry = 100;
    if(retry == 0) {
        printf("data access error");
    };
    enable_flicker_detection(true);
    __delay_ms(700);
    flicker = as7341_i2c_read(AS7341_STATUS);
    enable_flicker_detection(false);
    switch(flicker){
      case 44:
        flicker = 1;
        break;
      case 45:
        flicker = 50;
        break;
      case 46:
        flicker = 60;
        break;
      default:
        flicker = 0;
    }
    return flicker;
};

uint16_t get_channel_data(uint8_t channel) {
    uint8_t data[2];
    uint16_t channelData = 0x0000;
    data[0] = as7341_i2c_read(AS7341_CH0_DATA_L + channel*2);
    data[1] = as7341_i2c_read(AS7341_CH0_DATA_H + channel*2);
    channelData = data[1];
    channelData = (channelData<<8) | data[0];
    return channelData;
};

F1F4_CLEAR_NIR_DATA_t read_spectral_data_one() {
    F1F4_CLEAR_NIR_DATA_t data;
    data.ADC_CHANNEL_F1 = get_channel_data(0);
    data.ADC_CHANNEL_F2 = get_channel_data(1);
    data.ADC_CHANNEL_F3 = get_channel_data(2);
    data.ADC_CHANNEL_F4 = get_channel_data(3);
    return data;
};

F5F8_CLEAR_NIR_DATA_t read_spectral_data_two() {
    F5F8_CLEAR_NIR_DATA_t data;
    data.ADC_CHANNEL_F5 = get_channel_data(0);
    data.ADC_CHANNEL_F6 = get_channel_data(1);
    data.ADC_CHANNEL_F7 = get_channel_data(2);
    data.ADC_CHANNEL_F8 = get_channel_data(3);
    data.ADC_CHANNEL_CLEAR = get_channel_data(4);
    data.ADC_CHANNEL_NIR = get_channel_data(5);
    return data;
};

bool getIsDataReady() {
    bool isDataReady = false;
    uint8_t regVal = as7341_i2c_read(AS7341_STATUS2); //0xA3
    if ( (regVal & 0x40) == 0x40) {
      return isDataReady = true;
    } else {
      return isDataReady = false; 
    };
};




void set_gpio(bool connect){
    uint8_t data = as7341_i2c_read(AS7341_GPIO);
    if(connect == true) {
        data = set_bit_to_1(data, bitwise_OR_mask_bit1);
    } else {
        data = set_bit_to_0(data, bitwise_AND_mask_bit1);
    }
     as7341_i2c_write(AS7341_GPIO, data);
}

void set_gpio_mode(uint8_t mode) {
    uint8_t data = as7341_i2c_read(AS7341_GPIO2);
    if(mode == 1) {    // input
        data = set_bit_to_1(data, bitwise_OR_mask_bit2);
    } else if(mode == 0){ // output
        data = set_bit_to_0(data, bitwise_AND_mask_bit2);
    };
    as7341_i2c_write(AS7341_GPIO2, data);
};

void enable_led(bool state) {
    uint8_t data=0;
    uint8_t data1=0;
    set_bank(1);
    data = as7341_i2c_read(AS7341_CONFIG);
    data1 = as7341_i2c_read(AS7341_LED);
    if(state == true){
      data = data | (1<<3);
      data1 = data1 | (1<<7);
    } else {
      data = data & (~(1<<3));
      data1 = data1 & (~(1<<7));
    };
    as7341_i2c_write(AS7341_CONFIG, data);
    as7341_i2c_write(AS7341_LED, data1);
    set_bank(0);
};




void set_bank(uint8_t reg) {
    uint8_t data = 0;
    data = as7341_i2c_read(AS7341_CFG0);
    if(reg == 1){ // write 0x66 ~0x74
        data = data | (1<<4); // 
    };
    if(reg == 0) { // write 0x80 and above
        data = data & (~(1<<4));
    };
    as7341_i2c_write(AS7341_CFG0, data);
};


void control_led(uint8_t current) {
    uint8_t data=0; 
    if(current < 1) current = 1;
      current--;
    if(current > 19) current = 19;
    set_bank(1);
    data = 0;
    data = data | (1<<7);           // 0x90
    data = data | (current & 0x7F); // current 0x90
    as7341_i2c_write(AS7341_LED, 0x84); 
    __delay_ms(100);
    set_bank(0);
};

// ----------------------------------------------------------------
void set_smux_command(uint8_t command) {
  as7341_i2c_write(AS7341_CFG6, 0x10);
};
// System Interrupt SMUX Operation.
void enable_smux_interrupt(bool state) {
  uint8_t data = as7341_i2c_read(AS7341_CFG9);
  if(state == true){
    data = data | (1<<4);
  } else {
    data = data & (~(1<<4));
  };
  as7341_i2c_write(AS7341_CFG9, data);
};


void set_int(bool connect) {
    uint8_t data;
    data = as7341_i2c_read(AS7341_GPIO);
    if(connect == true){
      data = data | (1<<1);
    } else {
      data = data & (~(1<<1));
    };
    as7341_i2c_write(AS7341_GPIO, data);
};

void enable__sys_int(bool state) {
  uint8_t data = as7341_i2c_read(AS7341_INTENAB);
  if(state == true){
    data = data | (1<<0);
  } else {
    data = data & (~(1<<0));
  };
  as7341_i2c_write(AS7341_INTENAB, data);
}

void enable_fifo_int(bool state) {
    uint8_t data = as7341_i2c_read(AS7341_INTENAB);
    if(state == true){
        data = data | (1<<2);
    } else {
        data = data & (~(1<<2));
    };
    as7341_i2c_write(AS7341_INTENAB, data);
}

void enable_spectral_int(bool state) {
  uint8_t data = as7341_i2c_read(AS7341_INTENAB);
  if(state == true){
    data = data | (1<<3);
  } else {
    data = data & (~(1<<3));
  };
  as7341_i2c_write(AS7341_INTENAB, data);
}


/*
 * 0x80 0x01
 * b2 10
 * f9 01
 * af 10
 * ~~~~
 * 0x80 11
 * 0x80 00
 enable__sys_int
 smux_interrupt
 */

// as soon as smux command has finished interrupt is activate.
void enable_measure_interrupt() {

};
//
void end_sleep() {
  uint8_t data = as7341_i2c_read(AS7341_INTENAB);
  data = data | (1<<3);
  as7341_i2c_write(AS7341_INTENAB, data);
}
//
void clear_FIFO() {
  uint8_t data = as7341_i2c_read(AS7341_CONTROL);
  data = data | (1<<0);
  data = data & (~(1<<0));
  as7341_i2c_write(AS7341_CONTROL, data);
}

void spectral_auto_zero() {
  uint8_t data = as7341_i2c_read(AS7341_CONTROL);
  data = data | (1<<1);
  as7341_i2c_write(AS7341_CONTROL, data);
}

void enable_flicker_int(bool state) {
  uint8_t data = as7341_i2c_read(AS7341_INTENAB);
  if(state == true){
    data = data | (1<<2);
  } else {
    data = data & (~(1<<2));
  };
  as7341_i2c_write(AS7341_INTENAB, data);
}

void set_ATIME(uint8_t value) {
  as7341_i2c_write(AS7341_ATIME, value);
}

void set_AGAIN(uint8_t value) {
  if(value > 10) value = 10;
  as7341_i2c_write(AS7341_CFG1, value);
}

void set_ASTEP(uint16_t value) {
  uint8_t highValue, lowValue;
  lowValue = value & 0x00FF;
  highValue = value >> 8 ;
  as7341_i2c_write(AS7341_ASTEP_L, lowValue);
  as7341_i2c_write(AS7341_ASTEP_H, highValue);
};


float get_integration_time() {

    uint8_t data;
    uint8_t astepData[2]={0};
    uint16_t astep;
    data = as7341_i2c_read(AS7341_ATIME);
    astepData[0] = as7341_i2c_read(AS7341_ASTEP_L);
    astepData[1] = as7341_i2c_read(AS7341_ASTEP_L);
    astep = astepData[1];
    astep = (astep << 8) | astepData[0];
    if(data == 0) { 
        return astep; // test
    } else if(data > 0 && data < 255) {
        return astep; // test
    } else if(data == 255){
        return astep; // test
    };
    return 0;
};

void set_WTIME(uint8_t value) {
    as7341_i2c_write(AS7341_WTIME, value);
};

float get_WTIME() {
    float value;
    uint8_t data = as7341_i2c_read(AS7341_WTIME);
    if(data == 0) {
      value = 2.78;
    } else if(data == 1) {
      value = 5.56;
    } else if(data >1 && data <255) {
      value = 2.78*(data+1);
    }  else if(data == 255) {
      value = 711;
    };
    return value;
};

void set_threshold(uint16_t lowTh, uint16_t highTh) {
    if(lowTh >= highTh) return;
    as7341_i2c_write(AS7341_SP_HIGH_TH_H, highTh >> 8);
    as7341_i2c_write(AS7341_SP_HIGH_TH_L, highTh);
    as7341_i2c_write(AS7341_SP_LOW_TH_H, lowTh >> 8);
    as7341_i2c_write(AS7341_SP_LOW_TH_L, lowTh);
    __delay_ms(20);
};

uint16_t get_low_threshold() {
    uint16_t data;
    uint8_t data_high, data_low;
    data_high = as7341_i2c_read(AS7341_SP_LOW_TH_H);
    data_low = as7341_i2c_read(AS7341_SP_LOW_TH_L);
    data = (uint16_t) data_high << 8 | data_low;
    return data;
};

uint16_t get_high_Threshold() {
    uint16_t data;
    uint8_t data_high, data_low;
    data_high = as7341_i2c_read(AS7341_SP_HIGH_TH_H);
    data_low = as7341_i2c_read(AS7341_SP_HIGH_TH_L);
    data = (uint16_t) data_high << 8 | data_low;
    return data;
};

void clear_interrupt() {
  as7341_i2c_write(AS7341_STATUS, 0xFF);
}

void enable_spectral_interrupt(bool state) {
    uint8_t data;
    data = as7341_i2c_read(AS7341_INTENAB);
    if (state == true) {
      data = data | (1<<3);
      as7341_i2c_write(AS7341_INTENAB, data);
    } else {
      data = data & (~(1<<3));
      as7341_i2c_write(AS7341_INTENAB, data);
    };
}

void set_int_channel(uint8_t channel) {
    if(channel >= 5) return;
    uint8_t data;
    data = as7341_i2c_read(AS7341_CFG12);
    data = data & (~(7));
    data = data | (channel);
    as7341_i2c_write(AS7341_CFG12, data);
};

void set_APERS(uint8_t num) {
    uint8_t data;
    data = as7341_i2c_read(AS7341_PERS);
    data = data & (~(15));
    data = data | (num);
    as7341_i2c_write(AS7341_PERS, data);
}

uint8_t get_int_source() {
    return as7341_i2c_read(AS7341_STATUS3);
}

bool as7341_interrupt() {
    uint8_t data = as7341_i2c_read(AS7341_STATUS);
    if(data & 0x80){
      return true;
    } else {
      return false;
    };
};

bool check_WTIME() {
    uint8_t status;
    uint8_t value;
    status = as7341_i2c_read(AS7341_STATUS6);
    value = status & (1<<2);
    if(value > 0){
      return false;
    } else {
      return  true;
    };
}


uint8_t as7341_i2c_write(uint8_t reg, uint8_t value) {
    uint8_t data;
    I2C_Start();
    I2C_Write(AS7341_SLAVE_ADDRESS << 1); // slave address + write,
    I2C_Write(reg); 
    data = I2C_Write(value);
    I2C_Stop();
    if(data == true) {
        return 1;
    } else {
        return 0;
    };
    
};

uint8_t as7341_i2c_read(uint8_t reg) {
    uint8_t buf;
    I2C_Start();
    I2C_Write(AS7341_SLAVE_ADDRESS << 1);
    I2C_Write(reg);
    I2C_Start();
    I2C_Write((AS7341_SLAVE_ADDRESS << 1) | 0x01);
    buf = I2C_Read(1); // nack 1, ack 0
    I2C_Stop();
    return buf;
};

void as7341_write_byte(uint8_t reg, uint8_t value) {
    I2C_Write(reg); 
    I2C_Write(value); 
};


// 0 -> 1
uint8_t set_bit_to_1(uint8_t byte, Bitwise_OR mask_bit) {
    return byte | mask_bit;
};
// 1 -> 0
uint8_t set_bit_to_0(uint8_t byte, Bitwise_AND mask_bit) {
    return byte & mask_bit;
};



