#ifndef DS18B20_TYPES_H
#define DS18B20_TYPES_H

typedef enum    DS18B20_powermode_t         DS18B20_powermode_t;
typedef enum    DS18B20_resolution_t        DS18B20_resolution_t;

typedef float                               DS18B20_temperature_out_t;
typedef int8_t                              DS18B20_temperature_in_t;

enum DS18B20_powermode_t
{
    DS18B20_PM_PARASITE = 0,
    DS18B20_PM_EXTERNAL_SUPPLY,
    DS18B20_PM_COUNT
};

enum DS18B20_resolution_t
{
    DS18B20_RESOLUTION_09 = 0,
    DS18B20_RESOLUTION_10,
    DS18B20_RESOLUTION_11,
    DS18B20_RESOLUTION_12,
    DS18B20_RESOLUTION_COUNT
};

#endif /* DS18B20_TYPES_H */
