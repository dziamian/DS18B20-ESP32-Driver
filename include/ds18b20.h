/**
 * @file ds18b20.h
 * @author Damian Ślusarczyk
 * @brief Contains high-level functions to communicate with DS18B20 using One-Wire protocol.
 * 
 */

#ifndef DS18B20_H
#define DS18B20_H

#include <stdbool.h>

#include "ds18b20_low.h"
#include "ds18b20_types_res.h"
#include "ds18b20_error_codes.h"

/** Means that function will not check if device has ended specified operation - it will wait the maximum defined time */
#define DS18B20_NO_CHECK_PERIOD         0
/** Minimum period time (in milliseconds) for checking if device has ended specified operation */
#define DS18B20_CHECK_PERIOD_MIN_MS     10
/** The minimum temperature value the device can measure */
#define DS18B20_TEMP_MIN                -55
/** The maximum temperature value the device can measure */
#define DS18B20_TEMP_MAX                125

typedef struct DS18B20_config_t DS18B20_config_t;

/**
 * @brief Describes configuration options for DS18B20.
 * 
 */
struct DS18B20_config_t
{
    DS18B20_temperature_in_t            upperAlarm; /**< Upper temperature alarm value to configure */
    DS18B20_temperature_in_t            lowerAlarm; /**< Lower temperature alarm value to configure */
    DS18B20_resolution_t                resolution; /**< Temperature convertion resolution to configure */
};

/**
 * @brief Initializes One-Wire instance and DS18B20 device instances.
 * 
 * Prepares given GPIO to communicate with One-Wire protocol, searches for specified amount of devices,
 * reads and sets their ROM addresses for proper identification, power modes and scratchpad memory.
 * This method need to be called before using any other high-level driver functions.
 * 
 * @param onewire Pointer to One-Wire bus characteristics instance to initialize
 * @param bus Chosen GPIO for One-Wire bus
 * @param devices Array of device characteristics instances to initialize
 * @param devicesNo Number of devices to initialize
 * @param checksum Specifies if CRC checksum should be calculated during all performed operations
 * @return DS18B20_error_t Status code of the operation
 */
DS18B20_error_t ds18b20__InitOneWire(DS18B20_onewire_t * const onewire, const int bus, DS18B20_t * const devices, const size_t devicesNo, const bool checksum);

/**
 * @brief Initializes configuration options of DS18B20 with the default values (power-on reset values).
 * 
 * For more details about default configuration of DS18B20 please check @see ds18b20_registers header file.
 * 
 * @param config Pointer to device configuration instance to initialize
 * @return DS18B20_error_t Status code of the operation
 */
DS18B20_error_t ds18b20__InitConfigDefault(DS18B20_config_t * const config);

/**
 * @brief Only requests chosen DS18B20 for temperature convertion without reading its value.
 * 
 * Waits the maximum possible time required to perform this operation.
 * 
 * @param onewire Pointer to One-Wire bus characteristics instance
 * @param deviceIndex Index of the selected device
 * @return DS18B20_error_t Status code of the operation
 */
DS18B20_error_t ds18b20__RequestTemperatureC(const DS18B20_onewire_t * const onewire, const size_t deviceIndex);

/**
 * @brief Only requests chosen DS18B20 for temperature convertion without reading its value while periodically checking if performing operation by the device has ended.
 * 
 * Waits until this operation has finished by periodically checking its status.
 * This method cannot be used if selected DS18B20 is working in parasite mode!
 * 
 * @param onewire Pointer to One-Wire bus characteristics instance
 * @param deviceIndex Index of the selected device
 * @param checkPeriodMs Specifies how often the status of the temperature convertion will be checked (in milliseconds),
 * given value cannot be less than @see DS18B20_CHECK_PERIOD_MIN_MS,
 * value equals to @see DS18B20_NO_CHECK_PERIOD means that method will wait the maximum possible time required for temperature convertion
 * @return DS18B20_error_t Status code of the operation
 */
DS18B20_error_t ds18b20__RequestTemperatureCWithChecking(const DS18B20_onewire_t * const onewire, const size_t deviceIndex, uint16_t checkPeriodMs);

