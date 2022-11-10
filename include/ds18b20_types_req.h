#ifndef DS18B20_TYPES_REQ_H
#define DS18B20_TYPES_REQ_H

typedef enum    DS18B20_resolution_t        DS18B20_resolution_t;

typedef int8_t                              DS18B20_temperature_in_t;

enum DS18B20_resolution_t
{
    DS18B20_RESOLUTION_09 = 0,
    DS18B20_RESOLUTION_10,
    DS18B20_RESOLUTION_11,
    DS18B20_RESOLUTION_12,
    DS18B20_RESOLUTION_COUNT
};

#endif /* DS18B20_TYPES_REQ_H */
