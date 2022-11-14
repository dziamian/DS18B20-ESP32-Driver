/**
 * @file ds18b20_registers.h
 * @author Damian Ślusarczyk
 * @brief Contains all DS18B20 ROM information as a set of macros.
 * 
 * They are used for appropriate identifying any DS18B20.
 * 
 */

#ifndef DS18B20_ROM_H
#define DS18B20_ROM_H

#define DS18B20_ROM_FAMILY_CODE_BYTE    0 /**< ROM byte index for the family code */
#define DS18B20_ROM_CRC_BYTE            7 /**< ROM byte index for the ROM CRC */

#endif /* DS18B20_ROM_H */
