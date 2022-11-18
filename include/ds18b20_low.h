/*
 * MIT License
 *
 * Copyright (c) 2022 Damian Ślusarczyk
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
 * TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */
/**
 * @file ds18b20_low.h
 * @author Damian Ślusarczyk
 * @brief Contains low-level functions to communicate with DS18B20 using One-Wire protocol.
 * 
 */

#ifndef DS18B20_LOW_H
#define DS18B20_LOW_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "ds18b20_types_req.h"
#include "ds18b20_error_codes.h"

#define DS18B20_1W_SINGLEDEVICE             1 /**< Means that One-Wire bus is connected to only one device */

#define DS18B20_ROM_SIZE                    8 /**< DS18B20 ROM address size in bytes */
#define DS18B20_SP_SIZE                     9 /**< DS18B20 scratchpad size in bytes */

typedef enum    DS18B20_powermode_t         DS18B20_powermode_t;
typedef struct  DS18B20_onewire_t           DS18B20_onewire_t;
typedef struct  DS18B20_t                   DS18B20_t;

typedef uint8_t                             DS18B20_rom_t[DS18B20_ROM_SIZE]; /**< DS18B20 ROM address */
typedef uint8_t                             DS18B20_scratchpad_t[DS18B20_SP_SIZE]; /**< DS18B20 scratchpad memory */

/**
 * @brief Describes power mode of specific DS18B20.
 * 
 */
enum DS18B20_powermode_t
{
    DS18B20_PM_PARASITE = 0,                /**< Parasite power supply */
    DS18B20_PM_EXTERNAL_SUPPLY,             /**< External power supply */
    DS18B20_PM_COUNT                        /**< Number of available power modes */
};

/**
 * @brief Describes characteristics of single DS18B20.
 * 
 */
struct DS18B20_t
{
    DS18B20_rom_t                           rom; /**< Stores ROM address of the device */
    DS18B20_scratchpad_t                    scratchpad; /**< Stores scratchpad memory of the device */
    DS18B20_resolution_t                    resolution; /**< Temperature resolution convertion */
    DS18B20_powermode_t                     powerMode; /**< Used power mode */
};

/**
 * @brief Describes characteristics of One-Wire bus, containing access to single or multiple DS18B20.
 * 
 */
struct DS18B20_onewire_t
{
    int                                     bus; /**< Selected GPIO for One-Wire communication */
    DS18B20_t                               *devices; /**< Devices connected to the bus */
    size_t                                  devicesNo; /**< Number of connected devices */

    size_t                                  lastSearchedDeviceNumber; /**< Number (not index) of the last found device during search procedure */
    int8_t                                  lastSearchConflictUnresolved; /**< Bit index of the last unresolved conflict in connected devices' ROMs */
    int8_t                                  lastSearchConflict; /**< Bit index of the last resolved conflict in connected devices' ROMs */
    bool                                    alarmSearchMode; /**< Indicates which search mode has been chosen lately */
};

/* Basic functions */

/**
 * @brief Writes single bit on the One-Wire bus.
 * 
 * Interrupts are disabled while this operation is performed. 
 * 
 * @param onewire Pointer to specified One-Wire bus characteristics instance
 * @param bit Bit value to be written on the bus, any value other than 0 is treated as 1
 */
void ds18b20_write_bit(const DS18B20_onewire_t * const onewire, const uint8_t bit);

/**
 * @brief Writes single byte (8 bits) on the One-Wire bus.
 * 
 * Interrupts are disabled while single bit is written.
 * 
 * @param onewire Pointer to specified One-Wire bus characteristics instance
 * @param byte Full value to be written on the bus, least significant bits first
 */
void ds18b20_write_byte(const DS18B20_onewire_t * const onewire, const uint8_t byte);

/**
 * @brief Reads single bit from the One-Wire bus.
 * 
 * Interrupts are disabled while this operation is performed. 
 * 
 * @param onewire Pointer to specified One-Wire bus characteristics instance
 * @return uint8_t Value read from the bus - 0 or 1
 */
uint8_t ds18b20_read_bit(const DS18B20_onewire_t * const onewire);

/**
 * @brief Reads single byte (8 bits) from the One-Wire bus.
 * 
 * Interrupts are disabled while single bit is read.
 * 
 * @param onewire Pointer to specified One-Wire bus characteristics instance
 * @return uint8_t Full value read from the bus
 */
uint8_t ds18b20_read_byte(const DS18B20_onewire_t * const onewire);

