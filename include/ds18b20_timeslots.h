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
 * @file ds18b20_timeslots.h
 * @author Damian Ślusarczyk
 * @brief Contains all DS18B20 read/write timeslot characteristics as a set of macros.
 * 
 * They are required for appropriate reading/writing signals from the device.
 */

#ifndef DS18B20_TIMESLOTS_H
#define DS18B20_TIMESLOTS_H

// These are recommended (standard) values according to Maxim Integrated documentation:
// https://www.maximintegrated.com/en/design/technical-documents/app-notes/1/126.html

#define WRITE_BIT0_DELAY0_US        60  /**< Timeslot duration for writing bit's value 0 before releasing the bus (us) */
#define WRITE_BIT0_DELAY1_US        10  /**< Timeslot duration for writing bit's value 0 after releasing the bus (us) */
#define WRITE_BIT1_DELAY0_US        6   /**< Timeslot duration for writing bit's value 1 before releasing the bus (us) */
#define WRITE_BIT1_DELAY1_US        64  /**< Timeslot duration for writing bit's value 1 after releasing the bus (us) */

#define READ_BIT_DELAY0_US          6   /**< Timeslot duration for reading bit's value before releasing the bus (us) */
#define READ_BIT_DELAY1_US          9   /**< Timeslot duration for reading bit's value after releasing the bus and before receiving the signal (us) */
#define READ_BIT_DELAY2_US          55  /**< Timeslot duration for reading bit's value after receiving the signal (us) */

#define RESET_DELAY0_US             480 /**< Timeslot duration for reset signal before releasing the bus (us) */
#define RESET_DELAY1_US             70  /**< Timeslot duration for reset signal after releasing the bus and before receiving the presence signal (us) */
#define RESET_DELAY2_US             410 /**< Timeslot duration for reset signal after receiving the presence signal (us) */

#endif /* DS18B20_TIMESLOTS_H */
