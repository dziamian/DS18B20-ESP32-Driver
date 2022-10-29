#ifndef DS18B20_LOW_H
#define DS18B20_LOW_H

#include <stdint.h>
#include <stddef.h>

#include "ds18b20_types.h"
#include "ds18b20_error_codes.h"

#define DS18B20_ROM_SIZE    8
#define DS18B20_SP_SIZE     9

typedef struct  DS18B20_onewire_t           DS18B20_onewire_t;
typedef struct  DS18B20_t                   DS18B20_t;

typedef int16_t                             DS18B20_gpio_t;
typedef uint8_t                             DS18B20_rom_t[DS18B20_ROM_SIZE];
typedef uint8_t                             DS18B20_scratchpad_t[DS18B20_SP_SIZE];

struct DS18B20_onewire_t
{
    DS18B20_gpio_t          bus;
    DS18B20_t               *devices;
    size_t                  devicesNo;

    size_t                  lastSearchedDeviceNumber; 
    int8_t                  lastSearchConflictUnresolved;
    int8_t                  lastSearchConflict;
};

struct DS18B20_t
{
    DS18B20_rom_t           rom;
    DS18B20_scratchpad_t    scratchpad;
    DS18B20_resolution_t    resolution;
    DS18B20_powermode_t     powerMode;
};

/* Basic functions */
void ds18b20_write_bit(DS18B20_onewire_t *onewire, uint8_t bit);
void ds18b20_write_byte(DS18B20_onewire_t *onewire, uint8_t byte);
uint8_t ds18b20_read_bit(DS18B20_onewire_t *onewire);
uint8_t ds18b20_read_byte(DS18B20_onewire_t *onewire);
uint8_t ds18b20_reset(DS18B20_onewire_t *onewire);
void ds18b20_parasite_end_pullup(DS18B20_onewire_t *onewire);
uint8_t ds18b20_restart_search(DS18B20_onewire_t *onewire);

/* ROM commands */
uint8_t ds18b20_search_rom(DS18B20_onewire_t *onewire);
uint8_t ds18b20_read_rom(DS18B20_onewire_t *onewire);
uint8_t ds18b20_select(DS18B20_onewire_t *onewire, size_t deviceIndex);
uint8_t ds18b20_skip_select(DS18B20_onewire_t *onewire);
void ds18b20_search_alarm(DS18B20_onewire_t *onewire);

/* Function commands */
void ds18b20_convert_temperature(DS18B20_onewire_t *onewire, size_t deviceIndex);
void ds18b20_write_scratchpad(DS18B20_onewire_t *onewire, size_t deviceIndex);
uint8_t ds18b20_read_scratchpad(DS18B20_onewire_t *onewire, size_t deviceIndex);
uint8_t ds18b20_read_scratchpad_with_stop(DS18B20_onewire_t *onewire, size_t deviceIndex, uint8_t bytesToRead);
void ds18b20_copy_scratchpad(DS18B20_onewire_t *onewire, size_t deviceIndex);
void ds18b20_recall_e2(DS18B20_onewire_t *onewire);
void ds18b20_read_powermode(DS18B20_onewire_t *onewire, size_t deviceIndex);

/* Helpers */
uint16_t ds18b20_millis_to_wait_for_convertion(DS18B20_resolution_t resolution);

#endif /* DS18B20_LOW_H */
