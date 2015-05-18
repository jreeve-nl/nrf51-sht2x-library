/* -*- mode: c; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

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
 * @defgroup nrf51_sht2x_sht21 sht2x.c
 * @{
 * @ingroup nrf51_sht2x_sht21
 * @brief SHT2x I2C library for nRF51 devices
 *
 * This file contains library code for communicating with
 * 
 */
 
#include "sht2x.h"
#include "twi_master.h"
#include "nrf_delay.h"

/*lint ++flb "Enter library region" */
const uint16_t POLYNOMIAL = 0x131;  //P(x)=x^8+x^5+x^4+1 = 100110001

typedef enum {
  SHT2X_ERROR_NONE                    = 0x00, // No errror
  SHT2X_ERROR_READ_FAILED             = 0x01, // failed to read the user register
  SHT2X_ERROR_CRC                     = 0x02  // CRC error
} sht2x_error_t;

typedef uint8_t user_register_t;

typedef union
{
  struct
  {
    user_register_t user_register;
    uint8_t         checksum;
  } result;
  uint16_t raw;
} read_config_result_t;

typedef nt16 measurand_t;

typedef union
{
  struct
  {
    measurand_t measurand;
    uint8_t     checksum;
  } result;
  uint8_t raw[3];
} measure_result_t;

/**
 * @brief Function for reading the current configuration of the sensor.
 *
 * @return uint8_t Zero if communication with the sensor failed. Contents (always non-zero) of configuration register (@ref SHT2X_ONESHOT_MODE and @ref SHT2X_CONVERSION_DONE) if communication succeeded.
 */
static uint8_t sht2x_check_crc(uint8_t data[], uint8_t num_bytes, uint8_t checksum)
{
  uint8_t crc = 0;	
  uint8_t byte_index;
  //calculates 8-Bit checksum with given polynomial
  for (byte_index = 0; byte_index < num_bytes; ++byte_index)
  {
    crc ^= (data[byte_index]);
    for (uint8_t bit = 8; bit > 0; --bit)
    {
      if (crc & 0x80)
      {
        crc = (crc << 1) ^ POLYNOMIAL;
      }
      else
      {
        crc = (crc << 1);
      }
    }
  }
  if (crc != checksum)
  {
    return SHT2X_ERROR_CRC;
  }
  else
  {
    return 0;
  }
}


/**
 * @brief Function for reading the current configuration of the sensor.
 *
 * @return uint8_t Zero if communication with the sensor failed. Contents (always non-zero) of configuration register (@ref SHT2X_ONESHOT_MODE and @ref SHT2X_CONVERSION_DONE) if communication succeeded.
 */
static sht2x_error_t sht2x_config_read(user_register_t* user_register)
{
  read_config_result_t result;

  sht2x_error_t error = SHT2X_ERROR_NONE;
  uint8_t command = USER_REG_R;

  // Write: command protocol
  if (twi_master_transfer(SHT2X_I2C_ADDRESS, &command, 1, TWI_DONT_ISSUE_STOP))
  {
    if (twi_master_transfer(SHT2X_I2C_ADDRESS | TWI_READ_BIT, (uint8_t*)&result.raw, sizeof(read_config_result_t), TWI_DONT_ISSUE_STOP)) // Read: current configuration
    {
      // validate checksum
      uint8_t checksum_error = sht2x_check_crc((uint8_t*)&result.result.user_register, sizeof(user_register_t), result.result.checksum);

      if (checksum_error == SHT2X_ERROR_CRC || checksum_error)
      {
        // propagate out the error.  Poor design probably.
        return SHT2X_ERROR_CRC;
      }
      // Read succeeded
      *user_register = result.raw;
    }
    else
    {
      error = SHT2X_ERROR_READ_FAILED;
    }
  }
  else
  {
    error = SHT2X_ERROR_READ_FAILED;
  }
  return error;
}

