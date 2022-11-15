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
 * @file ds18b20_commands.h
 * @author Damian Ślusarczyk
 * @brief Contains all DS18B20 command codes as a set of macros.
 * 
 * Command codes are used for proper communication with the mentioned device.
 * They define an action to be performed and are sent in the first step in the process of data exchange.
 * 
 */

#ifndef DS18B20_COMMANDS_H
#define DS18B20_COMMANDS_H

/* ROM commands */
#define DS18B20_SEARCH_ROM          0xF0 /**< Search ROM command value */
#define DS18B20_READ_ROM            0x33 /**< Read ROM command value */
#define DS18B20_MATCH_ROM           0x55 /**< Match ROM command value */
#define DS18B20_SKIP_ROM            0xCC /**< Skip ROM command value */
#define DS18B20_ALARM_SEARCH        0xEC /**< Alarm Search ROM command value */

/* Function commands */
#define DS18B20_CONVERT_T           0x44 /**< Convert Temperature command value */
#define DS18B20_WRITE_SCRATCHPAD    0x4E /**< Write Scratchpad command value */
#define DS18B20_READ_SCRATCHPAD     0xBE /**< Read Scratchpad command value */
#define DS18B20_COPY_SCRATCHPAD     0x48 /**< Copy Scratchpad into EEPROM command value */
#define DS18B20_RECALL_E2           0xB8 /**< Recall EEPROM command value */
#define DS18B20_READ_POWER_SUPPLY   0xB4 /**< Read type of Power Supply command value */

#endif /* DS18B20_COMMANDS_H */
