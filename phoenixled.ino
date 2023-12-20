#include "FastLED.h"
#include <WiFi.h>
#include <WebServer.h>
 

#define NUM_LEDS  100     // Enter the total number of LEDs on the strip
#define PIN       10      // The pin connected to DATA line to control the LEDs

const char* ssid = "Phoenix";
const char* password = "Flaekegosler";
int FlameHeight;
int Sparks;
int DelayDuration;

CRGB leds[NUM_LEDS];

WebServer server(80);
String html;


void handleRoot() {
  server.send(200, "text/html", html);
}

void handleNotFound() {
  server.send(404, "text/plain", "404: Not Found");
}

void handleUpdate() {
  String flameHeightValue = server.arg("FlameHeight");
  String sparksValue = server.arg("Sparks");
  String delayDurationValue = server.arg("DelayDuration");

  FlameHeight = flameHeightValue.toInt();
  Sparks = sparksValue.toInt();
  DelayDuration = delayDurationValue.toInt();

  server.send(200, "text/plain", "Update empfangen");
}
void setupHtml() {
  html = "<html><head><script>function sendData() {";
  html += "var xhr = new XMLHttpRequest();";
  html += "xhr.open('GET', '/update?FlameHeight=' + document.getElementById('FlameHeight').value";
  html += " + '&Sparks=' + document.getElementById('Sparks').value + '&DelayDuration=' + document.getElementById('DelayDuration').value, true);";
  html += "xhr.send();}</script></head><body>";
  html += "<h1>ESP32 Slider Steuerung</h1>";
  html += "<p>FlameHeight: <input type='range' id='FlameHeight' min='0' max='100' onchange='sendData()'></p>";
  html += "<p>Sparks: <input type='range' id='Sparks' min='0' max='255' onchange='sendData()'></p>";
  html += "<p>DelayDuration: <input type='range' id='DelayDuration' min='0' max='1000' onchange='sendData()'></p>";
  html += "</body></html>";
}

void setup() {
  Serial.begin(115200);

  WiFi.softAP(ssid, password);
  Serial.println("Access Point gestartet");
  Serial.println("IP-Adresse des ESP32 im eigenen WLAN:");
  Serial.println(WiFi.softAPIP());

  FastLED.addLeds<WS2812B, PIN, GRB>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 1500);
  FastLED.clear();

  setupHtml();

  server.on("/", handleRoot);
  server.on("/update", HTTP_GET, handleUpdate);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP-Server gestartet");
  
  FastLED.addLeds<WS2812B, PIN, GRB>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  FastLED.setMaxPowerInVoltsAndMilliamps(5, 1500);    // Set power limit of LED strip to 5V, 1500mA

  FastLED.clear();                                    // Initialize all LEDs to "OFF"
}
 

// FlameHeight - Use larger value for shorter flames, default=50.

// Sparks - Use larger value for more ignitions and a more active fire (between 0 to 255), default=100.

// DelayDuration - Use larger value for slower flame speed, default=10.

 

void Fire(int FlameHeight, int Sparks, int DelayDuration) {

  static byte heat[NUM_LEDS];

  int cooldown;

  // Cool down each cell a little

  for(int i = 0; i < NUM_LEDS; i++) {

    cooldown = random(0, ((FlameHeight * 10) / NUM_LEDS) + 2);

    if(cooldown > heat[i]) {

      heat[i] = 0;

    }

    else {

      heat[i] = heat[i] - cooldown;

    }

  }

  // Heat from each cell drifts up and diffuses slightly

  for(int k = (NUM_LEDS - 1); k >= 2; k--) {

    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;

  }

  // Randomly ignite new Sparks near bottom of the flame

  if(random(255) < Sparks) {

    int y = random(7);

    heat[y] = heat[y] + random(160, 255);

  }

 

  // Convert heat to LED colors

  for(int j = 0; j < NUM_LEDS; j++) {

    setPixelHeatColor(j, heat[j]);

  }

 

  FastLED.show();

  delay(DelayDuration);

}

 

void setPixelHeatColor(int Pixel, byte temperature) {

  // Rescale heat from 0-255 to 0-191

  byte t192 = round((temperature / 255.0) * 191);

 

  // Calculate ramp up from

  byte heatramp = t192 & 0x3F; // 0...63

  heatramp <<= 2; // scale up to 0...252

 

  // Figure out which third of the spectrum we're in:

  if(t192 > 0x80) {                    // hottest

    leds[Pixel].setRGB(255, 255, heatramp);

  }

  else if(t192 > 0x40) {               // middle

    leds[Pixel].setRGB(255, heatramp, 0);

  }

  else {                               // coolest

    leds[Pixel].setRGB(heatramp, 0, 0);

  }

}


void loop() {
  server.handleClient();
  Fire(FlameHeight, Sparks, DelayDuration);
}