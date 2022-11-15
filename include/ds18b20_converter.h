/*
 * MIT License
 *
 * Copyright (c) 2022 Damian Ślusarczyk
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
 * TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */
/**
 * @file ds18b20_converter.h
 * @author Damian Ślusarczyk
 * @brief Contains all convertion methods used for proper communication between the user and DS18B20.
 * 
 */

#ifndef DS18B20_CONVERTER_H
#define DS18B20_CONVERTER_H

#include "ds18b20_low.h"
#include "ds18b20_types_res.h"

/**
 * @brief Converts temperature bytes received from scratchpad of DS18B20 for specified resolution into human-readable result.
 * 
 * @param msb The most significant byte of temperature
 * @param lsb The least significant byte of temperature
 * @param resolution Resolution set on the device during convertion
 * @return DS18B20_temperature_out_t Human-readable temperature
 */
DS18B20_temperature_out_t ds18b20_convert_temperature_bytes(const uint8_t msb, uint8_t lsb, const DS18B20_resolution_t resolution);

/**
 * @brief Converts user-defined resolution into configuration byte readable for the device.
 * 
 * @param resolution Specified resolution
 * @return uint8_t Configuration byte readable for the DS18B20
 */
uint8_t ds18b20_resolution_to_config_byte(const DS18B20_resolution_t resolution);

/**
 * @brief Converts configuration byte received from scratchpad of DS18B20 into human-readable resolution.
 * 
 * @param config_byte Configuration byte
 * @return DS18B20_resolution_t Human-readable device resolution
 */
DS18B20_resolution_t ds18b20_config_byte_to_resolution(const uint8_t config_byte);

#endif /* DS18B20_CONVERTER_H */
