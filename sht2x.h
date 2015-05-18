/* -*- mode: c; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

#ifndef NRF51_SHT2X_SHT2X_H
#define NRF51_SHT2X_SHT2X_H

/* Copyright 2015 Argusat Limited 

*  Licensed under the Apache License, Version 2.0 (the "License"); 
*  you may not use this file except in compliance with the License. 
*  You may obtain a copy of the License at 
*
*    http://www.apache.org/licenses/LICENSE-2.0 
*
*  Unless required by applicable law or agreed to in writing, software
*  distributed under the License is distributed on an "AS IS" BASIS,
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*  See the License for the specific language governing permissions and
*  limitations under the License.
*/

/** @file
 *
 * @defgroup nrf51_sht2x_sht2x sht2x.h
 * @{
 * @ingroup nrf51_sht2x_sht2x
 * @brief SHT2x I2C library for nRF51 devices
 *
 * This file contains library code for communicating with Sensiron's
 * range of humidity and temperature sensors.
 */

#include <stdint.h>
#include <stdbool.h>

#include "typedefs.h"

// sensor command
typedef enum {
  TRIG_T_MEASUREMENT_HM    = 0xE3, // command trig. temp meas. hold master
  TRIG_RH_MEASUREMENT_HM   = 0xE5, // command trig. humidity meas. hold master
  TRIG_T_MEASUREMENT_POLL  = 0xF3, // command trig. temp meas. no hold master
  TRIG_RH_MEASUREMENT_POLL = 0xF5, // command trig. humidity meas. no hold master
  USER_REG_W               = 0xE6, // command writing user register
  USER_REG_R               = 0xE7, // command reading user register
  SOFT_RESET               = 0xFE  // command soft reset
} sht2x_command_t;

typedef enum {
  SHT2X_RES_12_14BIT       = 0x00, // RH=12bit, T=14bit
  SHT2X_RES_8_12BIT        = 0x01, // RH= 8bit, T=12bit
  SHT2X_RES_10_13BIT       = 0x80, // RH=10bit, T=13bit
  SHT2X_RES_11_11BIT       = 0x81, // RH=11bit, T=11bit
  SHT2X_RES_MASK           = 0x81  // Mask for res. bits (7,0) in user reg.
} sht2x_resolution_t;

typedef enum {
  SHT2X_EOB_ON             = 0x40, // end of battery
  SHT2X_EOB_MASK           = 0x40, // Mask for EOB bit(6) in user reg.
} sht2x_eob_t;

typedef enum {
  SHT2X_HEATER_ON          = 0x04, // heater on
  SHT2X_HEATER_OFF         = 0x00, // heater off
  SHT2X_HEATER_MASK        = 0x04, // Mask for Heater bit(2) in user reg.
} sht2x_heater_t;

// measurement signal selection
typedef enum {
  SHT2X_HUMIDITY,
  SHT2X_TEMP
} sht2x_measure_type_t;

typedef enum {
  SHT2X_I2C_ADDRESS        = 0x80,   // sensor I2C address
} sht2x_i2c_header_t;



/**
 * @brief Function for initializing SHT2x temperature and humidity sensors
 *
 * @note Before calling this function, you must initialize twi_master first.
 *
 * @return
 * @retval true If communication succeeded with the device.
 * @retval false If communication failed with the device.
 */
bool sht2x_init();

/**
 * @brief Function for starting a SHT2x temperature or humidity measurement
 *
 * @note Before calling this function, you must have called sht2x_init()
 *       and it must have returned true.
 *
 * @return
 * @retval true If conversion started successfully
 * @retval false If conversion failed to start.
 */
bool sht2x_start_conversion(sht2x_measure_type_t measure_type, nt16 *pMeasurand);

/**
 * @brief Function for polling to test if a conversion is complete.
 *
 * @note Before calling this function, you must have called sht2x_start_conversion()
 *       and it must have returned true.  
 *
 * @return
 * @retval true If conversion started successfully
 * @retval false If conversion failed to start.
 */
bool sht2x_is_conversion_done(void);

/**
 * @brief Function for making a SHT2x temperature or humidity Hold Master measurement.
 *
 * @note Before calling this function, you must have called sht2x_init()
 *       and it must have returned true.  This function waits for a hold master
 *       until measurement is ready or a timeout occurred.
 *
 * @return
 * @retval true If measurement successful.
 * @retval false If conversion failed to start.
 */
bool sht2x_measure(sht2x_measure_type_t measure_type, nt16 *pMeasurand);


/**
 * @brief Function for performing a soft reset on the SHT2x device.
 *
 * @note Before calling this function, you must have called sht2x_init()
 *       and it must have returned true.
 *
 * @return
 * @retval true If reset successful
 * @retval false If failed to perform reset
 */
bool sht2x_soft_reset();

/**
 * @brief Function for performing a conversion from raw sensor data to RH
 *
 *
 * @return The RH value in percentage points.
 */
float sht2x_calc_humidity_rh(uint16_t raw_rh);
/**
 * @brief Function for performing a conversion from raw sensor data to Temperature in degrees celsius
 *
 *
 * @return The temperature value in degrees celsius
 */
float sht2x_calc_temp_celsius(uint16_t raw_temperature);
/**
 * @brief Function for getting serial number of SHT2x according application note "How To
 *        Read-Out the Serial Number"
 *
 * @note Readout of this function is not CRC checked.
 *
 * @param serial_number Array of 8 bytes (64Bits)
 *         MSB                                         LSB
 *         u8SerialNumber[7]             u8SerialNumber[0]
 *         SNA_1 SNA_0 SNB_3 SNB_2 SNB_1 SNB_0 SNC_1 SNC_0
 *
 * @return
 * @retval true If serial number obtained successfuly.
 * @retval false If failed to obtain serial number.
 */
bool sht2x_get_serial_number(uint8_t serial_number[]);

#endif // NRF51_SHT2X_SHT2X_H
