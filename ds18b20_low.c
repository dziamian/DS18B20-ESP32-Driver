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

#define DS18B20_NO_SEARCHED_DEVICES 0
#define DS18B20_NO_SEARCH_CONFLICTS -1

#define noInterrupts()              portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;taskENTER_CRITICAL(&mux)
#define interrupts()                taskEXIT_CRITICAL(&mux)

static uint16_t resolution_delays_ms[DS18B20_RESOLUTION_COUNT] =
{
    [DS18B20_RESOLUTION_09] DS18B20_RESOLUTION_09_DELAY_MS,
    [DS18B20_RESOLUTION_10] DS18B20_RESOLUTION_10_DELAY_MS,
    [DS18B20_RESOLUTION_11] DS18B20_RESOLUTION_11_DELAY_MS,
    [DS18B20_RESOLUTION_12] DS18B20_RESOLUTION_12_DELAY_MS
};

void ds18b20_write_bit(DS18B20_onewire_t *onewire, uint8_t bit)
{
    gpio_set_direction(onewire->bus, GPIO_MODE_OUTPUT);
    
    noInterrupts();
        gpio_set_level(onewire->bus, DS18B20_LEVEL_LOW);
        ets_delay_us(bit ? WRITE_BIT1_DELAY0_US : WRITE_BIT0_DELAY0_US);
        gpio_set_direction(onewire->bus, GPIO_MODE_INPUT);
        ets_delay_us(bit ? WRITE_BIT1_DELAY1_US : WRITE_BIT0_DELAY1_US);
    interrupts();
}

void ds18b20_write_byte(DS18B20_onewire_t *onewire, uint8_t byte)
{
    for (uint8_t mask = 1; mask != 0; mask <<= 1)
    {
        ds18b20_write_bit(onewire, byte & mask);
    }
}

uint8_t ds18b20_read_bit(DS18B20_onewire_t *onewire)
{
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

uint8_t ds18b20_read_byte(DS18B20_onewire_t *onewire)
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

uint8_t ds18b20_reset(DS18B20_onewire_t *onewire)
{
    gpio_set_direction(onewire->bus, GPIO_MODE_OUTPUT);
    
    noInterrupts();
        gpio_set_level(onewire->bus, DS18B20_LEVEL_LOW);
        ets_delay_us(RESET_DELAY0);
        gpio_set_level(onewire->bus, DS18B20_LEVEL_HIGH);
        gpio_set_direction(onewire->bus, GPIO_MODE_INPUT);
        ets_delay_us(RESET_DELAY1);
        uint8_t presence = !gpio_get_level(onewire->bus);
        ets_delay_us(RESET_DELAY2);
    interrupts();

    return presence;
}

//TODO:
void ds18b20_end_pullup_optionally(DS18B20_onewire_t *onewire, size_t deviceIndex);

uint8_t ds18b20_restart_search(DS18B20_onewire_t *onewire)
{
    if (!onewire)
    {
        return DS18B20_ERR;
    }

    onewire->lastSearchedDeviceNumber = DS18B20_NO_SEARCHED_DEVICES;
    onewire->lastSearchConflictUnresolved = DS18B20_NO_SEARCH_CONFLICTS;
    onewire->lastSearchConflict = DS18B20_NO_SEARCH_CONFLICTS;

    return DS18B20_OK;
}

uint8_t ds18b20_search_rom(DS18B20_onewire_t *onewire)
{
    if (!onewire)
    {
        return DS18B20_ERR;
    }

    if (DS18B20_NO_SEARCH_CONFLICTS == onewire->lastSearchConflict 
        && DS18B20_NO_SEARCHED_DEVICES != onewire->lastSearchedDeviceNumber)
    {   // Restart search procedure to first cycle
        // FIXME: two different errors
        if (DS18B20_OK != ds18b20_restart_search(onewire))
        {
            return DS18B20_ERR;
        }
        return DS18B20_ERR;
    }

    if (!ds18b20_reset(onewire))
    {
        return DS18B20_ERR;
    }

    ds18b20_write_byte(onewire, DS18B20_SEARCH_ROM);

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
                // FIXME: two different errors
                if (DS18B20_OK != ds18b20_restart_search(onewire))
                {
                    return DS18B20_ERR;
                }
                return DS18B20_ERR;
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
                onewire->devices[onewire->lastSearchedDeviceNumber].rom[byteNo] |= bitMask;
            }

            ++romBitNo;
        }
    }

    ++onewire->lastSearchedDeviceNumber;

    return DS18B20_OK;
}

