# README.md
This project is created with ESP32 IDF. Version 4.2. 

This is a framework for building applications. It provides the following:
-Network setup. With a setup webpage.
-JSON API building block.
-Time services.
-HTTP server.
-Uploading files (for webpages).
-Filesystem.
-Storage of settings in NVRAM.
-mDNS support.

Can also be used as an example how to use the various ESP32 IDF api's. This is also used in the alarmclock github
repository.

A web interface allows for network settings. Accessible through http://x.x.x.x/setup. A simple example for the index.html page is provided. In idf.py menuconfig settings are available for initial WiFi params. 

Most of the files are MIT licensed. Only some bugfixing for the NEWLIB library is licensed different. 

## Remarks
-It works on a ESP32 Pico kit with 4MB of flash. No extra PSRAM needed. 
-Most of the software can be used seperate.  
-Almost everything runs in FreeRTOS tasks. Lots of CPU time left
-Year >2038 support is not yet in IDF (NEWLIB) and compiler. Is possible. But enable 64bit time_t in idf.py menuconfig. And needs a toolchain with 64bit time_t. Tested with this enabled. 
-Network setup is all DHCP. Some fields are defined in the setup parameters for manual ip configuration. But not yet implemented.
-Default ip address of AP is 192.168.4.1.
-Web pages use JSON to communicate with the ESP32. See files in spiffs directory. And some tests with curl in tests.
-Do not connect directly to the internet. Local networks only. Https is not used in the webserver. Some sanity checks and checks for buffer length are done. But problably not enough. Use at your own risk! 

*Have Fun building! Udo*
