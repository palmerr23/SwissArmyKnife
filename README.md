# SwissArmyKnife
A SCPI-controlled test bench instrument published in Silicon Chip April 2023

There are two sets of code provided:
* A simple hardware testing program that also provides for OTA uploading (OTA-test). If you are compiling your own SwissARmy code, the OTA feature is not used.
* The main code (SwissArmy)

Also included are device and help files for TestController https://lygte-info.dk/project/TestControllerIntro%20UK.html

For the main SwissArmy code several non-ESP32 core libraries are required:
* <ADS1X15.h>   https://github.com/RobTillaart/ADS1X15
* <ESPTelnet.h>     https://github.com/LennartHennigs/ESPTelnet 
* <TelnetStream.h>  https://github.com/jandrassy/TelnetStream
* <Vrekrer_scpi_parser.h>  https://github.com/Vrekrer/Vrekrer_scpi_parser

There is a fault on Rev A boards sourced from Silicon Chip, where the pins 20-38 on teh ESP32 have been reversed in production.

instructions to correct this error are found in "Swiss Army Knife Rev A PCB Corrections.pdf"
