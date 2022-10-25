#ifndef DS18B20_COMMANDS_H
#define DS18B20_COMMANDS_H

/* ROM commands */
#define DS18B20_SEARCH_ROM          0xF0
#define DS18B20_READ_ROM            0x33
#define DS18B20_MATCH_ROM           0x55
#define DS18B20_SKIP_ROM            0xCC
#define DS18B20_ALARM_SEARCH        0xEC

/* Function commands */
#define DS18B20_CONVERT_T           0x44
#define DS18B20_WRITE_SCRATCHPAD    0x4E
#define DS18B20_READ_SCRATCHPAD     0xBE
#define DS18B20_COPY_SCRATCHPAD     0x48
#define DS18B20_RECALL_E2           0xB8
#define DS18B20_READ_POWER_SUPPLY   0xB4

#endif /* DS18B20_COMMANDS_H */
