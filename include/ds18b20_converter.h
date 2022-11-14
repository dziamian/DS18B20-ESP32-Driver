#ifndef DS18B20_CONVERTER_H
#define DS18B20_CONVERTER_H

/**
 * @file ds18b20_converter.h
 * @author Damian Ślusarczyk
 * @brief Contains all convertion methods used for proper communication between the user and DS18B20.
 * 
 */

#include "ds18b20_low.h"
#include "ds18b20_types_res.h"

/**
 * @brief Converts temperature bytes received from scratchpad of DS18B20 for specified resolution into human-readable result.
 * 
 * @param msb The most significant byte of temperature
 * @param lsb The least significant byte of temperature
 * @param resolution Resolution set on the device during convertion
 * @return DS18B20_temperature_out_t Human-readable temperature
 */
DS18B20_temperature_out_t ds18b20_convert_temperature_bytes(const uint8_t msb, uint8_t lsb, const DS18B20_resolution_t resolution);

/**
 * @brief Converts user-defined resolution into configuration byte readable for the device.
 * 
 * @param resolution Specified resolution
 * @return uint8_t Configuration byte readable for the DS18B20
 */
uint8_t ds18b20_resolution_to_config_byte(const DS18B20_resolution_t resolution);

/**
 * @brief Converts configuration byte received from scratchpad of DS18B20 into human-readable resolution.
 * 
 * @param config_byte Configuration byte
 * @return DS18B20_resolution_t Human-readable device resolution
 */
DS18B20_resolution_t ds18b20_config_byte_to_resolution(const uint8_t config_byte);

#endif /* DS18B20_CONVERTER_H */
