<div align="center">

# Hydration Monitor (ESP32)

**Wearable that estimates paediatric hydration from skin conductance with a personalised baseline**

![Domain](https://img.shields.io/badge/Domain-Biosignal_Sensing-00F3FF?style=for-the-badge) ![Platform](https://img.shields.io/badge/Platform-ESP32_Edge-9D00FF?style=for-the-badge) ![Calibration](https://img.shields.io/badge/Calibration-Per_User-0066FF?style=for-the-badge)

![ESP32](https://img.shields.io/badge/ESP32-0D1117?style=flat-square&logo=espressif&logoColor=white) ![Arduino](https://img.shields.io/badge/Arduino-0D1117?style=flat-square&logo=arduino&logoColor=white) ![C++](https://img.shields.io/badge/C++-0D1117?style=flat-square&logo=cplusplus&logoColor=white) ![GSR](https://img.shields.io/badge/GSR-0D1117?style=flat-square) ![DS18B20](https://img.shields.io/badge/DS18B20-0D1117?style=flat-square) ![SSD1306](https://img.shields.io/badge/SSD1306-0D1117?style=flat-square)

</div>

---

## Overview

A wearable prototype that estimates body hydration in young children from skin conductance (GSR) and
skin temperature, showing the result on an OLED display with an alert LED.

The interesting part is not the sensor — it is the calibration model. Absolute GSR readings are
meaningless across different children, skin types and electrode placements, so the device measures
each wearer against *their own* well-hydrated baseline, stored in flash so it survives power-off.

This is a rebuild of the original project firmware after the source files were lost to a hard disk
failure. The sensor choices below are the standard modules for this class of build.

## Domain &amp; Techniques

| Layer | Implementation |
| :--- | :--- |
| **Noise Reduction** | 64-sample block average followed by an exponential moving average, refreshed every 500 ms |
| **Personal Baselining** | A button press captures the wearer's hydrated-state conductance and persists it to flash — all subsequent readings are relative to that baseline, not an absolute scale |
| **Hydration Index** | Current conductance expressed as a percentage drop from baseline, banded into NORMAL / MILD / LOW |
| **Temperature Compensation** | Skin temperature at or above 37.5&nbsp;&deg;C tightens every threshold by 25%, since fever accelerates fluid loss |
| **Polarity Handling** | `GSR_INVERTED` flag accommodates modules whose voltage rises rather than falls with conductance |

## Pipeline

```
GSR electrode --> 64-sample average --> EMA smoothing --+
                                                       |
DS18B20 skin temp --------------------------------------+--> hydration index (% of baseline)
                                                       |          |
        flash-stored personal baseline -----------------+          v
                                                          NORMAL / MILD / LOW --> OLED + alert LED
```

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

## How It Works

1. **Calibrate**: with the child well hydrated, press the BOOT button. The current smoothed skin
   conductance is saved to flash as the baseline, so it survives power-off.
2. **Measure**: the GSR signal is averaged (64 samples) and smoothed with an exponential moving
   average every 500 ms.
3. **Hydration index**: current conductance as a percentage of the baseline.
   - Less than 15% drop: `NORMAL`
   - 15-30% drop: `MILD - monitor`
   - More than 30% drop: `LOW - give fluids`, alert LED on
4. **Temperature**: if skin temperature is 37.5 C or above, the thresholds tighten by 25%, since fever
   speeds up fluid loss.

If your GSR module's voltage rises with conductance instead of falling, set `GSR_INVERTED` to `false`.

## Setup

Install these libraries in the Arduino IDE: **OneWire**, **DallasTemperature**, **Adafruit SSD1306**,
**Adafruit GFX**. Then select your ESP32 board and upload.

## Repository Layout

| Path | Purpose |
| :--- | :--- |
| `HydrationMonitor.ino` | Main firmware — sampling, baselining, banding, display |

## Project Status

**Implemented:** flash-persisted per-user calibration, two-stage smoothing, temperature-compensated
banding, OLED readout and alert LED.

**Roadmap:** log the hydration index over time so trends (rather than instantaneous state) drive the
alert, and expose readings over BLE to a caregiver phone.

> Research prototype. Skin conductance is an indirect indicator of hydration. This is not a medical
> device and does not replace clinical assessment.

---

<div align="center">
  <sub>
    Part of the <b>AI + Robotics</b> engineering portfolio of
    <a href="https://github.com/divyansh-sachdev">Divyansh Sachdev</a><br>
    90+ national &amp; international competition wins &middot; IIT / NIT / IIIT podiums
  </sub>
</div>
