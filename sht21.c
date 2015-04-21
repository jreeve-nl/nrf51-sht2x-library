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
 * @defgroup ble_sdk_app_ess_main main.c
 * @{
 * @ingroup ble_sdk_app_ess
 * @brief Health Thermometer Service Sample Application main file.
 *
 * This file contains the source code for the iGrow Pebble 
 * This application uses the @ref srvlib_conn_params module.
 */
 
#include "sht21.h"
#include "twi_master.h"
#include "nrf_delay.h"

/*lint ++flb "Enter library region" */

#define DS1634_BASE_ADDRESS 0x90 //!< 4 MSBs of the DS1624 TWI address

#define DS1624_ONESHOT_MODE 0x01 //!< Bit in configuration register for 1-shot mode 
#define DS1624_CONVERSION_DONE 0x80 //!< Bit in configuration register to indicate completed temperature conversion

static uint8_t m_device_address; //!< Device address in bits [7:1]

const uint8_t command_access_memory = 0x17; //!< Reads or writes to 256-byte EEPROM memory
const uint8_t command_access_config = 0xAC; //!< Reads or writes configuration data to configuration register 
const uint8_t command_read_temp = 0xAA; //!< Reads last converted temperature value from temperature register
const uint8_t command_start_convert_temp = 0xEE; //!< Initiates temperature conversion.
const uint8_t command_stop_convert_temp = 0x22; //!< Halts temperature conversion.

/**
 * @brief Function for reading the current configuration of the sensor.
 *
 * @return uint8_t Zero if communication with the sensor failed. Contents (always non-zero) of configuration register (@ref DS1624_ONESHOT_MODE and @ref DS1624_CONVERSION_DONE) if communication succeeded.
 */
static uint8_t ds1624_config_read(void)
{
  uint8_t config = 0;
    
  // Write: command protocol
  if (twi_master_transfer(m_device_address, (uint8_t*)&command_access_config, 1, TWI_DONT_ISSUE_STOP))
  {
    if (twi_master_transfer(m_device_address | TWI_READ_BIT, &config, 1, TWI_ISSUE_STOP)) // Read: current configuration
    {
      // Read succeeded, configuration stored to variable "config"
    }
    else
    {
      // Read failed
      config = 0;
    }
  } 

  return config;
}

bool ds1624_init(uint8_t device_address)
{
  bool transfer_succeeded = true;
  m_device_address = DS1634_BASE_ADDRESS + (uint8_t)(device_address << 1);

  uint8_t config = ds1624_config_read();  

  if (config != 0)
  {
    // Configure DS1624 for 1SHOT mode if not done so already.
    if (!(config & DS1624_ONESHOT_MODE))
    {
      uint8_t data_buffer[2];
  
      data_buffer[0] = command_access_config;
      data_buffer[1] = DS1624_ONESHOT_MODE;
  
      transfer_succeeded &= twi_master_transfer(m_device_address, data_buffer, 2, TWI_ISSUE_STOP);
    }
  }
  else
  {
    transfer_succeeded = false;
  }

  return transfer_succeeded;
}

bool ds1624_start_temp_conversion(void)
{
  return twi_master_transfer(m_device_address, (uint8_t*)&command_start_convert_temp, 1, TWI_ISSUE_STOP);
}

bool ds1624_is_temp_conversion_done(void)
{
  uint8_t config = ds1624_config_read();

  if (config & DS1624_CONVERSION_DONE)
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool ds1624_temp_read(int8_t *temperature_in_celcius, int8_t *temperature_fraction)
{
  bool transfer_succeeded = false;

  // Write: Begin read temperature command
  if (twi_master_transfer(m_device_address, (uint8_t*)&command_read_temp, 1, TWI_DONT_ISSUE_STOP))
  {
    uint8_t data_buffer[2];

    // Read: 2 temperature bytes to data_buffer
    if (twi_master_transfer(m_device_address | TWI_READ_BIT, data_buffer, 2, TWI_ISSUE_STOP)) 
    {
      *temperature_in_celcius = (int8_t)data_buffer[0];
      *temperature_fraction = (int8_t)data_buffer[1];
      
      transfer_succeeded = true;
    }
  }

  return transfer_succeeded;
}

/*lint --flb "Leave library region" */ 
