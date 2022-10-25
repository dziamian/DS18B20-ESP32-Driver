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

#define DS18B20_WAITING_END                     0

#define DS18B20_READ_TEMPERATURE_BYTES          2
#define DS18B20_READ_CONFIGURATION_BYTES        5

static void ds18b20_waitWithChecking(DS18B20_onewire_t *onewire, uint16_t waitPeriodMs, uint16_t checkPeriodMs);
static bool ds18b20_readRegisters(DS18B20_onewire_t *onewire, size_t deviceIndex, uint8_t bytesToRead, bool checksum);
static bool ds18b20_readRom(DS18B20_onewire_t *onewire, bool checksum);
static bool ds18b20_searchRom(DS18B20_onewire_t *onewire, size_t deviceIndex, bool checksum);
static bool ds18b20_selectDevice(DS18B20_onewire_t *onewire, size_t deviceIndex);

bool ds18b20__InitOneWire(DS18B20_onewire_t *onewire, DS18B20_gpio_t bus, DS18B20_t *devices, size_t devicesNo, bool checksum)
{
    if (!onewire || !devices || !devicesNo)
    {
        return false;
    }

    if (ESP_OK != gpio_reset_pin(bus))
    {
        return false;
    }

    onewire->bus = bus;
    onewire->devices = devices;
    onewire->devicesNo = devicesNo;

    if (DS18B20_OK != ds18b20_restart_search(onewire))
    {
        return false;
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
            if (!ds18b20_searchRom(onewire, deviceIndex, checksum))
            {
                return false;
            }
        }
        else
        {
            // Read ROM from device and set it.
            if (!ds18b20_readRom(onewire, checksum))
            {
                return false;
            }
        }

        // Read power mode and set it.
        if (!ds18b20_selectDevice(onewire, deviceIndex))
        {
            return false;
        }
        ds18b20_read_powermode(onewire, deviceIndex);

        // Default resolution after power-up is 12-bit, but prefer to check it and set it.
        if (!ds18b20_selectDevice(onewire, deviceIndex))
        {
            return false;
        }
        if (!ds18b20_readRegisters(onewire, deviceIndex, checksum ? DS18B20_SP_SIZE : DS18B20_READ_CONFIGURATION_BYTES, checksum))
        {
            return false;
        }
    }

    return true;
}

bool ds18b20__InitConfigDefault(DS18B20_config_t *config)
{
    config->upperAlarm = DS18B20_SP_TEMP_HIGH_DEFAULT_VALUE;
    config->lowerAlarm = DS18B20_SP_TEMP_LOW_DEFAULT_VALUE;
    config->resolution = ds18b20_config_byte_to_resolution(DS18B20_SP_CONFIG_DEFAULT_VALUE);
    
    return true;
}

bool ds18b20__GetTemperatureC(DS18B20_onewire_t *onewire, size_t deviceIndex, DS18B20_temperature_out_t *temperatureOut, bool checksum)
{
    return ds18b20__GetTemperatureCWithChecking(onewire, deviceIndex, temperatureOut, DS18B20_NO_CHECK_PERIOD, checksum);
}

bool ds18b20__GetTemperatureCWithChecking(DS18B20_onewire_t *onewire, size_t deviceIndex, DS18B20_temperature_out_t *temperatureOut, uint16_t checkPeriodMs, bool checksum)
{
    if (!onewire || deviceIndex >= onewire->devicesNo || !temperatureOut)
    {
        temperatureOut = NULL;
        return false;
    }

    uint16_t waitPeriodMs = ds18b20_millis_to_wait_for_convertion(onewire->devices[deviceIndex].resolution);
    if (DS18B20_NO_CHECK_PERIOD == checkPeriodMs)
    {
        checkPeriodMs = waitPeriodMs;
    }
    else if (DS18B20_PM_PARASITE == onewire->devices[deviceIndex].powerMode)
    {
        temperatureOut = NULL;
        return false;
    }

    if (!ds18b20_selectDevice(onewire, deviceIndex))
    {
        return false;
    }
    ds18b20_convert_temperature(onewire, deviceIndex);

    ds18b20_waitWithChecking(onewire, waitPeriodMs, checkPeriodMs);

    if (DS18B20_PM_PARASITE == onewire->devices[deviceIndex].powerMode)
    {
        // TODO: end strong pullup
    }

    if (!ds18b20_selectDevice(onewire, deviceIndex))
    {
        return false;
    }
    if (!ds18b20_readRegisters(onewire, deviceIndex, checksum ? DS18B20_SP_SIZE : DS18B20_READ_TEMPERATURE_BYTES, checksum))
    {
        return false;
    }
    *temperatureOut = ds18b20_convert_temperature_bytes(
        onewire->devices[deviceIndex].scratchpad[DS18B20_SP_TEMP_MSB_BYTE], 
        onewire->devices[deviceIndex].scratchpad[DS18B20_SP_TEMP_LSB_BYTE],
        onewire->devices[deviceIndex].resolution
    );

    return true;
}

