#include "FastLED.h"
#include "settings_html.h"
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <Wire.h>

#define NUM_LEDS 10  // Enter the total number of LEDs on the strip
#define NUM_EYE_LEDS 100

#define INA233_ADDRESS_DEV1 0x40
#define INA233_ADDRESS_DEV2 0x41

#define INA233_READ_EIN 0x86
#define INA233_READ_VIN 0x88
#define INA233_READ_IN 0x89
#define INA233_READ_PIN 0x97

#define VOLTAGE_MEAS_PERIOD_MS 2000
#define VOLTAGE_BAT_EMPTY_DEFAULT 1400
#define VOLTAGE_BAT_TURN_ON_HYST 300

const char *ssid = "Mungg";
const char *password = "Flaekegosler";
int FlameHeight;
int Sparks;
int DelayDuration;
int EyeBrightness;
uint16_t voltage_bat1 = 0;
uint16_t voltage_bat2  = 0;
uint16_t voltage_bat_empty = 0;
int last_time_voltage_read_millis = 0;
bool bat_empty = false;
String SelectedMode;
Preferences preferences;
TaskHandle_t task_webapp_handle;

CRGB leds[NUM_LEDS];
CRGB eye_leds[NUM_EYE_LEDS];

WebServer server(80);
String html;

void task_webapp(void * pvParameters) {
  for (;;) {
    server.handleClient();
  }
}

void saveSettings(int f, int s, int d, int e, String m, int b) {
  preferences.begin("myApp", false);
  preferences.putInt("FlameHeight", f);
  preferences.putInt("Sparks", s);
  preferences.putInt("DelayDuration", d);
  preferences.putInt("EyeBrightness", e);
  preferences.putString("SelectedMode", m);
  preferences.putInt("VoltageBatEmpty", b);
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
  EyeBrightness = preferences.getInt("EyeBrightness", 0);
  if (EyeBrightness == 0) {
    EyeBrightness = 100;
  }
  SelectedMode = preferences.getString("SelectedMode");
  if (SelectedMode.equals("")) {
    SelectedMode = "fire";
  }
  voltage_bat_empty = preferences.getInt("VoltageBatEmpty");
  if (SelectedMode.equals("")) {
    voltage_bat_empty = VOLTAGE_BAT_EMPTY_DEFAULT;
  }
  preferences.end();
}

void handleRoot() {
  server.send(200, "text/html", html);
}

void handleNotFound() {
  server.send(404, "text/plain", "404: Not Found");
}

void setEyeLeds(int percentage_brigtness) {
  int rd, gn, bl;

  rd = 255;
  gn = 138;
  bl = 18;

  for (int i = 0; i < NUM_EYE_LEDS; i++) {
    eye_leds[i].setRGB(rd*percentage_brigtness/100, gn*percentage_brigtness/100, bl*percentage_brigtness/100);
  }
}

void handleUpdate() {
  String flameHeightValue = server.arg("FlameHeight");
  String sparksValue = server.arg("Sparks");
  String delayDurationValue = server.arg("DelayDuration");
  String eyeBrightnessValue = server.arg("EyeBrightness");
  String batEmptyValue = server.arg("BatEmpty");
  SelectedMode = server.arg("SelMode");

  FlameHeight = flameHeightValue.toInt();
  Sparks = sparksValue.toInt();
  DelayDuration = delayDurationValue.toInt();
  EyeBrightness = eyeBrightnessValue.toInt();
  voltage_bat_empty = batEmptyValue.toInt();

  Serial.write("Mode:");
  Serial.println(SelectedMode);

  server.send(200, "text/plain", "Update empfangen");

  saveSettings(FlameHeight, Sparks, DelayDuration, EyeBrightness, SelectedMode, voltage_bat_empty);
  setEyeLeds(EyeBrightness);
}

uint16_t ina233_read(uint8_t i2c_address) {
  uint8_t byte1;
  uint8_t byte2;

  Wire.beginTransmission(i2c_address);
  Wire.write(INA233_READ_VIN);
  Wire.endTransmission();

  Wire.requestFrom(i2c_address, 3);
  // We don't need first byte which holds how many bytes can be read
  Wire.read();

  if (Wire.available()) {
    byte1 = Wire.read();
  } else {
    Serial.write("byte1 was not available");
  }

  if (Wire.available()) {
    byte2 = Wire.read();
  } else {
    Serial.write("byte2 was not available");
  }

  return (byte1 << 8) | byte2;
}

