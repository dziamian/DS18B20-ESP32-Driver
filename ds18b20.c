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
#include "ds18b20.h"

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#include "ds18b20_specifications.h"
#include "ds18b20_registers.h"
#include "ds18b20_rom.h"
#include "ds18b20_converter.h"
#include "ds18b20_validator.h"

#define DS18B20_DEFAULT_VALUE                   0   /**< Default memory initialization value */
#define DS18B20_WAITING_END                     0   /**< A value specifying the end of the wait for operation to finish */

#define DS18B20_READ_TEMPERATURE_BYTES          2   /**< Specifies how many bytes are required to read to get measured temperature */
#define DS18B20_READ_CONFIGURATION_BYTES        5   /**< Specifies how many bytes are required to read to get configuration of the device */

/**
 * @brief Waits for DS18B20 operation to end with periodically checking its status.
 * 
 * @param onewire Pointer to One-Wire bus characteristics instance
 * @param waitPeriodMs Specifies what is the maximum operation time to wait for (in milliseconds)
 * @param checkPeriodMs Specifies how often the status of the specified operation will be checked (in milliseconds)
 */
static void ds18b20_waitWithChecking(const DS18B20_onewire_t * const onewire, uint16_t waitPeriodMs, const uint16_t checkPeriodMs);

/**
 * @brief Reads specified number of bytes from selected device scratchpad memory.
 * 
 * Optionally, validates received data from the One-Wire line with CRC checksum.
 * 
 * @param onewire Pointer to One-Wire bus characteristics instance
 * @param deviceIndex Index of the selected device
 * @param bytesToRead Number of bytes to read
 * @param checksum Specifies if CRC checksum should be calculated during all performed operations
 * @return DS18B20_error_t Status code of the operation
 */
static DS18B20_error_t ds18b20_readRegisters(const DS18B20_onewire_t * const onewire, const size_t deviceIndex, const uint8_t bytesToRead, const bool checksum);

/**
 * @brief Reads ROM address of the found device.
 * 
 * Optionally, validates received data from the One-Wire line with CRC checksum.
 * This method should be called only if one device is connected to the One-Wire bus!
 * 
 * @param onewire Pointer to One-Wire bus characteristics instance
 * @param checksum Specifies if CRC checksum should be calculated during all performed operations
 * @return DS18B20_error_t Status code of the operation
 */
static DS18B20_error_t ds18b20_readRom(const DS18B20_onewire_t * const onewire, const bool checksum);

/**
 * @brief Reads ROM address of the next found device.
 * 
 * Performs the next cycle of the search procedure.
 * Optionally, validates received data from the One-Wire line with CRC checksum.
 * 
 * @param onewire Pointer to One-Wire bus characteristics instance
 * @param deviceIndex Index of the selected device
 * @param checksum Specifies if CRC checksum should be calculated during all performed operations
 * @return DS18B20_error_t Status code of the operation
 */
static DS18B20_error_t ds18b20_searchRom(DS18B20_onewire_t * const onewire, const size_t deviceIndex, const bool checksum);

/**
 * @brief Reads ROM address of the next found device whose last measured temperature is within the specified alarm range.
 * 
 * Performs the next cycle of the search procedure.
 * Optionally, validates received data from the One-Wire line with CRC checksum.
 * 
 * @param onewire Pointer to One-Wire bus characteristics instance
 * @param buffer Pointer to buffer instance where found ROM address will be saved
 * @param checksum Specifies if CRC checksum should be calculated during all performed operations
 * @return DS18B20_error_t Status code of the operation
 */
static DS18B20_error_t ds18b20_searchAlarm(DS18B20_onewire_t * const onewire, DS18B20_rom_t * buffer, const bool checksum);

/**
 * @brief Selects specified device to communicate with on the One-Wire bus.
 * 
 * This method is required to call before using any of functional methods like converting temperature or reading registers.
 * 
 * @param onewire Pointer to One-Wire bus characteristics instance
 * @param deviceIndex Index of the selected device
 * @return DS18B20_error_t Status code of the operation
 */
static DS18B20_error_t ds18b20_selectDevice(const DS18B20_onewire_t * const onewire, const size_t deviceIndex);

/**
 * @brief Only requests chosen DS18B20 for temperature convertion without reading its value while periodically checking if performing operation by the device has ended.
 * 
 * Waits until this operation has finished by periodically checking its status.
 * 
 * @param onewire Pointer to One-Wire bus characteristics instance
 * @param deviceIndex Index of the selected device
 * @param checkPeriodMs Specifies how often the status of the specified operation will be checked (in milliseconds)
 * @return DS18B20_error_t Status code of the operation
 */
