/**
 * @file ds18b20_validator.h
 * @author Damian Ślusarczyk
 * @brief Contains method used to validate the data acquired during communication with DS18B20.
 * 
 */

#ifndef DS18B20_VALIDATOR_H
#define DS18B20_VALIDATOR_H

#include <stddef.h>
#include <stdint.h>

#include "ds18b20_error_codes.h"

#define DS18B20_CRC8_POLYNOMIAL_WITHOUT_MSB  0x8C   /**< CRC polynomial used by DS18B20 with the most significant bit discarded */

#define DS18B20_ROM_SIZE_TO_VALIDATE         7      /**< Number of ROM bytes to validate using CRC algorithm */
#define DS18B20_SP_SIZE_TO_VALIDATE          8      /**< Number of scrachpad bytes to validate using CRC algorithm */

/**
 * @brief Validates the data bytes using CRC checksum algorithm.
 * 
 * After copying the given data, CRC checksum is calculated for the given polynomial.
 * If the evaluated result is equal to the given value then the validation is successful.
 * 
 * @param data Pointer to data bytes to be validated
 * @param dataSize Size of the data
 * @param polynomialWithoutMsb CRC polynomial with the most significant bit discarded
 * @param crcValue Value to be checked with the calculated checksum
 * @return DS18B20_error_t Status of the operation
 */
DS18B20_error_t ds18b20_validate_crc8(const uint8_t * const data, const size_t dataSize, const uint8_t polynomialWithoutMsb, const uint8_t crcValue);

#endif /* DS18B20_VALIDATOR_H */
