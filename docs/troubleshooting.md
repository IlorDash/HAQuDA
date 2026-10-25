# LED codes and troubleshooting

The lamp reports problems as LED effects and as lines in its log. Most errors turn the matrix
dark, so the log tells you more than the matrix. Open `/webserial` when the lamp runs in your
network (see [Web interface](web-interface.md#log-page)) and copy the log lines when you report a
problem.

Effects in the tables use these names:

- **snake** — one or more lit LEDs run along the matrix.
- **fading** — the whole matrix lights up and dims step by step.
- **static** — the whole matrix keeps one colour.

---

## Start-up effects

| Effect | Cause | Action |
|------------------------|-------|------------|
| Lime snake | The lamp starts, or it reconnects to a stored WiFi network. | Wait for it to end. |
| Green, whole matrix, about 3 seconds | The lamp joined your WiFi network. | Nothing. The matrix goes dark, then shows colours after about a minute. |
| Dark matrix for about twenty seconds after the lime snake, then an aqua fading | No network is stored, or the lamp could not join the stored one. It opens its access point. | Nothing on the first start. Otherwise check the name and the password of your network and add it again ([Quick guide](quick-guide.md#connect-the-lamp-to-your-wifi)). |
| Aqua fading, once | The access point runs and the sensors start. | Join the lamp's network and open `http://192.168.4.22/`. |
| Dark matrix that stays dark, the lamp's WiFi network usually does not appear, the LED on the ESP32 board does not blink | The lamp could not create its access point and stops. | Unplug the lamp and plug it in again. If it repeats, check the access point password in your build (8 to 63 characters). |
| Dark matrix for about 15–20 seconds after the green flash or the aqua fading, then the lime snake starts again, again and again | The lamp could not start its sensors (the CCS811 does not answer). It tries up to three times to power the sensors off and on, then restarts itself. | Check the sensor power and the I2C wiring. |
| Pixels light up one by one in random colours; it does not stop | The lamp could not start its file system and stops. Known bug: the intended effect is a red fading; the firmware swaps the effects of the file system and sensor errors. | Unplug the lamp and plug it in again. If it repeats, flash the lamp again. |

---

## Running effects

| Effect | Cause | Action |
|------------------------|-------|------------|
| Red, whole matrix, about 3 seconds, then a lime snake | The lamp lost the WiFi connection and reconnects. | Wait. |
| Lime snake that does not end after the lamp lost its WiFi <span class="wip">WIP</span> | The router is gone. The lamp retries without a limit. | Bring the router back, or unplug the lamp and plug it in again. |

!!! note "WIP"
    Recovery after a WiFi loss is not ready: the lamp does not open its access point and may not
    return to the colours. Unplug the lamp and plug it in again.

---

## The matrix looks wrong

| Effect | Cause | Action |
|------------------------|-------|------------|
| The matrix is dark for the first minute after a start | The lamp averages the first minute of readings before it draws. | Wait. In the standard and night mode the matrix can show a meaningless colour during this minute. |
| The matrix stays dark after the green flash, although a minute has passed | After the lamp reconnects to WiFi, it does not return to the colours. | Unplug the lamp and plug it in again. |
| The matrix stays dark although the log shows `Started sensors` | The sensors started on a retry, but the lamp stays dark. | Unplug the lamp and plug it in again. |
| The picture does not change for several minutes | In multi mode, one of temperature, eCO2 or PM2.5 delivers no reading, and the lamp keeps the old picture. One missed CCS811 reading discards the whole minute of eCO2 and TVOC. | Open the log and look for `Failed to get ... meas` lines (see [Log messages](#log-messages)). Check the wiring of that sensor. |
| eCO2 and TVOC never show green | The lamp shows these two values in steps. | Nothing; see [Colour scale](display-modes.md#colour-scale). |
| The LED on the ESP32 board does not blink | The lamp has not finished its start, or its main loop is blocked, usually while it retries a lost WiFi connection. | Wait for the start or the reconnect. If the LED stays still for minutes, unplug the lamp and plug it in again. |

The LED on the ESP32 board blinks once per second while the lamp works normally.

---

## Log messages

Each line has the form `time - level : message`; see [Log page](web-interface.md#log-page).

| Message | Level | Cause | Action |
|---------|-------|-------|------------|
| `Started File system` | Info | The file system works. | Nothing. |
| `Connected to WiFi` | Info | The lamp joined your network. | Nothing. |
| `Created Acces Point` | Info | The lamp opened its access point. | Join it and open `http://192.168.4.22/`. |
| `Started sensors` | Info | The sensors answer. | Nothing. |
| `Failed connection to WiFi` | ERROR | The lamp could not join a stored network. | Check the name and the password of your network. |
| `Failed creating Acces Point` | ERROR | The access point cannot start. | See the dark matrix above. |
| `Failed to start sensors` | ERROR | The CCS811 does not answer. | See the dark matrix with the returning lime snake above. |
| `Failed to get eCO2 meas` | ERROR | The CCS811 delivered no eCO2 reading in the last 2 seconds. | A single line is harmless. Lines that repeat point to the CCS811 wiring. |
| `Failed to get TVOC meas` | ERROR | The CCS811 delivered no TVOC reading. | As above. |
| `Failed to get PM 2.5 meas` | ERROR | The ZH03B sent no valid frame. | Check the ZH03B wiring and its 5 V supply. |
| `Failed to get TEMP meas` | ERROR | The DHT11 delivered no temperature. | Check the DHT11 wiring. |
| `Failed to get HUMID meas` | ERROR | The DHT11 delivered no humidity. | Check the DHT11 wiring. |

Lines about a failed reading appear again every few seconds while the failure lasts.