static DS18B20_error_t ds18b20_requestTemperature(const DS18B20_onewire_t * const onewire, const size_t deviceIndex, uint16_t checkPeriodMs);

DS18B20_error_t ds18b20__InitOneWire(DS18B20_onewire_t * const onewire, const int bus, DS18B20_t * const devices, const size_t devicesNo, const bool checksum)
{
    DS18B20_error_t status;
    if (!onewire || !devices || !devicesNo)
    {
        return DS18B20_INV_ARG;
    }

    if (ESP_OK != gpio_reset_pin(bus))
    {
        return DS18B20_INV_CONF;
    }

    onewire->bus = bus;
    onewire->devices = devices;
    onewire->devicesNo = devicesNo;

    // Manually calling restart search for the first time, because internal values have not been set yet.
    status = ds18b20_restart_search(onewire, false);
    if (DS18B20_OK != status)
    {
        return status;
    }

    for (size_t deviceIndex = 0; deviceIndex < devicesNo; ++deviceIndex)
    {
        // Clear rom
        memset(onewire->devices[deviceIndex].rom, DS18B20_DEFAULT_VALUE, DS18B20_ROM_SIZE);

        // Clear scratchpad
        memset(onewire->devices[deviceIndex].scratchpad, DS18B20_DEFAULT_VALUE, DS18B20_SP_SIZE);

        if (DS18B20_1W_SINGLEDEVICE != devicesNo)
        {
            // Search ROM from next device and set it.
            status = ds18b20_searchRom(onewire, deviceIndex, checksum);
            if (DS18B20_OK != status)
            {
                return status;
            }
        }
        else
        {
            // Read ROM from device and set it.
            status = ds18b20_readRom(onewire, checksum);
            if (DS18B20_OK != status)
            {
                return status;
            }
        }
        
        // Default resolution after power-up is 12-bit, but prefer to check it and set it.
        status = ds18b20_selectDevice(onewire, deviceIndex);
        if (DS18B20_OK != status)
        {
            return status;
        }
        status = ds18b20_readRegisters(onewire, deviceIndex, checksum ? DS18B20_SP_SIZE : DS18B20_READ_CONFIGURATION_BYTES, checksum);
        if (DS18B20_OK != status)
        {
            return status;
        }

        // Read power mode and set it.
        // If parasite mode then perform first temperature convertion, because it will not be reliable.
        status = ds18b20_selectDevice(onewire, deviceIndex);
        if (DS18B20_OK != status)
        {
            return status;
        }
        status = ds18b20_read_powermode(onewire, deviceIndex);
        if (DS18B20_OK != status)
        {
            return status;
        }
        if (DS18B20_PM_PARASITE == onewire->devices[deviceIndex].powerMode)
        {
            status = ds18b20_requestTemperature(onewire, deviceIndex, DS18B20_NO_CHECK_PERIOD);
            if (DS18B20_OK != status)
            {
                return status;
            }
        }
    }

    return DS18B20_OK;
}

DS18B20_error_t ds18b20__InitConfigDefault(DS18B20_config_t * const config)
{
    if (!config)
    {
        return DS18B20_INV_ARG;
    }
    
    config->upperAlarm = DS18B20_SP_TEMP_HIGH_DEFAULT_VALUE;
    config->lowerAlarm = DS18B20_SP_TEMP_LOW_DEFAULT_VALUE;
    config->resolution = ds18b20_config_byte_to_resolution(DS18B20_SP_CONFIG_DEFAULT_VALUE);
    
    return DS18B20_OK;
}

DS18B20_error_t ds18b20__RequestTemperatureC(const DS18B20_onewire_t * const onewire, const size_t deviceIndex)
{
    return ds18b20__RequestTemperatureCWithChecking(onewire, deviceIndex, DS18B20_NO_CHECK_PERIOD);
}

DS18B20_error_t ds18b20__RequestTemperatureCWithChecking(const DS18B20_onewire_t * const onewire, const size_t deviceIndex, uint16_t checkPeriodMs)
{
    DS18B20_error_t status;
    if (!onewire || deviceIndex >= onewire->devicesNo)
    {
        return DS18B20_INV_ARG;
    }

    status = ds18b20_requestTemperature(onewire, deviceIndex, checkPeriodMs);
    if (DS18B20_OK != status)
    {
        return status;
    }

    return DS18B20_OK;
}

