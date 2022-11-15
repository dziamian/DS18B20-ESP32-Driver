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
#include "ds18b20_validator.h"

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "ds18b20_helpers.h"

#define DS18B20_MSB_1BYTE_MASK          0x80 /**< Mask value intended to get the most significant bit */
#define DS18B20_LSB_1BYTE_MASK          0x01 /**< Mask value intended to get the least significant bit */
#define DS18B20_DISCARD_LSB_SHIFT_VALUE 1    /**< Value for shifting the least significant bit to discard it */

/**
 * @brief Discards the least significant bit of the first data byte and shifts all remaining bits to the vacated positions.
 * 
 * @param data Pointer to data bytes to be modified
 * @param dataSize Size of the data
 */
static void ds18b20_discard_lsb(uint8_t * const data, const size_t dataSize);

DS18B20_error_t ds18b20_validate_crc8(const uint8_t * const data, const size_t dataSize, const uint8_t polynomialWithoutMsb, const uint8_t crcValue)
{
    // Copy data so the original will not be modified
    uint8_t *dataCopy = malloc(dataSize);
    memcpy(dataCopy, data, dataSize);
    
    for (size_t byteNo = 0; byteNo < dataSize; ++byteNo)
    {
        for (uint8_t bitNo = 0; bitNo < DS18B20_1BYTE_SIZE; ++bitNo)
        {
            bool performXor = (*dataCopy) & DS18B20_LSB_1BYTE_MASK;

            ds18b20_discard_lsb(dataCopy, dataSize - byteNo);

            if (performXor)
            {
                *dataCopy ^= polynomialWithoutMsb;
            }
        }
    }

    DS18B20_error_t status = ((*dataCopy) == crcValue) ? DS18B20_OK : DS18B20_CRC_FAIL;
    free(dataCopy);
    
    return status;
}

static void ds18b20_discard_lsb(uint8_t * const data, const size_t dataSize)
{
    *data >>= DS18B20_DISCARD_LSB_SHIFT_VALUE;

    for (size_t byteNo = 1; byteNo < dataSize; ++byteNo)
    {
        if (data[byteNo] & DS18B20_LSB_1BYTE_MASK)
        {
            data[byteNo - 1] |= DS18B20_MSB_1BYTE_MASK;
        }
        data[byteNo] >>= DS18B20_DISCARD_LSB_SHIFT_VALUE;
    }
}