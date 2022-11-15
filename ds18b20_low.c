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
#include "ds18b20_low.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp32/rom/ets_sys.h"

#include "ds18b20_commands.h"
#include "ds18b20_registers.h"
#include "ds18b20_specifications.h"
#include "ds18b20_timeslots.h"
#include "ds18b20_helpers.h"

/** Means that no devices has been found yet during search procedure */
#define DS18B20_NO_SEARCHED_DEVICES 0
/** Means that no conflicts occurred during the last search cycle */
#define DS18B20_NO_SEARCH_CONFLICTS -1

/** Means that bit was read incorrectly */
#define DS18B20_INVALID_READ        2
/** Means that DS18B20 device did not replied to the reset signal */
#define DS18B20_ABSENCE             0

/** Macro which disables FreeRTOS interrupts */
#define noInterrupts()              portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;taskENTER_CRITICAL(&mux)
/** Macro which enables back FreeRTOS interrupts */
#define interrupts()                taskEXIT_CRITICAL(&mux)

/** Look-up table for maximum temperature convertion waiting time and resolutions */
static const uint16_t resolution_delays_ms[DS18B20_RESOLUTION_COUNT] =
{
    [DS18B20_RESOLUTION_09] DS18B20_RESOLUTION_09_DELAY_MS,
    [DS18B20_RESOLUTION_10] DS18B20_RESOLUTION_10_DELAY_MS,
    [DS18B20_RESOLUTION_11] DS18B20_RESOLUTION_11_DELAY_MS,
    [DS18B20_RESOLUTION_12] DS18B20_RESOLUTION_12_DELAY_MS
};

void ds18b20_write_bit(const DS18B20_onewire_t * const onewire, const uint8_t bit)
{
    if (!onewire)
    {
        return;
    }

    gpio_set_direction(onewire->bus, GPIO_MODE_OUTPUT);
    
    noInterrupts();
        gpio_set_level(onewire->bus, DS18B20_LEVEL_LOW);
        ets_delay_us(bit ? WRITE_BIT1_DELAY0_US : WRITE_BIT0_DELAY0_US);
        gpio_set_direction(onewire->bus, GPIO_MODE_INPUT);
        ets_delay_us(bit ? WRITE_BIT1_DELAY1_US : WRITE_BIT0_DELAY1_US);
    interrupts();
}

void ds18b20_write_byte(const DS18B20_onewire_t * const onewire, const uint8_t byte)
{
    for (uint8_t mask = 1; mask != 0; mask <<= 1)
    {
        ds18b20_write_bit(onewire, byte & mask);
    }
}

uint8_t ds18b20_read_bit(const DS18B20_onewire_t * const onewire)
{
    if (!onewire)
    {
        return DS18B20_INVALID_READ;
    }
    
    gpio_set_direction(onewire->bus, GPIO_MODE_OUTPUT);
    
    noInterrupts();
        gpio_set_level(onewire->bus, DS18B20_LEVEL_LOW);
        ets_delay_us(READ_BIT_DELAY0_US);
        gpio_set_direction(onewire->bus, GPIO_MODE_INPUT);
        ets_delay_us(READ_BIT_DELAY1_US);
        uint8_t data = gpio_get_level(onewire->bus);
        ets_delay_us(READ_BIT_DELAY2_US);
    interrupts();
    
    return data;
}

uint8_t ds18b20_read_byte(const DS18B20_onewire_t * const onewire)
{
    uint8_t data = 0;

    for (uint8_t mask = 1; mask != 0; mask <<= 1)
    {
        if (ds18b20_read_bit(onewire))
        {
            data |= mask;
        }
    }

    return data;
}

