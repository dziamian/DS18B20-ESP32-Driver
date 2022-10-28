#include "ds18b20_converter.h"

#include "ds18b20_registers.h"

#define DS18B20_CONFIG_BYTE_MASK                0x1F
#define DS18B20_CONFIG_BYTE_CONVERTER_SHIFT     5
#define DS18B20_CONFIG_BYTE_CONVERTER_MASK      0x03

#define DS18B20_TEMP_CONVERTER_NEGATIVE_BASE    4096
#define DS18B20_TEMP_CONVERTER_MSB_DIVIDER      128
#define DS18B20_TEMP_CONVERTER_MSB_MULTIPLIER   256
#define DS18B20_TEMP_CONVERTER_LSB_DIVIDER      16

#define DS18B20_RESOLUTION_09_MASK              0xF8
#define DS18B20_RESOLUTION_10_MASK              0xFC
#define DS18B20_RESOLUTION_11_MASK              0xFE
#define DS18B20_RESOLUTION_12_MASK              0xFF

static uint8_t resolution_masks[DS18B20_RESOLUTION_COUNT] =
{
    [DS18B20_RESOLUTION_09] DS18B20_RESOLUTION_09_MASK,
    [DS18B20_RESOLUTION_10] DS18B20_RESOLUTION_10_MASK,
    [DS18B20_RESOLUTION_11] DS18B20_RESOLUTION_11_MASK,
    [DS18B20_RESOLUTION_12] DS18B20_RESOLUTION_12_MASK
};

DS18B20_temperature_out_t ds18b20_convert_temperature_bytes(uint8_t msb, uint8_t lsb, DS18B20_resolution_t resolution)
{
    lsb &= resolution_masks[resolution];
    return (DS18B20_temperature_out_t) 
                ((int16_t) (lsb + (msb * DS18B20_TEMP_CONVERTER_MSB_MULTIPLIER))) 
                    / DS18B20_TEMP_CONVERTER_LSB_DIVIDER 
                - DS18B20_TEMP_CONVERTER_NEGATIVE_BASE * (msb / DS18B20_TEMP_CONVERTER_MSB_DIVIDER);
}

uint8_t ds18b20_resolution_to_config_byte(DS18B20_resolution_t resolution)
{
    return DS18B20_CONFIG_BYTE_MASK | ((uint8_t) resolution << DS18B20_CONFIG_BYTE_CONVERTER_SHIFT);
}

DS18B20_resolution_t ds18b20_config_byte_to_resolution(uint8_t config_byte)
{
    return (DS18B20_resolution_t)
                (config_byte >> DS18B20_CONFIG_BYTE_CONVERTER_SHIFT)
                    & DS18B20_CONFIG_BYTE_CONVERTER_MASK;
}