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
 * @file ds18b20_error_codes.h
 * @author Damian Ślusarczyk
 * @brief Contains error codes as an enumeration.
 * 
 * They describe the status of performed operations with DS18B20.
 */

#ifndef DS18B20_ERROR_CODES_H
#define DS18B20_ERROR_CODES_H

typedef enum DS18B20_error_t    DS18B20_error_t;

/**
 * @brief Describes the status of performed operation with DS18B20.
 * 
 */
enum DS18B20_error_t
{
    DS18B20_OK = 0,             /**< No error occurred - everything went fine */
    DS18B20_INV_ARG,            /**< An invalid argument has been passed into the function */
    DS18B20_INV_CONF,           /**< An invalid driver configuration has been specified */
    DS18B20_INV_OP,             /**< An operation is invalid for the specified driver configuration */
    DS18B20_NO_MORE_DEVICES,    /**< No *more* devices have been found during next search procedure */
    DS18B20_NO_DEVICES,         /**< No devices have been found on One-Wire bus */
    DS18B20_DISCONNECTED,       /**< Device haven't reply with presence status within given time */
    DS18B20_DEVICE_NOT_FOUND,   /**< Couldn't find the device's ROM address in specified driver instance - it was not initialized properly in this case */
    DS18B20_CRC_FAIL,           /**< CRC validation has failed */
};

#endif /* DS18B20_ERROR_CODES_H */
