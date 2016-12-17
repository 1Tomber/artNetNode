ESP8266 - PCA9685 Setup Guide

Adapted from http://www.instructables.com/id/ESP8266-Artnet-to-DMX

-----------------------------------------------------------------------------------

1) Flash

Install flash tool:
pip2 install esptool

Flash ArtNetNode:

sudo python2.7 esptool.py --port PORT(e.g /dev/ttyUSB0) write_flash 0x00000 ESP8266_ArtNetNode_DMX_1.2.0.bin

2) Setup Arduino IDE

http://theelectromania.blogspot.de/2016/02/how-to-program-esp8266-esp-12e-nodemcu.html

3) Download libraries

 Requires:
  ESP8266 Core for Arduino: <https://github.com/esp8266/Arduino/>
  ESP-Dmx library: <https://github.com/Rickgg/ESP-Dmx>
  E1.31 library: <https://github.com/forkineye/E131>
  Adafruit-PWM-Servo-Driver-Library: https://github.com/adafruit/Adafruit-PWM-Servo-Driver-Library

4) Connect ESP8266 - PCA9685

GPIO5 - SCL
GPIO4 - SDA
Gnd - Gnd
3V3 - VCC

5) Program ESP8266

Upload ESP8266_ArtNetNode_DMX.ino to ESP8266 using Arduino IDE

6) Connect to WiFi Hotspot

When you first start the device, It will attempt to connect to the WiFi network defaultSSID (which I doubt exists). If it can't connect within 30 seconds, it will start a hotspot. The hotspot is nodeName_XXXX (nodeName by default is artNetNode and XXXX is a random number). Connect to this hotspot using the password artNodeXXXX (XXXX is the number from the hotspot SSID).

Go to 192.168.4.1 with web browser

7) Download DMX controller software:

http://www.qlcplus.org/ 