/**
 * @brief Reads the current temperature the device has measured (in Celsius).
 * 
 * Requests chosen DS18B20 from One-Wire bus for temperature convertion. 
 * Waits the maximum possible time required to perform this operation.
 * Reads measured temperature from the device memory where it has been stored and converts it into human-readable value. 
 * Optionally, validates received data from the One-Wire line with CRC checksum.
 * 
 * @param onewire Pointer to One-Wire bus characteristics instance
 * @param deviceIndex Index of the selected device
 * @param temperatureOut Pointer to variable where received temperature will be saved eventually
 * @param checksum Specifies if CRC checksum should be calculated during all performed operations
 * @return DS18B20_error_t Status code of the operation
 */
DS18B20_error_t ds18b20__GetTemperatureC(const DS18B20_onewire_t * const onewire, const size_t deviceIndex, DS18B20_temperature_out_t * const temperatureOut, const bool checksum);

/**
 * @brief Reads the current temperature the device has measured (in Celsius) while periodically checking if performing operation by the device has ended.
 * 
 * Requests chosen DS18B20 from One-Wire bus for temperature convertion.
 * Waits until this operation has finished by periodically checking its status.
 * Reads measured temperature from the device memory where it has been stored and converts it into human-readable value. 
 * Optionally, validates received data from the One-Wire line with CRC checksum.
 * This method cannot be used if selected DS18B20 is working in parasite mode!
 * 
 * @param onewire Pointer to One-Wire bus characteristics instance
 * @param deviceIndex Index of the selected device
 * @param temperatureOut Pointer to variable where received temperature will be saved eventually
 * @param checkPeriodMs Specifies how often the status of the temperature convertion will be checked (in milliseconds),
 * given value cannot be less than @see DS18B20_CHECK_PERIOD_MIN_MS,
 * value equals to @see DS18B20_NO_CHECK_PERIOD means that method will wait the maximum possible time required for temperature convertion
 * @param checksum Specifies if CRC checksum should be calculated during all performed operations
 * @return DS18B20_error_t Status code of the operation
 */
DS18B20_error_t ds18b20__GetTemperatureCWithChecking(const DS18B20_onewire_t * const onewire, const size_t deviceIndex, DS18B20_temperature_out_t * const temperatureOut, uint16_t checkPeriodMs, const bool checksum);

/**
 * @brief Configures the selected device with the specified options.
 * 
 * Changes set configuration of chosen DS18B20 and acquires it from the memory.
 * Optionally, validates received data from the One-Wire line with CRC checksum.
 * All configuration options must be specified, because all of them will be written into DS18B20 memory.
 * In order to initialize them with default values, please use @see ds18b20__InitConfigDefault method.
 * 
 * @param onewire Pointer to One-Wire bus characteristics instance
 * @param deviceIndex Index of the selected device
 * @param config Pointer to DS18B20 configuration instance
 * @param checksum Specifies if CRC checksum should be calculated during all performed operations
 * @return DS18B20_error_t Status code of the operation
 */
DS18B20_error_t ds18b20__Configure(const DS18B20_onewire_t * const onewire, const size_t deviceIndex, const DS18B20_config_t * const config, const bool checksum);

/**
 * @brief Searches for the next device whose last measured temperature is within the specified alarm range.
 * 
 * Performs one alarm search cycle and saves the index of the found device.
 * Optionally, validates received data (ROM address) from the One-Wire line with CRC checksum.
 * When no more or no devices have a temperature within the specified alarm range in their memory, 
 * then status code of the operation will indicate this with the proper value.
 * In order to request temperature for selected DS18B20, please use @see ds18b20__RequestTemperatureC or @see ds18b20__RequestTemperatureCWithChecking method.
 * 
 * @param onewire Pointer to One-Wire bus characteristics instance
 * @param deviceIndexOut Pointer to variable where index of the found device will be saved eventually
 * @param checksum Specifies if CRC checksum should be calculated during all performed operations
 * @return DS18B20_error_t Status code of the operation
 */
