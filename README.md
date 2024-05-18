# Hydration Monitor (ESP32)

Wearable prototype that estimates body hydration in young children from skin conductance (GSR) and skin temperature, and shows the result on an OLED display with an alert LED.

This is a rebuild of the original project firmware after the source files were lost to a hard disk failure. The sensor choices below are the standard modules for this kind of build.

## Hardware

| Part | Purpose |
| --- | --- |
| ESP32 dev board | Microcontroller |
| GSR sensor module (e.g. Grove GSR) | Skin conductance |
| DS18B20 (waterproof probe) | Skin temperature |
| SSD1306 0.96" OLED (I2C) | Display |
| LED + 220R resistor | Low-hydration alert |

## Wiring

| Signal | ESP32 pin |
| --- | --- |
| GSR analog out | GPIO 34 |
| DS18B20 data | GPIO 4 (4.7k pull-up to 3.3V) |
| OLED SDA | GPIO 21 |
| OLED SCL | GPIO 22 |
| Alert LED | GPIO 2 |
| Calibrate button | GPIO 0 (on-board BOOT) |

## How it works

1. **Calibrate**: with the child well hydrated, press the BOOT button. The current smoothed skin conductance is saved to flash as the baseline, so it survives power-off.
2. **Measure**: the GSR signal is averaged (64 samples) and smoothed with an exponential moving average every 500 ms.
3. **Hydration index**: current conductance as a percentage of the baseline.
   - Less than 15% drop: `NORMAL`
   - 15-30% drop: `MILD - monitor`
   - More than 30% drop: `LOW - give fluids`, alert LED on
4. **Temperature**: if skin temperature is 37.5 C or above, the thresholds tighten by 25%, since fever speeds up fluid loss.

If your GSR module's voltage rises with conductance instead of falling, set `GSR_INVERTED` to `false`.

## Setup

Install these libraries in the Arduino IDE: **OneWire**, **DallasTemperature**, **Adafruit SSD1306**, **Adafruit GFX**. Then select your ESP32 board and upload.

## Disclaimer

Research prototype. Skin conductance is an indirect indicator of hydration. This is not a medical device and does not replace clinical assessment.