bool ds18b20__Configure(DS18B20_onewire_t *onewire, size_t deviceIndex, DS18B20_config_t *config, bool checksum)
{
    if (!onewire || deviceIndex >= onewire->devicesNo)
    {
        return false;
    }

    onewire->devices[deviceIndex].scratchpad[DS18B20_SP_TEMP_HIGH_BYTE] = config->upperAlarm;
    onewire->devices[deviceIndex].scratchpad[DS18B20_SP_TEMP_LOW_BYTE] = config->lowerAlarm;
    onewire->devices[deviceIndex].scratchpad[DS18B20_SP_CONFIG_BYTE] = ds18b20_resolution_to_config_byte(config->resolution);

    if (!ds18b20_selectDevice(onewire, deviceIndex))
    {
        return false;
    }
    ds18b20_write_scratchpad(onewire, deviceIndex);

    if (!ds18b20_selectDevice(onewire, deviceIndex))
    {
        return false;
    }
    if (!ds18b20_readRegisters(onewire, deviceIndex, checksum ? DS18B20_SP_SIZE : DS18B20_READ_CONFIGURATION_BYTES, checksum))
    {
        return false;
    }

    return true;
}

bool ds18b20__StoreRegisters(DS18B20_onewire_t *onewire, size_t deviceIndex)
{
    return ds18b20__StoreRegistersWithChecking(onewire, deviceIndex, DS18B20_NO_CHECK_PERIOD);
}

bool ds18b20__StoreRegistersWithChecking(DS18B20_onewire_t *onewire, size_t deviceIndex, uint16_t checkPeriodMs)
{
    if (!onewire || deviceIndex >= onewire->devicesNo)
    {
        return false;
    }

    uint16_t waitPeriodMs = DS18B20_SCRATCHPAD_COPY_DELAY_MS;
    if (DS18B20_NO_CHECK_PERIOD == checkPeriodMs)
    {
        checkPeriodMs = waitPeriodMs;
    }
    else if (DS18B20_PM_PARASITE == onewire->devices[deviceIndex].powerMode)
    {
        return false;
    }

    if (!ds18b20_selectDevice(onewire, deviceIndex))
    {
        return false;
    }
    ds18b20_copy_scratchpad(onewire, deviceIndex);

    ds18b20_waitWithChecking(onewire, waitPeriodMs, checkPeriodMs);

    if (DS18B20_PM_PARASITE == onewire->devices[deviceIndex].powerMode)
    {
        // TODO: end strong pullup
    }

    return true;
}

bool ds18b20__RestoreRegisters(DS18B20_onewire_t *onewire, size_t deviceIndex, bool checksum)
{
    return ds18b20__RestoreRegistersWithChecking(onewire, deviceIndex, DS18B20_NO_CHECK_PERIOD, checksum);
}

bool ds18b20__RestoreRegistersWithChecking(DS18B20_onewire_t *onewire, size_t deviceIndex, uint16_t checkPeriodMs, bool checksum)
{
    if (!onewire || deviceIndex >= onewire->devicesNo)
    {
        return false;
    }

    uint16_t waitPeriodMs = DS18B20_EEPROM_RESTORE_DELAY_MS;
    if (DS18B20_NO_CHECK_PERIOD == checkPeriodMs)
    {
        checkPeriodMs = waitPeriodMs;
    }

    if (!ds18b20_selectDevice(onewire, deviceIndex))
    {
        return false;
    }
    ds18b20_recall_e2(onewire);

    ds18b20_waitWithChecking(onewire, waitPeriodMs, checkPeriodMs);

    if (!ds18b20_selectDevice(onewire, deviceIndex))
    {
        return false;
    }
    if (!ds18b20_readRegisters(onewire, deviceIndex, checksum ? DS18B20_SP_SIZE : DS18B20_READ_CONFIGURATION_BYTES, checksum))
    {
        return false;
    }

    return true;
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

static bool ds18b20_readRegisters(DS18B20_onewire_t *onewire, size_t deviceIndex, uint8_t bytesToRead, bool checksum)
{
    if (DS18B20_OK != ds18b20_read_scratchpad_with_stop(onewire, deviceIndex, bytesToRead))
    {
        return false;
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

    return true;
}

static bool ds18b20_readRom(DS18B20_onewire_t *onewire, bool checksum)
{
    if (DS18B20_OK != ds18b20_read_rom(onewire))
    {
        return false;
    }
    
    if (checksum)
    {
        return ds18b20_validate_crc8(onewire->devices->rom, DS18B20_ROM_SIZE_TO_VALIDATE, 
            DS18B20_CRC8_POLYNOMIAL_WITHOUT_MSB, onewire->devices->rom[DS18B20_ROM_CRC_BYTE]);
    }

    return true;
}

static bool ds18b20_searchRom(DS18B20_onewire_t *onewire, size_t deviceIndex, bool checksum)
{
    if (DS18B20_OK != ds18b20_search_rom(onewire))
    {
        return false;
    }
    
    if (checksum)
    {
        return ds18b20_validate_crc8(onewire->devices[deviceIndex].rom, DS18B20_ROM_SIZE_TO_VALIDATE, 
            DS18B20_CRC8_POLYNOMIAL_WITHOUT_MSB, onewire->devices[deviceIndex].rom[DS18B20_ROM_CRC_BYTE]);
    }

    return true;
}

static bool ds18b20_selectDevice(DS18B20_onewire_t *onewire, size_t deviceIndex)
{
    if (DS18B20_1W_SINGLEDEVICE != onewire->devicesNo)
    {
        if (DS18B20_OK != ds18b20_select(onewire, deviceIndex))
        {
            return false;
        }
    }
    else
    {
        if (DS18B20_OK != ds18b20_skip_select(onewire))
        {
            return false;
        }
    }

    return true;
}