/*
ESP8266_ArtNetNode_DMX
Copyright (c) 2016, Matthew Tong
https://github.com/mtongnz/ESP8266_ArtNetNode_DMX
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public
License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any
later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.
If not, see http://www.gnu.org/licenses/
*/


#define FIRMWARE_VERSION "v1.2.0"

// Uncomment the following line to enable serial debug output on Serial0
//#define VERBOSE

// Uncomment the following line to reload the default settings.  You need to comment out again to enable saving settings
// #define LOAD_DEFAULTS


#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <FS.h>
#include <espDMX.h>
#include <pgmspace.h>
#include <string.h>
#include <Wire.h>
//#include <Adafruit_PWMServoDriver.h>

#define FASTLED_ALLOW_INTERRUPTS 0
#define FASTLED_ESP8266_NODEMCU_PIN_ORDER
#include "FastLED.h"

#include <NeoPixelBus.h>


// How many leds in your strip?
#define NUM_LEDS 300 

// For led chips like Neopixels, which have a data line, ground, and power, you just
// need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
// ground, and power), like the LPD8806, define both DATA_PIN and CLOCK_PIN
#define DATA_PIN 1

#define FRAMES_PER_SECOND 4

// Define the array of leds
CRGB leds[NUM_LEDS];

NeoPixelBus<NeoGrbFeature, NeoEsp8266Dma800KbpsMethod> strip(NUM_LEDS);

// called this way, it uses the default address 0x40
//Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

extern "C" {
  #include "user_interface.h"
}

// Global variables and all that messy stuff is in this file
#include "globals.h"


/* setup()
 *  Initializes our EEPROM, serial debug output and dmx output.
 *  Starts our WiFi, UDP listener and WebServer.
 *  Send an ArtNet response to alert all devices to our presence.
 */

void setup() {
  EEPROM.begin(512);

  // Init PWM
//  pwm.begin();
//  pwm.setPWMFreq(400);

  storeInit();
  
  #ifdef VERBOSE
    Serial.begin(115200);
    Serial.println();
    Serial.println("ArtnetDMX");
    Serial.print("Firmware Version: ");
    Serial.println(FIRMWARE_VERSION);
  #endif

  #ifdef LOAD_DEFAULTS
    #ifdef VERBOSE
      Serial.println("Load Default Settings:");
    #endif
    
    saveSettings();
    startHotSpot();
  #else
    // If we cant load settings from EEPROM, start the hotspot
    if (!loadSettings())
      startHotSpot();
  #endif
  
  // Start hotspot if we're in standAlone mode otherwise we start the wifi
  if (standAlone == 1)
    startHotSpot();
  else {
    startWifi();
    
    // Dont allow hotspot to run after this initial setup - must restart device to start it again
    allowHotSpot = 0;
  }
  
  // Start WebServer
  startWebServer();
  
  // Start listening for UDP packets
  eUDP.begin(ARTNET_PORT);

  // Send ArtNet Reply
  sendArtNetReply();

//FASTLED TEST tom

  strip.Begin();
  strip.Show(); // Set to black (^= off) after reset
  
}

/************************************************************************
  The main loop checks WiFi connection.  It will continuously try to reconnect if WiFi is dropped.
  It then checks for and reads packets from UDP. When a packet is recieved, it is checked to see if it is
  valid Art-Net and the artDMXReceived function is called, sending the DMX values to the output.
  Then it checks when the last full DMX universe was sent.  If it was more than 1 second, we transmit a full
  universe.
  The last thing we do is handle any web requests.
*************************************************************************/


/* void fadeall() { for(int i = 0; i < NUM_LEDS; i++) { leds[i].nscale8(250); } } */

void loop() {

  
  // Check WiFi conection
  if (standAlone != 1 && WiFi.status() != WL_CONNECTED) {
    #ifdef VERBOSE
      Serial.println("Wifi Connection Lost");
      Serial.println("Reconnecting");
    #endif

    // Connect WiFi
    startWifi();
  }
  
  // UDP packet
  int packetSize = eUDP.parsePacket();
  
  // If any packets are received
  if( packetSize ) {
    eUDP.read(packetBuffer,ARTNET_BUFFER_MAX);
    int opcode = artNetOpCode(packetBuffer);
    // If DMX then get data
    if ( opcode == ARTNET_ARTDMX ) {
      // ------------------------------------------- This function handles our DMX - PWM control ------------------------------------------------
      artDMXReceived(packetBuffer);

    // If ArtPoll then send reply
    } else if ( opcode == ARTNET_ARTPOLL ) {
      #ifdef VERBOSE
        Serial.println("Artnet Poll Received");
      #endif

      sendArtNetReply();
    }
  }
  
  // Handle web requests
  webServer.handleClient();

/*FASTLED TEST tom
  static uint8_t hue = 0;
  Serial.print("x");
  // First slide the led in one direction
  for(int i = 0; i < NUM_LEDS; i++) {
    // Set the i'th led to red 
    leds[i] = CHSV(hue++, 255, 255);
 
  
  static uint32_t fireTimer;
  if (millis() > fireTimer + 1000 / FRAMES_PER_SECOND)
  {
    fireTimer = millis();

    RgbColor pixel;
    for (int i = 0; i < NUM_LEDS; i++)
    {
      pixel = RgbColor(leds[i].r, leds[i].g, leds[i].b);
      strip.SetPixelColor(i, pixel);
    }
    strip.Show();
  }
  }
    fadeall();
  */
}