/**
 * @brief Function for initialising the sensor
 *
 * @return uint8_t Zero if communication with the sensor failed. Contents (always non-zero) of configuration register (@ref SHT2X_ONESHOT_MODE and @ref SHT2X_CONVERSION_DONE) if communication succeeded.
 */
bool sht2x_init()
{
  nrf_delay_ms(15); // delay at least 15ms for the sensor to power up
  bool transfer_succeeded = true;
  user_register_t user_register;

  uint8_t result = sht2x_config_read(&user_register);

  if (result == SHT2X_ERROR_NONE)
  {
    // do stuff
  }
  else
  {
    transfer_succeeded = false;
  }

  return transfer_succeeded;
}

bool sht2x_start_temp_conversion(void)
{
  //  return twi_master_transfer(SHT2X_I2C_ADDRESS, (uint8_t*)&command_start_convert_temp, 1, TWI_ISSUE_STOP);
  return true;
}

bool sht2x_is_temp_conversion_done(void)
{
  return true;
}


float sht2x_calc_humidity_rh(uint16_t u16sRH)
{

  // The raw values arrived in Big Endian / Network Byte Order
  // and we are a Little Endian system so swap byte order.

  float humidity_rh;              // variable for result
  uint16_t swapped = ((nt16)u16sRH).s16.u8L << 8 | ((nt16)u16sRH).s16.u8H;
  
  swapped &= ~0x0003;          // clear bits [1..0] (status bits)
  //-- calculate relative humidity [%RH] --

  humidity_rh = -6.0 + 125.0/65536 * (float)swapped; // RH= -6 + 125 * SRH/2^16
  return humidity_rh;
}

float sht2x_calc_temp_celsius(uint16_t u16sT)
{
  // The raw values arrived in Big Endian / Network Byte Order
  // and we are a Little Endian system so swap byte order.

  float temperature_c;            // variable for result
  uint16_t swapped = ((nt16)u16sT).s16.u8L << 8 | ((nt16)u16sT).s16.u8H;
  swapped &= ~0x0003;           // clear bits [1..0] (status bits)

  //-- calculate temperature [°C] --
  temperature_c = -46.85 + 175.72/65536 *(float)swapped; //T= -46.85 + 175.72 * ST/2^16
  return temperature_c;
}


bool sht2x_measure(sht2x_measure_type_t measure_type, measurand_t *pMeasurand)
{
  bool transfer_succeeded = false;

  sht2x_command_t command;
  switch (measure_type)
  {
  case SHT2X_HUMIDITY:
    command = TRIG_RH_MEASUREMENT_HM;
    break;
  case SHT2X_TEMP:
    command = TRIG_T_MEASUREMENT_HM;
    break;
  }
  // Write:
  if (twi_master_transfer(SHT2X_I2C_ADDRESS, (uint8_t*)&command, 1, TWI_DONT_ISSUE_STOP))
  {
    measure_result_t measure_result;
    // Read: 2 temperature bytes to data_buffer
    if (twi_master_transfer(SHT2X_I2C_ADDRESS | TWI_READ_BIT, (uint8_t*)&measure_result.raw, sizeof(measure_result_t), TWI_ISSUE_STOP)) 
    {

      uint8_t checksum_error = sht2x_check_crc((uint8_t*)&measure_result.result.measurand, sizeof(measurand_t), measure_result.result.checksum);

      if (checksum_error == SHT2X_ERROR_CRC || checksum_error)
      {
        return SHT2X_ERROR_CRC;
      }
      else
      {
        transfer_succeeded = true;
        pMeasurand->s16.u8H = (int8_t)measure_result.result.measurand.s16.u8H;
        pMeasurand->s16.u8L = (int8_t)measure_result.result.measurand.s16.u8L;
      }
    }
  }
  return transfer_succeeded;
}

bool sht2x_soft_reset()
{
  return twi_master_transfer(SHT2X_I2C_ADDRESS, (uint8_t*)SOFT_RESET, 1, TWI_ISSUE_STOP);
}

bool sht2x_get_serial_number(uint8_t serial_number[])
{
  return true;
}

/*lint --flb "Leave library region" */ 
