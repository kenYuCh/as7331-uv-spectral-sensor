
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
//    if(read_ID() == ERR_DATA_BUS){
//        return ERR_DATA_BUS;
//    };
    __delay_ms(200);
    printf("AS734 successfully initialized !  \r\n");

    enableAS7341(true);
    __delay_ms(200);
//    as7341_i2c_write(AS7341_CFG0, 0x00);
    
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

void setting_as734_parameter() {
//    enable_wait(true);
////    set_WTIME(50);
    //Integration time = (ATIME + 1) x (ASTEP + 1) x 2.78µs
    //Set the value of register ATIME, through which the value of Integration time can be calculated. The value represents the time that must be spent during data reading.
    set_ATIME(10); // 100
    //Set the value of register ASTEP, through which the value of Integration time can be calculated. The value represents the time that must be spent during data reading.
    set_ASTEP(599);   // 599
    //Set gain value(0~10 corresponds to X0.5,X1,X2,X4,X8,X16,X32,X64,X128,X256,X512)
    set_AGAIN(AS7341_GAIN_128X); // 7 spectral sensitivity.
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

// PON, bit[0]
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

void enable_spetral_measurement(bool state) {
    uint8_t data = as7341_i2c_read(AS7341_ENABLE);
    if(state == true) {
        data = data | (1<<1);
    } else {
        data = data & (~(1<<1));
    };
    as7341_i2c_write(AS7341_ENABLE, data);
};

void enable_wait(bool state) {
    uint8_t data = as7341_i2c_read(AS7341_ENABLE);
    if(state == true){
      data = data | (1<<3);
    } else {
      data = data & (~(1<<3));
    };
    as7341_i2c_write(AS7341_ENABLE, data);
};

/*
 
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
    data = (data & 0x10) >> 4;
    if (data == 1) { // byte[5] = smux.
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
    uint8_t data = as7341_i2c_read(AS7341_CONFIG);
//    printf("MEAS_MODE: %u \r\n", data);
    set_bank(0);
};


void set_F1F4_Clear_NIR() {
//    I2C_Start();
//    I2C_Write(AS7341_SLAVE_ADDRESS << 1);
    as7341_i2c_write(0x00, 0x30); // F3 left set to ADC2
    as7341_i2c_write(0x01, 0x01); // F1 left set to ADC0
    as7341_i2c_write(0x02, 0x00); // Reserved or disabled
    as7341_i2c_write(0x03, 0x00); // F8 left disabled
    as7341_i2c_write(0x04, 0x00); // F6 left disabled
    
    as7341_i2c_write(0x05, 0x42); // F4 left connected to ADC3/f2 left connected to ADC1
    as7341_i2c_write(0x06, 0x00); // F5 left disbled
    as7341_i2c_write(0x07, 0x00); // F7 left disbled
    as7341_i2c_write(0x08, 0x50); // CLEAR connected to ADC4
    as7341_i2c_write(0x09, 0x00); // F5 right disabled
    
    as7341_i2c_write(0x0A, 0x00); // F7 right disabled
    as7341_i2c_write(0x0B, 0x00); // Reserved or disabled
    as7341_i2c_write(0x0C, 0x20); // F2 right connected to ADC1
    as7341_i2c_write(0x0D, 0x04); // F4 right connected to ADC3
    as7341_i2c_write(0x0E, 0x00); // F6/F8 right disabled
    
    as7341_i2c_write(0x0F, 0x30); // F3 right connected to AD2
    as7341_i2c_write(0x10, 0x01); // F1 right connected to AD0
    as7341_i2c_write(0x11, 0x50); // CLEAR right connected to AD4
    as7341_i2c_write(0x12, 0x00); // Reserved or disabled
    as7341_i2c_write(0x13, 0x06); // NIR connected to ADC5
    
//    I2C_Stop();
};

void set_F5F8_Clear_NIR() {
    I2C_Start();
    I2C_Write(AS7341_SLAVE_ADDRESS << 1);
    as7341_write_byte(0x00, 0x00); // F3 left disable
    as7341_write_byte(0x01, 0x00); // F1 left disable
    as7341_write_byte(0x02, 0x00); // reserved/disable
    as7341_write_byte(0x03, 0x40); // F8 left connected to ADC3
    as7341_write_byte(0x04, 0x02); // F6 left connected to ADC1
    
    as7341_write_byte(0x05, 0x00); // F4/ F2 disabled
    as7341_write_byte(0x06, 0x10); // F5 left connected to ADC0
    as7341_write_byte(0x07, 0x03); // F7 left connected to ADC2
    as7341_write_byte(0x08, 0x50); // CLEAR Connected to ADC4
    as7341_write_byte(0x09, 0x10); // F5 right connected to ADC0
    
    as7341_write_byte(0x0A, 0x03); // F7 right connected to ADC2
    as7341_write_byte(0x0B, 0x00); // Reserved or disabled
    as7341_write_byte(0x0C, 0x00); // F2 right disabled
    as7341_write_byte(0x0D, 0x00); // F4 right disabled
    as7341_write_byte(0x0E, 0x24); // F8 right connected to ADC2/ F6 right connected to ADC1
    
    as7341_write_byte(0x0F, 0x00); // F3 right disabled
    as7341_write_byte(0x10, 0x00); // F1 right disabled
    as7341_write_byte(0x11, 0x50); // CLEAR right connected to AD4
    as7341_write_byte(0x12, 0x00); // Reserved or disabled
    as7341_write_byte(0x13, 0x06); // NIR connected to ADC5
    I2C_Stop();
};

void set_FDConfig() {
    I2C_Start();
    I2C_Write(AS7341_SLAVE_ADDRESS << 1);
    as7341_i2c_write(0x00, 0x00); // F3 left disable
    as7341_i2c_write(0x01, 0x00); // F1 left disable
    as7341_i2c_write(0x02, 0x00); // reserved/disable
    as7341_i2c_write(0x03, 0x00); // F8 left connected to ADC3
    as7341_i2c_write(0x04, 0x00); // F6 left connected to ADC1
    as7341_i2c_write(0x05, 0x00); // F4/ F2 disabled
    as7341_i2c_write(0x06, 0x00); // F5 left connected to ADC0
    as7341_i2c_write(0x07, 0x00); // F7 left connected to ADC2
    as7341_i2c_write(0x08, 0x00); // CLEAR Connected to ADC4
    as7341_i2c_write(0x09, 0x00); // F5 right connected to ADC0
    as7341_i2c_write(0x0A, 0x00); // F7 right connected to ADC2
    as7341_i2c_write(0x0B, 0x00); // Reserved or disabled
    as7341_i2c_write(0x0C, 0x00); // F2 right disabled
    as7341_i2c_write(0x0D, 0x00); // F4 right disabled
    as7341_i2c_write(0x0E, 0x00); // F8 right connected to ADC2/ F6 right connected to ADC1
    as7341_i2c_write(0x0F, 0x00); // F3 right disabled
    as7341_i2c_write(0x10, 0x00); // F1 right disabled
    as7341_i2c_write(0x11, 0x00); // CLEAR right connected to AD4
    as7341_i2c_write(0x12, 0x00); // Reserved or disabled
    as7341_i2c_write(0x13, 0x60); // NIR connected to ADC5
    I2C_Stop();
};

uint8_t start_measure(channel_t channel) {
    set_bank(0);
    enable_spetral_measurement(false);
    smux_config_RAM();
    __delay_ms(100);
    if(channel  == F1F4ClearNIR) {
        printf("measurement_channel: F1F4ClearNIR \r\n");
        set_F1F4_Clear_NIR(); //
    } else if(channel == F5F8ClearNIR) { 
        printf("measurement_channel: F5F8ClearNIR \r\n");
        set_F5F8_Clear_NIR(); //
    };

//    printf("get_enable_smux: %u \r\n", get_enable_smux());
    enable_smux(true);
//    printf("get_enable_smux: %u \r\n", get_enable_smux());
    __delay_ms(100);
    if(measurement_mode == SYNS){
        printf("measurement_mode: SYNS \r\n");
        set_gpio_mode(1);       // 1: INPUT, 0: OUTPUT
        config(SYNS);
    } else if(measurement_mode == SPM){
        printf("measurement_mode: SPM \r\n");
        config(SPM);
    };
//    __delay_ms(100);
    enable_spetral_measurement(true);
//    if(measurement_mode == SPM){
//        printf("Measuring...");
//        while(!measure_complete()){
//          __delay_ms(1000);
//        };
//        printf("measurement completed! ");
//    };
//    __delay_ms(100);
    
    //uint16_t d = get_integration_time() * 100;
    //printf("get_integration_time: %u \r\n", d);
    return 0;
};

bool measure_complete(){
//    uint8_t status = as7341_i2c_read(AS7341_STATUS2);
//    if((status & (~(1<<6)))) {
//      printf("measurement state! %u \r\n", status);
//      return true;
//    } else{
//      return false;
//    };
    return true;
};

uint8_t read_flicker_data() {
    uint8_t flicker;
    set_bank(0);
    enable_spetral_measurement(false);
    smux_config_RAM();
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
    __delay_ms(10);
//    printf("Hex CH_%u L: 0x%02X, H: 0x%02X \r\n", channel, data[0], data[1]);
//    uint8_t data2[10];
//    uint16_t ch_1_data = 0;
//    uint16_t ch_2_data = 0;
//    uint16_t ch_3_data = 0;
//    uint16_t ch_4_data = 0;
//    uint16_t ch_5_data = 0;
//    data2[0] = as7341_i2c_read(AS7341_CH0_DATA_L); // 0x95
//    data2[1] = as7341_i2c_read(AS7341_CH0_DATA_H);
//    data2[2] = as7341_i2c_read(AS7341_CH1_DATA_L); // 0x97
//    data2[3] = as7341_i2c_read(AS7341_CH1_DATA_H);
//    data2[4] = as7341_i2c_read(AS7341_CH2_DATA_L); // 0x99
//    data2[5] = as7341_i2c_read(AS7341_CH2_DATA_H);
//    data2[6] = as7341_i2c_read(AS7341_CH3_DATA_L); // 0x9B
//    data2[7] = as7341_i2c_read(AS7341_CH3_DATA_H);
//    data2[8] = as7341_i2c_read(AS7341_CH4_DATA_L); // 0x9D
//    data2[9] = as7341_i2c_read(AS7341_CH4_DATA_H);
//    ch_1_data = (uint16_t)(data2[1] << 8) | (uint16_t)(data2[0] << 0);
//    ch_2_data = (uint16_t)(data2[3] << 8) | (uint16_t)(data2[2] << 0);
//    ch_3_data = (uint16_t)(data2[5] << 8) | (uint16_t)(data2[4] << 0);
//    ch_4_data = (uint16_t)(data2[7] << 8) | (uint16_t)(data2[6] << 0);
//    ch_5_data = (uint16_t)(data2[9] << 8) | (uint16_t)(data2[8] << 0);
//    printf("CH_L/H: \r\n"
//            "CH0_L %2d \r\n"
//            "CH0_H %2d \r\n"
//            "CH1_L %2d \r\n"
//            "CH1_H %2d \r\n"
//            "CH2_L %2d \r\n"
//            "CH2_H %2d \r\n"
//            "CH3_L %2d \r\n"
//            "CH3_H %2d \r\n"
//            "CH4_L %2d \r\n"
//            "CH4_H %2d \r\n"
//            "CH5_L %2d \r\n"
//            "CH5_H %2d \r\n",
//            data2[0],
//            data2[1],
//            data2[2],
//            data2[3],
//            data2[4],
//            data2[5],
//            data2[6],
//            data2[7],
//            data2[8],
//            data2[9]
//            );
//    printf( "ch1_data: %u \r\n"
//            "ch2_data: %u \r\n"
//            "ch3_data: %u \r\n"
//            "ch4_data: %u \r\n",
//                ch_1_data, ch_2_data, ch_3_data, ch_4_data
//            );
    return channelData;
};


F1F4_CLEAR_NIR_DATA_t read_spectral_data_one() {
    F1F4_CLEAR_NIR_DATA_t data;
//    printf("read_spectral_data_one....\r\n");
    data.ADC_CHANNEL_F1 = get_channel_data(0);
    data.ADC_CHANNEL_F2 = get_channel_data(1);
    data.ADC_CHANNEL_F3 = get_channel_data(2);
    data.ADC_CHANNEL_F4 = get_channel_data(3);
    data.ADC_CHANNEL_CLEAR = get_channel_data(4);
    data.ADC_CHANNEL_NIR = get_channel_data(5);
    
    return data;
};
F5F8_CLEAR_NIR_DATA_t read_spectral_data_two() {
    F5F8_CLEAR_NIR_DATA_t data;
    data.ADC_CHANNEL_F5 = get_channel_data(0);
    data.ADC_CHANNEL_F6 = get_channel_data(1);
    data.ADC_CHANNEL_F7 = get_channel_data(2);
    data.ADC_CHANNEL_F8 = get_channel_data(3);
//    data.ADC_CHANNEL_CLEAR = get_channel_data(4);
//    data.ADC_CHANNEL_NIR = get_channel_data(5);
    return data;
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
//      data = data | (1<<3);
//      data1 = data1 | (1<<7);
        data = set_bit_to_1(data, bitwise_OR_mask_bit3);
        data1 = set_bit_to_1(data1, bitwise_OR_mask_bit7);
    } else {
//      data = data & (~(1<<3));
//      data1 = data1 & (~(1<<7));
        data = set_bit_to_0(data, bitwise_AND_mask_bit3);
        data1 = set_bit_to_0(data1, bitwise_AND_mask_bit7);
    };
    as7341_i2c_write(AS7341_CONFIG, data);
    as7341_i2c_write(AS7341_LED, data1);
    set_bank(0);
};




void set_bank(uint8_t reg) {
    uint8_t data = 0;
    data = as7341_i2c_read(AS7341_CFG0);
    if(reg == 1){ // write 0x66 ~0x74
//        data = set_bit_to_1(data, bitwise_OR_mask_bit4);
        data = data | (1<<4); // 
//        data = 0x50; // 
    };
    if(reg == 0) { // write 0x80 and above
        data = data & (~(1<<4));
//        data = 0x40;
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
void smux_config_RAM() {
  as7341_i2c_write(AS7341_CFG6, 0x10);
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




void printf_marquee_dot() {
    for(uint8_t i=0; i<5, i++;) {
        printf(".");
        __delay_ms(1000); // 100000 ???0.1 ????
    };
    printf("\r\n");
};



// 0 -> 1
uint8_t set_bit_to_1(uint8_t byte, Bitwise_OR mask_bit) {
    return byte | mask_bit;
};
// 1 -> 0
uint8_t set_bit_to_0(uint8_t byte, Bitwise_AND mask_bit) {
    return byte & mask_bit;
};



