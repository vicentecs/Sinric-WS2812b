/*
 * If you encounter any issues:
 * - check the readme.md at https://github.com/sinricpro/esp8266-esp32-sdk/blob/master/README.md
 * - ensure all dependent libraries are installed
 *   - see https://github.com/sinricpro/esp8266-esp32-sdk/blob/master/README.md#arduinoide
 *   - see https://github.com/sinricpro/esp8266-esp32-sdk/blob/master/README.md#dependencies
 * - open serial monitor and check whats happening
 * - check full user documentation at https://sinricpro.github.io/esp8266-esp32-sdk
 * - visit https://github.com/sinricpro/esp8266-esp32-sdk/issues and check for existing issues or open a new one
 */

// Uncomment the following line to enable serial debug output
//#define ENABLE_DEBUG

#ifdef ENABLE_DEBUG
       #define DEBUG_ESP_PORT Serial
       #define NODEBUG_WEBSOCKETS
       #define NDEBUG
#endif 

#include <Arduino.h>
#ifdef ESP8266 
       #include <ESP8266WiFi.h>
#endif 
#ifdef ESP32   
       #include <WiFi.h>
#endif

#define FASTLED_ESP8266_DMA
#define FASTLED_ESP8266_RAW_PIN_ORDER
#include <FastLED.h>

#include "SinricPro.h"
#include "SinricProLight.h"
#include <Kelvin2RGB.h>
#include "credentials.h"

#define BAUD_RATE         115200              // Change baudrate to your need
#define NUM_LEDS          60                  // how much LEDs are on the stripe
#define LED_PIN           5                   // LED stripe is connected to PIN 3
#define UPDATES_PER_SECOND 100

bool powerState;        
int globalBrightness = 100;
CRGB leds[NUM_LEDS];

CRGBPalette16  currentPalette;
TBlendType     currentBlending;
bool           changerPalette;

//static uint8_t startIndex = 0; 

bool onPowerState(const String &deviceId, bool &state) {
  powerState = state;
  if (state) {
    FastLED.setBrightness(map(globalBrightness, 0, 100, 0, 255));
  } else {
    FastLED.setBrightness(0);
  }
  FastLED.show();
  return true; // request handled properly
}

bool onBrightness(const String &deviceId, int &brightness) {
  globalBrightness = brightness;
  FastLED.setBrightness(map(brightness, 0, 100, 0, 255));
  FastLED.show();
  return true;
}

bool onAdjustBrightness(const String &deviceId, int brightnessDelta) {
  globalBrightness += brightnessDelta;
  brightnessDelta = globalBrightness;
  FastLED.setBrightness(map(globalBrightness, 0, 100, 0, 255));
  FastLED.show();
  return true;
}

bool onColorTemperature(const String &deviceId, int &colorTemperature) {
  changerPalette = colorTemperature == 2700;
  Kelvin2RGB kel(colorTemperature, 100);
  fill_solid(leds, NUM_LEDS, CRGB(kel.Red, kel.Green, kel.Blue));
  FastLED.show();
  return true;
}

bool onColor(const String &deviceId, byte &r, byte &g, byte &b) {
  changerPalette = false;
  fill_solid(leds, NUM_LEDS, CRGB(r, g, b));
  FastLED.show();
  return true;
}

void setupFastLED() {
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(map(globalBrightness, 0, 100, 0, 255));
  fill_solid(leds, NUM_LEDS, CRGB::White);
  FastLED.show();
  currentPalette = RainbowColors_p;
  currentBlending = LINEARBLEND;
  changerPalette = false;
}

void setupWiFi() {
  Serial.printf("\r\n[Wifi]: Connecting");
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.printf(".");
    delay(250);
  }
  IPAddress localIP = WiFi.localIP();
  Serial.printf("connected!\r\n[WiFi]: IP-Address is %s\r\n", localIP.toString().c_str());
}

void setupSinricPro() {
  // get a new Light device from SinricPro
  SinricProLight &myLight = SinricPro[LIGHT_ID];

  // set callback function to device
  myLight.onPowerState(onPowerState);
  myLight.onBrightness(onBrightness);
  myLight.onAdjustBrightness(onAdjustBrightness);
  myLight.onColorTemperature(onColorTemperature);  
  myLight.onColor(onColor);

  // setup SinricPro
  SinricPro.onConnected([](){ Serial.printf("Connected to SinricPro\r\n"); }); 
  SinricPro.onDisconnected([](){ Serial.printf("Disconnected from SinricPro\r\n"); });
  SinricPro.restoreDeviceStates(true);
  SinricPro.begin(APP_KEY, APP_SECRET);
}

// FastLED provides several 'preset' palettes: RainbowColors_p, RainbowStripeColors_p,
// OceanColors_p, CloudColors_p, LavaColors_p, ForestColors_p, and PartyColors_p.
void ChangePalettePeriodically()
{
    uint8_t secondHand = (millis() / 1000) % 60;
    static uint8_t lastSecond = 99;
    
    if( lastSecond != secondHand) {
        lastSecond = secondHand;
        if( secondHand ==  0)  { currentPalette = RainbowColors_p;         currentBlending = LINEARBLEND; }
        if( secondHand == 10)  { currentPalette = RainbowStripeColors_p;   currentBlending = NOBLEND;  }
        if( secondHand == 15)  { currentPalette = RainbowStripeColors_p;   currentBlending = LINEARBLEND; }
        if( secondHand == 20)  { currentPalette = OceanColors_p;           currentBlending = LINEARBLEND; }
        if( secondHand == 25)  { currentPalette = CloudColors_p;           currentBlending = LINEARBLEND; }
        if( secondHand == 30)  { currentPalette = LavaColors_p;            currentBlending = NOBLEND; }
        if( secondHand == 35)  { currentPalette = LavaColors_p;            currentBlending = LINEARBLEND; }
        if( secondHand == 40)  { currentPalette = ForestColors_p;          currentBlending = NOBLEND; }
        if( secondHand == 45)  { currentPalette = ForestColors_p;          currentBlending = LINEARBLEND; }
        if( secondHand == 50)  { currentPalette = PartyColors_p;           currentBlending = NOBLEND;  }
        if( secondHand == 55)  { currentPalette = PartyColors_p;           currentBlending = LINEARBLEND; }
    }
}

void FillLEDsFromPaletteColors( uint8_t colorIndex) {
  if (!changerPalette) return;
   
  ChangePalettePeriodically();
  
  for( int i = 0; i < NUM_LEDS; ++i) {
      leds[i] = ColorFromPalette( currentPalette, colorIndex, globalBrightness, currentBlending);
      colorIndex += 3;
  }
  FastLED.show();
  FastLED.delay(1000 / UPDATES_PER_SECOND);
}

// main setup function
void setup() {
  Serial.begin(BAUD_RATE); Serial.printf("\r\n\r\n");
  setupWiFi();
  setupFastLED();  
  setupSinricPro();
}

void loop() {
  SinricPro.handle();
  static uint8_t startIndex = 0;
  startIndex = startIndex + 1; /* motion speed */
  FillLEDsFromPaletteColors(startIndex);
}
