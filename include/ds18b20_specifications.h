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
 * @file ds18b20_specifications.h
 * @author Damian Ślusarczyk
 * @brief Contains all DS18B20 time characteristics as a set of macros. 
 * 
 * They are required for an appropriate communication with the device.
 */

#ifndef DS18B20_SPECIFICATIONS_H
#define DS18B20_SPECIFICATIONS_H

#define DS18B20_RESOLUTION_09_DELAY_MS          94  /**< Maximum temperature convertion waiting time for resolution 09 (ms) */
#define DS18B20_RESOLUTION_10_DELAY_MS          188 /**< Maximum temperature convertion waiting time for resolution 10 (ms) */
#define DS18B20_RESOLUTION_11_DELAY_MS          375 /**< Maximum temperature convertion waiting time for resolution 11 (ms) */
#define DS18B20_RESOLUTION_12_DELAY_MS          750 /**< Maximum temperature convertion waiting time for resolution 12 (ms) */

#define DS18B20_EEPROM_RESTORE_DELAY_MS         10  /**< Maximum time required for restoring memory from EEPROM (ms) */
#define DS18B20_SCRATCHPAD_COPY_DELAY_MS        10  /**< Maximum time required for copying memory into EEPROM (ms) */
#define DS18B20_SCRATCHPAD_COPY_MIN_PULLUP_MS   10  /**< Minimum pull-up time during memory copying in parasite power mode (ms)*/

#endif /* DS18B20_SPECIFICATIONS_H */