/**
 * @brief Sends reset signal to all devices connected to One-Wire bus.
 * 
 * Initializes data exchange communication with the present devices.
 * Interrupts are disabled while this operation is performed.
 * Any alive device should respond with the 'presence' signal.
 * 
 * @param onewire Pointer to specified One-Wire bus characteristics instance
 * @return uint8_t Returns 1 if any device received and replied to the signal, otherwise returns 0
 */
uint8_t ds18b20_reset(const DS18B20_onewire_t * const onewire);

/**
 * @brief Starts strong pullup on One-Wire bus, which may be required during some operations in the parasite power mode.
 * 
 * @param onewire Pointer to specified One-Wire bus characteristics instance
 */
void ds18b20_parasite_start_pullup(const DS18B20_onewire_t * const onewire);

/**
 * @brief Releases the One-Wire bus, ending the strong pullup at the same time.
 * 
 * @param onewire Pointer to specified One-Wire bus characteristics instance
 */
void ds18b20_parasite_end_pullup(const DS18B20_onewire_t * const onewire);

/* ROM commands */

/**
 * @brief Performs one cycle of the device or alarm searching procedure.
 * 
 * Found ROM address is saved in specified buffer or in the next One-Wire device characteristics internal buffer if it was not defined by the user. 
 * Before the first use ds18b20_restart_search() call is required to initialize the internal search parameters.
 * In alarm search mode, buffer need to be always specified by the user, otherwise the function will not be executed properly.
 * Search parameters are reset automatically when all search cycles have been performed or search mode has been changed.
 * 
 * @param onewire Pointer to specified One-Wire bus characteristics instance
 * @param buffer Pointer to buffer instance where found ROM address will be saved
 * @param alarmSearchMode Specifies search mode - 1 means searching for alarms, 0 means searching for devices
 * @return DS18B20_error_t Status code of the operation
 */
DS18B20_error_t ds18b20_search_rom(DS18B20_onewire_t * const onewire, DS18B20_rom_t * buffer, const bool alarmSearchMode);

/**
 * @brief Performs reading of the device's ROM address and saving it in device characteristics internal buffer.
 * 
 * It can be used only if One-Wire bus is connected to one device. 
 * Otherwise bit signals from many devices could overlap causing unreliable results.
 * 
 * @param onewire Pointer to specified One-Wire bus characteristics instance
 * @return DS18B20_error_t Status code of the operation
 */
DS18B20_error_t ds18b20_read_rom(const DS18B20_onewire_t * const onewire);

/**
 * @brief Selects one device connected to One-Wire bus by sending its ROM address to the line.
 * 
 * This method is required to call before using any of functional commands like converting temperature.
 * In case when only one device is connected to the bus, ds18b20_skip_select() method could be use instead.
 * 
 * @param onewire Pointer to specified One-Wire bus characteristics instance
 * @param deviceIndex Index of the selected device from One-Wire bus
 * @return DS18B20_error_t Status code of the operation
 */
DS18B20_error_t ds18b20_select(const DS18B20_onewire_t * const onewire, const size_t deviceIndex);

/**
 * @brief Skips selecting device by sending an appropriate command code.
 * 
 * This method is required to call before using any of functional commands like converting temperature.
 * This method can be used only if one device is connected to the One-Wire bus.
 * In any other cases, please use ds18b20_select() method instead.
 * 
 * @param onewire Pointer to specified One-Wire bus characteristics instance
 * @return DS18B20_error_t Status code of the operation
 */
DS18B20_error_t ds18b20_skip_select(const DS18B20_onewire_t * const onewire);

/* Function commands */

/**
 * @brief Sends a request for converting temperature to the selected DS18B20.
 * 
 * Before calling this you need to select device by using one of these methods ds18b20_select() or ds18b20_skip_select().
 * If selected device is working in a parasite power mode, strong pullup will be enabled. 
 * In this specific case all interrupts are disabled while performing the operation.
 * 
 * @param onewire Pointer to specified One-Wire bus characteristics instance
 * @param deviceIndex Index of the selected device from One-Wire bus
 * @return DS18B20_error_t Status code of the operation
 */
DS18B20_error_t ds18b20_convert_temperature(const DS18B20_onewire_t * const onewire, const size_t deviceIndex);

/**
 * @brief Writes the scratchpad of the selected DS18B20.
 * 
 * Before calling this you need to select device by using one of these methods ds18b20_select() or ds18b20_skip_select().
 * Only configurable bytes stored in the internal buffer of DS18B20 characteristics instance will be written to the device memory.
 * 
 * @param onewire Pointer to specified One-Wire bus characteristics instance
 * @param deviceIndex Index of the selected device from One-Wire bus
 * @return DS18B20_error_t Status code of the operation
 */
