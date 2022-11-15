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
#include "ds18b20_tests.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include "ds18b20.h"

#define TAG                             "ds18b20"

#define DS18B20_1W_BUS                  19

#define DS18B20_DEVICES_NO              1
#define DS18B20_CHECKSUM                1

#define DS18B20_UPPER_ALARM             28
#define DS18B20_LOWER_ALARM             20
#define DS18B20_RESOLUTION              DS18B20_RESOLUTION_12

//MINIMUM 10 MS
#define DS18B20_TEMP_CHECK_PERIOD_MS    DS18B20_NO_CHECK_PERIOD
#define DS18B20_STORE_CHECK_PERIOD_MS   DS18B20_NO_CHECK_PERIOD
#define DS18B20_RESTORE_CHECK_PERIOD_MS 10

#define DS18B20_TASK_PERIOD_MS          1000

void ds18b20_init_test(void)
{
    DS18B20_onewire_t ds18b20_oneWire;
    DS18B20_t ds18b20_devices[DS18B20_DEVICES_NO];

    if (DS18B20_OK != ds18b20__InitOneWire(&ds18b20_oneWire, DS18B20_1W_BUS, ds18b20_devices, DS18B20_DEVICES_NO, DS18B20_CHECKSUM))
    {
        ESP_LOGI(TAG, "Failure while initializing DS18B20 One-Wire driver.");
        return;
    }

    for (size_t i = 0; i < DS18B20_DEVICES_NO; ++i)
    {
        ESP_LOGI(TAG, "Address %d: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x", i,
            ds18b20_devices[i].rom[0], ds18b20_devices[i].rom[1], ds18b20_devices[i].rom[2], ds18b20_devices[i].rom[3],
            ds18b20_devices[i].rom[4], ds18b20_devices[i].rom[5], ds18b20_devices[i].rom[6], ds18b20_devices[i].rom[7]
        );
        ESP_LOGI(TAG, "Scratchpad %d: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x", i,
            ds18b20_devices[i].scratchpad[0], ds18b20_devices[i].scratchpad[1], ds18b20_devices[i].scratchpad[2], 
            ds18b20_devices[i].scratchpad[3], ds18b20_devices[i].scratchpad[4], ds18b20_devices[i].scratchpad[5], 
            ds18b20_devices[i].scratchpad[6], ds18b20_devices[i].scratchpad[7], ds18b20_devices[i].scratchpad[8]
        );
        ESP_LOGI(TAG, "Resolution %d: %d", i, ds18b20_devices[i].resolution + 9);
        ESP_LOGI(TAG, "Power mode %d: %d", i, ds18b20_devices[i].powerMode);
    }

    return;
}

