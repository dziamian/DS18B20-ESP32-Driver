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
 * @file ds18b20_registers.h
 * @author Damian Ślusarczyk
 * @brief Contains all DS18B20 register information as a set of macros.
 * 
 * They are used for appropriate reading/writing values from/into scratchpad memory of DS18B20.
 * 
 */

#ifndef DS18B20_REGISTERS_H
#define DS18B20_REGISTERS_H

#define DS18B20_SP_TEMP_LSB_BYTE            0 /**< Memory byte index for the least significant byte of the measured temperature */
#define DS18B20_SP_TEMP_MSB_BYTE            1 /**< Memory byte index for the most significant byte of the measured temperature */
#define DS18B20_SP_TEMP_HIGH_BYTE           2 /**< Memory byte index for the upper temperature alarm */
#define DS18B20_SP_TEMP_LOW_BYTE            3 /**< Memory byte index for the lower temperature alarm */
#define DS18B20_SP_CONFIG_BYTE              4 /**< Memory byte index for the device configuration */
#define DS18B20_SP_CRC_BYTE                 8 /**< Memory byte index for the scratchpad CRC */

#define DS18B20_SP_TEMP_HIGH_DEFAULT_VALUE  0x55 /**< Power-on reset value for the upper temperature alarm */
#define DS18B20_SP_TEMP_LOW_DEFAULT_VALUE   0x00 /**< Power-on reset value for the lower temperature alarm */
#define DS18B20_SP_CONFIG_DEFAULT_VALUE     0x7F /**< Power-on reset value for the device configuration */

#endif /* DS18B20_REGISTERS_H */
