/**
 * @file ds18b20_specifications.h
 * @author Damian Ślusarczyk
 * @brief Contains all DS18B20 time characteristics as a set of macros. 
 * 
 * They are required for an appropriate communication with the device.
 */

#ifndef DS18B20_SPECIFICATIONS_H
#define DS18B20_SPECIFICATIONS_H

#define DS18B20_RESOLUTION_09_DELAY_MS          94  /**< Maximum temperature convertion waiting time for resolution 09 (ms) */
#define DS18B20_RESOLUTION_10_DELAY_MS          188 /**< Maximum temperature convertion waiting time for resolution 10 (ms) */
#define DS18B20_RESOLUTION_11_DELAY_MS          375 /**< Maximum temperature convertion waiting time for resolution 11 (ms) */
#define DS18B20_RESOLUTION_12_DELAY_MS          750 /**< Maximum temperature convertion waiting time for resolution 12 (ms) */

#define DS18B20_EEPROM_RESTORE_DELAY_MS         10  /**< Maximum time required for restoring memory from EEPROM (ms) */
#define DS18B20_SCRATCHPAD_COPY_DELAY_MS        10  /**< Maximum time required for copying memory into EEPROM (ms) */
#define DS18B20_SCRATCHPAD_COPY_MIN_PULLUP_MS   10  /**< Minimum pull-up time during memory copying in parasite power mode (ms)*/

#endif /* DS18B20_SPECIFICATIONS_H */