DS18B20_error_t ds18b20__GetTemperatureC(const DS18B20_onewire_t * const onewire, const size_t deviceIndex, DS18B20_temperature_out_t * const temperatureOut, const bool checksum)
{
    return ds18b20__GetTemperatureCWithChecking(onewire, deviceIndex, temperatureOut, DS18B20_NO_CHECK_PERIOD, checksum);
}

DS18B20_error_t ds18b20__GetTemperatureCWithChecking(const DS18B20_onewire_t * const onewire, const size_t deviceIndex, DS18B20_temperature_out_t * const temperatureOut, uint16_t checkPeriodMs, const bool checksum)
{
    DS18B20_error_t status = ds18b20__RequestTemperatureCWithChecking(onewire, deviceIndex, checkPeriodMs);
    if (DS18B20_OK != status)
    {
        return status;
    }

    status = ds18b20_selectDevice(onewire, deviceIndex);
    if (DS18B20_OK != status)
    {
        return status;
    }
    status = ds18b20_readRegisters(onewire, deviceIndex, checksum ? DS18B20_SP_SIZE : DS18B20_READ_TEMPERATURE_BYTES, checksum);
    if (DS18B20_OK != status)
    {
        return status;
    }

    *temperatureOut = ds18b20_convert_temperature_bytes(
        onewire->devices[deviceIndex].scratchpad[DS18B20_SP_TEMP_MSB_BYTE], 
        onewire->devices[deviceIndex].scratchpad[DS18B20_SP_TEMP_LSB_BYTE],
        onewire->devices[deviceIndex].resolution
    );

    return DS18B20_OK;
}

DS18B20_error_t ds18b20__Configure(const DS18B20_onewire_t * const onewire, const size_t deviceIndex, const DS18B20_config_t * const config, const bool checksum)
{
    DS18B20_error_t status;
    if (!onewire || deviceIndex >= onewire->devicesNo)
    {
        return DS18B20_INV_ARG;
    }

    onewire->devices[deviceIndex].scratchpad[DS18B20_SP_TEMP_HIGH_BYTE] = config->upperAlarm;
    onewire->devices[deviceIndex].scratchpad[DS18B20_SP_TEMP_LOW_BYTE] = config->lowerAlarm;
    onewire->devices[deviceIndex].scratchpad[DS18B20_SP_CONFIG_BYTE] = ds18b20_resolution_to_config_byte(config->resolution);

    status = ds18b20_selectDevice(onewire, deviceIndex);
    if (DS18B20_OK != status)
    {
        return status;
    }
    status = ds18b20_write_scratchpad(onewire, deviceIndex);
    if (DS18B20_OK != status)
    {
        return status;
    }

    status = ds18b20_selectDevice(onewire, deviceIndex);
    if (DS18B20_OK != status)
    {
        return status;
    }
    status = ds18b20_readRegisters(onewire, deviceIndex, checksum ? DS18B20_SP_SIZE : DS18B20_READ_CONFIGURATION_BYTES, checksum);
    if (DS18B20_OK != status)
    {
        return status;
    }

    return DS18B20_OK;
}

DS18B20_error_t ds18b20__FindNextAlarm(DS18B20_onewire_t * const onewire, size_t * const deviceIndexOut, const bool checksum)
{
    DS18B20_error_t status;
    if (!onewire || !deviceIndexOut)
    {
        return DS18B20_INV_ARG;
    }

    DS18B20_rom_t alarmRom;
    memset(alarmRom, DS18B20_DEFAULT_VALUE, DS18B20_ROM_SIZE);
    status = ds18b20_searchAlarm(onewire, &alarmRom, checksum);
    if (DS18B20_OK != status)
    {
        return status;
    }

    for (size_t deviceIndex = 0; deviceIndex < onewire->devicesNo; ++deviceIndex)
    {
        uint8_t i = 0;
        while (DS18B20_ROM_SIZE > i)
        {
            if (onewire->devices[deviceIndex].rom[i] != alarmRom[i])
            {
                break;
            }
            ++i;
        }
        
        if (i >= DS18B20_ROM_SIZE)
        {
            *deviceIndexOut = deviceIndex;
            return DS18B20_OK;
        }
    }

    return DS18B20_DEVICE_NOT_FOUND;
}

DS18B20_error_t ds18b20__StoreRegisters(const DS18B20_onewire_t * const onewire, const size_t deviceIndex)
{
    return ds18b20__StoreRegistersWithChecking(onewire, deviceIndex, DS18B20_NO_CHECK_PERIOD);
}

