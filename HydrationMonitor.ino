// Hydration Monitor - ESP32 prototype for estimating hydration in children
// GSR (skin conductance) + DS18B20 skin temperature -> hydration index on OLED
// Press the button once while the child is known to be well hydrated to set the baseline.

#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Preferences.h>

const int PIN_GSR = 34;         // GSR module analog out
const int PIN_TEMP = 4;         // DS18B20 data (4.7k pull-up to 3.3V)
const int PIN_CAL_BUTTON = 0;   // on-board BOOT button
const int PIN_ALERT_LED = 2;

// Some GSR modules output a lower voltage as conductance rises; set true for those.
const bool GSR_INVERTED = true;

const int GSR_SAMPLES = 64;
const float EMA_ALPHA = 0.1f;
const unsigned long UPDATE_MS = 500;

// Drop in conductance vs. baseline (percent) that maps to each status
const float MILD_DROP_PCT = 15.0f;
const float LOW_DROP_PCT = 30.0f;
// Elevated skin temperature makes dehydration more likely; tighten thresholds above this
const float FEVER_TEMP_C = 37.5f;

Adafruit_SSD1306 display(128, 64, &Wire, -1);
OneWire oneWire(PIN_TEMP);
DallasTemperature tempSensor(&oneWire);
Preferences prefs;

float conductanceEma = 0;
float baseline = 0;
unsigned long lastUpdate = 0;

float readConductance() {
  uint32_t sum = 0;
  for (int i = 0; i < GSR_SAMPLES; i++) {
    sum += analogReadMilliVolts(PIN_GSR);
    delayMicroseconds(200);
  }
  float mv = sum / (float)GSR_SAMPLES;
  return GSR_INVERTED ? (3300.0f - mv) : mv;
}

void saveBaseline(float value) {
  baseline = value;
  prefs.putFloat("baseline", baseline);
}

const char* classify(float dropPct, float tempC, bool& alert) {
  float feverFactor = tempC >= FEVER_TEMP_C ? 0.75f : 1.0f;
  if (dropPct >= LOW_DROP_PCT * feverFactor) {
    alert = true;
    return "LOW - give fluids";
  }
  if (dropPct >= MILD_DROP_PCT * feverFactor) {
    alert = false;
    return "MILD - monitor";
  }
  alert = false;
  return "NORMAL";
}

void drawScreen(float index, float tempC, const char* status) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Hydration Monitor");

  display.setTextSize(2);
  display.setCursor(0, 16);
  if (baseline > 0) {
    display.print((int)index);
    display.print("%");
  } else {
    display.print("CAL?");
  }

  display.setTextSize(1);
  display.setCursor(0, 38);
  display.print("Skin temp: ");
  display.print(tempC, 1);
  display.print(" C");

  display.setCursor(0, 52);
  display.print(baseline > 0 ? status : "Press BOOT to calibrate");

  display.display();
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_CAL_BUTTON, INPUT_PULLUP);
  pinMode(PIN_ALERT_LED, OUTPUT);
  analogReadResolution(12);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 not found - check wiring");
  }
  tempSensor.begin();

  prefs.begin("hydration", false);
  baseline = prefs.getFloat("baseline", 0);

  conductanceEma = readConductance();
}

void loop() {
  if (digitalRead(PIN_CAL_BUTTON) == LOW) {
    saveBaseline(conductanceEma);
    Serial.printf("Baseline set: %.1f\n", baseline);
    while (digitalRead(PIN_CAL_BUTTON) == LOW) delay(10);
  }

  unsigned long now = millis();
  if (now - lastUpdate < UPDATE_MS) return;
  lastUpdate = now;

  conductanceEma += EMA_ALPHA * (readConductance() - conductanceEma);

  tempSensor.requestTemperatures();
  float tempC = tempSensor.getTempCByIndex(0);

  // Index is conductance relative to the well-hydrated baseline (100% = baseline)
  float index = baseline > 0 ? (conductanceEma / baseline) * 100.0f : 0;
  float dropPct = max(0.0f, 100.0f - index);

  bool alert = false;
  const char* status = classify(dropPct, tempC, alert);
  digitalWrite(PIN_ALERT_LED, alert && baseline > 0);

  drawScreen(index, tempC, status);
  Serial.printf("gsr=%.1f index=%.1f%% temp=%.1fC status=%s\n", conductanceEma, index, tempC,
                baseline > 0 ? status : "uncalibrated");
}