uint8_t ds18b20_reset(const DS18B20_onewire_t * const onewire)
{
    if (!onewire)
    {
        return DS18B20_ABSENCE;
    }
    
    gpio_set_direction(onewire->bus, GPIO_MODE_OUTPUT);
    
    noInterrupts();
        gpio_set_level(onewire->bus, DS18B20_LEVEL_LOW);
        ets_delay_us(RESET_DELAY0_US);
        gpio_set_level(onewire->bus, DS18B20_LEVEL_HIGH);
        gpio_set_direction(onewire->bus, GPIO_MODE_INPUT);
        ets_delay_us(RESET_DELAY1_US);
        uint8_t presence = !gpio_get_level(onewire->bus);
        ets_delay_us(RESET_DELAY2_US);
    interrupts();

    return presence;
}

void ds18b20_parasite_start_pullup(const DS18B20_onewire_t * const onewire)
{
    if (!onewire)
    {
        return;
    }

    gpio_set_direction(onewire->bus, GPIO_MODE_OUTPUT);
    gpio_set_level(onewire->bus, DS18B20_LEVEL_HIGH);
}

void ds18b20_parasite_end_pullup(const DS18B20_onewire_t * const onewire)
{
    if (!onewire)
    {
        return;
    }

    gpio_set_direction(onewire->bus, GPIO_MODE_INPUT);
}

DS18B20_error_t ds18b20_search_rom(DS18B20_onewire_t * const onewire, DS18B20_rom_t * buffer, const bool alarmSearchMode)
{
    DS18B20_error_t status;
    if (!onewire)
    {
        return DS18B20_INV_ARG;
    }

    // Restart search procedure only if modes are not the same,
    // so there is no need to do it manually.
    if (alarmSearchMode != onewire->alarmSearchMode)
    {
        status = ds18b20_restart_search(onewire, alarmSearchMode);
        if (DS18B20_OK != status)
        {
            return status;
        }
    }
    else if (DS18B20_NO_SEARCH_CONFLICTS == onewire->lastSearchConflict 
        && DS18B20_NO_SEARCHED_DEVICES != onewire->lastSearchedDeviceNumber)
    {   // Restart search procedure to first cycle when it has been finished.
        status = ds18b20_restart_search(onewire, alarmSearchMode);
        if (DS18B20_OK != status)
        {
            return status;
        }
        return DS18B20_NO_MORE_DEVICES;
    }

    if (!buffer)
    {   // Buffer can only be set if this is not alarm search.
        if (!alarmSearchMode)
        {
            buffer = &onewire->devices[onewire->lastSearchedDeviceNumber].rom;
        }
        else
        {
            return DS18B20_INV_ARG;
        }
    }

    if (!ds18b20_reset(onewire))
    {
        return DS18B20_DISCONNECTED;
    }

    uint8_t searchMode = alarmSearchMode ? DS18B20_ALARM_SEARCH : DS18B20_SEARCH_ROM;
    ds18b20_write_byte(onewire, searchMode);

    uint8_t bitRead = 0;
    uint8_t complementRead = 1;
    uint8_t bitSet = 0;
    
    uint8_t romBitNo = 0;
    for (uint8_t byteNo = 0; byteNo < DS18B20_ROM_SIZE; ++byteNo)
    {
        for (uint8_t bitMask = 1; bitMask != 0; bitMask <<= 1)
        {
            bitRead = ds18b20_read_bit(onewire);
            complementRead = ds18b20_read_bit(onewire);

            if (bitRead && complementRead)
            {   // No devices connected to bus (data: 11)
                status = ds18b20_restart_search(onewire, alarmSearchMode);
                if (DS18B20_OK != status)
                {
                    return status;
                }
                return DS18B20_NO_DEVICES;
            }

            if (!bitRead && !complementRead)
            {   // Devices with conflicting bits (data: 00)
                if (romBitNo < onewire->lastSearchConflict)
                {   // Make decision like the last time
                    bitSet = 0 != (onewire->devices[onewire->lastSearchedDeviceNumber - 1].rom[byteNo] & bitMask);
                    if (!bitSet)
                    {
                        onewire->lastSearchConflictUnresolved = romBitNo;
                    }
                }
                else if (romBitNo == onewire->lastSearchConflict)
                {   // Take bit = 1
                    bitSet = 1;

                    onewire->lastSearchConflict = onewire->lastSearchConflictUnresolved;
                    onewire->lastSearchConflictUnresolved = DS18B20_NO_SEARCH_CONFLICTS;
                }
                else
                {   // Take bit = 0
                    bitSet = 0;

                    onewire->lastSearchConflict = romBitNo;
                }

            }
            else
            {   // All devices have same bit (data: 01 or 10)
                bitSet = bitRead;
            }
            
            // Select roms
            ds18b20_write_bit(onewire, bitSet);
            // Set current bit to ROM address buffer
            if (bitSet)
            {
                (*buffer)[byteNo] |= bitMask;
            }

            ++romBitNo;
        }
    }

    ++onewire->lastSearchedDeviceNumber;

    return DS18B20_OK;
}