DS18B20_error_t ds18b20__FindNextAlarm(DS18B20_onewire_t * const onewire, size_t * const deviceIndexOut, const bool checksum);


/**
 * @brief Copies stored configuration of the selected device into non-volatile memory.
 * 
 * Requests chosen DS18B20 from One-Wire bus for copying memory into its EEPROM. 
 * Waits the maximum possible time required to perform this operation.
 * This method should be called a reasonable number of times, because EEPROM of DS18B20 has limited lifetime!
 * 
 * @param onewire Pointer to One-Wire bus characteristics instance
 * @param deviceIndex Index of the selected device
 * @return DS18B20_error_t Status code of the operation
 */
DS18B20_error_t ds18b20__StoreRegisters(const DS18B20_onewire_t * const onewire, const size_t deviceIndex);

/**
 * @brief Copies stored configuration of the selected device into non-volatile memory while periodically checking if performing operation by the device has ended.
 * 
 * Requests chosen DS18B20 from One-Wire bus for copying memory into its EEPROM. 
 * Waits until this operation has finished by periodically checking its status.
 * This method should be called a reasonable number of times, because EEPROM of DS18B20 has limited lifetime!
 * This method cannot be used if selected DS18B20 is working in parasite mode!
 * 
 * @param onewire Pointer to One-Wire bus characteristics instance
 * @param deviceIndex Index of the selected device
 * @param checkPeriodMs Specifies how often the status of copying data will be checked (in milliseconds),
 * given value cannot be less than @see DS18B20_CHECK_PERIOD_MIN_MS,
 * value equals to @see DS18B20_NO_CHECK_PERIOD means that method will wait the maximum possible time required for temperature convertion
 * @return DS18B20_error_t Status code of the operation
 */
DS18B20_error_t ds18b20__StoreRegistersWithChecking(const DS18B20_onewire_t * const onewire, const size_t deviceIndex, uint16_t checkPeriodMs);

/**
 * @brief Restores configuration of the selected device from non-volatile memory.
 * 
 * Requests chosen DS18B20 from One-Wire bus for restoring memory from its EEPROM. 
 * Waits the maximum possible time required to perform this operation.
 * Reads the restored configuration from the device memory. 
 * Optionally, validates received data from the One-Wire line with CRC checksum.
 * There is no need to call this method after power-on reset as the restore is always done automatically by the device. 
 * 
 * @param onewire Pointer to One-Wire bus characteristics instance
 * @param deviceIndex Index of the selected device
 * @param checksum Specifies if CRC checksum should be calculated during all performed operations
 * @return DS18B20_error_t Status code of the operation
 */
DS18B20_error_t ds18b20__RestoreRegisters(const DS18B20_onewire_t * const onewire, const size_t deviceIndex, const bool checksum);

/**
 * @brief Restores configuration of the selected device from non-volatile memory.
 * 
 * Requests chosen DS18B20 from One-Wire bus for restoring memory from its EEPROM. 
 * Waits until this operation has finished by periodically checking its status.
 * Reads the restored configuration from the device memory. 
 * Optionally, validates received data from the One-Wire line with CRC checksum.
 * There is no need to call this method after power-on reset as the restore is always done automatically by the device. 
 * 
 * @param onewire Pointer to One-Wire bus characteristics instance
 * @param deviceIndex Index of the selected device
 * @param checkPeriodMs Specifies how often the status of restoring data will be checked (in milliseconds),
 * given value cannot be less than @see DS18B20_CHECK_PERIOD_MIN_MS,
 * value equals to @see DS18B20_NO_CHECK_PERIOD means that method will wait the maximum possible time required for temperature convertion
 * @param checksum Specifies if CRC checksum should be calculated during all performed operations
 * @return DS18B20_error_t Status code of the operation
 */
DS18B20_error_t ds18b20__RestoreRegistersWithChecking(const DS18B20_onewire_t * const onewire, const size_t deviceIndex, uint16_t checkPeriodMs, const bool checksum);

#endif /* DS18B20_H */