DS18B20_error_t ds18b20__StoreRegistersWithChecking(const DS18B20_onewire_t * const onewire, const size_t deviceIndex, uint16_t checkPeriodMs)
{
    DS18B20_error_t status;
    if (!onewire || deviceIndex >= onewire->devicesNo)
    {
        return DS18B20_INV_ARG;
    }

    uint16_t waitPeriodMs = DS18B20_SCRATCHPAD_COPY_DELAY_MS;
    if (DS18B20_NO_CHECK_PERIOD == checkPeriodMs)
    {
        checkPeriodMs = waitPeriodMs;
    }
    else if (DS18B20_PM_PARASITE == onewire->devices[deviceIndex].powerMode)
    {
        return DS18B20_INV_OP;
    }
    else if (DS18B20_CHECK_PERIOD_MIN_MS > checkPeriodMs)
    {
        return DS18B20_INV_ARG;
    }

    status = ds18b20_selectDevice(onewire, deviceIndex);
    if (DS18B20_OK != status)
    {
        return status;
    }
    status = ds18b20_copy_scratchpad(onewire, deviceIndex);
    if (DS18B20_OK != status)
    {
        return status;
    }

    ds18b20_waitWithChecking(onewire, waitPeriodMs, checkPeriodMs);

    if (DS18B20_PM_PARASITE == onewire->devices[deviceIndex].powerMode)
    {
        ds18b20_parasite_end_pullup(onewire);
    }

    return DS18B20_OK;
}

DS18B20_error_t ds18b20__RestoreRegisters(const DS18B20_onewire_t * const onewire, const size_t deviceIndex, const bool checksum)
{
    return ds18b20__RestoreRegistersWithChecking(onewire, deviceIndex, DS18B20_NO_CHECK_PERIOD, checksum);
}

DS18B20_error_t ds18b20__RestoreRegistersWithChecking(const DS18B20_onewire_t * const onewire, const size_t deviceIndex, uint16_t checkPeriodMs, const bool checksum)
{
    DS18B20_error_t status;
    if (!onewire || deviceIndex >= onewire->devicesNo)
    {
        return DS18B20_INV_ARG;
    }

    uint16_t waitPeriodMs = DS18B20_EEPROM_RESTORE_DELAY_MS;
    if (DS18B20_NO_CHECK_PERIOD == checkPeriodMs)
    {
        checkPeriodMs = waitPeriodMs;
    }
    else if (DS18B20_CHECK_PERIOD_MIN_MS > checkPeriodMs)
    {
        return DS18B20_INV_ARG;
    }

    status = ds18b20_selectDevice(onewire, deviceIndex);
    if (DS18B20_OK != status)
    {
        return status;
    }
    status = ds18b20_recall_e2(onewire);
    if (DS18B20_OK != status)
    {
        return status;
    }

    ds18b20_waitWithChecking(onewire, waitPeriodMs, checkPeriodMs);

    status = ds18b20_selectDevice(onewire, deviceIndex);
    if (DS18B20_OK != status)
    {
        return status;
    }
    status = ds18b20_readRegisters(onewire, deviceIndex, checksum ? DS18B20_SP_SIZE : DS18B20_READ_CONFIGURATION_BYTES, checksum);
    if (DS18B20_OK != status)
    {
        return status;
    }

    return DS18B20_OK;
}

static void ds18b20_waitWithChecking(const DS18B20_onewire_t * const onewire, uint16_t waitPeriodMs, const uint16_t checkPeriodMs)
{
    while (true)
    {
        vTaskDelay(pdMS_TO_TICKS(checkPeriodMs));

        if (DS18B20_WAITING_END >= (waitPeriodMs - checkPeriodMs) || ds18b20_read_bit(onewire))
        {
            break;
        }

        waitPeriodMs -= checkPeriodMs;
    }
}

