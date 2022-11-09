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

#define DS18B20_1W_SINGLEDEVICE                 1

#define DS18B20_DEFAULT_VALUE                   0
#define DS18B20_CHECK_PERIOD_MIN_MS             10
#define DS18B20_WAITING_END                     0

#define DS18B20_READ_TEMPERATURE_BYTES          2
#define DS18B20_READ_CONFIGURATION_BYTES        5

static void ds18b20_waitWithChecking(DS18B20_onewire_t *onewire, uint16_t waitPeriodMs, uint16_t checkPeriodMs);
static DS18B20_error_t ds18b20_readRegisters(DS18B20_onewire_t *onewire, size_t deviceIndex, uint8_t bytesToRead, bool checksum);
static DS18B20_error_t ds18b20_readRom(DS18B20_onewire_t *onewire, bool checksum);
static DS18B20_error_t ds18b20_searchRom(DS18B20_onewire_t *onewire, size_t deviceIndex, bool checksum);
static DS18B20_error_t ds18b20_searchAlarm(DS18B20_onewire_t *onewire, DS18B20_rom_t *buffer, bool checksum);
static DS18B20_error_t ds18b20_selectDevice(DS18B20_onewire_t *onewire, size_t deviceIndex);
static DS18B20_error_t ds18b20_requestTemperature(DS18B20_onewire_t *onewire, size_t deviceIndex, uint16_t checkPeriodMs);

DS18B20_error_t ds18b20__InitOneWire(DS18B20_onewire_t *onewire, DS18B20_gpio_t bus, DS18B20_t *devices, size_t devicesNo, bool checksum)
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

DS18B20_error_t ds18b20__InitConfigDefault(DS18B20_config_t *config)
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

DS18B20_error_t ds18b20__RequestTemperatureC(DS18B20_onewire_t *onewire, size_t deviceIndex)
{
    DS18B20_error_t status;
    if (!onewire || deviceIndex >= onewire->devicesNo)
    {
        return DS18B20_INV_ARG;
    }

    status = ds18b20_requestTemperature(onewire, deviceIndex, DS18B20_NO_CHECK_PERIOD);
    if (DS18B20_OK != status)
    {
        return status;
    }

    return DS18B20_OK;
}

DS18B20_error_t ds18b20__GetTemperatureC(DS18B20_onewire_t *onewire, size_t deviceIndex, DS18B20_temperature_out_t *temperatureOut, bool checksum)
{
    return ds18b20__GetTemperatureCWithChecking(onewire, deviceIndex, temperatureOut, DS18B20_NO_CHECK_PERIOD, checksum);
}

DS18B20_error_t ds18b20__GetTemperatureCWithChecking(DS18B20_onewire_t *onewire, size_t deviceIndex, DS18B20_temperature_out_t *temperatureOut, uint16_t checkPeriodMs, bool checksum)
{
    DS18B20_error_t status;
    if (!onewire || deviceIndex >= onewire->devicesNo || !temperatureOut)
    {
        return DS18B20_INV_ARG;
    }

    status = ds18b20_requestTemperature(onewire, deviceIndex, checkPeriodMs);
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

DS18B20_error_t ds18b20__Configure(DS18B20_onewire_t *onewire, size_t deviceIndex, DS18B20_config_t *config, bool checksum)
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

DS18B20_error_t ds18b20__FindNextAlarm(DS18B20_onewire_t *onewire, size_t *deviceIndexOut, bool checksum)
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

DS18B20_error_t ds18b20__StoreRegisters(DS18B20_onewire_t *onewire, size_t deviceIndex)
{
    return ds18b20__StoreRegistersWithChecking(onewire, deviceIndex, DS18B20_NO_CHECK_PERIOD);
}

DS18B20_error_t ds18b20__StoreRegistersWithChecking(DS18B20_onewire_t *onewire, size_t deviceIndex, uint16_t checkPeriodMs)
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

DS18B20_error_t ds18b20__RestoreRegisters(DS18B20_onewire_t *onewire, size_t deviceIndex, bool checksum)
{
    return ds18b20__RestoreRegistersWithChecking(onewire, deviceIndex, DS18B20_NO_CHECK_PERIOD, checksum);
}

DS18B20_error_t ds18b20__RestoreRegistersWithChecking(DS18B20_onewire_t *onewire, size_t deviceIndex, uint16_t checkPeriodMs, bool checksum)
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

static void ds18b20_waitWithChecking(DS18B20_onewire_t *onewire, uint16_t waitPeriodMs, uint16_t checkPeriodMs)
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

static DS18B20_error_t ds18b20_readRegisters(DS18B20_onewire_t *onewire, size_t deviceIndex, uint8_t bytesToRead, bool checksum)
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

static DS18B20_error_t ds18b20_readRom(DS18B20_onewire_t *onewire, bool checksum)
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

static DS18B20_error_t ds18b20_searchRom(DS18B20_onewire_t *onewire, size_t deviceIndex, bool checksum)
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

static DS18B20_error_t ds18b20_searchAlarm(DS18B20_onewire_t *onewire, DS18B20_rom_t *buffer, bool checksum)
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

static DS18B20_error_t ds18b20_selectDevice(DS18B20_onewire_t *onewire, size_t deviceIndex)
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

static DS18B20_error_t ds18b20_requestTemperature(DS18B20_onewire_t *onewire, size_t deviceIndex, uint16_t checkPeriodMs)
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