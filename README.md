# DS18B20-ESP32-Driver

## Introduction

Project includes component for controlling [Maxim Integrated DS18B20 1-Wire Digital Thermometer](https://www.maximintegrated.com/en/products/sensors/DS18B20.html) device. It is compatible with ESP32 microcontrollers. Source code of the driver has been written in C. 

It is written and tested for v4.4.2 of the [ESP-IDF](https://github.com/espressif/esp-idf) environment using the xtensa-esp32-elf toolchain (gcc version 8.4.0).

## Build

To include the component into your ESP-IDF project, you need to create `CMakeLists.txt` file containing:
```cmake
idf_component_register(
    SRC_DIRS "." 
    INCLUDE_DIRS "/include"
)
```

Consult [ESP-IDF documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/build-system.html) for more details about build system.

## Features

✔️ Static memory allocation <br />

✔️ Instantiation - using many driver instances simultaneously is possible <br />

✔️ Supports two power modes:

 * External supply
 * Parasitic - VDD connected to GND (device is not supporting all functionalities in this mode - check section [Using Parasite Mode](#using-parasite-mode))

✔️ Supports controlling multiple devices using single 1-Wire bus <br />

✔️ Automatic detection for specified amount of connected devices <br />

✔️ Optimized communication when only one device is connected to 1-Wire bus <br />

✔️ Supports CRC validations wherever possible <br />

✔️ Supports measurement of temperature in Celsius <br />

✔️ Configurable resolutions of temperature measurements - 9, 10, 11 or 12 bits (changes decimal precision) <br />

✔️ Supports status checking of some operations - increases responsiveness of the driver <br />

✔️ Supports alarm searching - finding devices that measured temperatures within specified ranges <br />

✔️ Supports usage of non-volatile memory (EEPROM) - copying and storing data is possible <br />

❌ Concurrency support - synchronization mechanism usage is required while accessing the same 1-Wire bus <br />

## Examples

Below are some examples of using the driver.

 * Periodically reading temperature from one device:
```c
#define DS18B20_1W_BUS          19 // 1-Wire bus GPIO
#define DS18B20_DEVICES_NUM     1  // Number of devices
#define DS18B20_FIRST_DEVICE    0  // Index of the first device
#define DS18B20_CHECKSUM        1  // Should checksum be calculated

DS18B20_onewire_t ds18b20_onewire;
DS18B20_t ds18b20_device;

if (DS18B20_OK != ds18b20__InitOneWire(&ds18b20_onewire, DS18B20_1W_BUS, &ds18b20_device, DS18B20_DEVICES_NUM, DS18B20_CHECKSUM))
{
    // Error handling
}

while (1)
{
    DS18B20_temperature_out_t temperature;
    if (DS18B20_OK != ds18b20__GetTemperatureC(&ds18b20_onewire, DS18B20_FIRST_DEVICE, &temperature, DS18B20_CHECKSUM))
    {
        // Error handling
    }
    printf("Temperature from device num %d: %.4f\n", i + 1, temperature);
    
    // Delay infinite loop for some time
}
```
 * Searching for alarms among several devices (using status checking)
```c
#define DS18B20_1W_BUS          19 // 1-Wire bus GPIO
#define DS18B20_DEVICES_NUM     2  // Number of devices
#define DS18B20_CHECKSUM        1  // Should checksum be calculated
#define DS18B20_UPPER_ALARM     26 // Upper alarm temperature value
#define DS18B20_LOWER_ALARM     20 // Lower alarm temperature value
#define DS18B20_RESOLUTION      11 // Temperature convertion resolution
#define DS18B20_CHECK_PERIOD_MS 10 // Check period for requesting temperature

DS18B20_onewire_t ds18b20_onewire;
DS18B20_t ds18b20_devices[DS18B20_DEVICES_NUM];
DS18B20_config_t ds18b20_config =
{
    .upperAlarm = DS18B20_UPPER_ALARM,
    .lowerAlarm = DS18B20_LOWER_ALARM,
    .resolution = DS18B20_RESOLUTION
};

if (DS18B20_OK != ds18b20__InitOneWire(&ds18b20_onewire, DS18B20_1W_BUS, ds18b20_devices, DS18B20_DEVICES_NUM, DS18B20_CHECKSUM))
{
    // Error handling
}

for (size_t i = 0; i < DS18B20_DEVICES_NUM; ++i)
{
    if (DS18B20_OK != ds18b20__Configure(&ds18b20_onewire, i, &ds18b20_config, DS18B20_CHECKSUM))
    {
        // Error handling
    }
}

while (1)
{
    for (size_t i = 0; i < DS18B20_DEVICES_NUM; ++i)
    {
        if (DS18B20_OK != ds18b20__RequestTemperatureCWithChecking(&ds18b20_onewire, i, DS18B20_CHECK_PERIOD_MS))
        {
            // Error handling
        }
    }
    
    size_t deviceIndex;
    while (DS18B20_OK == ds18b20__FindNextAlarm(&ds18b20_onewire, &deviceIndex, DS18B20_CHECKSUM))
    {
        printf("Alarm found in device no. %d!", deviceIndex + 1);
    }

    // Delay loop for some time
}
```
 * Storing data into EEPROM
```c
#define DS18B20_1W_BUS          19 // 1-Wire bus GPIO
#define DS18B20_DEVICES_NUM     1  // Number of devices
#define DS18B20_FIRST_DEVICE    0  // Index of the first device
#define DS18B20_CHECKSUM        1  // Should checksum be calculated
#define DS18B20_UPPER_ALARM     26 // Upper alarm temperature value
#define DS18B20_LOWER_ALARM     20 // Lower alarm temperature value
#define DS18B20_RESOLUTION      11 // Temperature convertion resolution

DS18B20_onewire_t ds18b20_onewire;
DS18B20_t ds18b20_device;
DS18B20_config_t ds18b20_config =
{
    .upperAlarm = DS18B20_UPPER_ALARM,
    .lowerAlarm = DS18B20_LOWER_ALARM,
    .resolution = DS18B20_RESOLUTION
};

if (DS18B20_OK != ds18b20__InitOneWire(&ds18b20_onewire, DS18B20_1W_BUS, &ds18b20_device, DS18B20_DEVICES_NUM, DS18B20_CHECKSUM))
{
    // Error handling
}

if (DS18B20_OK != ds18b20__Configure(&ds18b20_onewire, DS18B20_FIRST_DEVICE, &ds18b20_config, DS18B20_CHECKSUM))
{
    // Error handling
}

if (DS18B20_OK != ds18b20__StoreRegisters(&ds18b20_oneWire, DS18B20_FIRST_DEVICE))
{
    // Error handling
}
```
 * Restoring data from EEPROM 
```c
#define DS18B20_1W_BUS          19 // 1-Wire bus GPIO
#define DS18B20_DEVICES_NUM     1  // Number of devices
#define DS18B20_FIRST_DEVICE    0  // Index of the first device
#define DS18B20_CHECKSUM        1  // Should checksum be calculated
#define DS18B20_UPPER_ALARM     26 // Upper alarm temperature value
#define DS18B20_LOWER_ALARM     20 // Lower alarm temperature value
#define DS18B20_RESOLUTION      11 // Temperature convertion resolution

DS18B20_onewire_t ds18b20_onewire;
DS18B20_t ds18b20_device;
DS18B20_config_t ds18b20_config =
{
    .upperAlarm = DS18B20_UPPER_ALARM,
    .lowerAlarm = DS18B20_LOWER_ALARM,
    .resolution = DS18B20_RESOLUTION
};

if (DS18B20_OK != ds18b20__InitOneWire(&ds18b20_onewire, DS18B20_1W_BUS, &ds18b20_device, DS18B20_DEVICES_NUM, DS18B20_CHECKSUM))
{
    // Error handling
}

if (DS18B20_OK != ds18b20__Configure(&ds18b20_onewire, DS18B20_FIRST_DEVICE, &ds18b20_config, DS18B20_CHECKSUM))
{
    // Error handling
}

// Do something . . .

if (DS18B20_OK != ds18b20__RestoreRegisters(&ds18b20_oneWire, DS18B20_FIRST_DEVICE, DS18B20_CHECKSUM))
{
    // Error handling
}
```

## Using Parasite Mode

Some functionalities are unavailable when device is using parasite power mode. Checking status for operations like temperature convertion or copying memory into EEPROM is not possible because of technical issues. For this reason in such cases methods like `ds18b20__GetTemperatureCWithChecking`, `ds18b20__RequestTemperatureWithChecking` and `ds18b20__StoreRegistersWithChecking` will fail with an appropriate status code. Additionally, when the mentioned operations are performing, no other 1-Wire bus activity may take place during this time (like performing another operation with the next device).

Consult [DS18B20 datasheet](https://datasheets.maximintegrated.com/en/ds/DS18B20.pdf) for more details about parasite power mode. 

## Documentation

Generated API documentation is available [here](http://dziamian.github.io/DS18B20-ESP32-Driver).

## Source Code

The source code is available on [GitHub](https://github.com/dziamian/DS18B20-ESP32-Driver).

## License

The code in this project is licensed under the MIT license - see [LICENSE.md](https://github.com/dziamian/DS18B20-ESP32-Driver/blob/main/LICENSE.md) for details.

## Links
 
 * [ESP-IDF](https://github.com/espressif/esp-idf)
 * [Maxim Integrated DS18B20 1-Wire Digital Thermometer](https://www.maximintegrated.com/en/products/sensors/DS18B20.html)
 * [DS18B20 Datasheet](https://datasheets.maximintegrated.com/en/ds/DS18B20.pdf)
 * [1-Wire Communication Through Software](https://www.maximintegrated.com/en/design/technical-documents/app-notes/1/126.html)