DS18B20_error_t ds18b20_read_rom(const DS18B20_onewire_t * const onewire)
{
    if (!onewire)
    {
        return DS18B20_INV_ARG;
    }

    if (onewire->devicesNo > DS18B20_1W_SINGLEDEVICE)
    {
        return DS18B20_INV_OP;
    }

    if (!ds18b20_reset(onewire))
    {
        return DS18B20_DISCONNECTED;
    }

    ds18b20_write_byte(onewire, DS18B20_READ_ROM);

    for (uint8_t i = 0; i < DS18B20_ROM_SIZE; ++i)
    {
        onewire->devices->rom[i] = ds18b20_read_byte(onewire);
    }

    if (!ds18b20_reset(onewire))
    {
        return DS18B20_DISCONNECTED;
    }
    return DS18B20_OK;
}

DS18B20_error_t ds18b20_select(const DS18B20_onewire_t * const onewire, const size_t deviceIndex)
{
    if (!onewire || deviceIndex >= onewire->devicesNo)
    {
        return DS18B20_INV_ARG;
    }

    if (!ds18b20_reset(onewire))
    {
        return DS18B20_DISCONNECTED;
    }

    ds18b20_write_byte(onewire, DS18B20_MATCH_ROM);
    for (uint8_t i = 0; i < DS18B20_ROM_SIZE; ++i)
    {
        ds18b20_write_byte(onewire, onewire->devices[deviceIndex].rom[i]);
    }

    return DS18B20_OK;
}

DS18B20_error_t ds18b20_skip_select(const DS18B20_onewire_t * const onewire)
{
    if (!onewire)
    {
        return DS18B20_INV_ARG;
    }

    if (onewire->devicesNo > DS18B20_1W_SINGLEDEVICE)
    {
        return DS18B20_INV_OP;
    }

    if (!ds18b20_reset(onewire))
    {
        return DS18B20_DISCONNECTED;
    }
    
    ds18b20_write_byte(onewire, DS18B20_SKIP_ROM);

    return DS18B20_OK;
}

DS18B20_error_t ds18b20_convert_temperature(const DS18B20_onewire_t * const onewire, const size_t deviceIndex)
{
    if (!onewire || deviceIndex >= onewire->devicesNo)
    {
        return DS18B20_INV_ARG;
    }
    
    uint8_t isParasite = DS18B20_PM_PARASITE == onewire->devices[deviceIndex].powerMode;
    if (!isParasite)
    {
        ds18b20_write_byte(onewire, DS18B20_CONVERT_T);
    }
    else
    {
        noInterrupts();
            ds18b20_write_byte(onewire, DS18B20_CONVERT_T);
            ds18b20_parasite_start_pullup(onewire);
        interrupts();
    }

    return DS18B20_OK;
}

