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
 * @file ds18b20_types_req.h
 * @author Damian Ślusarczyk
 * @brief Contains all input types required to communicate with DS18B20 driver.
 * 
 */

#ifndef DS18B20_TYPES_REQ_H
#define DS18B20_TYPES_REQ_H

typedef enum DS18B20_resolution_t           DS18B20_resolution_t;

/** Alarm temperature value set during device configuration */
typedef int8_t                              DS18B20_temperature_in_t;

/**
 * @brief Describes device resolution for temperature convertion.
 * 
 * Lower resolution means lower measurement accuracy.
 */
enum DS18B20_resolution_t
{
    DS18B20_RESOLUTION_09 = 0,  /**< Resolution 09 - 1 decimal bit */
    DS18B20_RESOLUTION_10,      /**< Resolution 10 - 2 decimal bits */
    DS18B20_RESOLUTION_11,      /**< Resolution 11 - 3 decimal bits */
    DS18B20_RESOLUTION_12,      /**< Resolution 12 - 4 decimal bits */
    DS18B20_RESOLUTION_COUNT    /**< Number of available resolutions */
};

#endif /* DS18B20_TYPES_REQ_H */
