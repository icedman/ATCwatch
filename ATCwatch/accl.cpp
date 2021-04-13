/*
   Copyright (c) 2020 Aaron Christophel

   SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "accl.h"
#include "accl_config.h"
#include "Arduino.h"
#include "i2c.h"
#include "pinout.h"
#include "watchdog.h"
#include "inputoutput.h"
#include "ble.h"
#include "sleep.h"

struct accl_data_struct accl_data;

#ifdef USE_SC7A20

#define LOW_POWER
#include "lis3dh-motion-detection.h"
// #include "Wire.h"

uint16_t sampleRate = 10;  // HZ - Samples per second - 1, 10, 25, 50, 100, 200, 400, 1600, 5000
uint8_t accelRange = 2;   // Accelerometer range = 2, 4, 8, 16g

#ifdef COUNT_STEPS
#include "count_steps.h"
#define ACCUM_LENGTH (NUM_TUPLES * 3)
int8_t accum[ACCUM_LENGTH];
int8_t accumIndex = 0;
#endif

LIS3DH myIMU(0x18); //Default address is 0x19.

uint16_t errorsAndWarnings = 0;

void init_accl() {

    if( myIMU.begin(sampleRate, 1, 1, 1, accelRange) != 0 )
    {
        // Serial.print("Failed to initialize IMU.\n");
    }

    //Detection threshold can be from 1 to 127 and depends on the Range
    //chosen above, change it and test accordingly to your application
    //Duration = timeDur x Seconds / sampleRate
    myIMU.intConf(INT_1, DET_MOVE, 13, 2);
    myIMU.intConf(INT_2, DET_STOP, 13, 10, 1);  // also change the polarity to active-low, this will change both Interrupts behavior

    uint8_t readData = 0;

    // Confirm configuration:
    myIMU.readRegister(&readData, LIS3DH_INT1_CFG);
    myIMU.readRegister(&readData, LIS3DH_INT2_CFG);

    // Get the ID:
    // myIMU.readRegister(&readData, LIS3DH_WHO_AM_I);
    myIMU.readRegister(&readData, 0x11);
}

void accl_config_read_write(bool rw, uint8_t addr, uint8_t *data, uint32_t len, uint32_t offset)
{
}

void reset_accl() {
}

void reset_step_counter() {
    accl_data.steps = 0;
}

uint32_t read_step_data() {
    return accl_data.steps;
}

int last_y_acc = 0;
bool acc_input() {
    // update_accl_data();

    if ((accl_data.x + 335) <= 670 && accl_data.z < 0) {
        if (!get_sleep()) {
            if (accl_data.y <= 0) {
                return false;
            } else {
                last_y_acc = 0;
                return false;
            }
        }
        if (accl_data.y >= 0) {
            last_y_acc = 0;
            return false;
        }
        if (accl_data.y + 230 < last_y_acc) {
            last_y_acc = accl_data.y;
            return true;
        }
    }
    return false;
}

bool get_is_looked_at() {
//   update_accl_data();
    if ((accl_data.y + 300) <= 600 && (accl_data.x + 300) <= 600 && accl_data.z < 100)
        return true;
    return false;
}

accl_data_struct get_accl_data() {
    // update_accl_data();
    return accl_data;
}

int8_t accum_skip = 0;
void update_accl_data() {
    int16_t dataHighres = 0;

    if( myIMU.readRegisterInt16( &dataHighres, LIS3DH_OUT_X_L ) != 0 )
    {
        errorsAndWarnings++;
    }
    accl_data.y = (dataHighres / 0x10);

    if( myIMU.readRegisterInt16( &dataHighres, LIS3DH_OUT_Y_L ) != 0 )
    {
        errorsAndWarnings++;
    }
    accl_data.x = (dataHighres / 0x10);

    if( myIMU.readRegisterInt16( &dataHighres, LIS3DH_OUT_Z_L ) != 0 )
    {
        errorsAndWarnings++;
    }
    accl_data.z = (dataHighres / 0x10);

#ifdef COUNT_STEPS
    if (accum_skip++ > 0) {
        accum_skip = 0;
        accum[accumIndex++] = ((float)accl_data.x / 1000) * 127;
        accum[accumIndex++] = ((float)accl_data.y / 1000) * 127;
        accum[accumIndex++] = ((float)accl_data.z / 1000) * 127;
        if (accumIndex >= ACCUM_LENGTH) {
            accl_data.steps += count_steps(accum);
            accumIndex = 0;
        }
    }
#endif

}

void accl_write_reg(uint8_t reg, uint8_t data) {
}

uint8_t accl_read_reg(uint8_t reg) {
    return 0;
}


#else
// BMA421_INT

void init_accl() {
    pinMode(BMA421_INT, INPUT);
    reset_accl();
    delay(10);
    accl_write_reg(0x7C, 0x00);//Sleep disable
    delay(1);
    accl_write_reg(0x59, 0x00);//Write Blob
    accl_config_read_write(0, 0x5E, (uint8_t *)accl_config_file, sizeof(accl_config_file), 0);
    accl_write_reg(0x59, 0x01);
    delay(10);
    accl_write_reg(0x7D, 0x04);//Accl Enable
    accl_write_reg(0x40, 0b00101000);//Acc Conf
    reset_step_counter();//Enable and Reset
    accl_write_reg(0x7C, 0x03);//Sleep Enable
}

void accl_config_read_write(bool rw, uint8_t addr, uint8_t *data, uint32_t len, uint32_t offset)
{
    for (int i = 0; i < len; i += 16) {
        accl_write_reg(0x5B, (offset + (i / 2)) & 0x0F);
        accl_write_reg(0x5C, (offset + (i / 2)) >> 4);
        if (rw)
            user_i2c_read(0x18, 0x5E, data + i, (len - i >= 16) ? 16 : (len - i));
        else
            user_i2c_write(0x18, 0x5E, data + i, (len - i >= 16) ? 16 : (len - i));
    }
}

void reset_accl() {
    accl_write_reg(0x7E, 0xB6);
}

void reset_step_counter() {
    uint8_t feature_config[0x47] = { 0 };
    accl_write_reg(0x7C, 0x00);//Sleep disable
    delay(1);
    accl_config_read_write(1, 0x5E, feature_config, 0x46, 256);
    feature_config[0x3A + 1] = 0x34;
    accl_config_read_write(0, 0x5E, feature_config, 0x46, 256);
    accl_write_reg(0x7C, 0x03);//Sleep Enable
}

uint32_t read_step_data() {
    uint32_t data;
    user_i2c_read(0x18, 0x1E, (uint8_t *)&data, 4);
    return data;
}

int last_y_acc = 0;
bool acc_input() {
    update_accl_data();
    if ((accl_data.x + 335) <= 670 && accl_data.z < 0) {
        if (!get_sleep()) {
            if (accl_data.y <= 0) {
                return false;
            } else {
                last_y_acc = 0;
                return false;
            }
        }
        if (accl_data.y >= 0) {
            last_y_acc = 0;
            return false;
        }
        if (accl_data.y + 230 < last_y_acc) {
            last_y_acc = accl_data.y;
            return true;
        }
    }
    return false;
}

bool get_is_looked_at() {
    update_accl_data();
    if ((accl_data.y + 300) <= 600 && (accl_data.x + 300) <= 600 && accl_data.z < 100)
        return true;
    return false;
}

accl_data_struct get_accl_data() {
    update_accl_data();
    accl_data.steps = read_step_data();
    accl_data.activity = accl_read_reg(0x27);
    accl_data.temp = accl_read_reg(0x22) + 23;
    return accl_data;
}

void update_accl_data() {
    user_i2c_read(0x18, 0x12, (uint8_t *)&accl_data.x, 6);
#ifdef SWITCH_X_Y // pinetime has 90° rotated Accl
    short tempX = accl_data.x;
    accl_data.x = accl_data.y;
    accl_data.y = tempX;
#endif
    accl_data.x = (accl_data.x / 0x10);
    accl_data.y = (accl_data.y / 0x10);
    accl_data.z = (accl_data.z / 0x10);
}

void accl_write_reg(uint8_t reg, uint8_t data) {
    user_i2c_write(0x18, reg, &data, 1);
}

uint8_t accl_read_reg(uint8_t reg) {
    uint8_t data;
    user_i2c_read(0x18, reg, &data, 1);
    return data;
}
#endif