DS18B20_error_t ds18b20_write_scratchpad(const DS18B20_onewire_t * const onewire, const size_t deviceIndex)
{
    if (!onewire || deviceIndex >= onewire->devicesNo)
    {
        return DS18B20_INV_ARG;
    }

    ds18b20_write_byte(onewire, DS18B20_WRITE_SCRATCHPAD);
    ds18b20_write_byte(onewire, onewire->devices[deviceIndex].scratchpad[DS18B20_SP_TEMP_HIGH_BYTE]);
    ds18b20_write_byte(onewire, onewire->devices[deviceIndex].scratchpad[DS18B20_SP_TEMP_LOW_BYTE]);
    ds18b20_write_byte(onewire, onewire->devices[deviceIndex].scratchpad[DS18B20_SP_CONFIG_BYTE]);

    return DS18B20_OK;
}

DS18B20_error_t ds18b20_read_scratchpad(const DS18B20_onewire_t * const onewire, const size_t deviceIndex)
{
    return ds18b20_read_scratchpad_with_stop(onewire, deviceIndex, DS18B20_SP_SIZE);
}

DS18B20_error_t ds18b20_read_scratchpad_with_stop(const DS18B20_onewire_t * const onewire, const size_t deviceIndex, uint8_t bytesToRead)
{
    if (!onewire || deviceIndex >= onewire->devicesNo)
    {
        return DS18B20_INV_ARG;
    }

    if (bytesToRead > DS18B20_SP_SIZE)
    {
        bytesToRead = DS18B20_SP_SIZE;
    }

    ds18b20_write_byte(onewire, DS18B20_READ_SCRATCHPAD);

    for (uint8_t i = 0; i < bytesToRead; ++i)
    {
        onewire->devices[deviceIndex].scratchpad[i] = ds18b20_read_byte(onewire);
    }

    if (!ds18b20_reset(onewire))
    {
        return DS18B20_DISCONNECTED;
    }
    return DS18B20_OK;
}

DS18B20_error_t ds18b20_copy_scratchpad(const DS18B20_onewire_t * const onewire, const size_t deviceIndex)
{
    if (!onewire || deviceIndex >= onewire->devicesNo)
    {
        return DS18B20_INV_ARG;
    }

    uint8_t isParasite = DS18B20_PM_PARASITE == onewire->devices[deviceIndex].powerMode;
    if (!isParasite)
    {
        ds18b20_write_byte(onewire, DS18B20_COPY_SCRATCHPAD);
    }
    else
    {
        noInterrupts();
            ds18b20_write_byte(onewire, DS18B20_COPY_SCRATCHPAD);
            ds18b20_parasite_start_pullup(onewire);
        interrupts();
    }

    return DS18B20_OK;
}

DS18B20_error_t ds18b20_recall_e2(const DS18B20_onewire_t * const onewire)
{
    if (!onewire)
    {
        return DS18B20_INV_ARG;
    }

    ds18b20_write_byte(onewire, DS18B20_RECALL_E2);

    return DS18B20_OK;
}

DS18B20_error_t ds18b20_read_powermode(const DS18B20_onewire_t * const onewire, const size_t deviceIndex)
{
    if (!onewire || deviceIndex >= onewire->devicesNo)
    {
        return DS18B20_INV_ARG;
    }
    
    ds18b20_write_byte(onewire, DS18B20_READ_POWER_SUPPLY);

    onewire->devices[deviceIndex].powerMode = ds18b20_read_bit(onewire);
    
    return DS18B20_OK;
}

DS18B20_error_t ds18b20_restart_search(DS18B20_onewire_t * const onewire, const bool alarmSearchMode)
{
    if (!onewire)
    {
        return DS18B20_INV_ARG;
    }

    onewire->lastSearchedDeviceNumber = DS18B20_NO_SEARCHED_DEVICES;
    onewire->lastSearchConflictUnresolved = DS18B20_NO_SEARCH_CONFLICTS;
    onewire->lastSearchConflict = DS18B20_NO_SEARCH_CONFLICTS;
    onewire->alarmSearchMode = alarmSearchMode;

    return DS18B20_OK;
}

uint16_t ds18b20_millis_to_wait_for_convertion(const DS18B20_resolution_t resolution)
{
    return resolution_delays_ms[resolution];
}