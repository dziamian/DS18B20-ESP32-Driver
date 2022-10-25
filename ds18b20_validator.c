#include "ds18b20_validator.h"

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "ds18b20_helpers.h"

#define DS18B20_MSB_1BYTE_MASK          0x80
#define DS18B20_LSB_1BYTE_MASK          0x01
#define DS18B20_DISCARD_LSB_SHIFT_VALUE 1

static void ds18b20_discard_lsb(uint8_t *data, size_t dataSize);

bool ds18b20_validate_crc8(uint8_t *data, size_t dataSize, uint8_t polynomialWithoutMsb, uint8_t crcValue)
{
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

    bool status = (*dataCopy) == crcValue;
    free(dataCopy);
    
    return status;
}

static void ds18b20_discard_lsb(uint8_t *data, size_t dataSize)
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