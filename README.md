# BLE-Weather-Station-Cloud-Gateway

ESP32 Arduino Weather Station example + SHT31 temperature and humidity + Bluetooth BLE + Blynk IoT Cloud

[![GitHub version](https://img.shields.io/github/release/ldab/BLE-Weather-Station-Cloud-Gateway.svg)](https://github.com/ldab/BLE-Weather-Station-Cloud-Gateway/releases/latest)
[![Build Status](https://travis-ci.org/ldab/BLE-Weather-Station-Cloud-Gateway.svg?branch=master)](https://travis-ci.org/ldab/BLE-Weather-Station-Cloud-Gateway)
[![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)](https://github.com/ldab/BLE-Weather-Station-Cloud-Gateway/blob/master/LICENSE)

[![GitHub last commit](https://img.shields.io/github/last-commit/ldab/BLE-Weather-Station-Cloud-Gateway.svg?style=social)](https://github.com/ldab/BLE-Weather-Station-Cloud-Gateway)

[![EVK-NINA-W10](https://www.u-blox.com/sites/default/files/styles/product_full/public/products/EVK-NINA-W1-2CI.png)](https://www.u-blox.com/en/product/nina-W10-series)

## How to build PlatformIO based project

1. [Install PlatformIO Core](http://docs.platformio.org/page/core.html)
2. Download [development the repository with examples](https://github.com/ldab/BLE-Weather-Station-Cloud-Gateway)
3. Extract ZIP archive
4. Run these commands:

```
# Change directory to example
> cd BLE-Weather-Station-Cloud-Gateway

# Build project
> platformio run

# Upload firmware
> platformio run --target upload

# Build specific environment
> platformio run -e nina_W10

# Upload firmware for the specific environment
> platformio run -e nina_W10 --target upload

# Clean build files
> platformio run --target clean
```

## TODO

- [ ] 
- [ ] 

## BLE Server, Client, Central, Peripheral ????? 😕

BLE roles are a bit confusing, at least to me, [Neil Kolban](https://github.com/nkolban) the same person behind the `ESP32 BLE Lib` has made some material available on his [YouTube Channel](https://www.youtube.com/watch?v=UgI7WRr5cgE)

Basicaly:
* Peripheral  -> Advertises
* Central     -> Scans for Peripherals
* GATT Server -> Device which has the database and provide resources to the Client. PS. Server does not send data, unless Client requests.
* GATT Client -> Access remote Server resources.

Generally, Peripheral = Server. Therefore if you're working on a end device, an activity tracker for example, it's likely to be set as a Peripheral.

## Examples

* Two examples are provided:
  * [Client](./Client/), connecting to a Peripheral GATT Server;
  * [Server](./Server/), connecting to a Central GATT Client;

## Using with s-center

* In order to test this example, NINA-W10 (Central/Client) connects to NINA-B1 (Peripheral/Server) via s-center and write the SHT31 temperature to the `FFE1` characteristics and humidity to `2A6F`.

![](./extras/s-center.png)

## Partition Table and Flash size

* You can create a custom partitions table (CSV) following [ESP32 Partition Tables](https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/partition-tables.html) documentation.

* Partitions examples are available at [GitHub arduino-esp32](https://github.com/espressif/arduino-esp32/tree/master/tools/partitions)

* The scheme is changed it in order to free some space up used by spiffs found on `min_spiffs.csv` [here](https://github.com/espressif/arduino-esp32/tree/master/tools/partitions)

## Erase Flash

`pio run -t erase` - > all data will be replaced with 0xFF bytes.

## Bluetooth iOS and Android app 

* The [nRF Connect for Mobile](https://www.nordicsemi.com/Software-and-Tools/Development-Tools/nRF-Connect-for-mobile) App from Nordic Semiconductor can alse be used in order to communicate and learn more about BLE and its details:

![nRF Connect](./extras/nRF%20Connect.jpg)

## Credits

Weather Icons and layout inspired by [ThingPulse ](https://github.com/ThingPulse/minigrafx)

Github Shields and Badges created with [Shields.io](https://github.com/badges/shields/)

Adafruit [SHT31 Library](https://www.adafruit.com/product/2857)

ESP32 BLE Arduino [Library](https://github.com/nkolban/ESP32_BLE_Arduino?utm_source=platformio&utm_medium=piohome)
