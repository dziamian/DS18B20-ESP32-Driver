#ifndef DS18B20_H
#define DS18B20_H

#include <stdbool.h>

#include "ds18b20_low.h"
#include "ds18b20_types_res.h"
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

DS18B20_error_t ds18b20__InitOneWire(DS18B20_onewire_t * const onewire, const DS18B20_gpio_t bus, DS18B20_t * const devices, const size_t devicesNo, const bool checksum);
DS18B20_error_t ds18b20__InitConfigDefault(DS18B20_config_t * const config);

DS18B20_error_t ds18b20__RequestTemperatureC(const DS18B20_onewire_t * const onewire, const size_t deviceIndex);
DS18B20_error_t ds18b20__GetTemperatureC(const DS18B20_onewire_t * const onewire, const size_t deviceIndex, DS18B20_temperature_out_t * const temperatureOut, const bool checksum);
DS18B20_error_t ds18b20__GetTemperatureCWithChecking(const DS18B20_onewire_t * const onewire, const size_t deviceIndex, DS18B20_temperature_out_t * const temperatureOut, uint16_t checkPeriodMs, const bool checksum);
DS18B20_error_t ds18b20__Configure(const DS18B20_onewire_t * const onewire, const size_t deviceIndex, const DS18B20_config_t * const config, const bool checksum);
DS18B20_error_t ds18b20__FindNextAlarm(DS18B20_onewire_t * const onewire, size_t * const deviceIndexOut, const bool checksum);

DS18B20_error_t ds18b20__StoreRegisters(const DS18B20_onewire_t * const onewire, const size_t deviceIndex);
DS18B20_error_t ds18b20__StoreRegistersWithChecking(const DS18B20_onewire_t * const onewire, const size_t deviceIndex, uint16_t checkPeriodMs);
DS18B20_error_t ds18b20__RestoreRegisters(const DS18B20_onewire_t * const onewire, const size_t deviceIndex, const bool checksum);
DS18B20_error_t ds18b20__RestoreRegistersWithChecking(const DS18B20_onewire_t * const onewire, const size_t deviceIndex, uint16_t checkPeriodMs, const bool checksum);

#endif /* DS18B20_H */
