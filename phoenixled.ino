#include "FastLED.h"
#include "settings_html.h"
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>

#define NUM_LEDS 75  // Enter the total number of LEDs on the strip

const char *ssid = "Phoenix";
const char *password = "Flaekegosler";
int FlameHeight;
int Sparks;
int DelayDuration;
String SelectedMode;
Preferences preferences;

CRGB leds[NUM_LEDS];

WebServer server(80);
String html;

void saveSettings(int f, int s, int d, String m) {
  preferences.begin("myApp", false);
  preferences.putInt("FlameHeight", f);
  preferences.putInt("Sparks", s);
  preferences.putInt("DelayDuration", d);
  preferences.putString("SelectedMode", m);
  preferences.end();
}

void loadSettings() {
  preferences.begin("myApp", true);
  FlameHeight = preferences.getInt("FlameHeight", 0);
  if (FlameHeight == 0) {
    FlameHeight = 50;
  }
  Sparks = preferences.getInt("Sparks", 0);
  if (Sparks == 0) {
    Sparks = 100;
  }
  DelayDuration = preferences.getInt("DelayDuration", 0);
  if (DelayDuration == 0) {
    DelayDuration = 10;
  }
  SelectedMode = preferences.getString("SelectedMode");
  if (SelectedMode.equals("")) {
    SelectedMode = "fire";
  }
  preferences.end();
}

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
  SelectedMode = server.arg("SelMode");

  FlameHeight = flameHeightValue.toInt();
  Sparks = sparksValue.toInt();
  DelayDuration = delayDurationValue.toInt();

  saveSettings(FlameHeight, Sparks, DelayDuration, SelectedMode);

  Serial.write("Mode:");
  Serial.println(SelectedMode);

  server.send(200, "text/plain", "Update empfangen");
}

void setupHtml() {
  html = htmlContent;  // Assign the content from the header file

  html.replace("{{FlameHeight}}", String(FlameHeight));
  html.replace("{{Sparks}}", String(Sparks));
  html.replace("{{DelayDuration}}", String(DelayDuration));
}

void setup() {
  Serial.begin(115200);

  loadSettings();

  WiFi.softAP(ssid, password);
  Serial.println("Access Point gestartet");
  Serial.println("IP-Adresse des ESP32 im eigenen WLAN:");
  Serial.println(WiFi.softAPIP());

  setupHtml();

  server.on("/", handleRoot);
  server.on("/update", HTTP_GET, handleUpdate);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP-Server gestartet");

  FastLED.addLeds<WS2812B, 2, GRB>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<WS2812B, 3, GRB>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<WS2812B, 4, GRB>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<WS2812B, 5, GRB>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<WS2812B, 6, GRB>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<WS2812B, 7, GRB>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<WS2812B, 8, GRB>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<WS2812B, 9, GRB>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<WS2812B, 10, GRB>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  FastLED.clearData();
  FastLED.show();
}

// FlameHeight - Use larger value for shorter flames, default=50.
// Sparks - Use larger value for more ignitions and a more active fire (between 0 to 255), default=100.
// DelayDuration - Use larger value for slower flame speed, default=10.
void Fire(int FlameHeight, int Sparks, int DelayDuration) {
  static byte heat[NUM_LEDS];
  int cooldown;

  // Cool down each cell a little
  for (int i = 0; i < NUM_LEDS; i++) {

    cooldown = random(0, ((FlameHeight * 10) / NUM_LEDS) + 2);

    if (cooldown > heat[i]) {
      heat[i] = 0;
    } else {
      heat[i] = heat[i] - cooldown;
    }
  }

  // Heat from each cell drifts up and diffuses slightly
  for (int k = (NUM_LEDS - 1); k >= 2; k--) {
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
  }

  // Randomly ignite new Sparks near bottom of the flame
  if (random(255) < Sparks) {
    int y = random(7);
    heat[y] = heat[y] + random(160, 255);
  }

  // Convert heat to LED colors
  for (int j = 0; j < NUM_LEDS; j++) {
    setPixelHeatColor(j, heat[j]);
  }

  FastLED.show();
  delay(DelayDuration);
}

void Rainbow(int DelayDuration) {
    for (int j = 0; j < 255; j++) {
      for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CHSV(i - (j * 2), 255, 255); /* The higher the value 4 the less fade there is and vice versa */ 
      }
      FastLED.show();
      delay(DelayDuration); /* Change this to your hearts desire, the lower the value the faster your colors move (and vice versa) */
    }
}

void Sparkle(int delayDuration, int numberOfSparkles) {
  fill_solid(leds, NUM_LEDS, CRGB(0,0,0));

  for (int i=0; i<numberOfSparkles; i++) {
    leds[random(NUM_LEDS)].setRGB(255, 255, 255);
  }

  FastLED.show();
  delay(delayDuration);
}

void Dark() {
  FastLED.clearData();
  FastLED.show();
  delay(100);
}

void Sun() {
  CHSV color = CHSV(FlameHeight, Sparks, 255);
  fill_solid(leds, NUM_LEDS, color);
  FastLED.show();
  delay(100);
}

void setPixelHeatColor(int Pixel, byte temperature) {
  // Rescale heat from 0-255 to 0-191
  byte t192 = round((temperature / 255.0) * 191);

  // Calculate ramp up from
  byte heatramp = t192 & 0x3F;  // 0...63
  heatramp <<= 2;               // scale up to 0...252

  // Figure out which third of the spectrum we're in:
  if (t192 > 0x80) {  // hottest
    leds[Pixel].setRGB(255, 255, heatramp);
  }

  else if (t192 > 0x40) {  // middle
    leds[Pixel].setRGB(255, heatramp, 0);
  } else {  // coolest
    leds[Pixel].setRGB(heatramp, 0, 0);
  }
}

void loop() {
  server.handleClient();
  if (SelectedMode.equals("rainbow_pony")) {
    Rainbow(DelayDuration/5);
  } else if (SelectedMode.equals("star")) {
    Sparkle(DelayDuration, Sparks/5);
  } else if (SelectedMode.equals("fire")) {
    Fire(FlameHeight, Sparks, DelayDuration);
  } else if (SelectedMode.equals("sun")) {
    Sun();
  } else if (SelectedMode.equals("ape")) {
    Dark();
  }
}