#ifndef DS18B20_CONVERTER_H
#define DS18B20_CONVERTER_H

#include "ds18b20_low.h"

DS18B20_temperature_out_t ds18b20_convert_temperature_bytes(const uint8_t msb, uint8_t lsb, const DS18B20_resolution_t resolution);
uint8_t ds18b20_resolution_to_config_byte(const DS18B20_resolution_t resolution);
DS18B20_resolution_t ds18b20_config_byte_to_resolution(const uint8_t config_byte);

#endif /* DS18B20_CONVERTER_H */