DS18B20_error_t ds18b20_write_scratchpad(const DS18B20_onewire_t * const onewire, const size_t deviceIndex);

/**
 * @brief Reads the all scratchpad memory from the selected DS18B20.
 * 
 * Before calling this you need to select device by using one of these methods ds18b20_select() or ds18b20_skip_select().
 * All bytes from device memory will be read and stored in the internal buffer of chosen DS18B20 characteristics instance.
 * If you want to read only part of the memory, please use ds18b20_read_scratchpad_with_stop() method instead.
 * 
 * @param onewire Pointer to specified One-Wire bus characteristics instance
 * @param deviceIndex Index of the chosen device from One-Wire bus
 * @return DS18B20_error_t Status code of the operation
 */
DS18B20_error_t ds18b20_read_scratchpad(const DS18B20_onewire_t * const onewire, const size_t deviceIndex);

/**
 * @brief Reads the specified part of the scratchpad memory from the selected DS18B20.
 * 
 * Before calling this you need to select device by using one of these methods ds18b20_select() or ds18b20_skip_select().
 * Only number of selected bytes from device memory will be read and stored in the internal buffer of chosen DS18B20 characteristics instance.
 * The remaining bytes in the buffer will be unchanged.
 * 
 * @param onewire Pointer to specified One-Wire bus characteristics instance
 * @param deviceIndex Index of the chosen device from One-Wire bus
 * @param bytesToRead Number of bytes to be read from the scratchpad 
 * @return DS18B20_error_t Status code of the operation
 */
DS18B20_error_t ds18b20_read_scratchpad_with_stop(const DS18B20_onewire_t * const onewire, const size_t deviceIndex, uint8_t bytesToRead);

/**
 * @brief Sends a request for copying scratchpad into non-volatile EEPROM memory of the selected DS18B20.
 * 
 * Before calling this you need to select device by using one of these methods ds18b20_select() or ds18b20_skip_select().
 * Only configurable bytes stored in device memory will be copied into its EEPROM.
 * If selected device is working in a parasite power mode, strong pullup will be enabled. 
 * In this specific case all interrupts are disabled while performing the operation.
 * 
 * @param onewire Pointer to specified One-Wire bus characteristics instance
 * @param deviceIndex Index of the selected device from One-Wire bus
 * @return DS18B20_error_t Status code of the operation
 */
DS18B20_error_t ds18b20_copy_scratchpad(const DS18B20_onewire_t * const onewire, const size_t deviceIndex);

/**
 * @brief Sends a request for recalling scratchpad from non-volatile EEPROM memory of the selected DS18B20.
 * 
 * Before calling this you need to select device by using one of these methods ds18b20_select() or ds18b20_skip_select().
 * Only configurable bytes stored in device EEPROM memory will be recalled.
 * 
 * @param onewire Pointer to specified One-Wire bus characteristics instance
 * @return DS18B20_error_t Status code of the operation
 */
DS18B20_error_t ds18b20_recall_e2(const DS18B20_onewire_t * const onewire);

/**
 * @brief Reads the used power mode for the selected DS18B20.
 * 
 * It stores the read power mode in the internal buffer of chosen DS18B20 characteristics instance.
 * 
 * @param onewire Pointer to specified One-Wire bus characteristics instance
 * @param deviceIndex Index of the selected device from One-Wire bus
 * @return DS18B20_error_t Status code of the operation
 */
DS18B20_error_t ds18b20_read_powermode(const DS18B20_onewire_t * const onewire, const size_t deviceIndex);

/* Helpers */

/**
 * @brief Restarts the search procedure by resetting values of the internal search parameters.
 * 
 * It needs to be called at least once before using ds18b20_search_rom() method.
 * 
 * @param onewire Pointer to specified One-Wire bus characteristics instance
 * @param alarmSearchMode Specifies search mode - 1 means searching for alarms, 0 means searching for devices
 * @return DS18B20_error_t Status code of the operation
 */
DS18B20_error_t ds18b20_restart_search(DS18B20_onewire_t * const onewire, const bool alarmSearchMode);

/**
 * @brief Returns the maximum time to wait before completing the temperature convertion by the device for selected resolution.
 * 
 * @param resolution Selected convertion resolution
 * @return uint16_t Maximum time to wait before temperature convertion is complete
 */
uint16_t ds18b20_millis_to_wait_for_convertion(const DS18B20_resolution_t resolution);

#endif /* DS18B20_LOW_H */