static DS18B20_error_t ds18b20_readRegisters(const DS18B20_onewire_t * const onewire, const size_t deviceIndex, const uint8_t bytesToRead, const bool checksum)
{
    DS18B20_error_t status = ds18b20_read_scratchpad_with_stop(onewire, deviceIndex, bytesToRead);
    if (DS18B20_OK != status)
    {
        return status;
    }
    
    if (bytesToRead > DS18B20_SP_CONFIG_BYTE)
    {
        onewire->devices[deviceIndex].resolution = ds18b20_config_byte_to_resolution(onewire->devices[deviceIndex].scratchpad[DS18B20_SP_CONFIG_BYTE]);
    }

    if (checksum)
    {
        return ds18b20_validate_crc8(onewire->devices[deviceIndex].scratchpad, DS18B20_SP_SIZE_TO_VALIDATE, 
            DS18B20_CRC8_POLYNOMIAL_WITHOUT_MSB, onewire->devices[deviceIndex].scratchpad[DS18B20_SP_CRC_BYTE]);
    }

    return DS18B20_OK;
}

static DS18B20_error_t ds18b20_readRom(const DS18B20_onewire_t * const onewire, const bool checksum)
{
    DS18B20_error_t status = ds18b20_read_rom(onewire);
    if (DS18B20_OK != status)
    {
        return status;
    }
    
    if (checksum)
    {
        return ds18b20_validate_crc8(onewire->devices->rom, DS18B20_ROM_SIZE_TO_VALIDATE, 
            DS18B20_CRC8_POLYNOMIAL_WITHOUT_MSB, onewire->devices->rom[DS18B20_ROM_CRC_BYTE]);
    }

    return DS18B20_OK;
}

static DS18B20_error_t ds18b20_searchRom(DS18B20_onewire_t * const onewire, const size_t deviceIndex, const bool checksum)
{
    DS18B20_error_t status = ds18b20_search_rom(onewire, NULL, false);
    if (DS18B20_OK != status)
    {
        return status;
    }
    
    if (checksum)
    {
        return ds18b20_validate_crc8(onewire->devices[deviceIndex].rom, DS18B20_ROM_SIZE_TO_VALIDATE, 
            DS18B20_CRC8_POLYNOMIAL_WITHOUT_MSB, onewire->devices[deviceIndex].rom[DS18B20_ROM_CRC_BYTE]);
    }

    return DS18B20_OK;
}

static DS18B20_error_t ds18b20_searchAlarm(DS18B20_onewire_t * const onewire, DS18B20_rom_t * buffer, const bool checksum)
{
    DS18B20_error_t status = ds18b20_search_rom(onewire, buffer, true);
    if (DS18B20_OK != status)
    {
        return status;
    }
    
    if (checksum)
    {
        return ds18b20_validate_crc8(*buffer, DS18B20_ROM_SIZE_TO_VALIDATE, 
            DS18B20_CRC8_POLYNOMIAL_WITHOUT_MSB, (*buffer)[DS18B20_ROM_CRC_BYTE]);
    }

    return DS18B20_OK;
}

static DS18B20_error_t ds18b20_selectDevice(const DS18B20_onewire_t * const onewire, const size_t deviceIndex)
{
    DS18B20_error_t status;
    if (DS18B20_1W_SINGLEDEVICE != onewire->devicesNo)
    {
        status = ds18b20_select(onewire, deviceIndex);
        if (DS18B20_OK != status)
        {
            return status;
        }
    }
    else
    {
        status = ds18b20_skip_select(onewire);
        if (DS18B20_OK != status)
        {
            return status;
        }
    }

    return DS18B20_OK;
}

static DS18B20_error_t ds18b20_requestTemperature(const DS18B20_onewire_t * const onewire, const size_t deviceIndex, uint16_t checkPeriodMs)
{
    DS18B20_error_t status;
    uint16_t waitPeriodMs = ds18b20_millis_to_wait_for_convertion(onewire->devices[deviceIndex].resolution);
    if (DS18B20_NO_CHECK_PERIOD == checkPeriodMs)
    {
        checkPeriodMs = waitPeriodMs;
    }
    else if (DS18B20_PM_PARASITE == onewire->devices[deviceIndex].powerMode)
    {
        return DS18B20_INV_OP;
    }
    else if (DS18B20_CHECK_PERIOD_MIN_MS > checkPeriodMs)
    {
        return DS18B20_INV_ARG;
    }

    status = ds18b20_selectDevice(onewire, deviceIndex);
    if (DS18B20_OK != status)
    {
        return status;
    }
    status = ds18b20_convert_temperature(onewire, deviceIndex);
    if (DS18B20_OK != status)
    {
        return status;
    }

    ds18b20_waitWithChecking(onewire, waitPeriodMs, checkPeriodMs);

    if (DS18B20_PM_PARASITE == onewire->devices[deviceIndex].powerMode)
    {
        ds18b20_parasite_end_pullup(onewire);
    }

    return DS18B20_OK;
}