uint8_t ds18b20_read_rom(DS18B20_onewire_t *onewire)
{
    if (!ds18b20_reset(onewire))
    {
        return DS18B20_ERR;
    }

    ds18b20_write_byte(onewire, DS18B20_READ_ROM);

    for (uint8_t i = 0; i < DS18B20_ROM_SIZE; ++i)
    {
        onewire->devices->rom[i] = ds18b20_read_byte(onewire);
    }

    return ds18b20_reset(onewire);
}

uint8_t ds18b20_select(DS18B20_onewire_t *onewire, size_t deviceIndex)
{
    if (!ds18b20_reset(onewire))
    {
        return DS18B20_ERR;
    }

    ds18b20_write_byte(onewire, DS18B20_MATCH_ROM);
    for (uint8_t i = 0; i < DS18B20_ROM_SIZE; ++i)
    {
        ds18b20_write_byte(onewire, onewire->devices[deviceIndex].rom[i]);
    }

    return DS18B20_OK;
}

uint8_t ds18b20_skip_select(DS18B20_onewire_t *onewire)
{
    if (!ds18b20_reset(onewire))
    {
        return DS18B20_ERR;
    }
    
    ds18b20_write_byte(onewire, DS18B20_SKIP_ROM);

    return DS18B20_OK;
}

//TODO:
void ds18b20_search_alarm(DS18B20_onewire_t *onewire);

void ds18b20_convert_temperature(DS18B20_onewire_t *onewire, size_t deviceIndex)
{
    ds18b20_write_byte(onewire, DS18B20_CONVERT_T);

    // TODO: handle parasite with noInterrupts
}

void ds18b20_write_scratchpad(DS18B20_onewire_t *onewire, size_t deviceIndex)
{
    ds18b20_write_byte(onewire, DS18B20_WRITE_SCRATCHPAD);
    ds18b20_write_byte(onewire, onewire->devices[deviceIndex].scratchpad[DS18B20_SP_TEMP_HIGH_BYTE]);
    ds18b20_write_byte(onewire, onewire->devices[deviceIndex].scratchpad[DS18B20_SP_TEMP_LOW_BYTE]);
    ds18b20_write_byte(onewire, onewire->devices[deviceIndex].scratchpad[DS18B20_SP_CONFIG_BYTE]);
}

uint8_t ds18b20_read_scratchpad(DS18B20_onewire_t *onewire, size_t deviceIndex)
{
    return ds18b20_read_scratchpad_with_stop(onewire, deviceIndex, DS18B20_SP_SIZE);
}

uint8_t ds18b20_read_scratchpad_with_stop(DS18B20_onewire_t *onewire, size_t deviceIndex, uint8_t bytesToRead)
{
    if (bytesToRead > DS18B20_SP_SIZE)
    {
        bytesToRead = DS18B20_SP_SIZE;
    }

    ds18b20_write_byte(onewire, DS18B20_READ_SCRATCHPAD);

    for (uint8_t i = 0; i < bytesToRead; ++i)
    {
        onewire->devices[deviceIndex].scratchpad[i] = ds18b20_read_byte(onewire);
    }

    return ds18b20_reset(onewire);
}

void ds18b20_copy_scratchpad(DS18B20_onewire_t *onewire, size_t deviceIndex)
{
    ds18b20_write_byte(onewire, DS18B20_COPY_SCRATCHPAD);

    // TODO: handle parasite with noInterrupts
}

void ds18b20_recall_e2(DS18B20_onewire_t *onewire)
{
    ds18b20_write_byte(onewire, DS18B20_RECALL_E2);
}

void ds18b20_read_powermode(DS18B20_onewire_t *onewire, size_t deviceIndex)
{
    ds18b20_write_byte(onewire, DS18B20_READ_POWER_SUPPLY);

    onewire->devices[deviceIndex].powerMode = ds18b20_read_bit(onewire);
}

uint16_t ds18b20_millis_to_wait_for_convertion(DS18B20_resolution_t resolution)
{
    return resolution_delays_ms[resolution];
}