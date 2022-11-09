#ifndef DS18B20_LOW_H
#define DS18B20_LOW_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "ds18b20_types.h"
#include "ds18b20_error_codes.h"

#define DS18B20_ROM_SIZE    8
#define DS18B20_SP_SIZE     9

typedef struct  DS18B20_onewire_t           DS18B20_onewire_t;
typedef struct  DS18B20_t                   DS18B20_t;

typedef int16_t                             DS18B20_gpio_t;
typedef uint8_t                             DS18B20_rom_t[DS18B20_ROM_SIZE];
typedef uint8_t                             DS18B20_scratchpad_t[DS18B20_SP_SIZE];

struct DS18B20_t
{
    DS18B20_rom_t                           rom;
    DS18B20_scratchpad_t                    scratchpad;
    DS18B20_resolution_t                    resolution;
    DS18B20_powermode_t                     powerMode;
};

struct DS18B20_onewire_t
{
    DS18B20_gpio_t                          bus;
    DS18B20_t                               *devices;
    size_t                                  devicesNo;

    size_t                                  lastSearchedDeviceNumber; 
    int8_t                                  lastSearchConflictUnresolved;
    int8_t                                  lastSearchConflict;
    bool                                    alarmSearchMode;
};

/* Basic functions */
void ds18b20_write_bit(const DS18B20_onewire_t * const onewire, const uint8_t bit);
void ds18b20_write_byte(const DS18B20_onewire_t * const onewire, const uint8_t byte);
uint8_t ds18b20_read_bit(const DS18B20_onewire_t * const onewire);
uint8_t ds18b20_read_byte(const DS18B20_onewire_t * const onewire);
uint8_t ds18b20_reset(const DS18B20_onewire_t * const onewire);
void ds18b20_parasite_start_pullup(const DS18B20_onewire_t * const onewire);
void ds18b20_parasite_end_pullup(const DS18B20_onewire_t * const onewire);

/* ROM commands */
DS18B20_error_t ds18b20_search_rom(DS18B20_onewire_t * const onewire, DS18B20_rom_t * buffer, const bool alarmSearchMode);
DS18B20_error_t ds18b20_read_rom(const DS18B20_onewire_t * const onewire);
DS18B20_error_t ds18b20_select(const DS18B20_onewire_t * const onewire, const size_t deviceIndex);
DS18B20_error_t ds18b20_skip_select(const DS18B20_onewire_t * const onewire);

/* Function commands */
DS18B20_error_t ds18b20_convert_temperature(const DS18B20_onewire_t * const onewire, const size_t deviceIndex);
DS18B20_error_t ds18b20_write_scratchpad(const DS18B20_onewire_t * const onewire, const size_t deviceIndex);
DS18B20_error_t ds18b20_read_scratchpad(const DS18B20_onewire_t * const onewire, const size_t deviceIndex);
DS18B20_error_t ds18b20_read_scratchpad_with_stop(const DS18B20_onewire_t * const onewire, const size_t deviceIndex, uint8_t bytesToRead);
DS18B20_error_t ds18b20_copy_scratchpad(const DS18B20_onewire_t * const onewire, const size_t deviceIndex);
DS18B20_error_t ds18b20_recall_e2(const DS18B20_onewire_t * const onewire);
DS18B20_error_t ds18b20_read_powermode(const DS18B20_onewire_t * const onewire, const size_t deviceIndex);

/* Helpers */
DS18B20_error_t ds18b20_restart_search(DS18B20_onewire_t * const onewire, const bool alarmSearchMode);
uint16_t ds18b20_millis_to_wait_for_convertion(const DS18B20_resolution_t resolution);

#endif /* DS18B20_LOW_H */
