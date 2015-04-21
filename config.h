/* -*- mode: c; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

#ifndef NRF51_SHT21_CONFIG_H
#define NRF51_SHT21_CONFIG_H


/*
 * @(#)config.h
 *
 * Copyright (c) 2015 Argusat Limited
 * 10 Underwood Road,  Southampton.  UK
 * All rights reserved.
 *
 * This software is the confidential and proprietary information of
 * Argusat Limited. ("Confidential Information").  You shall not
 * disclose such Confidential Information and shall use it only in
 * accordance with the terms of the license agreement you entered into
 * with Argusat Limited.
 */

/** @file
 *
 * @defgroup argusat_nordic_pebble_config config.h
 * @{
 * @ingroup argusat_nordic_pebble
 * @brief Health Thermometer Service Sample Application main file.
 *
 * This file contains the source code for the iGrow Pebble 
 * This application uses the @ref srvlib_conn_params module.
 */

#define DEVICE_NEC_V850     1
#define DEVICE_NORDIC_NRF51 2

//  Change this depending on the platform you're targetting
#define DEVICE = DEVICE_NORDIC_NRF51

#endif // NRF51_SHT21_CONFIG_H
