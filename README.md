# FLIR

Attempting to connect a Lepton FLIR module (https://www.sparkfun.com/products/13233) and a Sharp Memory display (https://www.sparkfun.com/products/13192) to an ESP-12F (https://www.amazon.com/ED-ESP-12F-ESP8266-Wifi-Board/dp/B018XHGVCM/ref=sr_1_2?ie=UTF8&qid=1472524387&sr=8-2&keywords=esp-12F) at the same time using esp-open-rtos.

# Status

Sharp Memory Display - Can clear and set pixels using buffer, need to convert more of the adafruit libs over to esp

Lepton - Not working yet, long way to go :(

Speaker - working, implemented ISR to turn alarm on / off, need to fix silkscreen / pinouts for speaker, is also bassakwards

LED - working, is triggered on/off with speaker, need to fix silkscreen / pin assignments for LED, they are bassakwards :P

Network - working soft AP mode



# Resources:

https://github.com/groupgets/LeptonModule

https://github.com/adafruit/Adafruit_SHARP_Memory_Display

https://github.com/adafruit/Adafruit-GFX-Library

https://github.com/SuperHouse/esp-open-rtos

https://github.com/pfalcon/esp-open-sdk

https://github.com/Ruphobia/mydev-esp-open-rtos

https://github.com/maxritter/DIY-Thermocam

# TODO

Drop the use of bitbanginging for SPI stuff and use the esp-open-rtos spi libs

Get the camera working

Clean up the code and add license / credit information for the code used from above resources

Learn to spell :P

