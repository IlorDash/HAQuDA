# HAQuDA

HAQuDA (Home Air Quality Display And so on) is a desk lamp that shows the state of the home in
the colour of its light: air quality, temperature and humidity.

The lamp passes the state of the home to a person at a glance: you read a colour instead of
analysing numbers. Each value has three limits: blue below the lower limit, red above the upper
limit, and a blend through green in between. eCO2 and TVOC change in steps instead of blending;
see [Colour scale](display-modes.md#colour-scale).

![Render of the HAQuDA lamp: a tube with a 12 × 9 LED matrix on a round base](assets/Assembly.png)

---

## Specifications

| Parameter | Value |
|-----------|-------|
| Firmware version | {{ app_version }} |
| Controller | ESP32-WROOM-32 module |
| Sensors | DHT11 (temperature, humidity), CCS811 (eCO2, TVOC), ZH03B (particulate matter) |
| LED matrix | 12 × 9 = 108 addressable WS2812B LEDs on nine strips of twelve, on a tube fixed to the board |
| Diffuser | Matte translucent white cover over the tube; opaque base with the electronics |
| Dimensions | Cylinder, 90 mm diameter, 130 mm high |
| Power | 5 V through the Micro-USB socket, supply 5 V 2 A |
| Outputs | Two USB-A sockets for charging other devices |
| Connectivity | WiFi station on a stored network; own access point at `192.168.4.22` as a fallback |
| Services | Web server on port 80, WebSerial log, time from NTP with a fixed offset of UTC+3 (station mode only) |
| Dynamic DNS (DuckDNS) | <span class="wip">WIP</span> |
| Display update | Sensors are read every few seconds; the matrix is redrawn once a minute with the average of that minute |
| Error indication | LED effects and log lines |

---

## Measured parameters

| Parameter | Sensor | Unit | Typical accuracy | Shown on the matrix |
|-----------|--------|------|------------------|---------------------|
| Temperature | DHT11 | °C | ±2 °C | Multi (lower zone), standard, night |
| Humidity | DHT11 | % | ±5 %RH | Standard, night |
| eCO2 | CCS811 | ppm | — | Multi (middle zone), standard, night |
| TVOC | CCS811 | ppb | — | Standard, night |
| PM2.5 | ZH03B | µg/m³ | — | Multi (upper zone), standard, night |
| PM1.0, PM10 | ZH03B | µg/m³ | — | No |

The accuracy figures are the manufacturer's, not measured on the lamp.

The limits of every value and the resulting colours are in [Display modes](display-modes.md#colour-scale).

---

## Display modes

| Mode | The matrix shows |
|------|------------------|
| Multi (default) | Three horizontal zones: temperature, eCO2 and PM2.5. |
| Standard | The whole matrix in the colour of one chosen value. |
| Night | The colour of one chosen value in three columns and a white progress bar of the night in the others. |

The lamp starts in multi mode. Selecting the display mode without changing the firmware is not
ready <span class="wip">WIP</span>; see [Mode selection](quick-guide.md#select-the-display-mode).

Details are in [Display modes](display-modes.md).

---

## Documentation

| Page | Content |
|------|---------|
| [Quick guide](quick-guide.md) | First start, WiFi setup, display mode selection. |
| [Display modes](display-modes.md) | Picture update, colour scale, the three modes. |
| [Web interface](web-interface.md) | Pages, controls, replies and the log page. |
| [LED codes and troubleshooting](troubleshooting.md) | LED effects, matrix symptoms, log messages. |
| [Building and flashing](build-and-flash.md) | Requirements, build variables, build and flash steps. |
| [Release notes](release-2-0-0.md) | Changes per version and known limitations. |

Features that are not ready are marked <span class="wip">WIP</span>.

This guide comes in English and Russian; use the language switcher in the header. The lamp's web
pages exist in English only, so both languages quote their texts as they appear on the screen.