void ds18b20_read_temperature_test(void)
{
    DS18B20_onewire_t ds18b20_oneWire;
    DS18B20_t ds18b20_devices[DS18B20_DEVICES_NO];
    DS18B20_config_t ds18b20_config =
    {
        .upperAlarm = DS18B20_UPPER_ALARM,
        .lowerAlarm = DS18B20_LOWER_ALARM,
        .resolution = DS18B20_RESOLUTION
    };

    if (DS18B20_OK != ds18b20__InitOneWire(&ds18b20_oneWire, DS18B20_1W_BUS, ds18b20_devices, DS18B20_DEVICES_NO, DS18B20_CHECKSUM))
    {
        ESP_LOGI(TAG, "Failure while initializing DS18B20 One-Wire driver.");
        return;
    }

    for (size_t i = 0; i < DS18B20_DEVICES_NO; ++i)
    {
        ESP_LOGI(TAG, "Address %d: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x", i,
            ds18b20_devices[i].rom[0], ds18b20_devices[i].rom[1], ds18b20_devices[i].rom[2], ds18b20_devices[i].rom[3],
            ds18b20_devices[i].rom[4], ds18b20_devices[i].rom[5], ds18b20_devices[i].rom[6], ds18b20_devices[i].rom[7]
        );
        ESP_LOGI(TAG, "Scratchpad %d: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x", i,
            ds18b20_devices[i].scratchpad[0], ds18b20_devices[i].scratchpad[1], ds18b20_devices[i].scratchpad[2], 
            ds18b20_devices[i].scratchpad[3], ds18b20_devices[i].scratchpad[4], ds18b20_devices[i].scratchpad[5], 
            ds18b20_devices[i].scratchpad[6], ds18b20_devices[i].scratchpad[7], ds18b20_devices[i].scratchpad[8]
        );
        ESP_LOGI(TAG, "Resolution %d: %d", i, ds18b20_devices[i].resolution + 9);
        ESP_LOGI(TAG, "Power mode %d: %d", i, ds18b20_devices[i].powerMode);

        if (DS18B20_OK != ds18b20__Configure(&ds18b20_oneWire, i, &ds18b20_config, DS18B20_CHECKSUM))
        {
            ESP_LOGI(TAG, "Failure while configuring device no. %d.", i);
            return;
        }

        ESP_LOGI(TAG, "Successfully configured device no. %d.", i);

        ESP_LOGI(TAG, "Scratchpad %d: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x", i,
            ds18b20_devices[i].scratchpad[0], ds18b20_devices[i].scratchpad[1], ds18b20_devices[i].scratchpad[2], 
            ds18b20_devices[i].scratchpad[3], ds18b20_devices[i].scratchpad[4], ds18b20_devices[i].scratchpad[5], 
            ds18b20_devices[i].scratchpad[6], ds18b20_devices[i].scratchpad[7], ds18b20_devices[i].scratchpad[8]
        );
        ESP_LOGI(TAG, "Resolution %d: %d", i, ds18b20_devices[i].resolution + 9);
    }

    while (1)
    {
        DS18B20_temperature_out_t temperature;
        for (size_t i = 0; i < DS18B20_DEVICES_NO; ++i)
        {
            if (DS18B20_OK != ds18b20__GetTemperatureCWithChecking(&ds18b20_oneWire, i, &temperature, DS18B20_TEMP_CHECK_PERIOD_MS, DS18B20_CHECKSUM))
            {
                ESP_LOGI(TAG, "Failure while reading temperature from device no. %d...", i);
            }
            else
            {
                ESP_LOGI(TAG, "Temperature %d: %.4f", i, temperature);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(DS18B20_TASK_PERIOD_MS));
    }
}

void ds18b20_store_registers_test(void)
{
    DS18B20_onewire_t ds18b20_oneWire;
    DS18B20_t ds18b20_devices[DS18B20_DEVICES_NO];
    DS18B20_config_t ds18b20_config =
    {
        .upperAlarm = DS18B20_UPPER_ALARM,
        .lowerAlarm = DS18B20_LOWER_ALARM,
        .resolution = DS18B20_RESOLUTION
    };

    if (DS18B20_OK != ds18b20__InitOneWire(&ds18b20_oneWire, DS18B20_1W_BUS, ds18b20_devices, DS18B20_DEVICES_NO, DS18B20_CHECKSUM))
    {
        ESP_LOGI(TAG, "Failure while initializing DS18B20 One-Wire driver.");
        return;
    }

    for (size_t i = 0; i < DS18B20_DEVICES_NO; ++i)
    {
        ESP_LOGI(TAG, "Address %d: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x", i,
            ds18b20_devices[i].rom[0], ds18b20_devices[i].rom[1], ds18b20_devices[i].rom[2], ds18b20_devices[i].rom[3],
            ds18b20_devices[i].rom[4], ds18b20_devices[i].rom[5], ds18b20_devices[i].rom[6], ds18b20_devices[i].rom[7]
        );
        ESP_LOGI(TAG, "Scratchpad %d: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x", i,
            ds18b20_devices[i].scratchpad[0], ds18b20_devices[i].scratchpad[1], ds18b20_devices[i].scratchpad[2], 
            ds18b20_devices[i].scratchpad[3], ds18b20_devices[i].scratchpad[4], ds18b20_devices[i].scratchpad[5], 
            ds18b20_devices[i].scratchpad[6], ds18b20_devices[i].scratchpad[7], ds18b20_devices[i].scratchpad[8]
        );
        ESP_LOGI(TAG, "Resolution %d: %d", i, ds18b20_devices[i].resolution + 9);
        ESP_LOGI(TAG, "Power mode %d: %d", i, ds18b20_devices[i].powerMode);

        if (DS18B20_OK != ds18b20__Configure(&ds18b20_oneWire, i, &ds18b20_config, DS18B20_CHECKSUM))
        {
            ESP_LOGI(TAG, "Failure while configuring device no. %d.", i);
            return;
        }

        ESP_LOGI(TAG, "Successfully configured device no. %d.", i);

        ESP_LOGI(TAG, "Scratchpad %d: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x", i,
            ds18b20_devices[i].scratchpad[0], ds18b20_devices[i].scratchpad[1], ds18b20_devices[i].scratchpad[2], 
            ds18b20_devices[i].scratchpad[3], ds18b20_devices[i].scratchpad[4], ds18b20_devices[i].scratchpad[5], 
            ds18b20_devices[i].scratchpad[6], ds18b20_devices[i].scratchpad[7], ds18b20_devices[i].scratchpad[8]
        );
        ESP_LOGI(TAG, "Resolution %d: %d", i, ds18b20_devices[i].resolution + 9);
    }

    for (size_t i = 0; i < DS18B20_DEVICES_NO; ++i)
    {
        while (DS18B20_OK != ds18b20__StoreRegistersWithChecking(&ds18b20_oneWire, i, DS18B20_STORE_CHECK_PERIOD_MS))
        {
            ESP_LOGI(TAG, "Failure while trying to store registers into EEPROM (device no. %d).", i);

            vTaskDelay(pdMS_TO_TICKS(DS18B20_TASK_PERIOD_MS));
        }
        
        ESP_LOGI(TAG, "Successfully stored registers into EEPROM (device no. %d).", i);
    }

    return;
}

void ds18b20_restore_registers_test(void)
{
    DS18B20_onewire_t ds18b20_oneWire;
    DS18B20_t ds18b20_devices[DS18B20_DEVICES_NO];
    DS18B20_config_t ds18b20_config =
    {
        .upperAlarm = DS18B20_UPPER_ALARM,
        .lowerAlarm = DS18B20_LOWER_ALARM,
        .resolution = DS18B20_RESOLUTION
    };

    if (DS18B20_OK != ds18b20__InitOneWire(&ds18b20_oneWire, DS18B20_1W_BUS, ds18b20_devices, DS18B20_DEVICES_NO, DS18B20_CHECKSUM))
    {
        ESP_LOGI(TAG, "Failure while initializing DS18B20 One-Wire driver.");
        return;
    }

    for (size_t i = 0; i < DS18B20_DEVICES_NO; ++i)
    {
        ESP_LOGI(TAG, "Address %d: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x", i,
            ds18b20_devices[i].rom[0], ds18b20_devices[i].rom[1], ds18b20_devices[i].rom[2], ds18b20_devices[i].rom[3],
            ds18b20_devices[i].rom[4], ds18b20_devices[i].rom[5], ds18b20_devices[i].rom[6], ds18b20_devices[i].rom[7]
        );
        ESP_LOGI(TAG, "Scratchpad %d: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x", i,
            ds18b20_devices[i].scratchpad[0], ds18b20_devices[i].scratchpad[1], ds18b20_devices[i].scratchpad[2], 
            ds18b20_devices[i].scratchpad[3], ds18b20_devices[i].scratchpad[4], ds18b20_devices[i].scratchpad[5], 
            ds18b20_devices[i].scratchpad[6], ds18b20_devices[i].scratchpad[7], ds18b20_devices[i].scratchpad[8]
        );
        ESP_LOGI(TAG, "Resolution %d: %d", i, ds18b20_devices[i].resolution + 9);
        ESP_LOGI(TAG, "Power mode %d: %d", i, ds18b20_devices[i].powerMode);

        if (DS18B20_OK != ds18b20__Configure(&ds18b20_oneWire, i, &ds18b20_config, DS18B20_CHECKSUM))
        {
            ESP_LOGI(TAG, "Failure while configuring device no. %d.", i);
            return;
        }

        ESP_LOGI(TAG, "Successfully configured device no. %d.", i);

        ESP_LOGI(TAG, "Scratchpad %d: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x", i,
            ds18b20_devices[i].scratchpad[0], ds18b20_devices[i].scratchpad[1], ds18b20_devices[i].scratchpad[2], 
            ds18b20_devices[i].scratchpad[3], ds18b20_devices[i].scratchpad[4], ds18b20_devices[i].scratchpad[5], 
            ds18b20_devices[i].scratchpad[6], ds18b20_devices[i].scratchpad[7], ds18b20_devices[i].scratchpad[8]
        );
        ESP_LOGI(TAG, "Resolution %d: %d", i, ds18b20_devices[i].resolution + 9);
    }

    for (size_t i = 0; i < DS18B20_DEVICES_NO; ++i)
    {
        while (DS18B20_OK != ds18b20__RestoreRegistersWithChecking(&ds18b20_oneWire, i, DS18B20_RESTORE_CHECK_PERIOD_MS, DS18B20_CHECKSUM))
        {
            ESP_LOGI(TAG, "Failure while trying to store registers into EEPROM (device no. %d).", i);

            vTaskDelay(pdMS_TO_TICKS(DS18B20_TASK_PERIOD_MS));
        }
        
        ESP_LOGI(TAG, "Successfully restored registers from EEPROM (device no. %d).", i);

        ESP_LOGI(TAG, "Scratchpad %d: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x", i,
            ds18b20_devices[i].scratchpad[0], ds18b20_devices[i].scratchpad[1], ds18b20_devices[i].scratchpad[2], 
            ds18b20_devices[i].scratchpad[3], ds18b20_devices[i].scratchpad[4], ds18b20_devices[i].scratchpad[5], 
            ds18b20_devices[i].scratchpad[6], ds18b20_devices[i].scratchpad[7], ds18b20_devices[i].scratchpad[8]
        );
        ESP_LOGI(TAG, "Resolution %d: %d", i, ds18b20_devices[i].resolution + 9);
    }

    return;
}

void ds18b20_find_alarms_test(void)
{
    DS18B20_onewire_t ds18b20_oneWire;
    DS18B20_t ds18b20_devices[DS18B20_DEVICES_NO];
    DS18B20_config_t ds18b20_config =
    {
        .upperAlarm = DS18B20_UPPER_ALARM,
        .lowerAlarm = DS18B20_LOWER_ALARM,
        .resolution = DS18B20_RESOLUTION
    };

    if (DS18B20_OK != ds18b20__InitOneWire(&ds18b20_oneWire, DS18B20_1W_BUS, ds18b20_devices, DS18B20_DEVICES_NO, DS18B20_CHECKSUM))
    {
        ESP_LOGI(TAG, "Failure while initializing DS18B20 One-Wire driver.");
        return;
    }

    for (size_t i = 0; i < DS18B20_DEVICES_NO; ++i)
    {
        ESP_LOGI(TAG, "Address %d: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x", i,
            ds18b20_devices[i].rom[0], ds18b20_devices[i].rom[1], ds18b20_devices[i].rom[2], ds18b20_devices[i].rom[3],
            ds18b20_devices[i].rom[4], ds18b20_devices[i].rom[5], ds18b20_devices[i].rom[6], ds18b20_devices[i].rom[7]
        );
        ESP_LOGI(TAG, "Scratchpad %d: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x", i,
            ds18b20_devices[i].scratchpad[0], ds18b20_devices[i].scratchpad[1], ds18b20_devices[i].scratchpad[2], 
            ds18b20_devices[i].scratchpad[3], ds18b20_devices[i].scratchpad[4], ds18b20_devices[i].scratchpad[5], 
            ds18b20_devices[i].scratchpad[6], ds18b20_devices[i].scratchpad[7], ds18b20_devices[i].scratchpad[8]
        );
        ESP_LOGI(TAG, "Resolution %d: %d", i, ds18b20_devices[i].resolution + 9);
        ESP_LOGI(TAG, "Power mode %d: %d", i, ds18b20_devices[i].powerMode);

        if (DS18B20_OK != ds18b20__Configure(&ds18b20_oneWire, i, &ds18b20_config, DS18B20_CHECKSUM))
        {
            ESP_LOGI(TAG, "Failure while configuring device no. %d.", i);
            return;
        }

        ESP_LOGI(TAG, "Successfully configured device no. %d.", i);

        ESP_LOGI(TAG, "Scratchpad %d: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x", i,
            ds18b20_devices[i].scratchpad[0], ds18b20_devices[i].scratchpad[1], ds18b20_devices[i].scratchpad[2], 
            ds18b20_devices[i].scratchpad[3], ds18b20_devices[i].scratchpad[4], ds18b20_devices[i].scratchpad[5], 
            ds18b20_devices[i].scratchpad[6], ds18b20_devices[i].scratchpad[7], ds18b20_devices[i].scratchpad[8]
        );
        ESP_LOGI(TAG, "Resolution %d: %d", i, ds18b20_devices[i].resolution + 9);
    }

    while (1)
    {
        for (size_t i = 0; i < DS18B20_DEVICES_NO; ++i)
        {
            if (DS18B20_OK != ds18b20__RequestTemperatureC(&ds18b20_oneWire, i))
            {
                ESP_LOGI(TAG, "Failure while requesting temperature from device no. %d...", i);
            }
            else
            {
                ESP_LOGI(TAG, "Temperature requested from device no. %d!", i);
            }
        }
        ESP_LOGI(TAG, "Searching for alarms...");
        
        size_t deviceIndex;
        while (DS18B20_OK == ds18b20__FindNextAlarm(&ds18b20_oneWire, &deviceIndex, DS18B20_CHECKSUM))
        {
            ESP_LOGI(TAG, "Alarm found in device no. %d!", deviceIndex);
        }

        vTaskDelay(pdMS_TO_TICKS(DS18B20_TASK_PERIOD_MS));
    }
}