#ifndef DS18B20_H
#define DS18B20_H

#include <stdbool.h>

#include "ds18b20_low.h"
#include "ds18b20_error_codes.h"

#define DS18B20_NO_CHECK_PERIOD 0
#define DS18B20_TEMP_MIN        -55
#define DS18B20_TEMP_MAX        125

typedef struct DS18B20_config_t DS18B20_config_t;

struct DS18B20_config_t
{
    DS18B20_temperature_in_t upperAlarm;
    DS18B20_temperature_in_t lowerAlarm;
    DS18B20_resolution_t resolution;
};

bool ds18b20__InitOneWire(DS18B20_onewire_t *onewire, DS18B20_gpio_t bus, DS18B20_t *devices, size_t devicesNo, bool checksum);
bool ds18b20__InitConfigDefault(DS18B20_config_t *config);

bool ds18b20__RequestTemperatureC(DS18B20_onewire_t *onewire, size_t deviceIndex);
bool ds18b20__GetTemperatureC(DS18B20_onewire_t *onewire, size_t deviceIndex, DS18B20_temperature_out_t *temperatureOut, bool checksum);
bool ds18b20__GetTemperatureCWithChecking(DS18B20_onewire_t *onewire, size_t deviceIndex, DS18B20_temperature_out_t *temperatureOut, uint16_t checkPeriodMs, bool checksum);
bool ds18b20__Configure(DS18B20_onewire_t *onewire, size_t deviceIndex, DS18B20_config_t *config, bool checksum);
bool ds18b20__FindNextAlarm(DS18B20_onewire_t *onewire, size_t *deviceIndexOut, bool checksum);

bool ds18b20__StoreRegisters(DS18B20_onewire_t *onewire, size_t deviceIndex);
bool ds18b20__StoreRegistersWithChecking(DS18B20_onewire_t *onewire, size_t deviceIndex, uint16_t checkPeriodMs);
bool ds18b20__RestoreRegisters(DS18B20_onewire_t *onewire, size_t deviceIndex, bool checksum);
bool ds18b20__RestoreRegistersWithChecking(DS18B20_onewire_t *onewire, size_t deviceIndex, uint16_t checkPeriodMs, bool checksum);

#endif /* DS18B20_H */
