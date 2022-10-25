#ifndef DS18B20_TIMESLOTS_H
#define DS18B20_TIMESLOTS_H

// These are recommended (standard) values according to Maxim Integrated documentation:
// https://www.maximintegrated.com/en/design/technical-documents/app-notes/1/126.html

#define WRITE_BIT0_DELAY0_US        60
#define WRITE_BIT0_DELAY1_US        10
#define WRITE_BIT1_DELAY0_US        6
#define WRITE_BIT1_DELAY1_US        64

#define READ_BIT_DELAY0_US          6
#define READ_BIT_DELAY1_US          9
#define READ_BIT_DELAY2_US          55

#define RESET_DELAY0                480
#define RESET_DELAY1                70
#define RESET_DELAY2                410

#endif /* DS18B20_TIMESLOTS_H */
