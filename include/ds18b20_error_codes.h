/**
 * @file ds18b20_error_codes.h
 * @author Damian Ślusarczyk
 * @brief Contains error codes as an enumeration.
 * 
 * They describe the status of performed operations with DS18B20.
 */

#ifndef DS18B20_ERROR_CODES_H
#define DS18B20_ERROR_CODES_H

typedef enum DS18B20_error_t    DS18B20_error_t;

/**
 * @brief Describes the status of performed operation with DS18B20.
 * 
 */
enum DS18B20_error_t
{
    DS18B20_OK = 0,             /**< No error occurred - everything went fine */
    DS18B20_INV_ARG,            /**< An invalid argument has been passed into the function */
    DS18B20_INV_CONF,           /**< An invalid driver configuration has been specified */
    DS18B20_INV_OP,             /**< An operation is invalid for the specified driver configuration */
    DS18B20_NO_MORE_DEVICES,    /**< No *more* devices have been found during next search procedure */
    DS18B20_NO_DEVICES,         /**< No devices have been found on One-Wire bus */
    DS18B20_DISCONNECTED,       /**< Device haven't reply with presence status within given time */
    DS18B20_DEVICE_NOT_FOUND,   /**< Couldn't find the device's ROM address in specified driver instance - it was not initialized properly in this case */
    DS18B20_CRC_FAIL,           /**< CRC validation has failed */
};

#endif /* DS18B20_ERROR_CODES_H */
