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
#include "ds18b20_converter.h"

#include "ds18b20_registers.h"

/** Default value (the lowest resolution) for converting resolution into configuration byte */
#define DS18B20_CONFIG_BYTE_MASK                0x1F
/** Value for shifting configuration bits to get or set the resolution */
#define DS18B20_CONFIG_BYTE_CONVERTER_SHIFT     5
/** Specifies which configuration bits mean resolution (after shifting) */
#define DS18B20_CONFIG_BYTE_CONVERTER_MASK      0x03

/** Base temperature value (0) after convertion when it came out negative */
#define DS18B20_TEMP_CONVERTER_NEGATIVE_BASE    4096
/** The most significant byte divider value for temperature convertion to get the number sign */
#define DS18B20_TEMP_CONVERTER_MSB_DIVIDER      128
/** The most significant byte multiplier value for temperature convertion to get human-readable result */
#define DS18B20_TEMP_CONVERTER_MSB_MULTIPLIER   256
/** The least significant byte divider value for temperature convertion to get human-readable result */
#define DS18B20_TEMP_CONVERTER_LSB_DIVIDER      16

/** Mask value intended to ignore temperature's undefined bits for resolution 09 */
#define DS18B20_RESOLUTION_09_MASK              0xF8
/** Mask value intended to ignore temperature's undefined bits for resolution 10 */
#define DS18B20_RESOLUTION_10_MASK              0xFC
/** Mask value intended to ignore temperature's undefined bits for resolution 11 */
#define DS18B20_RESOLUTION_11_MASK              0xFE
/** Mask value intended to ignore temperature's undefined bits for resolution 12 */
#define DS18B20_RESOLUTION_12_MASK              0xFF

/** Look-up table for temperature's undefined bits masks and resolutions */
static const uint8_t resolution_masks[DS18B20_RESOLUTION_COUNT] =
{
    [DS18B20_RESOLUTION_09] DS18B20_RESOLUTION_09_MASK,
    [DS18B20_RESOLUTION_10] DS18B20_RESOLUTION_10_MASK,
    [DS18B20_RESOLUTION_11] DS18B20_RESOLUTION_11_MASK,
    [DS18B20_RESOLUTION_12] DS18B20_RESOLUTION_12_MASK
};

DS18B20_temperature_out_t ds18b20_convert_temperature_bytes(const uint8_t msb, uint8_t lsb, const DS18B20_resolution_t resolution)
{
    // Ignore undefined bits for specified resolution
    lsb &= resolution_masks[resolution];
    return (DS18B20_temperature_out_t) 
                ((int16_t) (lsb + (msb * DS18B20_TEMP_CONVERTER_MSB_MULTIPLIER))) 
                    / DS18B20_TEMP_CONVERTER_LSB_DIVIDER 
                - DS18B20_TEMP_CONVERTER_NEGATIVE_BASE * (msb / DS18B20_TEMP_CONVERTER_MSB_DIVIDER);
}

uint8_t ds18b20_resolution_to_config_byte(const DS18B20_resolution_t resolution)
{
    return DS18B20_CONFIG_BYTE_MASK | ((uint8_t) resolution << DS18B20_CONFIG_BYTE_CONVERTER_SHIFT);
}

DS18B20_resolution_t ds18b20_config_byte_to_resolution(const uint8_t config_byte)
{
    return (DS18B20_resolution_t)
                (config_byte >> DS18B20_CONFIG_BYTE_CONVERTER_SHIFT)
                    & DS18B20_CONFIG_BYTE_CONVERTER_MASK;
}