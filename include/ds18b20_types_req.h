/**
 * @file ds18b20_types_req.h
 * @author Damian Ślusarczyk
 * @brief Contains all input types required to communicate with DS18B20 driver.
 * 
 */

#ifndef DS18B20_TYPES_REQ_H
#define DS18B20_TYPES_REQ_H

typedef enum DS18B20_resolution_t           DS18B20_resolution_t;

/** Alarm temperature value set during device configuration */
typedef int8_t                              DS18B20_temperature_in_t;

/**
 * @brief Describes device resolution for temperature convertion.
 * 
 * Lower resolution means lower measurement accuracy.
 */
enum DS18B20_resolution_t
{
    DS18B20_RESOLUTION_09 = 0,  /**< Resolution 09 - 1 decimal bit */
    DS18B20_RESOLUTION_10,      /**< Resolution 10 - 2 decimal bits */
    DS18B20_RESOLUTION_11,      /**< Resolution 11 - 3 decimal bits */
    DS18B20_RESOLUTION_12,      /**< Resolution 12 - 4 decimal bits */
    DS18B20_RESOLUTION_COUNT    /**< Number of available resolutions */
};

#endif /* DS18B20_TYPES_REQ_H */