uint16_t ina233_read_voltage(uint8_t i2c_addr) {
  return ina233_read(i2c_addr)/8;
}

uint16_t ina233_read_voltage1() {
  return ina233_read_voltage(INA233_ADDRESS_DEV1);
}

uint16_t ina233_read_voltage2() {
  return ina233_read_voltage(INA233_ADDRESS_DEV2);
}

void handleVoltages() {  
  // Konvertierung der Spannungen in Volt (angenommen, die Werte sind in Centi-Volt)
  float voltage1InVolts = voltage_bat1 / 100.0;
  float voltage2InVolts = voltage_bat2 / 100.0;
  
  // Erstellen eines Strings, der beide Spannungswerte enthält
  String voltagesString = "Bat1: " + String(voltage1InVolts, 2) + "V, Bat2: " + String(voltage2InVolts, 2) + "V";
  
  server.send(200, "text/plain", voltagesString);
}

void setupHtml() {
  html = htmlContent;  // Assign the content from the header file

  html.replace("{{FlameHeight}}", String(FlameHeight));
  html.replace("{{Sparks}}", String(Sparks));
  html.replace("{{DelayDuration}}", String(DelayDuration));
  html.replace("{{EyeBrightness}}", String(EyeBrightness));
  html.replace("{{BatEmpty}}", String(voltage_bat_empty));
}

void setup() {
  Serial.begin(115200);
  Wire.begin();

  loadSettings();

  WiFi.softAP(ssid, password);
  Serial.println("Access Point gestartet");
  Serial.println("IP-Adresse des ESP32 im eigenen WLAN:");
  Serial.println(WiFi.softAPIP());

  setupHtml();

  server.on("/", handleRoot);
  server.on("/update", HTTP_GET, handleUpdate);
  server.on("/voltages", HTTP_GET, handleVoltages);
  server.onNotFound(handleNotFound);

  xTaskCreatePinnedToCore(
                    task_webapp,   /* Task function. */
                    "task_webapp",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &task_webapp_handle,      /* Task handle to keep track of created task */
                    1);          /* pin task to core 1, loop is running on 0 */

  server.begin();
  Serial.println("HTTP-Server gestartet");

  FastLED.addLeds<WS2812B, 1, GRB>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<WS2812B, 2, GRB>(eye_leds, NUM_EYE_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<WS2812B, 3, GRB>(eye_leds, NUM_EYE_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<WS2812B, 4, GRB>(eye_leds, NUM_EYE_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<WS2812B, 5, GRB>(eye_leds, NUM_EYE_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<WS2812B, 6, GRB>(eye_leds, NUM_EYE_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<WS2812B, 7, GRB>(eye_leds, NUM_EYE_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<WS2812B, 8, GRB>(eye_leds, NUM_EYE_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<WS2812B, 9, GRB>(eye_leds, NUM_EYE_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<WS2812B, 10, GRB>(eye_leds, NUM_EYE_LEDS).setCorrection(TypicalLEDStrip);

  FastLED.clearData();

  setEyeLeds(100);

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

void periodically_read_voltage() {
  if ((millis() - last_time_voltage_read_millis) < VOLTAGE_MEAS_PERIOD_MS)
    return;

  last_time_voltage_read_millis = millis();

  voltage_bat1 = ina233_read_voltage1();
  voltage_bat2 = ina233_read_voltage2();
  Serial.printf("Volt1: %d, Volt2: %d\n", voltage_bat1, voltage_bat2);
}

void loop() {
  periodically_read_voltage();

  if ((voltage_bat1 > (voltage_bat_empty + VOLTAGE_BAT_TURN_ON_HYST)) || (voltage_bat2 > (voltage_bat_empty + VOLTAGE_BAT_TURN_ON_HYST))) {
    bat_empty = false;
  }

  if (bat_empty)
    return;

  if (voltage_bat1 < voltage_bat_empty && voltage_bat2 < voltage_bat_empty && !(voltage_bat1 == 0 || voltage_bat2 == 0)) {
    Serial.printf("Voltage too low detected, declare battery empty\n");
    bat_empty = true;
    FastLED.clearData();
    FastLED.show();
    return;
  }
  
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